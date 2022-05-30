#ifndef CCGL_APP_MASK_RASTERIO_H
#define CCGL_APP_MASK_RASTERIO_H

#include "data_raster.hpp"

using namespace ccgl;
using namespace data_raster;


enum IOMODE {
    MASK,  ///< mask by single area
    DEC,   ///< decompose input raster by the mask layer
    COM,   ///< combine input raster by the mask layer
    UNKNOWNMODE
};

IOMODE StringToIOMode(const string& str);

enum DATAFMT {
    SFILE, ///< Single file format, e.g., .asc and .tif
    GFS,   ///< MongoDB GridFS file
    UNKNOWNFMT
};

DATAFMT StringToDataFormat(const string& str);

/// Full usage information
void Usage(const string& appname, const string& error_msg = std::string());


/*!
 * \brief Parse input, output, and mask data formatand path
 *
 * \example
 *
 *     1. tag: IN, strs: {"/path/to/single_file.tif"}
 *     2. tag: IN, strs: {"single_gfsname"}
 *     3. tag: IN, strs: {"file", "/path/to/single_file.tif"}
 *     4. tag: IN, strs: {"gfs", "single_gfsname"}
 *     5. tag: IN, strs: {"/path/to/file.tif;/path/to/file2.tif"}
 *     6. tag: IN, strs: {"file", "/path/to/file.tif;/path/to/file2.tif"}
 *     7. tag: IN, strs: {"file"} or {"gfs"}
 * 
 */ 
bool parse_fmt_paths(string& tag, vector<string>& strs, DATAFMT& fmt, vector<string>& paths);


#endif /* CCGL_APP_MASK_RASTERIO_H */
