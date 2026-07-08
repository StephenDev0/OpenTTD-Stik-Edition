/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file townzone.h Town zones with per-town demand (CITYSIM fork feature). */

#ifndef TOWNZONE_H
#define TOWNZONE_H

#include "town_type.h"

struct HouseSpec;
struct Town;

/** Building zones of a town, Cities-Skylines-style. */
enum class TownZone : uint8_t {
	Residential,
	Commercial,
	Industrial,
};

/** Minimum demand (0..100) required to place a building in a zone. */
static constexpr uint TOWNZONE_PLACE_THRESHOLD = 20;

bool IsCommercialHouseSpec(const HouseSpec &hs);
bool IsResidentialHouseSpec(const HouseSpec &hs);
uint GetTownZoneDemand(const Town *t, TownZone zone);

void ShowTownZonesWindow(TownID town);

#endif /* TOWNZONE_H */
