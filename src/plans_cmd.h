/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file order_cmd.h Command definitions related to plans. */

#ifndef PLANS_CMD_H
#define PLANS_CMD_H

#include "command_type.h"
#include "plans_base.h"
#include "misc/endian_buffer.hpp"

std::tuple<CommandCost, PlanID> CmdAddPlan(DoCommandFlag flags, Owner o);
CommandCost CmdAddPlanLine(DoCommandFlag flags, PlanID index, uint32_t line_length, const std::string &data);
CommandCost CmdChangePlanVisibility(DoCommandFlag flags, PlanID index, bool visible);
CommandCost CmdRemovePlan(DoCommandFlag flags, PlanID index);
CommandCost CmdRemovePlanLine(DoCommandFlag flags, PlanID index, PlanLineID line_index);

DEF_CMD_TRAIT(CMD_ADD_PLAN,               CmdAddPlan,              CMD_DEITY, CMDT_OTHER_MANAGEMENT)
DEF_CMD_TRAIT(CMD_ADD_PLAN_LINE,          CmdAddPlanLine,          CMD_DEITY, CMDT_OTHER_MANAGEMENT)
DEF_CMD_TRAIT(CMD_CHANGE_PLAN_VISIBILITY, CmdChangePlanVisibility, CMD_DEITY, CMDT_OTHER_MANAGEMENT)
DEF_CMD_TRAIT(CMD_REMOVE_PLAN,            CmdRemovePlan,           CMD_DEITY, CMDT_OTHER_MANAGEMENT)
DEF_CMD_TRAIT(CMD_REMOVE_PLAN_LINE,       CmdRemovePlanLine,       CMD_DEITY, CMDT_OTHER_MANAGEMENT)

void CcAddPlan(Commands cmd, const CommandCost &result, PlanID new_plan);

#endif /* PLANS_CMD_H */
