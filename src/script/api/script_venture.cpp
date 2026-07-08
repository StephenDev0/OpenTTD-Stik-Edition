/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file script_venture.cpp Implementation of ScriptVenture (CITYSIM fork feature). */

#include "../../stdafx.h"
#include "script_venture.hpp"
#include "script_companymode.hpp"
#include "script_error.hpp"
#include "../../venture_cmd.h"
#include "../../company_base.h"
#include "../../strings_func.h"
#include "../../string_func.h"

#include "../../safeguards.h"

/* static */ SQInteger ScriptVenture::GetVentureCount()
{
	return NUM_VENTURES;
}

/* static */ bool ScriptVenture::IsValidVenture(SQInteger slot)
{
	return slot >= 0 && slot < static_cast<SQInteger>(NUM_VENTURES);
}

/* static */ std::optional<std::string> ScriptVenture::GetName(SQInteger slot)
{
	if (!IsValidVenture(slot)) return std::nullopt;

	return ::StrMakeValid(::GetString(GetVentureNameString(_ventures[slot])), {});
}

/* static */ ScriptVenture::VentureState ScriptVenture::GetState(SQInteger slot)
{
	if (!IsValidVenture(slot)) return VENTURE_STATE_EMPTY;

	return static_cast<VentureState>(_ventures[slot].state);
}

/* static */ ScriptVenture::VentureStage ScriptVenture::GetStage(SQInteger slot)
{
	if (!IsValidVenture(slot)) return VENTURE_STAGE_SEED;

	return static_cast<VentureStage>(_ventures[slot].stage);
}

/* static */ Money ScriptVenture::GetValuation(SQInteger slot)
{
	if (!IsValidVenture(slot)) return -1;

	return _ventures[slot].valuation;
}

/* static */ SQInteger ScriptVenture::GetStake(SQInteger slot)
{
	if (!IsValidVenture(slot)) return -1;
	::CompanyID company = ScriptObject::GetCompany();
	if (!::Company::IsValidID(company)) return -1;

	return _ventures[slot].stakes[company.base()];
}

/* static */ SQInteger ScriptVenture::GetTotalStake(SQInteger slot)
{
	if (!IsValidVenture(slot)) return -1;

	return GetTotalVentureStakeBp(_ventures[slot]);
}

/* static */ bool ScriptVenture::BuyStake(SQInteger slot, SQInteger basis_points)
{
	EnforceCompanyModeValid(false);
	EnforcePrecondition(false, IsValidVenture(slot));
	EnforcePrecondition(false, basis_points >= 100 && basis_points <= UINT16_MAX);

	return ScriptObject::Command<Commands::BuyVentureStake>::Do(static_cast<uint8_t>(slot), static_cast<uint16_t>(basis_points));
}

/* static */ bool ScriptVenture::SellStake(SQInteger slot, SQInteger basis_points)
{
	EnforceCompanyModeValid(false);
	EnforcePrecondition(false, IsValidVenture(slot));
	EnforcePrecondition(false, basis_points >= 100 && basis_points <= UINT16_MAX);

	return ScriptObject::Command<Commands::SellVentureStake>::Do(static_cast<uint8_t>(slot), static_cast<uint16_t>(basis_points));
}
