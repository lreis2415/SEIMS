/*!
 * \file seims.h
 * \brief The SEIMS related definitions and utilities header.
 * \author Liang-Jun Zhu
 * \date 2017-3-22
 */
#ifndef SEIMS_HEADER
#define SEIMS_HEADER

#include "data_raster.h"

/*!
 * \enum LayeringMethod
 * \ingroup base
 * \brief Grid layering method for routing and parallel computing.
 *        Reference: Liu et al., 2014, EM&S, 51, 221-227. https://doi.org/10.1016/j.envsoft.2013.10.005
 */
enum LayeringMethod {
    UP_DOWN, ///< layering-from-source method, default
    DOWN_UP  ///< layering-from-outlet method
};

/*!
 * \enum GroupMethod
 * \ingroup base
 * \brief Group method for parallel task scheduling.
 */
enum GroupMethod {
    KMETIS = 0, ///< KMETIS, default
    PMETIS = 1  ///< PMETIS
};
const char* const GroupMethodString[] = {"KMETIS", "PMETIS"};

/*!
 * \enum ScheduleMethod
 * \ingroup base
 * \brief Parallel task scheduling strategy at subbasin level by MPI. TESTED!
 */
enum ScheduleMethod {
    SPATIAL = 0,          ///< Sceduled by spatial, default, refers to Liu et al., 2016, EM&S
    TEMPOROSPATIAL = 1    ///< Sceduled by temporal-spatial discretization method, refers to Wang et al., 2013, C&G
};
const char* const ScheduleMethodString[] = {"SPATIAL", "TEMPOROSPATIAL"};

/*!
 * \def DiagonalCCW
 * \ingroup base
 * \brief Whether diagonal counter clockwise from east
 *
 * \code
 *      // the first element is set to 0, for indexing convenient.
 *      //    1  0  1
 *      //    0     0
 *      //    1  0  1
 *      // e.g. the corresponding D8 flow direction of TauDEM rule:
 *      //    4  3  2
 *      //    5     1
 *      //    6  7  8
 * \endcode
 */
const int DiagonalCCW[9] = {0, 0, 1, 0, 1, 0, 1, 0, 1};

///
/// Common used const.
///

const float _1div3 = 0.3333333333333333f; ///< 1. / 3.
const float _2div3 = 0.6666666666666666f; ///< 2. / 3.
const float _8div3 = 2.6666666666666665f; ///< 8. / 3.
const float SQ2 = 1.4142135623730951f; ///< sqrt(2.0)
const float deg2rad = 0.017453292519943295f; ///< PI / 180.
const float rad2deg = 57.29577951308232f; ///< 180. / PI

#define MIN_FLUX        1e-12f ///< \def minimum flux (m3/s) in kinematic wave
#define MAX_ITERS_KW    10     ///< \def maximum iterate number in kinematic wave method
#define MIN_SLOPE       1e-4f  ///< \def minimum slope (tan value)

/*! Raster with integer datatype */
#define IntRaster       ccgl::data_raster::clsRasterData<int>
/*! Raster with float datatype */
#define FloatRaster     ccgl::data_raster::clsRasterData<float>

#endif /* SEIMS_HEADER */
