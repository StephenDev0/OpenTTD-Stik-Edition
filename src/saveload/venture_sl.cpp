/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file venture_sl.cpp Code handling saving and loading of the venture board (CITYSIM fork feature). */

#include "../stdafx.h"

#include "saveload.h"

#include "../venture.h"

#include "../safeguards.h"

static const SaveLoad _venture_desc[] = {
	SLE_VAR(Venture, name_index,        SLE_UINT8),
	SLE_VAR(Venture, state,             SLE_UINT8),
	SLE_VAR(Venture, stage,             SLE_UINT8),
	SLE_VAR(Venture, valuation,         SLE_INT64),
	SLE_VAR(Venture, quarters_in_stage, SLE_UINT8),
	SLE_VAR(Venture, respawn_timer,     SLE_UINT8),
	SLE_ARR(Venture, stakes,            SLE_UINT16, MAX_COMPANIES),
	SLE_CONDARR(Venture, cost_basis,    SLE_INT64,  MAX_COMPANIES, SLV_CITYSIM_VENTURES, SL_MAX_VERSION),
};

struct VNTRChunkHandler : ChunkHandler {
	VNTRChunkHandler() : ChunkHandler('VNTR', CH_TABLE) {}

	void Save() const override
	{
		SlTableHeader(_venture_desc);

		for (uint i = 0; i < NUM_VENTURES; i++) {
			SlSetArrayIndex(i);
			SlObject(&_ventures[i], _venture_desc);
		}
	}

	void Load() const override
	{
		const std::vector<SaveLoad> slt = SlTableHeader(_venture_desc);

		int index;
		while ((index = SlIterateArray()) != -1) {
			if (static_cast<uint>(index) >= NUM_VENTURES) SlErrorCorrupt("Too many ventures");
			SlObject(&_ventures[index], slt);
		}
	}
};

static const VNTRChunkHandler VNTR;
static const ChunkHandlerRef venture_chunk_handlers[] = {
	VNTR,
};

extern const ChunkHandlerTable _venture_chunk_handlers(venture_chunk_handlers);
