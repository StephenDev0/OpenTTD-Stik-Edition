/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file finance_cmd.cpp Commands for earned financing: IPO and dividends (CITYSIM fork feature). */

#include "stdafx.h"
#include "command_func.h"
#include "company_base.h"
#include "company_func.h"
#include "finance_cmd.h"
#include "finance_valuation.h"
#include "news_func.h"
#include "strings_func.h"
#include "window_func.h"

#include "table/strings.h"

#include "safeguards.h"

/**
 * Take the company public through an initial public offering.
 *
 * The IPO is gated on company age, a streak of profitable quarters and a
 * minimum income-based valuation; the proceeds are priced off that valuation
 * (which excludes held cash) so equity cannot be inflated with raised money.
 *
 * @param flags Type of operation.
 * @param float_pct Percentage of the company to sell to the public.
 * @return The (negative) IPO proceeds, or an error explaining the unmet gate.
 */
CommandCost CmdFileIpo(DoCommandFlags flags, uint8_t float_pct)
{
	Company *c = Company::GetIfValid(_current_company);
	if (c == nullptr) return CMD_ERROR;

	if (float_pct < IPO_MIN_FLOAT_PCT || float_pct > IPO_MAX_FLOAT_PCT) return CMD_ERROR;

	IpoEligibility eligibility = CheckIpoEligibility(c);
	if (eligibility.already_public) return CommandCost(STR_ERROR_IPO_ALREADY_PUBLIC);
	if (!eligibility.age_ok) return CommandCost(STR_ERROR_IPO_COMPANY_TOO_YOUNG);
	if (!eligibility.income_ok) return CommandCost(STR_ERROR_IPO_NOT_PROFITABLE);
	if (!eligibility.valuation_ok) return CommandCost(STR_ERROR_IPO_VALUATION_TOO_LOW);

	Money proceeds = eligibility.valuation * float_pct / 100;

	if (flags.Test(DoCommandFlag::Execute)) {
		c->is_public = true;
		c->public_float_pct = float_pct;
		c->shares_outstanding = IPO_SHARES_OUTSTANDING;
		c->share_price = eligibility.valuation / IPO_SHARES_OUTSTANDING;
		SetWindowDirty(WindowClass::Finances, c->index);
		AddNewsItem(GetEncodedString(STR_NEWS_COMPANY_IPO, c->index, float_pct, proceeds), NewsType::Economy, NewsStyle::Normal, {});
	}

	/* Negative cost: the framework credits the IPO proceeds to the company. */
	return CommandCost(ExpensesType::Other, -proceeds);
}

/**
 * Pay a cash dividend to the public shareholders.
 * @param flags Type of operation.
 * @param amount Total dividend to distribute.
 * @return The cost of the dividend, or an error.
 */
CommandCost CmdIssueDividend(DoCommandFlags flags, Money amount)
{
	Company *c = Company::GetIfValid(_current_company);
	if (c == nullptr) return CMD_ERROR;

	if (!c->is_public) return CommandCost(STR_ERROR_DIVIDEND_NOT_PUBLIC);
	if (amount <= 0 || amount > IPO_MIN_VALUATION * 100) return CMD_ERROR;

	if (flags.Test(DoCommandFlag::Execute)) {
		SetWindowDirty(WindowClass::Finances, c->index);
		AddNewsItem(GetEncodedString(STR_NEWS_DIVIDEND_PAID, c->index, amount), NewsType::Economy, NewsStyle::Normal, {});
	}

	/* The framework charges the amount and refuses it when unaffordable. */
	return CommandCost(ExpensesType::Other, amount);
}

/**
 * Set the automatic dividend policy of the company.
 * Each quarter this percentage of the quarter's operating profit is paid out
 * to the public shareholders (see UpdatePublicCompaniesFinance).
 * @param flags Type of operation.
 * @param percent Percentage of quarterly profit to pay out (0 disables).
 * @return Empty cost or an error.
 */
CommandCost CmdSetDividendPolicy(DoCommandFlags flags, uint8_t percent)
{
	Company *c = Company::GetIfValid(_current_company);
	if (c == nullptr) return CMD_ERROR;

	if (percent > 100) return CMD_ERROR;
	if (!c->is_public) return CommandCost(STR_ERROR_DIVIDEND_NOT_PUBLIC);

	if (flags.Test(DoCommandFlag::Execute)) {
		c->dividend_policy = percent;
		SetWindowDirty(WindowClass::Finances, c->index);
	}

	return CommandCost();
}
