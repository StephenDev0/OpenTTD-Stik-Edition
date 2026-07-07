/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file finance_cmd.h Command definitions for earned financing: IPO and dividends (CITYSIM fork feature). */

#ifndef FINANCE_CMD_H
#define FINANCE_CMD_H

#include "command_type.h"
#include "economy_type.h"

CommandCost CmdFileIpo(DoCommandFlags flags, uint8_t float_pct);
CommandCost CmdIssueDividend(DoCommandFlags flags, Money amount);
CommandCost CmdSetDividendPolicy(DoCommandFlags flags, uint8_t percent);

DEF_CMD_TRAIT(CMD_FILE_IPO,            CmdFileIpo,            {}, CommandType::MoneyManagement)
DEF_CMD_TRAIT(CMD_ISSUE_DIVIDEND,      CmdIssueDividend,      {}, CommandType::MoneyManagement)
DEF_CMD_TRAIT(CMD_SET_DIVIDEND_POLICY, CmdSetDividendPolicy,  {}, CommandType::CompanySetting)

#endif /* FINANCE_CMD_H */
