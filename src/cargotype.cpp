/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file cargotype.cpp Implementation of cargoes. */

#include "stdafx.h"
#include "cargotype.h"
#include "newgrf_cargo.h"
#include "string_func.h"
#include "strings_func.h"
#include "settings_type.h"

#include "table/sprites.h"
#include "table/strings.h"
#include "table/cargo_const.h"

#include "safeguards.h"

CargoSpec CargoSpec::array[NUM_CARGO];

/**
 * Bitmask of cargo types available. This includes phony cargoes like regearing cargoes.
 * Initialized during a call to #SetupCargoForClimate.
 */
CargoTypes _cargo_mask;

/**
 * Bitmask of real cargo types available. Phony cargoes like regearing cargoes are excluded.
 */
CargoTypes _standard_cargo_mask;

/**
 * Set up the default cargo types for the given landscape type.
 * @param l Landscape
 */
void SetupCargoForClimate(LandscapeID l)
{
	assert(l < lengthof(_default_climate_cargo));

	/* Reset and disable all cargo types */
	for (CargoID i = 0; i < lengthof(CargoSpec::array); i++) {
		*CargoSpec::Get(i) = {};
		CargoSpec::Get(i)->bitnum = INVALID_CARGO;

		/* Set defaults for newer properties, which old GRFs do not know */
		CargoSpec::Get(i)->multiplier = 0x100;
	}

	_cargo_mask = 0;

	for (CargoID i = 0; i < lengthof(_default_climate_cargo[l]); i++) {
		CargoLabel cl = _default_climate_cargo[l][i];

		/* Bzzt: check if cl is just an index into the cargo table */
		if (cl < lengthof(_default_cargo)) {
			/* Copy the indexed cargo */
			CargoSpec *cargo = CargoSpec::Get(i);
			*cargo = _default_cargo[cl];
			if (cargo->bitnum != INVALID_CARGO) SetBit(_cargo_mask, i);
			continue;
		}

		/* Loop through each of the default cargo types to see if
		 * the label matches */
		for (uint j = 0; j < lengthof(_default_cargo); j++) {
			if (_default_cargo[j].label == cl) {
				*CargoSpec::Get(i) = _default_cargo[j];

				/* Populate the available cargo mask */
				SetBit(_cargo_mask, i);
				break;
			}
		}
	}
}

/**
 * Get the cargo ID of a default cargo, if present.
 * @param l Landscape
 * @param ct Default cargo type.
 * @return ID number if the cargo exists, else #CT_INVALID
 */
CargoID GetDefaultCargoID(LandscapeID l, CargoType ct)
{
	assert(l < lengthof(_default_climate_cargo));

	if (!IsValidCargoType(ct)) return CT_INVALID;

	assert(ct < lengthof(_default_climate_cargo[0]));
	CargoLabel cl = _default_climate_cargo[l][ct];
	/* Bzzt: check if cl is just an index into the cargo table */
	if (cl < lengthof(_default_cargo)) {
		cl = _default_cargo[cl].label;
	}

	return GetCargoIDByLabel(cl);
}

/**
 * Get the cargo ID by cargo label.
 * @param cl Cargo type to get.
 * @return ID number if the cargo exists, else #CT_INVALID
 */
CargoID GetCargoIDByLabel(CargoLabel cl)
{
	for (const CargoSpec *cs : CargoSpec::Iterate()) {
		if (cs->label == cl) return cs->Index();
	}

	/* No matching label was found, so it is invalid */
	return CT_INVALID;
}


/**
 * Find the CargoID of a 'bitnum' value.
 * @param bitnum 'bitnum' to find.
 * @return First CargoID with the given bitnum, or #CT_INVALID if not found or if the provided \a bitnum is invalid.
 */
CargoID GetCargoIDByBitnum(uint8_t bitnum)
{
	if (bitnum == INVALID_CARGO) return CT_INVALID;

	for (const CargoSpec *cs : CargoSpec::Iterate()) {
		if (cs->bitnum == bitnum) return cs->Index();
	}

	/* No matching label was found, so it is invalid */
	return CT_INVALID;
}

/**
 * Get sprite for showing cargo of this type.
 * @return Sprite number to use.
 */
SpriteID CargoSpec::GetCargoIcon() const
{
	SpriteID sprite = this->sprite;
	if (sprite == 0xFFFF) {
		/* A value of 0xFFFF indicates we should draw a custom icon */
		sprite = GetCustomCargoSprite(this);
	}

	if (sprite == 0) sprite = SPR_CARGO_GOODS;

	return sprite;
}

std::vector<const CargoSpec *> _sorted_cargo_specs;   ///< Cargo specifications sorted alphabetically by name.
span<const CargoSpec *> _sorted_standard_cargo_specs; ///< Standard cargo specifications sorted alphabetically by name.

/** Sort cargo specifications by their name. */
static bool CargoSpecNameSorter(const CargoSpec * const &a, const CargoSpec * const &b)
{
	std::string a_name = GetString(a->name);
	std::string b_name = GetString(b->name);

	int res = StrNaturalCompare(a_name, b_name); // Sort by name (natural sorting).

	/* If the names are equal, sort by cargo bitnum. */
	return (res != 0) ? res < 0 : (a->bitnum < b->bitnum);
}

/** Sort cargo specifications by their cargo class. */
static bool CargoSpecClassSorter(const CargoSpec * const &a, const CargoSpec * const &b)
{
	int res = (b->classes & CC_PASSENGERS) - (a->classes & CC_PASSENGERS);
	if (res == 0) {
		res = (b->classes & CC_MAIL) - (a->classes & CC_MAIL);
		if (res == 0) {
			res = (a->classes & CC_SPECIAL) - (b->classes & CC_SPECIAL);
			if (res == 0) {
				return CargoSpecNameSorter(a, b);
			}
		}
	}

	return res < 0;
}

/** Initialize the list of sorted cargo specifications. */
void InitializeSortedCargoSpecs()
{
	_sorted_cargo_specs.clear();
	/* Add each cargo spec to the list. */
	for (const CargoSpec *cargo : CargoSpec::Iterate()) {
		_sorted_cargo_specs.push_back(cargo);
	}

	/* Sort cargo specifications by cargo class and name. */
	std::sort(_sorted_cargo_specs.begin(), _sorted_cargo_specs.end(), &CargoSpecClassSorter);

	/* Count the number of standard cargos and fill the mask. */
	_standard_cargo_mask = 0;
	uint8_t nb_standard_cargo = 0;
	for (const auto &cargo : _sorted_cargo_specs) {
		if (cargo->classes & CC_SPECIAL) break;
		nb_standard_cargo++;
		SetBit(_standard_cargo_mask, cargo->Index());
	}

	/* _sorted_standard_cargo_specs is a subset of _sorted_cargo_specs. */
	_sorted_standard_cargo_specs = { _sorted_cargo_specs.data(), nb_standard_cargo };
}

uint64_t CargoSpec::WeightOfNUnitsInTrain(uint32_t n) const
{
	if (this->is_freight) n *= _settings_game.vehicle.freight_trains;
	return this->WeightOfNUnits(n);
}
