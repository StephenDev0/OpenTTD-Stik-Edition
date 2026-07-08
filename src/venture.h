/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file venture.h Venture capital: investable fictional startups (CITYSIM fork feature). */

#ifndef VENTURE_H
#define VENTURE_H

#include "company_type.h"
#include "economy_type.h"
#include "strings_type.h"

/** Number of startup slots on the venture board. */
static constexpr uint NUM_VENTURES = 12;
/** Number of distinct startup names in the name pool. */
static constexpr uint NUM_VENTURE_NAMES = 24;

/** Maximum stake a single company may hold in one startup, in basis points. */
static constexpr uint16_t VENTURE_MAX_COMPANY_STAKE_BP = 4900;
/** Maximum combined stake of all companies in one startup, in basis points. */
static constexpr uint16_t VENTURE_MAX_TOTAL_STAKE_BP = 8000;
/** Smallest tradable stake, in basis points (1%). */
static constexpr uint16_t VENTURE_TRADE_UNIT_BP = 100;
/** Fraction (percent) of pro-rata value received when selling before an exit. */
static constexpr uint VENTURE_SELL_PCT = 70;

/** Lifecycle of a startup slot. */
enum class VentureState : uint8_t {
	Empty,    ///< No startup in this slot yet.
	Active,   ///< Startup is private and investable.
	Failed,   ///< Startup went bankrupt; stakes were wiped.
	ExitedIpo,      ///< Startup went public; holders were paid a windfall.
	ExitedAcquired, ///< Startup was acquired; holders were paid a premium.
};

/** Funding stage of an active startup. */
enum class VentureStage : uint8_t {
	Seed,
	SeriesA,
	SeriesB,
	SeriesC,
};

/** One startup slot on the venture board. */
struct Venture {
	uint8_t name_index = 0; ///< Index into the startup name pool.
	VentureState state = VentureState::Empty;
	VentureStage stage = VentureStage::Seed;
	Money valuation = 0; ///< Current (or final, after fail/exit) valuation.
	uint8_t quarters_in_stage = 0;
	uint8_t respawn_timer = 0; ///< Quarters until a fresh startup takes a non-active slot.
	std::array<uint16_t, MAX_COMPANIES> stakes{}; ///< Stake per company, in basis points.
};

extern std::array<Venture, NUM_VENTURES> _ventures;

void InitializeVentures();
void UpdateVentures();
StringID GetVentureNameString(const Venture &v);
StringID GetVentureStatusString(const Venture &v);
uint16_t GetTotalVentureStakeBp(const Venture &v);

void ShowVenturesWindow();

#endif /* VENTURE_H */
