#pragma once
#include <string>

struct Material
{
	std::string id;
	std::string desc;
	std::string formula;

	// The average density of the material, in kg / m3
	// "density"
	float density;

	// Melting point in K (Kelvin)
	// "melting / melting point"
	float melting;

	// Boiling point in K (Kelvin)
	// "boiling / boiling point"
	float boiling;

	// INTENSIVE specific heat in J / kgK
	// "heat / specific heat"
	float specific_heat;

	// Molar mass in u (C-14 masses)
	// (Calculated from formula)
	float molar_mass;

	// Cost in $/kg
	float cost;
};

