/*!
 * \file seims.h
 * \brief The SEIMS related definitions and utilities header.
 *
 * Changelog:
 *   - 1. 2017-03-22 - lj - Initial implementation.
 *   - 2. 2021-04-06 - lj - Add Flow direction method enum.
 *
 * \author Liang-Jun Zhu
 * \date 2017-3-22
 */
#ifndef SEIMS_HEADER
#define SEIMS_HEADER

#include "data_raster.hpp"

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
const char* const LayeringMethodString[] = {"_UP_DOWN", "_DOWN_UP"};

/*!
 * \enum FlowDirMethod
 * \ingroup base
 * \brief Flow direction method for flow routing.
 */
enum FlowDirMethod {
    D8,     ///< D8 (O'Callaghan and Mark, 1984), default
    Dinf,   ///< Dinf (Tarboton, 1997)
    MFDmd   ///< Multiple Flow Direction based on maximum downslope gradient (Qin et al., 2007)
};
const char* const FlowDirMethodString[] = { "_D8", "_DINF", "_MFDMD" };

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
 *      //      the flow direction of ArcGIS rule:
 *      //   32  64 128
 *      //   16     1
 *      //    8  4  2
 * \endcode
 */
const int DiagonalCCW[9] = {0, 0, 1, 0, 1, 0, 1, 0, 1};
/*!
 * \def FlowDirCCW
 * \ingroup base
 * \brief Flow directions of ArcGIS rule
 *
 *        32  64 128
 *        16     1
 *         8  4  2
 */
const int FlowDirCCW[9] = { 0, 1, 128, 64, 32, 16, 8, 4, 2 };
const int CCWDeltaRow[9] = { 0, 0, -1, -1, -1, 0, 1, 1, 1 }; ///< Delta Row (Y-axis) according to FlowDirCCW
const int CCWDeltaCol[9] = { 0, 1, 1, 0, -1, -1, -1, 0, 1 }; ///< Delta Col (X-axis) according to FlowDirCCW

#ifdef USE_FLOAT32
typedef float FLTPT;
#else
typedef double FLTPT;
#endif

///
/// Common used const.
///
const FLTPT _pi = 3.14159265358979323846; ///< PI
const FLTPT _1div3 = 0.3333333333333333; ///< 1. / 3.
const FLTPT _2div3 = 0.6666666666666666; ///< 2. / 3.
const FLTPT _8div3 = 2.6666666666666665; ///< 8. / 3.
const FLTPT SQ2 = 1.4142135623730951; ///< sqrt(2.0)
const FLTPT deg2rad = 0.017453292519943295; ///< PI / 180.
const FLTPT rad2deg = 57.29577951308232; ///< 180. / PI

const FLTPT MIN_FLUX = 1e-12; ///< \def minimum flux (m3/s) in kinematic wave
const int MAX_ITERS_KW = 10; ///< \def maximum iterate number in kinematic wave method
const FLTPT MIN_SLOPE = 1e-4;  ///< \def minimum slope (tan value)

#ifndef IntRaster
/*! Integer-typed raster */
#define IntRaster   ccgl::data_raster::clsRasterData<int>
#endif
#ifndef FloatRaster
/*! Float-typed raster */
#define FloatRaster ccgl::data_raster::clsRasterData<FLTPT>
#endif
#ifndef IntMaskedRaster
/*! Int-typed raster with int-typed mask */
#define IntMaskedRaster ccgl::data_raster::clsRasterData<int, int>
#endif
#ifndef FloatMaskedRaster
/*! Float-typed raster with int-typed mask */
#define FloatMaskedRaster ccgl::data_raster::clsRasterData<FLTPT, int>
#endif

#endif /* SEIMS_HEADER */
