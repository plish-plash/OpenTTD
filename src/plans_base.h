/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file plans_base.h Base class for plans. */

#ifndef PLANS_BASE_H
#define PLANS_BASE_H

#include "plans_func.h"
#include "core/pool_type.hpp"
#include "company_type.h"
#include "company_func.h"
#include "command_func.h"
#include "map_func.h"
#include "date_type.h"
#include "viewport_func.h"
#include "core/endian_func.hpp"
#include <vector>

typedef Pool<Plan, PlanID, 16, 64000> PlanPool;
typedef std::vector<TileIndex> TileVector;
typedef std::vector<PlanLine*> PlanLineVector;
extern PlanPool _plan_pool;

static const size_t MAX_PLAN_LINE_LENGTH = 256;

struct PlanLine {
	bool visible;
	bool focused;
	TileVector tiles;

	PlanLine();
	~PlanLine();

	void Clear();
	bool AppendTile(TileIndex tile);

	void SetFocus(bool focused);
	bool ToggleVisibility();
	void SetVisibility(bool visible);
	void MarkDirty();

	TileIndex *Export(size_t *buffer_length);
	void Import(const TileIndex* data, const size_t data_length);
};

struct Plan : PlanPool::PoolItem<&_plan_pool> {
	Owner owner;
	PlanLineVector lines;
	PlanLine *temp_line;
	bool visible;
	bool visible_by_all;
	bool show_lines;
	TimerGameCalendar::Date creation_date;

	Plan(Owner owner = INVALID_OWNER);
	~Plan();

	void SetFocus(bool focused);
	void SetVisibility(bool visible, bool do_lines = true);
	bool ToggleVisibility();

	PlanLine *NewLine();
	bool StoreTempTile(TileIndex tile);
	bool ValidateNewLine();

	bool IsListable();
	bool IsVisible();
	bool ToggleVisibilityByAll();
};

#endif /* PLANS_BASE_H */