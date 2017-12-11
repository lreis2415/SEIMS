/*!
 * \brief Define some common used function in Nutrient cycling modules, e.g., NUTRMV, NUTRSED
 * \author Liang-Jun Zhu
 * \date 2016-9-28
 */
#ifndef SEIMS_NUTRIENT_COMMON_H
#define SEIMS_NUTRIENT_COMMON_H

#include "utilities.h"

/*!
 * \class NutrCommon
 * \ingroup data
 *
 * \brief A basic class for nutrient cycling modules
 *
 */
class NutrCommon {
public:
    /// Constructor
    NutrCommon() = default;

    /// Destructor
    ~NutrCommon() = default;

public:
    /*!
     * \brief Calculate enrichment ratio for nutrient transport with runoff and sediment
	 *           enrsb.f of SWAT
	 * \param sedyld sediment yield, kg
	 * \param surfq surface runoff, mm
	 * \param area area, ha
	 */
    static float CalEnrichmentRatio(float sedyld, float surfq, float area);
};

#endif /* SEIMS_NUTRIENT_COMMON_H */