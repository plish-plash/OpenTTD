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

static std::vector<TileIndex> _line_tiles;

class SlPlanLines : public DefaultSaveLoadHandler<SlPlanLines, Plan> {
public:
	inline static const SaveLoad description[] = {
		SLE_VAR(PlanLine,    visible,     SLE_BOOL),
		SLEG_VECTOR("tiles", _line_tiles, SLE_UINT32),
	};
	inline const static SaveLoadCompatTable compat_description = {};

	void Save(Plan *p) const override
	{
		SlSetStructListLength(p->lines.size());

		for (auto *pl : p->lines) {
			_line_tiles = pl->tiles;
			SlObject(pl, this->GetDescription());
		}
		_line_tiles.clear();
	}

	void Load(Plan *p) const override
	{
		size_t len = SlGetStructListLength(UINT32_MAX);
		p->lines.reserve(len);

		for (size_t i = 0; i < len; ++i) {
			PlanLine *pl = new PlanLine();
			SlObject(pl, this->GetDescription());
			pl->tiles = _line_tiles;
			p->lines.push_back(pl);
			_line_tiles.clear();
		}
	}
};

/** Description of a plan within the savegame. */
static const SaveLoad _plan_desc[] = {
	SLE_VAR(Plan, owner,          SLE_UINT8),
	SLE_VAR(Plan, visible,        SLE_BOOL),
	SLE_VAR(Plan, visible_by_all, SLE_BOOL),
	SLE_VAR(Plan, creation_date,  SLE_INT32),
	SLEG_STRUCTLIST("lines", SlPlanLines),
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
		SlTableHeader(_plan_desc);

		int index;
		while ((index = SlIterateArray()) != -1) {
			Plan *p = new (index) Plan();
			SlObject(p, _plan_desc);
		}
	}
};

static const PLANChunkHandler PLAN;
static const ChunkHandlerRef plan_chunk_handlers[] = {
	PLAN,
};

extern const ChunkHandlerTable _plan_chunk_handlers(plan_chunk_handlers);
