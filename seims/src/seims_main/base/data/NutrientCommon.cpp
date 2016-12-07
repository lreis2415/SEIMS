#include "NutrientCommon.h"
#include "util.h"

NutrCommon::NutrCommon()
{
}

NutrCommon::~NutrCommon()
{
}

float NutrCommon::CalEnrichmentRatio(float sedyld, float surfq, float area)
{
	/// if no surface runoff, or very little sediment,
	/// then no nutrient will be attached to sediment, and loss with surface runoff	
	if (surfq < UTIL_ZERO || sedyld < 0.1f)
		return 0.f;

	// CREAMS method for calculating enrichment ratio
	float cy = 0.f, enratio = 0.f;
	// Calculate sediment concentration, equation 4:2.2.3 and 4:2.2.4 in SWAT Theory 2009, p272
	cy = sedyld / 1000.f / (10.f * area * surfq + 1.e-6f) ; /// Mg sed/m^3 H2O
	if (cy > 1.e-6f){
		enratio = 0.78f * pow(cy, -0.2468f);
	} else{
		enratio = 0.f;
	}
	if (enratio > 3.5f) enratio = 3.5f;
	return enratio;
}