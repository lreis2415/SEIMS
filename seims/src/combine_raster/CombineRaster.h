/*!
 * \brief Combine raster data of each subbasins.
 *        Support ASC, TIFF, and MongoDB GridFs formats.
 * \author Liangjun Zhu, Junzhi Liu
 * \changelog 2018-05-05 - lj - Refactor as stand-alone program.\n
 */
#ifndef COMBINE_RASTER_H
#define COMBINE_RASTER_H

#ifdef USE_MONGODB
#include "db_mongoc.h"
#endif /* USE_MONGODB */
#include "data_raster.hpp"

using namespace ccgl;
#ifdef USE_MONGODB
using namespace db_mongoc;
#endif /* USE_MONGODB */
using namespace data_raster;

#ifndef FloatRaster
#define FloatRaster     clsRasterData<float>
#endif
#ifndef IntRaster
#define IntRaster       clsRasterData<int>
#endif

/*!
 * \brief Combine rasters as one, for both 1D and 2D raster data, clsRasterData
 * \param[in] all_raster_data Key is subbasinID (start from 1), value is clsRasterData<float>
 * \return Combined raster data
 */
FloatRaster* CombineRasters(map<int, FloatRaster *>& all_raster_data);

/*!
 * \brief Combined rasters of each subbasin and output as file
 * \param[in] folder Directory that contains the raster files
 * \param[in] s_var Core file name, e.g., lai
 * \param[in] file_type File type of existed raster files, e.g., asc, tif (need GDAL support)
 * \param[in] n_subbasins Subbasin count, e.g., 5 means 1_lai.tif, ..., 5_lai.tif will be combined as lai.tif
 */
void CombineRasterResults(const string& folder, const string& s_var,
                          const string& file_type, int n_subbasins);

#ifdef USE_MONGODB
/*!
 * \brief Combine rasters of each subbasin store as GridFs in MongoDB and output to MongoDB as GridFs
 *        And, if output as file if `folder` is specified.
 *
 *        Currently, I cannot find a way to store GridFS files with the same filename but with
 *        different metadata information by mongo-c-driver, which can be done by pymongo.
 *        So, temporarily, I decided to append scenario ID and calibration ID to the filename.
 *
 *        The format of filename of OUPUT by SEIMS MPI version is:
 *
 *        <SubbasinID>_CoreFileName_ScenarioID_CalibrationID
 *
 *        If no ScenarioID or CalibrationID, i.e., with a value of -1, just left blank. e.g.,
 *        - 1_SED_OL_SUM_1_ means ScenarioID is 1 and Calibration ID is -1
 *        - 1_SED_OL_SUM__ means ScenarioID is -1 and Calibration ID is -1
 *        - 1_SED_OL_SUM_0_2 means ScenarioID is 0 and Calibration ID is 2
 *
 * \param[in] gfs MongoGridFs
 * \param[in] s_var Core file name, e.g., lai
 * \param[in] n_subbasins Subbasin count, e.g., 5 means 1_lai, ..., 5_lai will be combined as lai
 * \param[in] folder Optional. If specified, the combined raster will be outputed as file simultaneously.
 * \param[in] scenario_id Scenario ID stored in metadata, 0 by default
 * \param[in] calibration_id Calibration ID stored in metadata, -1 by default
 * \return True if succeed. Do not exit directly or throw exceptions! -LJ.
 */
bool CombineRasterResultsMongo(MongoGridFs* gfs, const string& s_var,
                               int n_subbasins, const string& folder = "",
                               int scenario_id = 0, int calibration_id = -1);
#endif /* USE_MONGODB */

#endif /* COMBINE_RASTER_H */
