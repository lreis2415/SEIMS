/*!
* \file gdal_handler.h
 * \brief Simple wrappers of GDAL API that compatible with all versions
 *
 * \remarks
 *   - 1. 2025-09-07 - lj - Initial implementation and debugged on multiple compilers.
 *
 * \note No exceptions will be thrown.
 * \author Liangjun Zhu, zlj(at)lreis.ac.cn
 * \version 0.2
 */
#ifndef CCGL_DB_GDAL_H_
#define CCGL_DB_GDAL_H_
#ifdef USE_GDAL

#include <memory>

#include "gdal.h"
#include "gdal_priv.h"
#include <ogrsf_frmts.h>
#include "cpl_string.h"
#include "ogr_spatialref.h"

//==================== Common: define unified types ====================//
// Vector dataset: OGRDataSource in GDAL 1.x, GDALDataset in 2.x+
#if GDAL_VERSION_MAJOR >= 2
typedef GDALDataset GDALVectorDS;
#else
typedef OGRDataSource GDALVectorDS;
#endif

// Raster dataset: GDALDataset in all versions
typedef GDALDataset GDALRasterDS;

//==================== Vector: Open / Close ====================//

inline GDALVectorDS* OpenVector(const char* path, int read_only=true) {
#if GDAL_VERSION_MAJOR >= 2
    GDALAllRegister();
    int flags = GDAL_OF_VECTOR | (read_only ? GDAL_OF_READONLY : GDAL_OF_UPDATE);
    return static_cast<GDALVectorDS*>(GDALOpenEx(path, flags, nullptr, nullptr, nullptr));
#else
    OGRRegisterAll();
    return OGRSFDriverRegistrar::Open(path, read_only ? FALSE : TRUE);
#endif
}

inline void CloseVector(GDALVectorDS* ds) {
    if (!ds) return;
#if GDAL_VERSION_MAJOR >= 2
    GDALClose(ds);
#else
    OGRDataSource::DestroyDataSource(ds);
#endif
}

struct GDALVectorDSDeleter {
    void operator()(GDALVectorDS* ds) const { CloseVector(ds); }
};
typedef std::unique_ptr<GDALVectorDS, GDALVectorDSDeleter> GDALVectorDSHandle;

//==================== Raster: Open / Close / Create ====================//

inline GDALRasterDS* OpenRaster(const char* path, int read_only=true,
                                const char* const* open_options=nullptr) {
#if GDAL_VERSION_MAJOR >= 2
    GDALAllRegister();
    int flags = GDAL_OF_RASTER | (read_only ? GDAL_OF_READONLY : GDAL_OF_UPDATE);
    return static_cast<GDALRasterDS*>(GDALOpenEx(path, flags, nullptr, open_options, nullptr));
#else
    (void)open_options; // avoid unused warnings
    GDALAllRegister();
    return static_cast<GDALRasterDS*>(GDALOpen(path, read_only ? GA_ReadOnly : GA_Update));
#endif
}

inline GDALRasterDS* CreateRaster(const char* driverName, const char* filename,
                                  int xsize, int ysize,
                                  int bands, GDALDataType dtype,
                                  char** papszOptions=nullptr) {
    if (!driverName || !driverName[0] || !filename) return nullptr;
    GDALAllRegister();
    GDALDriver* drv = GetGDALDriverManager()->GetDriverByName(driverName);
    if (!drv) return nullptr;
    return drv->Create(filename, xsize, ysize, bands, dtype, papszOptions);
}

inline void CloseRaster(GDALRasterDS* ds) {
    if (!ds) return;
    GDALClose(ds);
}

struct GDALRasterDSDeleter {
    void operator()(GDALRasterDS* ds) const { CloseRaster(ds); }
};
typedef std::unique_ptr<GDALRasterDS, GDALRasterDSDeleter> GDALRasterDSHandle;
#endif // USE_GDAL
#endif // CCGL_DB_GDAL_H_
