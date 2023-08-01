/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file plans_sl.cpp Code handling saving and loading of plans data. */

#include "../stdafx.h"
#include "../plans_base.h"
#include "../fios.h"

#include "saveload.h"

/** Description of a plan within the savegame. */
static const SaveLoad _plan_desc[] = {
	SLE_VAR(Plan, owner,          SLE_UINT8),
	SLE_VAR(Plan, visible,        SLE_BOOL),
	SLE_VAR(Plan, visible_by_all, SLE_BOOL),
	SLE_VAR(Plan, creation_date,  SLE_INT32),
};

struct PLANChunkHandler : ChunkHandler {
	PLANChunkHandler() : ChunkHandler('PLAN', CH_TABLE) {}

	void Save() const override
	{
		SlTableHeader(_plan_desc);

		for (Plan *p : Plan::Iterate()) {
			SlSetArrayIndex(p->index);
			SlObject(p, _plan_desc);
		}
	}

	void Load() const override
	{
		int index;
		while ((index = SlIterateArray()) != -1) {
			Plan *p = new (index) Plan();
			SlObject(p, _plan_desc);
		}
	}
};

struct PLLNChunkHandler : ChunkHandler {
	PLLNChunkHandler() : ChunkHandler('PLLN', CH_ARRAY) {}

	void Save() const override
	{
		for (Plan *p : Plan::Iterate()) {
			for (size_t i = 0; i < p->lines.size(); i++) {
				SlSetArrayIndex((uint) p->index << 16 | (uint) i);
				PlanLine *pl = p->lines[i];
				size_t plsz = pl->tiles.size();
				SlSetLength(plsz * sizeof(TileIndex));
				SlArray(&pl->tiles[0], plsz, SLE_UINT32);
			}
		}
	}

	void Load() const override
	{
		int index;
		while ((index = SlIterateArray()) != -1) {
			Plan *p = Plan::Get((uint) index >> 16);
			uint line_index = index & 0xFFFF;
			if (p->lines.size() <= line_index) p->lines.resize(line_index + 1);
			PlanLine *pl = new PlanLine();
			p->lines[line_index] = pl;
			size_t plsz = SlGetFieldLength() / sizeof(TileIndex);
			pl->tiles.resize(plsz);
			SlArray(&pl->tiles[0], plsz, SLE_UINT32);
		}

		for (Plan *p : Plan::Iterate()) {
			p->SetVisibility(false);
		}
	}
};

static const PLANChunkHandler PLAN;
static const PLLNChunkHandler PLLN;
static const ChunkHandlerRef plan_chunk_handlers[] = {
	PLAN,
	PLLN,
};

extern const ChunkHandlerTable _plan_chunk_handlers(plan_chunk_handlers);
