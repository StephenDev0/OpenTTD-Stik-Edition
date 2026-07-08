/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file citysim.cpp Test functionality for CITYSIM fork helpers. */

#include "../stdafx.h"

#include "../3rdparty/catch2/catch.hpp"

#include "../finance_valuation.h"
#include "../venture.h"

#include "../safeguards.h"

TEST_CASE("Venture stake values")
{
	Venture v;
	v.valuation = 1000000;

	CHECK(GetVentureStakeValue(v, 100) == 10000);
	CHECK(GetVentureStakeValue(v, 4900) == 490000);
	CHECK(GetVentureSellValue(v, 100) == 7000);
}

TEST_CASE("Venture total stake")
{
	Venture v;
	v.stakes[0] = 100;
	v.stakes[1] = 2500;
	v.stakes[2] = 4900;

	CHECK(GetTotalVentureStakeBp(v) == 7500);
}

TEST_CASE("IPO eligibility gates")
{
	IpoEligibility eligible;
	eligible.age_ok = true;
	eligible.income_ok = true;
	eligible.valuation_ok = true;
	CHECK(eligible.IsEligible());

	eligible.already_public = true;
	CHECK(!eligible.IsEligible());

	eligible.already_public = false;
	eligible.income_ok = false;
	CHECK(!eligible.IsEligible());
}
