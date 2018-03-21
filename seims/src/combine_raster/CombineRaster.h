#ifndef COMBINE_RASTER_H
#define COMBINE_RASTER_H
#ifndef USE_MONGODB
#define USE_MONGODB
#endif

#include "utilities.h"
#include "clsRasterData.h"

#include <sstream>

using namespace std;

#ifndef FloatRaster
#define FloatRaster     clsRasterData<float>
#endif
#ifndef IntRaster
#define IntRaster       clsRasterData<int>
#endif

/*!
 * \brief Combine rasters as one, for both 1D and 2D raster data, \sa clsRasterData
 * \param[in] allRasterData Key is subbasinID (start from 1), value is clsRasterData<float>
 * \return Combined raster data
 */
FloatRaster *CombineRasters(map<int, FloatRaster *> &allRasterData);

void CombineRasterResults(const string &folder, const string &sVar, const string &fileType, int nSubbasins);

void CombineRasterResultsMongo(MongoGridFS *gfs, const string &sVar, int nSubbasins, const string &folder = "");

#endif /* COMBINE_RASTER_H */
