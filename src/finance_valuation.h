/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file finance_valuation.h Income-based company valuation for earned financing (CITYSIM fork feature). */

#ifndef FINANCE_VALUATION_H
#define FINANCE_VALUATION_H

#include "economy_type.h"

struct Company;

/** Number of quarters of income history used for the valuation. */
static constexpr uint VALUATION_QUARTERS = 8;
/** Valuation is this multiple of annualised operating profit. */
static constexpr uint VALUATION_EARNINGS_MULTIPLE = 10;

/** Minimum age of a company before it may go public, in economy years. */
static constexpr int IPO_MIN_AGE_YEARS = 3;
/** Number of consecutive profitable quarters required before an IPO. */
static constexpr uint IPO_REQUIRED_PROFITABLE_QUARTERS = 4;
/** Minimum income-based valuation required before an IPO. */
static constexpr Money IPO_MIN_VALUATION = 500000;
/** Default percentage of the company sold to the public in the IPO. */
static constexpr uint8_t IPO_FLOAT_PCT = 25;
/** Bounds for the IPO float percentage. */
static constexpr uint8_t IPO_MIN_FLOAT_PCT = 10;
static constexpr uint8_t IPO_MAX_FLOAT_PCT = 49;
/** Number of shares outstanding after the IPO. */
static constexpr uint32_t IPO_SHARES_OUTSTANDING = 100000;
/** Amount of dividend paid per click of the dividend button. */
static constexpr Money DIVIDEND_INTERVAL = 10000;

/** Result of testing all IPO gates for a company. */
struct IpoEligibility {
	Money valuation = 0; ///< Current income-based valuation.
	uint age_years = 0; ///< Company age in economy years.
	uint profitable_quarters = 0; ///< Consecutive profitable quarters counted from the newest quarter.
	bool already_public = false; ///< Company is already listed.
	bool age_ok = false; ///< Company is old enough.
	bool income_ok = false; ///< Enough consecutive profitable quarters.
	bool valuation_ok = false; ///< Valuation meets the minimum.

	bool IsEligible() const { return !this->already_public && this->age_ok && this->income_ok && this->valuation_ok; }
};

Money CalculateIncomeBasedValuation(const Company *c);
IpoEligibility CheckIpoEligibility(const Company *c);
void UpdatePublicCompaniesFinance();

#endif /* FINANCE_VALUATION_H */
