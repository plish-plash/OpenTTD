/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file infrastructure_func.h Functions for access to (shared) infrastructure */

#ifndef INFRASTRUCTURE_FUNC_H
#define INFRASTRUCTURE_FUNC_H

#include "vehicle_base.h"
#include "company_func.h"
#include "tile_map.h"

void PayStationSharingFee(Vehicle *v, const Station *st);

/**
 * Check whether a vehicle of a given owner and type can use the infrastructure of a given company.
 * @param type        Type of vehicle we are talking about.
 * @param veh_owner   Owner of the vehicle in question.
 * @param infra_owner The owner of the infrastructure.
 * @return            True if infrastructure usage is allowed, false otherwise.
 */
static inline bool IsInfraUsageAllowed(VehicleType type, Owner veh_owner, Owner infra_owner)
{
	return infra_owner == veh_owner || infra_owner == OWNER_NONE || _settings_game.economy.infrastructure_sharing[type];
}

/**
 * Check whether a vehicle of a given owner and type can use the infrastructure on a given tile.
 * @param type        Type of vehicle we are talking about.
 * @param veh_owner   Owner of the vehicle in question.
 * @param tile        The tile that may or may not be used.
 * @return            True if infrastructure usage is allowed, false otherwise.
 */
static inline bool IsInfraTileUsageAllowed(VehicleType type, Owner veh_owner, TileIndex tile)
{
	return IsInfraUsageAllowed(type, veh_owner, GetTileOwner(tile));
}

/**
 * Is a vehicle owned by _current_company allowed to use the infrastructure of infra_owner?
 * If this is not allowed, this function provides the appropriate error message.
 * @see IsInfraUsageAllowed
 * @see CheckOwnership
 * @param type        Type of vehicle.
 * @param infra_owner Owner of the infrastructure.
 * @param tile        Tile of the infrastructure.
 * @return            CommandCost indicating success or failure.
 */
static inline CommandCost CheckInfraUsageAllowed(VehicleType type, Owner infra_owner, TileIndex tile = 0)
{
	if (infra_owner == OWNER_NONE || _settings_game.economy.infrastructure_sharing[type]) return CommandCost();
	return CheckOwnership(infra_owner, tile);
}

/**
 * Do signal states propagate from the tracks of one owner to the other?
 * @note This function should be consistent, so if it returns true for (a, b) and (b, c),
 * it should also return true for (a, c).
 * @param o1 First track owner.
 * @param o2 Second track owner.
 * @return   True if tracks of the two owners are part of the same signal block.
 */
static inline bool IsOneSignalBlock(Owner o1, Owner o2)
{
	return o1 == o2 || _settings_game.economy.infrastructure_sharing[VEH_TRAIN];
}

#endif /* INFRASTRUCTURE_FUNC_H */