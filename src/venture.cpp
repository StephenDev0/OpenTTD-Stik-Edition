/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file venture.cpp Venture capital: investable fictional startups (CITYSIM fork feature). */

#include "stdafx.h"
#include "company_base.h"
#include "company_func.h"
#include "company_gui.h"
#include "core/random_func.hpp"
#include "venture.h"
#include "window_func.h"

#include "table/strings.h"

#include "safeguards.h"

std::array<Venture, NUM_VENTURES> _ventures{};

/** Per-quarter bankruptcy chance (percent) for each funding stage. */
static const uint8_t _venture_fail_chance[] = {12, 9, 7, 5};
/** Chance (percent) to advance to the next stage, once settled in the current one. */
static constexpr uint VENTURE_ADVANCE_CHANCE = 30;
/** Quarters a startup must spend in a stage before it can advance. */
static constexpr uint VENTURE_MIN_QUARTERS_PER_STAGE = 2;
/** Valuation multiplier (in tenths) when advancing a funding stage. */
static constexpr uint VENTURE_ADVANCE_MULT_TENTHS = 22;
/** Payout multiplier (in tenths) for an IPO exit. */
static constexpr uint VENTURE_IPO_MULT_TENTHS = 30;
/** Payout multiplier (in tenths) for an acquisition exit. */
static constexpr uint VENTURE_ACQUIRE_MULT_TENTHS = 13;
/** Quarters a failed/exited slot stays visible before a new startup appears. */
static constexpr uint8_t VENTURE_RESPAWN_QUARTERS = 4;

/** @return The name string of the startup in this slot. */
StringID GetVentureNameString(const Venture &v)
{
	return STR_VENTURE_NAME_FIRST + v.name_index;
}

/** @return The stage/state string of the startup in this slot. */
StringID GetVentureStatusString(const Venture &v)
{
	switch (v.state) {
		case VentureState::Active: return STR_VENTURE_STAGE_SEED + to_underlying(v.stage);
		case VentureState::Failed: return STR_VENTURE_STATE_FAILED;
		case VentureState::ExitedIpo: return STR_VENTURE_STATE_IPO;
		case VentureState::ExitedAcquired: return STR_VENTURE_STATE_ACQUIRED;
		default: return STR_VENTURE_STATE_EMPTY;
	}
}

/** @return Combined stake of all companies in this startup, in basis points. */
uint16_t GetTotalVentureStakeBp(const Venture &v)
{
	uint total = 0;
	for (uint16_t bp : v.stakes) total += bp;
	return static_cast<uint16_t>(total);
}

/** Spawn a fresh seed-stage startup into a slot, using the synced RNG. */
static void SpawnVenture(Venture &v)
{
	/* Prefer a name not already on the board. */
	uint8_t name_index = 0;
	for (uint attempt = 0; attempt < 8; attempt++) {
		name_index = static_cast<uint8_t>(RandomRange(NUM_VENTURE_NAMES));
		bool in_use = std::ranges::any_of(_ventures, [&](const Venture &other) {
			return other.state == VentureState::Active && other.name_index == name_index;
		});
		if (!in_use) break;
	}

	v.name_index = name_index;
	v.state = VentureState::Active;
	v.stage = VentureStage::Seed;
	v.valuation = 100000 + static_cast<int64_t>(RandomRange(300)) * 1000;
	v.quarters_in_stage = 0;
	v.respawn_timer = 0;
	v.stakes.fill(0);
}

/** Reset the venture board for a new game. */
void InitializeVentures()
{
	for (Venture &v : _ventures) {
		v = {};
		SpawnVenture(v);
	}
}

/**
 * Pay all shareholders of an exiting startup their share of the exit value.
 * The proceeds are credited directly to the company balance (like a loan),
 * NOT through the income statement, so venture windfalls cannot inflate the
 * income-based valuation that IPO proceeds are priced off.
 */
static void PayVentureExit(Venture &v, Money exit_value)
{
	for (Company *c : Company::Iterate()) {
		uint16_t bp = v.stakes[c->index.base()];
		if (bp == 0) continue;

		Money payout = exit_value * bp / 10000;
		if (c->money > Money::max() - payout) continue;
		c->money += payout;
		InvalidateCompanyWindows(c);
	}
}

/**
 * Quarterly update of the venture board, from the deterministic game loop.
 * Each active startup can fail, advance a funding stage, exit, or drift in
 * value; failed/exited slots respawn with a new startup after a few quarters.
 */
void UpdateVentures()
{
	for (Venture &v : _ventures) {
		/* Drop stakes of companies that no longer exist, so a future company
		 * reusing the pool slot does not inherit them. */
		for (uint i = 0; i < MAX_COMPANIES; i++) {
			if (v.stakes[i] != 0 && !Company::IsValidID(static_cast<CompanyID>(i))) v.stakes[i] = 0;
		}

		if (v.state != VentureState::Active) {
			if (v.respawn_timer > 0) {
				v.respawn_timer--;
			} else {
				SpawnVenture(v);
			}
			continue;
		}

		v.quarters_in_stage++;

		uint roll = RandomRange(100);
		uint fail_chance = _venture_fail_chance[to_underlying(v.stage)];

		if (roll < fail_chance) {
			/* Bankrupt: stakes are wiped. */
			v.state = VentureState::Failed;
			v.valuation = 0;
			v.stakes.fill(0);
			v.respawn_timer = VENTURE_RESPAWN_QUARTERS;
		} else if (v.quarters_in_stage >= VENTURE_MIN_QUARTERS_PER_STAGE && roll < fail_chance + VENTURE_ADVANCE_CHANCE) {
			if (v.stage != VentureStage::SeriesC) {
				/* Next funding round at a higher valuation. */
				v.stage = static_cast<VentureStage>(to_underlying(v.stage) + 1);
				v.quarters_in_stage = 0;
				v.valuation = v.valuation * VENTURE_ADVANCE_MULT_TENTHS / 10;
			} else {
				/* Exit: IPO windfall or acquisition premium. */
				bool ipo = Chance16(1, 2);
				Money exit_value = v.valuation * (ipo ? VENTURE_IPO_MULT_TENTHS : VENTURE_ACQUIRE_MULT_TENTHS) / 10;
				PayVentureExit(v, exit_value);
				v.state = ipo ? VentureState::ExitedIpo : VentureState::ExitedAcquired;
				v.valuation = exit_value;
				v.respawn_timer = VENTURE_RESPAWN_QUARTERS;
			}
		} else {
			/* Drift between -15% and +15%. */
			int drift = static_cast<int>(RandomRange(31)) - 15;
			v.valuation += v.valuation * drift / 100;
			if (v.valuation < 10000) v.valuation = 10000;
		}
	}

	SetWindowDirty(WC_VENTURE_CAPITAL, 0);
}
