/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file finance_valuation.cpp Income-based company valuation for earned financing (CITYSIM fork feature). */

#include "stdafx.h"
#include "company_base.h"
#include "company_func.h"
#include "core/backup_type.hpp"
#include "finance_valuation.h"
#include "news_func.h"
#include "strings_func.h"
#include "timer/timer_game_economy.h"
#include "window_func.h"

#include "table/strings.h"

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
	result.age_years = static_cast<uint>(std::max<int32_t>(0, TimerGameEconomy::year.base() - c->inaugurated_year.base()));
	result.age_ok = TimerGameEconomy::year >= c->inaugurated_year + IPO_MIN_AGE_YEARS;
	result.valuation_ok = result.valuation >= IPO_MIN_VALUATION;

	result.income_ok = c->num_valid_stat_ent >= IPO_REQUIRED_PROFITABLE_QUARTERS;
	for (uint i = 0; i < std::min<uint>(c->num_valid_stat_ent, IPO_REQUIRED_PROFITABLE_QUARTERS); i++) {
		if (c->old_economy[i].income + c->old_economy[i].expenses <= 0) break;
		result.profitable_quarters++;
	}
	if (result.income_ok) {
		if (result.profitable_quarters < IPO_REQUIRED_PROFITABLE_QUARTERS) {
			result.income_ok = false;
		}
	}

	return result;
}

/**
 * Quarterly update of all public companies: refresh the share price from the
 * current valuation, and pay out the automatic dividend policy.
 *
 * Must be called from the deterministic game loop right after the quarterly
 * economy history has rotated, so that old_economy[0] is the just-closed
 * quarter (see CompaniesGenStatistics).
 */
void UpdatePublicCompaniesFinance()
{
	Backup<CompanyID> cur_company(_current_company);

	for (Company *c : Company::Iterate()) {
		if (!c->is_public) continue;

		if (c->shares_outstanding != 0) {
			Money old_price = c->share_price;
			c->share_price = CalculateIncomeBasedValuation(c) / c->shares_outstanding;
			if (old_price > 0 && c->share_price > 0) {
				Money delta = c->share_price > old_price ? c->share_price - old_price : old_price - c->share_price;
				if (delta * 100 >= old_price * 20) {
					AddNewsItem(GetEncodedString(STR_NEWS_SHARE_PRICE_MOVES, c->index, old_price, c->share_price), NewsType::Economy, NewsStyle::Normal, {});
				}
			}
		}

		if (c->dividend_policy > 0) {
			Money profit = c->old_economy[0].income + c->old_economy[0].expenses;
			if (profit > 0) {
				Money payout = profit * c->dividend_policy * c->public_float_pct / 10000;
				if (payout > 0 && GetAvailableMoney(c->index) >= payout) {
					cur_company.Change(c->index);
					SubtractMoneyFromCompany(CommandCost(ExpensesType::Other, payout));
					AddNewsItem(GetEncodedString(STR_NEWS_DIVIDEND_PAID, c->index, payout), NewsType::Economy, NewsStyle::Normal, {});
				}
			}
		}

		SetWindowDirty(WindowClass::Finances, c->index);
	}

	cur_company.Restore();
}
