/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file citybuild_cmd.h Command definitions for player-driven city building (CITYSIM fork feature). */

#ifndef CITYBUILD_CMD_H
#define CITYBUILD_CMD_H

#include "command_type.h"
#include "townzone.h"

struct Town;
struct HouseSpec;
using HouseID = uint16_t;

CommandCost CmdPlacePlayerHouse(DoCommandFlags flags, TileIndex tile, TownZone zone);

DEF_CMD_TRAIT(Commands::PlacePlayerHouse, CmdPlacePlayerHouse, {}, CommandType::LandscapeConstruction)

/* Bridge into town_cmd.cpp's static house construction path; defined there (tagged CITYSIM). */
void BuildPlayerHouse(Town *t, TileIndex tile, const HouseSpec *hs, HouseID house, uint8_t random_bits);

#endif /* CITYBUILD_CMD_H */
