/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file finance_valuation.cpp Income-based company valuation for earned financing (CITYSIM fork feature). */

#include "stdafx.h"
#include "company_base.h"
#include "finance_valuation.h"
#include "timer/timer_game_economy.h"

#include "safeguards.h"

/**
 * Derive a company value purely from its quarterly income history.
 *
 * Held cash, loans and property are deliberately excluded: equity raised in
 * an IPO is priced off this value, and pricing equity off held cash would
 * allow raising money to inflate the valuation to raise yet more money.
 *
 * @param c The company to value.
 * @return Valuation in money units; 0 for companies without profitable history.
 */
Money CalculateIncomeBasedValuation(const Company *c)
{
	uint quarters = std::min<uint>(c->num_valid_stat_ent, VALUATION_QUARTERS);
	if (quarters == 0) return 0;

	Money total_profit = 0;
	for (uint i = 0; i < quarters; i++) {
		/* expenses are accumulated negative, so income + expenses is the net operating profit. */
		total_profit += c->old_economy[i].income + c->old_economy[i].expenses;
	}

	Money annualised = total_profit * 4 / static_cast<int>(quarters);
	return std::max<Money>(annualised * VALUATION_EARNINGS_MULTIPLE, 0);
}

/**
 * Test all IPO gates for a company.
 * @param c The company that wants to go public.
 * @return The per-gate results and the current valuation.
 */
IpoEligibility CheckIpoEligibility(const Company *c)
{
	IpoEligibility result;

	result.valuation = CalculateIncomeBasedValuation(c);
	result.already_public = c->is_public;
	result.age_ok = TimerGameEconomy::year >= c->inaugurated_year + IPO_MIN_AGE_YEARS;
	result.valuation_ok = result.valuation >= IPO_MIN_VALUATION;

	result.income_ok = c->num_valid_stat_ent >= IPO_REQUIRED_PROFITABLE_QUARTERS;
	if (result.income_ok) {
		for (uint i = 0; i < IPO_REQUIRED_PROFITABLE_QUARTERS; i++) {
			if (c->old_economy[i].income + c->old_economy[i].expenses <= 0) {
				result.income_ok = false;
				break;
			}
		}
	}

	return result;
}
