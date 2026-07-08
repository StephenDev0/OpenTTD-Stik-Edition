/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file townzone.cpp Town zones with per-town demand (CITYSIM fork feature). */

#include "stdafx.h"
#include "core/math_func.hpp"
#include "economy_func.h"
#include "house.h"
#include "industry.h"
#include "map_func.h"
#include "tile_map.h"
#include "town.h"
#include "town_map.h"
#include "townzone.h"

#include "table/strings.h"

#include "safeguards.h"

/**
 * Whether a house type counts as commercial (shops and offices).
 * Classified by goods acceptance: in the base sets shops/offices are the
 * house types that accept goods.
 */
bool IsCommercialHouseSpec(const HouseSpec &hs)
{
	for (const CargoLabel &label : hs.accepts_cargo_label) {
		if (label == CT_GOODS) return true;
	}
	return false;
}

/** Whether a house type counts as residential (has population, no goods acceptance). */
bool IsResidentialHouseSpec(const HouseSpec &hs)
{
	return hs.population > 0 && !IsCommercialHouseSpec(hs);
}

/** Count the residential and commercial buildings of a town. */
static void CountTownZoneBuildings(const Town *t, uint &res, uint &com)
{
	res = 0;
	com = 0;
	for (const auto &hs : HouseSpec::Specs()) {
		uint n = t->cache.building_counts.id_count[hs.Index()];
		if (n == 0) continue;
		if (IsCommercialHouseSpec(hs)) {
			com += n;
		} else if (hs.population > 0) {
			res += n;
		}
	}
}

/** @return User-facing label for a town zone. */
StringID GetTownZoneLabel(TownZone zone)
{
	switch (zone) {
		case TownZone::Residential: return STR_TOWN_ZONES_RESIDENTIAL_NAME;
		case TownZone::Commercial:  return STR_TOWN_ZONES_COMMERCIAL_NAME;
		case TownZone::Industrial:  return STR_TOWN_ZONES_INDUSTRIAL_NAME;
	}
	NOT_REACHED();
}

/** @return Cost to place a building in this zone. */
Money GetTownZoneBuildCost(TownZone zone)
{
	switch (zone) {
		case TownZone::Residential: return std::max<Money>(_price[PR_BUILD_TOWN] / 40, _price[PR_CLEAR_HOUSE] * 2);
		case TownZone::Commercial:  return std::max<Money>(_price[PR_BUILD_TOWN] / 25, _price[PR_CLEAR_HOUSE] * 3);
		case TownZone::Industrial:  return _price[PR_BUILD_INDUSTRY];
	}
	NOT_REACHED();
}

/** Count population already committed by residential buildings still under construction. */
static uint CountPendingResidentialPopulation(const Town *t)
{
	if (!Map::IsInitialized()) return 0;

	uint pending = 0;
	for (Tile tile : Map::Iterate()) {
		TileIndex tile_index = tile;
		if (!IsTileType(tile_index, MP_HOUSE) || Town::GetByTile(tile_index) != t || IsHouseCompleted(tile_index)) continue;

		const HouseSpec *hs = HouseSpec::Get(GetHouseType(tile_index));
		if (IsResidentialHouseSpec(*hs)) pending += hs->population;
	}
	return pending;
}

/**
 * Compute the current demand of a town for a building zone.
 *
 * The demand is derived purely from live game state (population, building
 * mix, industries), so it needs no savegame data, cannot desync, and reacts
 * immediately when buildings are placed: residents want to live where there
 * are jobs, shops want customers, industries want workers.
 *
 * @param t The town.
 * @param zone The zone to compute demand for.
 * @return Demand in the range 0 (none) to 100 (desperate).
 */
TownZoneDemandDetails GetTownZoneDemandDetails(const Town *t, TownZone zone)
{
	TownZoneDemandDetails details;

	uint res, com;
	CountTownZoneBuildings(t, res, com);
	details.residential_buildings = res;
	details.commercial_buildings = com;

	uint industries = 0;
	for (const Industry *i : Industry::Iterate()) {
		if (i->town == t) industries++;
	}
	details.industries = industries;
	details.population = t->cache.population;
	details.pending_population = CountPendingResidentialPopulation(t);

	int pop = static_cast<int>(details.population + details.pending_population);
	int demand = 0;

	switch (zone) {
		case TownZone::Residential: {
			/* People move in when there are more jobs than workers. */
			int jobs = static_cast<int>(com) * 12 + static_cast<int>(industries) * 60;
			details.jobs = ClampTo<uint>(jobs);
			demand = 55 + (jobs - pop / 3) / 4;
			break;
		}

		case TownZone::Commercial:
			/* Shops want customers, and not too much competition. */
			demand = 20 + pop / 8 - static_cast<int>(com) * 12;
			break;

		case TownZone::Industrial:
			/* Industries want a workforce, and not too many rivals. */
			demand = 15 + pop / 12 - static_cast<int>(industries) * 20;
			break;
	}

	details.demand = static_cast<uint>(Clamp(demand, 0, 100));
	return details;
}

uint GetTownZoneDemand(const Town *t, TownZone zone)
{
	return GetTownZoneDemandDetails(t, zone).demand;
}
