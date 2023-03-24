#include "NutrientCommon.h"

#include <cmath>

#include "basic.h"

FLTPT CalEnrichmentRatio(const FLTPT sedyld, const FLTPT surfq, const FLTPT area) {
    /// if no surface runoff, or very little sediment,
    /// then no nutrient will be attached to sediment, and loss with surface runoff
    if (surfq < UTIL_ZERO || sedyld < 0.1) { return 0.; }

    // CREAMS method for calculating enrichment ratio
    FLTPT cy = 0.;
    FLTPT enratio = 0.;
    // Calculate sediment concentration, equation 4:2.2.3 and 4:2.2.4 in SWAT Theory 2009, p272
    cy = sedyld / 1000. / (10. * area * surfq + 1.e-6); /// Mg sed/m^3 H2O
    if (cy > 1.e-6) {
        enratio = 0.78 * CalPow(cy, -0.2468f);
    } else {
        enratio = 0.;
    }
    if (enratio > 3.5) { enratio = 3.5; }
    return enratio;
}
