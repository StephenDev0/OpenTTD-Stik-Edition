/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file venture_cmd.h Command definitions for venture capital investments (CITYSIM fork feature). */

#ifndef VENTURE_CMD_H
#define VENTURE_CMD_H

#include "command_type.h"

CommandCost CmdBuyVentureStake(DoCommandFlags flags, uint8_t slot, uint16_t bp);
CommandCost CmdSellVentureStake(DoCommandFlags flags, uint8_t slot, uint16_t bp);

DEF_CMD_TRAIT(CMD_BUY_VENTURE_STAKE,  CmdBuyVentureStake,  {}, CommandType::MoneyManagement)
DEF_CMD_TRAIT(CMD_SELL_VENTURE_STAKE, CmdSellVentureStake, {}, CommandType::MoneyManagement)

#endif /* VENTURE_CMD_H */
