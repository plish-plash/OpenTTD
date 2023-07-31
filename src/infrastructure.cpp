/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file infrastructure.cpp Implementation of infrastructure sharing */

#include "stdafx.h"
#include "infrastructure_func.h"
#include "station_base.h"

/**
 * Pay the fee for using another company's station.
 * @param v The vehicle that is using the station.
 * @param st The station that it uses.
 */
void PayStationSharingFee(Vehicle *v, const Station *st)
{
	if (v->owner == st->owner || st->owner == OWNER_NONE) return;
	Money sharing_fee = v->GetMaxWeight() << 8;
	v->profit_this_year -= sharing_fee;
	SubtractMoneyFromCompanyFract(v->owner, CommandCost(EXPENSES_SHARING_COST, sharing_fee));
	SubtractMoneyFromCompanyFract(st->owner, CommandCost(EXPENSES_SHARING_REVENUE, -sharing_fee));
}
