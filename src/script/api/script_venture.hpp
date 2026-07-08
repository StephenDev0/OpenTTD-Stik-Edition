/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file script_venture.hpp Everything to query and trade venture capital startups (CITYSIM fork feature). */

#ifndef SCRIPT_VENTURE_HPP
#define SCRIPT_VENTURE_HPP

#include "script_object.hpp"
#include "../../venture.h"

/**
 * Class that handles all venture capital related functions.
 * The venture board holds a fixed number of fictional startups that can be
 * invested in; most fail, survivors pay out on IPO or acquisition.
 * @api ai game
 */
class ScriptVenture : public ScriptObject {
public:
	/** The state of a startup slot on the venture board. */
	enum VentureState {
		VENTURE_STATE_EMPTY = static_cast<uint8_t>(::VentureState::Empty), ///< No startup in this slot.
		VENTURE_STATE_ACTIVE = static_cast<uint8_t>(::VentureState::Active), ///< Private and investable.
		VENTURE_STATE_FAILED = static_cast<uint8_t>(::VentureState::Failed), ///< Bankrupt; stakes wiped.
		VENTURE_STATE_EXITED_IPO = static_cast<uint8_t>(::VentureState::ExitedIpo), ///< Went public; holders paid out.
		VENTURE_STATE_EXITED_ACQUIRED = static_cast<uint8_t>(::VentureState::ExitedAcquired), ///< Acquired; holders paid out.
	};

	/** The funding stage of an active startup. */
	enum VentureStage {
		VENTURE_STAGE_SEED = static_cast<uint8_t>(::VentureStage::Seed),
		VENTURE_STAGE_SERIES_A = static_cast<uint8_t>(::VentureStage::SeriesA),
		VENTURE_STAGE_SERIES_B = static_cast<uint8_t>(::VentureStage::SeriesB),
		VENTURE_STAGE_SERIES_C = static_cast<uint8_t>(::VentureStage::SeriesC),
	};

	/**
	 * Get the number of slots on the venture board.
	 * @return The number of startup slots.
	 */
	static SQInteger GetVentureCount();

	/**
	 * Check whether a slot index is valid.
	 * @param slot The slot to check.
	 * @return True if the slot exists.
	 */
	static bool IsValidVenture(SQInteger slot);

	/**
	 * Get the name of the startup in a slot.
	 * @param slot The slot to query.
	 * @pre IsValidVenture(slot).
	 * @return The startup's name.
	 */
	static std::optional<std::string> GetName(SQInteger slot);

	/**
	 * Get the state of a startup slot.
	 * @param slot The slot to query.
	 * @pre IsValidVenture(slot).
	 * @return The state of the slot.
	 */
	static VentureState GetState(SQInteger slot);

	/**
	 * Get the funding stage of a startup.
	 * @param slot The slot to query.
	 * @pre IsValidVenture(slot).
	 * @return The funding stage (only meaningful while the startup is active).
	 */
	static VentureStage GetStage(SQInteger slot);

	/**
	 * Get the current valuation of a startup.
	 * @param slot The slot to query.
	 * @pre IsValidVenture(slot).
	 * @return The valuation; 0 for failed/empty slots.
	 */
	static Money GetValuation(SQInteger slot);

	/**
	 * Get your company's stake in a startup, in basis points (100 = 1%).
	 * @param slot The slot to query.
	 * @pre IsValidVenture(slot).
	 * @return Your stake in basis points.
	 * @api -game
	 */
	static SQInteger GetStake(SQInteger slot);

	/**
	 * Get the combined stake of all companies in a startup, in basis points.
	 * @param slot The slot to query.
	 * @pre IsValidVenture(slot).
	 * @return The total stake in basis points.
	 */
	static SQInteger GetTotalStake(SQInteger slot);

	/**
	 * Buy a stake in a startup at its current valuation.
	 * @param slot The slot to buy into.
	 * @param basis_points The stake to buy, in basis points (multiples of 100).
	 * @pre IsValidVenture(slot).
	 * @pre basis_points >= 100.
	 * @return True if the stake was bought.
	 * @api -game
	 */
	static bool BuyStake(SQInteger slot, SQInteger basis_points);

	/**
	 * Sell part of your stake in a startup, at a discount to pro-rata value.
	 * @param slot The slot to sell from.
	 * @param basis_points The stake to sell, in basis points (multiples of 100).
	 * @pre IsValidVenture(slot).
	 * @pre basis_points >= 100.
	 * @return True if the stake was sold.
	 * @api -game
	 */
	static bool SellStake(SQInteger slot, SQInteger basis_points);
};

#endif /* SCRIPT_VENTURE_HPP */
