/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file plans_cmd.cpp Handling of plan related commands. */

#include "stdafx.h"
#include "command_func.h"
#include "plans_base.h"
#include "plans_cmd.h"
#include "plans_func.h"
#include "window_func.h"
#include "company_func.h"
#include "window_gui.h"
#include "table/strings.h"

PlanLine::PlanLine()
{
	this->visible = true;
	this->focused = false;
}

PlanLine::~PlanLine()
{
	this->Clear();
}

void PlanLine::Clear()
{
	this->tiles.clear();
}

bool PlanLine::AppendTile(TileIndex tile)
{
	const size_t cnt = this->tiles.size();

	if (cnt > 0) {
		const TileIndex last_tile = this->tiles[cnt - 1];
		if (last_tile == tile) return false;
		MarkTileLineDirty(last_tile, tile);

		if (cnt > 1) {
			const TileIndex t0 = this->tiles[cnt - 2];
			const int x0 = (int) TileX(t0);
			const int y0 = (int) TileY(t0);
			const TileIndex t1 = this->tiles[cnt - 1];
			const int x1 = (int) TileX(t1);
			const int y1 = (int) TileY(t1);
			const int x2 = (int) TileX(tile);
			const int y2 = (int) TileY(tile);

			if ((y1 - y0) * (x2 - x1) == (y2 - y1) * (x1 - x0)) { // Same direction.
				if (abs(x2 - x1) <= abs(x2 - x0) && abs(y2 - y1) <= abs(y2 - y0)) { // Tile i+1 is between i and i+2.
					/* The new tile is in the continuity, just update the last tile. */
					this->tiles[cnt - 1] = tile;
					MarkTileLineDirty(t1, tile);
					return true;
				}
			}
		}
	}

	if (this->tiles.size() >= MAX_PLAN_LINE_LENGTH) return false;

	this->tiles.push_back(tile);
	return true;
}

void PlanLine::SetFocus(bool focused)
{
	if (this->focused != focused) this->MarkDirty();
	this->focused = focused;
}

bool PlanLine::ToggleVisibility()
{
	this->SetVisibility(!this->visible);
	return this->visible;
}

void PlanLine::SetVisibility(bool visible)
{
	if (this->visible != visible) this->MarkDirty();
	this->visible = visible;
}

void PlanLine::MarkDirty()
{
	const size_t sz = this->tiles.size();
	for (size_t i = 1; i < sz; i++) {
		MarkTileLineDirty(this->tiles[i-1], this->tiles[i]);
	}
}

TileIndex *PlanLine::Export(size_t *buffer_length)
{
	const size_t cnt = this->tiles.size();
	const size_t datalen = sizeof(TileIndex) * cnt;
	TileIndex *buffer = (TileIndex *) malloc(datalen);
	if (buffer) {
		for (size_t i = 0; i < cnt; i++) {
			buffer[i] = TO_LE32(this->tiles[i]);
		}
		if (buffer_length) *buffer_length = datalen;
	}
	return buffer;
}

void PlanLine::Import(const TileIndex* data, const size_t data_length)
{
	for (size_t i = data_length; i != 0; i--, data++) {
		this->tiles.push_back(FROM_LE32(*data));
	}
}

Plan::Plan(Owner owner)
{
	this->owner = owner;
	this->creation_date = TimerGameCalendar::date;
	this->visible = false;
	this->visible_by_all = false;
	this->show_lines = false;
	this->temp_line = new PlanLine();
}

Plan::~Plan()
{
	for (PlanLineVector::iterator it = lines.begin(); it != lines.end(); it++) {
		delete (*it);
	}
	this->lines.clear();
	delete temp_line;
}

void Plan::SetFocus(bool focused)
{
	for (PlanLineVector::iterator it = lines.begin(); it != lines.end(); it++) {
		(*it)->SetFocus(focused);
	}
}

void Plan::SetVisibility(bool visible, bool do_lines)
{
	this->visible = visible;

	if (!do_lines) return;
	for (PlanLineVector::iterator it = lines.begin(); it != lines.end(); it++) {
		(*it)->SetVisibility(visible);
	}
}

bool Plan::ToggleVisibility()
{
	this->SetVisibility(!this->visible);
	return this->visible;
}

PlanLine *Plan::NewLine()
{
	PlanLine *pl = new PlanLine();
	if (pl) this->lines.push_back(pl);
	return pl;
}

bool Plan::StoreTempTile(TileIndex tile)
{
	return this->temp_line->AppendTile(tile);
}

bool Plan::ValidateNewLine()
{
	bool ret = false;
	if (this->temp_line->tiles.size() > 1) {
		size_t buffer_length = 0;
		const TileIndex *buffer = this->temp_line->Export(&buffer_length);
		if (buffer) {
			_current_plan->SetVisibility(true, false);
			std::string buffer_str((const char *) buffer, buffer_length);
			ret = Command<CMD_ADD_PLAN_LINE>::Post(STR_ERROR_CAN_T_CHANGE_PLAN, _current_plan->index, (uint32_t) this->temp_line->tiles.size(), buffer_str);
			free(buffer);
		}
		_current_plan->temp_line->MarkDirty();
		this->temp_line->Clear();
	}
	return ret;
}

bool Plan::IsListable()
{
	return (this->owner == _local_company || this->visible_by_all);
}

bool Plan::IsVisible()
{
	if (!this->IsListable()) return false;
	return this->visible;
}

bool Plan::ToggleVisibilityByAll()
{
	if (_current_plan->owner == _local_company) Command<CMD_CHANGE_PLAN_VISIBILITY>::Post(_current_plan->index, !this->visible_by_all);
	return this->visible_by_all;
}

/**
 * Create a new plan.
 * @param flags type of operation
 * @param o owner of the plan
 * @return the cost of this operation or an error
 */
std::tuple<CommandCost, PlanID> CmdAddPlan(DoCommandFlag flags, Owner o)
{
	if (!Plan::CanAllocateItem()) return { CommandCost(STR_ERROR_TOO_MANY_PLANS), INVALID_PLAN };
	if (flags & DC_EXEC) {
		Plan *p = new Plan(o);
		if (o == _local_company) {
			p->SetVisibility(true);
			Window *w = FindWindowById(WC_PLANS, 0);
			if (w) w->InvalidateData(INVALID_PLAN, false);
		}
		return { CommandCost(), p->index };
	}
	return { CommandCost(), INVALID_PLAN };
}

/**
 * Create a new line in a plan.
 * @param tile unused
 * @param flags type of operation
 * @param p1 plan id
 * @param p2 number of nodes
 * @param text list of tile indexes that compose the line, encoded in base64
 * @return the cost of this operation or an error
 */
CommandCost CmdAddPlanLine(DoCommandFlag flags, PlanID index, uint32_t line_length, const std::string &data)
{
	if (flags & DC_EXEC) {
		Plan *p = Plan::Get(index);
		PlanLine *pl = p->NewLine();
		if (!pl) return_cmd_error(STR_ERROR_NO_MORE_SPACE_FOR_LINES);
		if (line_length > MAX_PLAN_LINE_LENGTH) return_cmd_error(STR_ERROR_TOO_MANY_NODES);
		pl->Import((const TileIndex *) data.c_str(), line_length);
		if (p->IsListable()) {
			pl->SetVisibility(p->visible);
			if (p->visible) pl->MarkDirty();
			Window *w = FindWindowById(WC_PLANS, 0);
			if (w) w->InvalidateData(INVALID_PLAN, false);
		}
	}
	return CommandCost();
}

/**
 * Edit the visibility of a plan.
 * @param flags type of operation
 * @param p1 plan id
 * @param p2 visibility
 * @return the cost of this operation or an error
 */
CommandCost CmdChangePlanVisibility(DoCommandFlag flags, PlanID index, bool visible)
{
	if (flags & DC_EXEC) {
		Plan *p = Plan::Get(index);
		p->visible_by_all = visible;
		Window *w = FindWindowById(WC_PLANS, 0);
		if (w) w->InvalidateData(INVALID_PLAN, false);
	}
	return CommandCost();
}

/**
 * Delete a plan.
 * @param flags type of operation
 * @param p1 plan id
 * @return the cost of this operation or an error
 */
CommandCost CmdRemovePlan(DoCommandFlag flags, PlanID index)
{
	if (flags & DC_EXEC) {
		Plan *p = Plan::Get(index);
		if (p->IsListable()) {
			p->SetVisibility(false);
			Window *w = FindWindowById(WC_PLANS, 0);
			if (w) w->InvalidateData(p->index, false);
		}
		if (p == _current_plan) _current_plan = NULL;
		delete p;
	}
	return CommandCost();
}

/**
 * Remove a line from a plan.
 * @param flags type of operation
 * @param p1 plan id
 * @param p2 line id
 * @return the cost of this operation or an error
 */
CommandCost CmdRemovePlanLine(DoCommandFlag flags, PlanID index, PlanLineID line_index)
{
	if (flags & DC_EXEC) {
		Plan *p = Plan::Get(index);
		PlanLineVector::iterator it = p->lines.begin();
		std::advance(it, line_index);
		(*it)->SetVisibility(false);
		delete *it;
		p->lines.erase(it);
		if (p->IsListable()) {
			Window *w = FindWindowById(WC_PLANS, 0);
			if (w) w->InvalidateData(p->index, false);
		}
	}
	return CommandCost();
}