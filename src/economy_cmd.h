/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file economy_cmd.h Command definitions related to the economy. */

#ifndef ECONOMY_CMD_H
#define ECONOMY_CMD_H

#include "command_type.h"
#include "company_type.h"
#include "industry_type.h"

CommandCost CmdBuyCompany(DoCommandFlag flags, CompanyID target_company, bool hostile_takeover);
std::tuple<CommandCost, IndustryID> CmdBuyContract(DoCommandFlag flags, byte unused);

DEF_CMD_TRAIT(CMD_BUY_COMPANY,           CmdBuyCompany,         0, CMDT_MONEY_MANAGEMENT)
DEF_CMD_TRAIT(CMD_BUY_CONTRACT,          CmdBuyContract,        0, CMDT_MONEY_MANAGEMENT)

void CcBuyContract(Commands cmd, const CommandCost &result, IndustryID industry, byte);

#endif /* ECONOMY_CMD_H */
