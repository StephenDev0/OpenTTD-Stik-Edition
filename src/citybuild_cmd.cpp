/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file citybuild_cmd.cpp Commands for player-driven city building (CITYSIM fork feature). */

#include "stdafx.h"
#include "command_func.h"
#include "core/random_func.hpp"
#include "economy_func.h"
#include "house.h"
#include "tile_map.h"
#include "landscape.h"
#include "landscape_cmd.h"
#include "slope_func.h"
#include "bridge_map.h"
#include "town.h"
#include "townzone.h"
#include "citybuild_cmd.h"
#include "timer/timer_game_calendar.h"

#include "table/strings.h"

#include "safeguards.h"

/**
 * Pick the house type to build for the player place-house command.
 *
 * The choice must be identical in the test and execute run of the command and
 * on all clients, so this uses no random numbers: among all 1x1 houses of the
 * requested zone valid for the tile's climate, town zone, year and slope, the
 * one with the highest NewGRF appearance probability (ties broken by lowest
 * HouseID) is selected.
 *
 * @param t The town the house will belong to.
 * @param tile The tile to build on.
 * @param slope The slope of that tile.
 * @param zone Whether to pick a residential or commercial building.
 * @return The selected house type, or #INVALID_HOUSE_ID if none is suitable.
 */
static HouseID SelectPlayerHouse(const Town *t, TileIndex tile, Slope slope, TownZone zone)
{
	/* Compute the zone requirements like TryBuildTownHouse does. */
	HouseZones zones{GetTownRadiusGroup(t, tile)};
	switch (_settings_game.game_creation.landscape) {
		case LandscapeType::Temperate: zones.Set(HouseZone::ClimateTemperate); break;
		case LandscapeType::Arctic: zones.Set(GetTileMaxZ(tile) > HighestSnowLine() ? HouseZone::ClimateSubarcticAboveSnow : HouseZone::ClimateSubarcticBelowSnow); break;
		case LandscapeType::Tropic: zones.Set(HouseZone::ClimateSubtropic); break;
		case LandscapeType::Toyland: zones.Set(HouseZone::ClimateToyland); break;
	}

	struct Candidate {
		HouseID house;
		uint probability;
	};
	std::vector<Candidate> candidates;
	uint total_probability = 0;

	for (const auto &hs : HouseSpec::Specs()) {
		if (!hs.enabled || hs.grf_prop.override_id != INVALID_HOUSE_ID) continue;
		if (!hs.building_availability.All(zones)) continue;

		/* 1x1 houses of the requested zone only. */
		if (!hs.building_flags.Test(BuildingFlag::Size1x1)) continue;
		if (hs.building_flags.Any({BuildingFlag::IsChurch, BuildingFlag::IsStadium})) continue;
		if (zone == TownZone::Commercial ? !IsCommercialHouseSpec(hs) : !IsResidentialHouseSpec(hs)) continue;

		if (hs.extra_flags.Test(HouseExtraFlag::BuildingIsHistorical)) continue;
		if (TimerGameCalendar::year < hs.min_year || TimerGameCalendar::year > hs.max_year) continue;
		if (hs.building_flags.Test(BuildingFlag::NotSloped) && slope != SLOPE_FLAT) continue;

		/* Don't let the town's per-type counters overflow. */
		if (hs.class_id != HOUSE_NO_CLASS) {
			if (t->cache.building_counts.class_count[hs.class_id] == UINT16_MAX) continue;
		} else {
			if (t->cache.building_counts.id_count[hs.Index()] == UINT16_MAX) continue;
		}

		uint probability = std::max<uint>(hs.probability, 1);
		candidates.emplace_back(hs.Index(), probability);
		total_probability += probability;
	}

	if (candidates.empty()) return INVALID_HOUSE_ID;

	uint seed = tile.base();
	seed ^= (t->index.base() + 0x9E3779B9U + (seed << 6) + (seed >> 2));
	seed ^= (TimerGameCalendar::year.base() + 0x85EBCA6BU + (seed << 6) + (seed >> 2));
	seed ^= (to_underlying(zone) + 0xC2B2AE35U + (seed << 6) + (seed >> 2));
	uint pick = seed % total_probability;
	for (const Candidate &candidate : candidates) {
		if (pick < candidate.probability) return candidate.house;
		pick -= candidate.probability;
	}

	return candidates.front().house;
}

/**
 * Place a single residential or commercial building as a company (city building).
 * Placement requires sufficient demand in the target zone of the town.
 * Tile validation mirrors CmdPlaceHouse; construction reuses the town house
 * construction path via BuildPlayerHouse.
 * @param flags Type of operation.
 * @param tile Tile on which to place the house.
 * @param zone Zone to build in (residential or commercial).
 * @return The cost of clearing the tile, or an error.
 */
CommandCost CmdPlacePlayerHouse(DoCommandFlags flags, TileIndex tile, TownZone zone)
{
	if (zone != TownZone::Residential && zone != TownZone::Commercial) return CMD_ERROR;
	if (Town::GetNumItems() == 0) return CommandCost(STR_ERROR_MUST_FOUND_TOWN_FIRST);

	Slope slope = GetTileSlope(tile);
	if (IsSteepSlope(slope)) return CommandCost(STR_ERROR_LAND_SLOPED_IN_WRONG_DIRECTION);
	if (IsBridgeAbove(tile)) return CommandCost(STR_ERROR_MUST_DEMOLISH_BRIDGE_FIRST);

	CommandCost cost = Command<Commands::LandscapeClear>::Do({DoCommandFlag::Auto, DoCommandFlag::NoWater}, tile);
	if (!cost.Succeeded()) return cost;

	Town *t = ClosestTownFromTile(tile, UINT_MAX);
	if (GetTownZoneDemand(t, zone) < TOWNZONE_PLACE_THRESHOLD) return CommandCost(STR_ERROR_ZONE_NO_DEMAND);

	HouseID house = SelectPlayerHouse(t, tile, slope, zone);
	if (house == INVALID_HOUSE_ID) return CommandCost(STR_ERROR_CITY_NO_SUITABLE_HOUSE);
	cost.AddCost(GetTownZoneBuildCost(zone));

	if (flags.Test(DoCommandFlag::Execute)) {
		BuildPlayerHouse(t, tile, HouseSpec::Get(house), house, static_cast<uint8_t>(Random()));
	}

	return cost;
}
