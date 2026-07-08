/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file venture_cmd.cpp Commands for venture capital investments (CITYSIM fork feature). */

#include "stdafx.h"
#include "command_func.h"
#include "company_base.h"
#include "company_func.h"
#include "company_gui.h"
#include "venture.h"
#include "venture_cmd.h"
#include "window_func.h"

#include "table/strings.h"

#include "safeguards.h"

/**
 * Buy a stake in a private startup at its current valuation.
 * The purchase is a balance-sheet event (paid directly from company money,
 * like taking a loan), so it does not distort the income statement that the
 * company's own valuation is derived from.
 * @param flags Type of operation.
 * @param slot Startup slot on the venture board.
 * @param bp Stake to buy, in basis points.
 * @return Empty cost or an error.
 */
CommandCost CmdBuyVentureStake(DoCommandFlags flags, uint8_t slot, uint16_t bp)
{
	Company *c = Company::GetIfValid(_current_company);
	if (c == nullptr) return CMD_ERROR;

	if (slot >= NUM_VENTURES) return CMD_ERROR;
	if (bp < VENTURE_TRADE_UNIT_BP || bp % VENTURE_TRADE_UNIT_BP != 0) return CMD_ERROR;

	Venture &v = _ventures[slot];
	if (v.state != VentureState::Active) return CommandCost(STR_ERROR_VENTURE_NOT_INVESTABLE);

	uint16_t held = v.stakes[c->index.base()];
	if (held + bp > VENTURE_MAX_COMPANY_STAKE_BP) return CommandCost(STR_ERROR_VENTURE_STAKE_LIMIT);
	if (GetTotalVentureStakeBp(v) + bp > VENTURE_MAX_TOTAL_STAKE_BP) return CommandCost(STR_ERROR_VENTURE_SOLD_OUT);

	Money cost = v.valuation * bp / 10000;
	if (GetAvailableMoneyForCommand() < cost) return CommandCostWithParam(STR_ERROR_CURRENCY_REQUIRED, cost);

	if (flags.Test(DoCommandFlag::Execute)) {
		c->money -= cost;
		v.stakes[c->index.base()] = held + bp;
		InvalidateCompanyWindows(c);
		SetWindowDirty(WC_VENTURE_CAPITAL, 0);
	}

	return CommandCost();
}

/**
 * Sell part of a stake in a private startup, at a discount to pro-rata value.
 * @param flags Type of operation.
 * @param slot Startup slot on the venture board.
 * @param bp Stake to sell, in basis points.
 * @return Empty cost or an error.
 */
CommandCost CmdSellVentureStake(DoCommandFlags flags, uint8_t slot, uint16_t bp)
{
	Company *c = Company::GetIfValid(_current_company);
	if (c == nullptr) return CMD_ERROR;

	if (slot >= NUM_VENTURES) return CMD_ERROR;
	if (bp < VENTURE_TRADE_UNIT_BP || bp % VENTURE_TRADE_UNIT_BP != 0) return CMD_ERROR;

	Venture &v = _ventures[slot];
	if (v.state != VentureState::Active) return CommandCost(STR_ERROR_VENTURE_NOT_INVESTABLE);

	uint16_t held = v.stakes[c->index.base()];
	if (held < bp) return CommandCost(STR_ERROR_VENTURE_NO_STAKE);

	Money proceeds = v.valuation * bp / 10000 * VENTURE_SELL_PCT / 100;

	if (flags.Test(DoCommandFlag::Execute)) {
		v.stakes[c->index.base()] = held - bp;
		if (c->money <= Money::max() - proceeds) c->money += proceeds;
		InvalidateCompanyWindows(c);
		SetWindowDirty(WC_VENTURE_CAPITAL, 0);
	}

	return CommandCost();
}
