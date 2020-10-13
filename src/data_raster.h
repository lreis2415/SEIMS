/*!
 * \file data_raster.h
 * \brief Define Raster class to handle raster data
 *        Part of the Common Cross-platform Geographic Library (CCGL)
 *
 * Features:
 *   - 1. Using GDAL and MongoDB (currently, mongo-c-driver 1.5.0+ is supported)
 *   - 2. Array1D and Array2D raster data are supported
 *   - 3. C++11 supported
 *   - 4. Unit Tests based Google Test.
 *
 * Changelog:
 *   - 1. Dec. 2016 lj Separated from SEIMS to a common library for widely use.
 *   - 2. Mar. 2017 lj VLD check, bug fixed, function enhanced.
 *   - 3. Apr. 2017 lj Avoid try...catch block
 *   - 4. May. 2017 lj Use MongoDB wrapper
 *   - 5. Nov. 2017 lj Code review based on C++11 and use a single header file.
 *   - 6. Dec. 2017 lj Add Unittest based on Google Test.
 *   - 7. Mar. 2018 lj Make part of CCGL, and make GDAL optional. Follows Google C++ Code Style.
 *   - 8. Jun. 2018 lj Use emplace and emplace_back rather than insert and push_back whenever possible.
 *   - 9. Nov. 2018 lj Add specific field-value as options of raster data, including SRS.
 *
 * \author Liangjun Zhu
 * \email zlj@lreis.ac.cn
 * \version 2.3
 */
#ifndef CCGL_DATA_RASTER_H
#define CCGL_DATA_RASTER_H

// include base headers
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
#include <typeinfo>
#include <cassert>
// include openmp if supported
#ifdef SUPPORT_OMP
#include <omp.h>
#endif /* SUPPORT_OMP */

// include GDAL, optional
#ifdef USE_GDAL
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"
#endif /* USE_GDAL */

#include "basic.h"
#include "utils_filesystem.h"
#include "utils_string.h"
#include "utils_array.h"
#include "utils_math.h"
// include MongoDB, optional
#ifdef USE_MONGODB
#include "db_mongoc.h"
#endif /* USE_MONGODB */

using std::map;
#ifndef HAS_VARIADIC_TEMPLATES // Not support emplace(), e.g., MSVC2010
using std::make_pair;
#endif
using std::setprecision;

namespace ccgl {
using namespace utils_string;
using namespace utils_filesystem;
using namespace utils_array;
using namespace utils_math;
#ifdef USE_MONGODB
using namespace db_mongoc;
#endif /* USE_MONGODB */

/*!
 * \namespace ccgl::data_raster
 * \brief Raster class to handle various raster data
 */
namespace data_raster {
/*! NoData value */
#define HEADER_RS_NODATA        "NODATA_VALUE"
/*! X coordinate value of left low center */
#define HEADER_RS_XLL           "XLLCENTER"  // or XLLCORNER
/*! Y coordinate value of left low center */
#define HEADER_RS_YLL           "YLLCENTER"  // or YLLCORNER
/*! Rows number */
#define HEADER_RS_NROWS         "NROWS"
/*! Column number */
#define HEADER_RS_NCOLS         "NCOLS"
/*! Cell size (length) */
#define HEADER_RS_CELLSIZE      "CELLSIZE"
/*! Layers number */
#define HEADER_RS_LAYERS        "LAYERS"
/*! Cell number */
#define HEADER_RS_CELLSNUM      "CELLSNUM"
/*! SRS */
#define HEADER_RS_SRS           "SRS"

/*! Valid cell number */
#define STATS_RS_VALIDNUM        "VALID_CELLNUMBER"
/*! Mean value */
#define STATS_RS_MEAN            "MEAN"
/*! Minimum value */
#define STATS_RS_MIN             "MIN"
/*! Maximum value */
#define STATS_RS_MAX             "MAX"
/*! Standard derivation value */
#define STATS_RS_STD             "STD"
/*! Range value */
#define STATS_RS_RANGE           "RANGE"

/*! ASCII extension */
#define ASCIIExtension          "asc"
/*! GeoTIFF extension */
#define GTiffExtension          "tif"

/*! Row and Col pair */
typedef std::pair<int, int> ROW_COL;
/*! Coordinate pair */
typedef std::pair<double, double> XY_COOR;

/** Common functions independent to clsRasterData **/
/// Print status while not running UnitTest
inline void StatusNoUnitTest(const string& status_str) {
#ifndef UNITTEST
    cout << status_str << endl;
#endif
}

/*!
 * \brief check the existence of given raster file
 */
inline bool CheckRasterFilesExist(const string& filename) {
    if (!FileExists(filename)) {
        StatusNoUnitTest("The raster file " + filename + " does not exist or has not read permission.");
        return false;
    }
    return true;
}

/*!
 * \brief check the existence of given vector of raster files
 * \return True if all existed, else false
 */
inline bool CheckRasterFilesExist(vector<string>& filenames) {
    for (auto it = filenames.begin(); it != filenames.end(); ++it) {
        if (!CheckRasterFilesExist(*it)) {
            return false;
        }
    }
    return true;
}

/*!
 * \brief Copy 2D Array raster data to 1D Array
 */
template <typename T1, typename T2>
void Copy2DRasterDataTo1DArray(T1* dst, T2* src, const int nr, const int nc) {
#pragma omp parallel for
    for (int i = 0; i < nr; i++) {
        for (int j = 0; j < nc; j++) {
            int index = i * nc + j;
            dst[index] = static_cast<T1>(src[index]);
        }
    }
}

/*!
 * \class clsRasterData
 * \brief Raster data (1D and 2D) I/O class
 *        Support I/O among ASCII file, TIFF, and MongoBD database.
 */
template <typename T, typename MASK_T = T>
class clsRasterData {
public:
    /************* Construct functions ***************/
    /*!
     * \brief Constructor an empty clsRasterData instance
     * By default, 1D raster data
     * Set \a m_rasterPositionData, \a raster_data_, \a m_mask to \a nullptr
     * \todo Add some Set functions to make this empty constructor usable.
     */
    clsRasterData();

    /*!
     * \brief Construtor 1D raster from necessary data.
     */
    clsRasterData(T* data, int cols, int rows, T nodata, double dx, double xll,
                  double yll, STRING_MAP opts = STRING_MAP());

    /*!
     * \brief Construtor 1D raster from necessary data.
     * \deprecated The input parameter `srs` is highly recommended replaced by `map<string, string>`.
     */
    clsRasterData(T* data, int cols, int rows, T nodata, double dx, double xll,
                  double yll, const string& srs);

    /*!
    * \brief Construtor 1D raster from necessary data
    */
    clsRasterData(T** data2d, int cols, int rows, int nlayers, T nodata,
                  double dx, double xll, double yll, STRING_MAP opts = STRING_MAP());

    /*!
     * \brief Construtor 1D raster from necessary data
     * \deprecated The input parameter `srs` is highly recommended replaced by `map<string, string>`.
     */
    clsRasterData(T** data2d, int cols, int rows, int nlayers, T nodata,
                  double dx, double xll, double yll, const string& srs);

    /*!
     * \brief Constructor of clsRasterData instance from TIFF, ASCII, or other GDAL supported format
     * \param[in] filename Full path of the raster file
     * \param[in] calc_pos Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<MASK_T> Mask layer
     * \param[in] use_mask_ext Use mask layer extent, even NoDATA exists.
     * \param[in] default_value Default value when mask data exceeds the raster extend.
     * \param[in] opts (Optional) Additional options of the raster data
     */
    explicit clsRasterData(const string& filename,
                           bool calc_pos = true,
                           clsRasterData<MASK_T>* mask = nullptr,
                           bool use_mask_ext = true,
                           T default_value = static_cast<T>(NODATA_VALUE),
                           STRING_MAP opts = STRING_MAP());

    /*!
     * \brief Validation check before the constructor of clsRasterData,
     *        i.e., the raster file is existed and supported.
     *        This is the recommended way to construct an instance of clsRasterData.
     *
     * \code
     *       clsRasterData<T, MASK_T> *rs = clsRasterData<T, MASK_T>::Init(filename)
     *       if (nullptr == rs) {
     *           throw exception("clsRasterData initialization failed!");
     *           // or other error handling code.
     *       }
     * \endcode
     */
    static clsRasterData<T, MASK_T>* Init(const string& filename,
                                          bool calc_pos = true,
                                          clsRasterData<MASK_T>* mask = nullptr,
                                          bool use_mask_ext = true,
                                          T default_value = static_cast<T>(NODATA_VALUE),
                                          STRING_MAP opts = STRING_MAP());

    /*!
     * \brief Constructor of clsRasterData instance from TIFF, ASCII, or other GDAL supported raster file
     * Support 1D and/or 2D raster data
     *
     * \sa ReadASCFile
     * \sa ReadByGDAL
     * \param[in] filenames Full paths vector of the 2D raster data
     * \param[in] calc_pos Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<MASK_T> Mask layer
     * \param[in] use_mask_ext Use mask layer extent, even NoDATA exists.
     * \param[in] default_value Default value when mask data exceeds the raster extend.
     * \param[in] opts (Optional) Additional options of the raster data
     */
    explicit clsRasterData(vector<string>& filenames,
                           bool calc_pos = true,
                           clsRasterData<MASK_T>* mask = nullptr,
                           bool use_mask_ext = true,
                           T default_value = static_cast<T>(NODATA_VALUE),
                           STRING_MAP opts = STRING_MAP());

    /*!
     * \brief Validation check before the constructor of clsRasterData,
     *        i.e., the raster files are all existed and supported.
     */
    static clsRasterData<T, MASK_T>* Init(vector<string>& filenames,
                                          bool calc_pos = true,
                                          clsRasterData<MASK_T>* mask = nullptr,
                                          bool use_mask_ext = true,
                                          T default_value = static_cast<T>(NODATA_VALUE),
                                          STRING_MAP opts = STRING_MAP());

    /*!
     * \brief Construct an clsRasterData instance by 1D array data and mask
     */
    clsRasterData(clsRasterData<MASK_T>* mask, T* const values, STRING_MAP opts = STRING_MAP());

    /*!
     * \brief Construct an clsRasterData instance by 2D array data and mask
     */
    clsRasterData(clsRasterData<MASK_T>* mask, T** const values, int lyrs,
                  STRING_MAP opts = STRING_MAP());

#ifdef USE_MONGODB

    /*!
     * \brief Constructor based on mongoDB
     * \sa ReadFromMongoDB()
     *
     * \param[in] gfs \a mongoc_gridfs_t
     * \param[in] remote_filename \a char*
     * \param[in] calc_pos Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<MASK_T> Mask layer
     * \param[in] use_mask_ext Use mask layer extent, even NoDATA exists.
     * \param[in] default_value Default value when mask data exceeds the raster extend.
     * \param[in] opts Optional key-value stored in metadata
     */
    clsRasterData(MongoGridFs* gfs, const char* remote_filename, bool calc_pos = true,
                  clsRasterData<MASK_T>* mask = nullptr, bool use_mask_ext = true,
                  T default_value = static_cast<T>(NODATA_VALUE),
                  STRING_MAP opts = STRING_MAP());

    /*!
     * \brief Validation check before the constructor of clsRasterData.
     */
    static clsRasterData<T, MASK_T>* Init(MongoGridFs* gfs, const char* remote_filename,
                                          bool calc_pos = true,
                                          clsRasterData<MASK_T>* mask = nullptr,
                                          bool use_mask_ext = true,
                                          T default_value = static_cast<T>(NODATA_VALUE),
                                          STRING_MAP opts = STRING_MAP());
#endif

    /*!
     * \brief Copy constructor
     *
     * Example:
     * \code
     *   // Method 1:
     *   clsRasterData<T> newraster(baseraster);
     *   // Method 2:
     *   clsRasterData<T> newraster;
     *   newraster.Copy(baseraster);
     *   // Method 3:
     *   clsRasterData<T>* newraster = new clsRasterData<T>(baseraster);
     *   delete newraster;
     * \endcode
     */
    explicit clsRasterData(clsRasterData<T, MASK_T>* another);

    //! Destructor
    ~clsRasterData();

    /************* Read functions ***************/
    /*!
     * \brief Read raster data from file, mask data is optional
     * \param[in] filename \a string
     * \param[in] calc_pos Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<MASK_T>
     * \param[in] use_mask_ext Use mask layer extent, even NoDATA exists.
     * \param[in] default_value Default value when mask data exceeds the raster extend.
     * \param[in] opts Optional key-value stored in metadata
     */
    bool ReadFromFile(const string& filename, bool calc_pos = true, clsRasterData<MASK_T>* mask = nullptr,
                      bool use_mask_ext = true, T default_value = static_cast<T>(NODATA_VALUE),
                      STRING_MAP opts = STRING_MAP());

#ifdef USE_MONGODB

    /*!
     * \brief Read raster data from MongoDB
     * \param[in] gfs \a mongoc_gridfs_t
     * \param[in] filename \a char*, raster file name
     * \param[in] calc_pos Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<MASK_T>
     * \param[in] use_mask_ext Use mask layer extent, even NoDATA exists.
     * \param[in] default_value Default value when mask data exceeds the raster extend.
     * \param[in] opts Optional key-value stored in metadata
     */
    bool ReadFromMongoDB(MongoGridFs* gfs, const string& filename, bool calc_pos = true,
                         clsRasterData<MASK_T>* mask = nullptr, bool use_mask_ext = true,
                         T default_value = static_cast<T>(NODATA_VALUE),
                         STRING_MAP opts = STRING_MAP());

#endif /* USE_MONGODB */
    /************* Write functions ***************/

    /*!
     * \brief Write raster to raster file, if 2D raster, output name will be filename_LyrNum
     * \param filename filename with prefix, e.g. ".asc" and ".tif"
     */
    bool OutputToFile(const string& filename);

    /*!
     * \brief Write 1D or 2D raster data into ASC file(s)
     * \param[in] filename \a string, output ASC file path, take the CoreName as prefix
     */
    bool OutputAscFile(const string& filename);

#ifdef USE_GDAL
    /*!
     * \brief Write 1D or 2D raster data into TIFF file by GDAL
     * \param[in] filename \a string, output TIFF file path
     */
    bool OutputFileByGdal(const string& filename);
#endif /* USE_GDAL */

#ifdef USE_MONGODB

    /*!
     * \brief Write raster data (matrix raster data) into MongoDB
     * \param[in] filename \a string, output file name
     * \param[in] gfs \a mongoc_gridfs_t
     * \param[in] opts (optional) Key-value map for user-specific metadata
     */
    bool OutputToMongoDB(const string& filename, MongoGridFs* gfs,
                         STRING_MAP opts = STRING_MAP());

#endif /* USE_MONGODB */

    /************************************************************************/
    /*    Set information functions                                         */
    /************************************************************************/

    //! Set new core file name
    void SetCoreName(const string& name) { core_name_ = name; }

    /*!
     * \brief Set value to the given position and layer
     */
    void SetValue(int row, int col, T value, int lyr = 1);

    /************************************************************************/
    /*    Get information functions                                         */
    /************************************************************************/

    /*!
     * \brief Calculate basic statistics values in one time
     * Mean, Max, Min, STD, Range, etc.
     */
    void CalculateStatistics();

    /*!
     * \brief Force to update basic statistics values
     * Mean, Max, Min, STD, Range, etc.
     */
    void UpdateStatistics();

    /*!
     * \brief Release statistics map of 2D raster data
     */
    void ReleaseStatsMap2D();

    /*!
     * \brief Get basic statistics value
     * Mean, Max, Min, STD, Range, etc.
     * \param[in] sindex \a string case insensitive
     * \param[in] lyr optional for 1D and the first layer of 2D raster data.
     * \return Statistics value or NODATA
     */
    double GetStatistics(string sindex, int lyr = 1);

    /*!
     * \brief Get basic statistics values for 2D raster data.
     * Mean, Max, Min, STD, Range, etc.
     * \param[in] sindex \a string case insensitive
     * \param[out] lyrnum \a int layer number
     * \param[out] values \a double* Statistics array or nullptr
     */
    void GetStatistics(string sindex, int* lyrnum, double** values);

    /*!
     * \brief Get the average of raster data
     * \param[in] lyr optional for 1D and the first layer of 2D raster data.
     */
    double GetAverage(const int lyr = 1) { return GetStatistics(STATS_RS_MEAN, lyr); }

    /*!
     * \brief Get the maximum of raster data
     * \sa GetAverage
     */
    double GetMaximum(const int lyr = 1) { return GetStatistics(STATS_RS_MAX, lyr); }

    /*!
     * \brief Get the minimum of raster data
     * \sa GetAverage
     */
    double GetMinimum(const int lyr = 1) { return GetStatistics(STATS_RS_MIN, lyr); }

    /*!
     * \brief Get the stand derivation of raster data
     * \sa GetAverage
     */
    double GetStd(const int lyr = 1) { return GetStatistics(STATS_RS_STD, lyr); }

    /*!
     * \brief Get the range of raster data
     * \sa GetMaximum
     * \sa GetMinimum
     */
    double GetRange(const int lyr = 1) { return GetStatistics(STATS_RS_RANGE, lyr); }

    /*!
     * \brief Get the average of 2D raster data
     * \param[out] lyrnum \a int layer number
     * \param[out] values \a double* Statistics array
     */
    void GetAverage(int* lyrnum, double** values) { GetStatistics(STATS_RS_MEAN, lyrnum, values); }

    /*!
     * \brief Get the maximum of 2D raster data
     * \sa GetAverage
     */
    void GetMaximum(int* lyrnum, double** values) { GetStatistics(STATS_RS_MAX, lyrnum, values); }

    /*!
     * \brief Get the minimum of 2D raster data
     * \sa GetAverage
     */
    void GetMinimum(int* lyrnum, double** values) { GetStatistics(STATS_RS_MIN, lyrnum, values); }

    /*!
     * \brief Get the standard derivation of 2D raster data
     * \sa GetAverage
     */
    void GetStd(int* lyrnum, double** values) { GetStatistics(STATS_RS_STD, lyrnum, values); }

    /*!
     * \brief Get the range of 2D raster data
     * \sa GetAverage
     */
    void GetRange(int* lyrnum, double** values) { GetStatistics(STATS_RS_RANGE, lyrnum, values); }

    /*!
     * \brief Get the non-NoDATA number of 2D raster data
     * \sa GetAverage
     */
    void GetValidNumber(int* lyrnum, double** values) { GetStatistics(STATS_RS_VALIDNUM, lyrnum, values); }

    /*!
     * \brief Get the non-NoDATA cells number of the given raster layer data
     * \sa GetCellNumber
     * \sa GetDataLength
     */
    int GetValidNumber(const int lyr = 1) { return CVT_INT(GetStatistics(STATS_RS_VALIDNUM, lyr)); }

    //! Get stored cell number of raster data
    int GetCellNumber() const { return n_cells_; }

    //! Get the first dimension size of raster data
    int GetDataLength() const { return n_cells_; }

    //! Get column number of raster data
    int GetCols() const { return CVT_INT(headers_.at(HEADER_RS_NCOLS)); }

    //! Get row number of raster data
    int GetRows() const { return CVT_INT(headers_.at(HEADER_RS_NROWS)); }

    //! Get cell size of raster data
    double GetCellWidth() const { return headers_.at(HEADER_RS_CELLSIZE); }

    //! Get X coordinate of left lower corner of raster data
    double GetXllCenter() const { return headers_.at(HEADER_RS_XLL); }

    //! Get Y coordinate of left lower corner of raster data
    double GetYllCenter() const { return headers_.at(HEADER_RS_YLL); }

    int GetLayers() const {
        assert(n_lyrs_ == CVT_INT(headers_.at(HEADER_RS_LAYERS)));
        return n_lyrs_;
    }

    //! Get NoDATA value of raster data
    T GetNoDataValue() const { return static_cast<T>(headers_.at(HEADER_RS_NODATA)); }

    //! Get NoDATA value of raster data
    T GetDefaultValue() const { return default_value_; }

    /*!
     * \brief Get position index in 1D raster data for specific row and column
     * \return -1 --- the position is nodata
     *         -2 --- the position is out of the extent, which indicates an error
     */
    int GetPosition(int row, int col);

    //! Get position index in 1D raster data for given coordinate (x,y)
    int GetPosition(float x, float y);

    //! Get position index in 1D raster data for given coordinate (x,y)
    int GetPosition(double x, double y);

    /*! \brief Get raster data, include valid cell number and data
     * \return true if the raster data has been initialized, otherwise return false and print error info.
     */
    bool GetRasterData(int* n_cells, T** data);

    /*!
     * \brief Get 2D raster data, include valid cell number of each layer, layer number, and data
     * \return true if the 2D raster has been initialized, otherwise return false and print error info.
     */
    bool Get2DRasterData(int* n_cells, int* n_lyrs, T*** data);

    //! Get raster header information
    const map<string, double>& GetRasterHeader() const { return headers_; }

    //! Get raster statistics information
    const map<string, double>& GetStatistics() const { return stats_; }

    //! Get raster statistics information of 2D raster
    const map<string, double *>& GetStatistics2D() const { return stats_2d_; };

    //! Get full path name
    string GetFilePath() const { return full_path_; }

    //! Get core name
    string GetCoreName() const { return core_name_; }

    /*!
     * \brief Get position index data and the data length
     * \param[out] datalength
     * \param[out] positiondata The pointer of 2D array (pointer)
     */
    void GetRasterPositionData(int* datalength, int*** positiondata);

    //! Get pointer of raster data
    T* GetRasterDataPointer() const { return raster_; }

    //! Get pointer of position data
    int** GetRasterPositionDataPointer() const { return raster_pos_data_; }

    //! Get pointer of 2D raster data
    T** Get2DRasterDataPointer() const { return raster_2d_; }

    //! Get the spatial reference
    const char* GetSrs();

    //! Get the spatial reference string
    string GetSrsString();

    //! Get option by key, including the spatial reference by "SRS"
    string GetOption(const char* key);

    //! Get options
    const STRING_MAP& GetOptions() const { return options_; }

    /*!
     * \brief Get raster data at the valid cell index
     * The default lyr is 1, which means the 1D raster data, or the first layer of 2D data.
     */
    T GetValueByIndex(int cell_index, int lyr = 1);

    /*!
     * \brief Get raster data at the valid cell index (both for 1D and 2D raster)
     * \return a float array with length as n_lyrs
     */
    void GetValueByIndex(int cell_index, int* n_lyrs, T** values);

    /*!
     * \brief Get raster data via row and col
     * The default lyr is 1, which means the 1D raster data, or the first layer of 2D data.
     */
    T GetValue(int row, int col, int lyr = 1);

    /*!
     * \brief Get raster data (both for 1D and 2D raster) at the (row, col)
     * \return a float array with length as n_lyrs
     */
    void GetValue(int row, int col, int* n_lyrs, T** values);

    /*!
     * \brief Check if the raster data is NoData via row and col
     * The default lyr is 1, which means the 1D raster data, or the first layer of 2D data.
     */
    T IsNoData(const int row, const int col, const int lyr = 1) {
        return FloatEqual(GetValue(row, col, lyr), no_data_value_);
    }

    //! Is 2D raster data?
    bool Is2DRaster() const { return is_2draster; }

    //! Calculate positions or not
    bool PositionsCalculated() const { return calc_pos_; }

    //! raster position data is stored as array (true), or just a pointer
    bool PositionsAllocated() const { return store_pos_; }

    //! Use mask extent or not
    bool MaskExtented() const { return use_mask_ext_; }

    //! Basic statistics has been calculated or not
    bool StatisticsCalculated() const { return stats_calculated_; }

    //! The instance of clsRasterData has been initialized or not
    bool Initialized() const { return initialized_; }

    /*!
     * \brief Validate the available of raster data, both 1D and 2D data
     */
    bool ValidateRasterData() {
        if ((!is_2draster && nullptr != raster_) || // Valid 1D raster
            (is_2draster && nullptr != raster_2d_)) {
            // Valid 2D raster
            return true;
        }
        StatusNoUnitTest("Please initialize the raster object first.");
        return false;
    }

    /*!
     * \brief Validate the input row and col
     */
    bool ValidateRowCol(const int row, const int col) {
        if ((row < 0 || row >= GetRows()) || (col < 0 || col >= GetCols())) {
            StatusNoUnitTest("The row must between 0 and " + ValueToString(GetRows() - 1) +
                             ", and the col must between 0 and " + ValueToString(GetCols() - 1));
            return false;
        }
        return true;
    }

    /*!
     * \brief Validate the input layer number
     */
    bool ValidateLayer(const int lyr) {
        if (lyr <= 0 || lyr > n_lyrs_) {
            StatusNoUnitTest("The layer must be 1 ");
            if (n_lyrs_ > 1) StatusNoUnitTest(" or between 1 and " + utils_string::ValueToString(n_lyrs_));
            return false;
        }
        return true;
    }

    /*!
     * \brief Validate the input index
     */
    bool ValidateIndex(const int idx) {
        if (idx < 0 || idx >= n_cells_) {
            StatusNoUnitTest("The index must between 0 and " + utils_string::ValueToString(n_cells_ - 1));
            return false;
        }
        return true;
    }

    //! Get full filename
    string GetFullFileName() const { return full_path_; }

    //! Get mask data pointer
    clsRasterData<MASK_T>* GetMask() const { return mask_; }

    /*!
     * \brief Copy clsRasterData object
     */
    void Copy(const clsRasterData<T, MASK_T>* orgraster);

    /*!
     * \brief Replace NoData value by the given value
     */
    void ReplaceNoData(T replacedv);

    /*!
     * \brief classify raster
     */
    void Reclassify(map<int, T> reclass_map);

    /************* Utility functions ***************/

    /*!
     * \brief Calculate XY coordinates by given row and col number
     * \sa GetPositionByCoordinate
     * \param[in] row
     * \param[in] col
     * \return pair<double x, double y>
     */
    XY_COOR GetCoordinateByRowCol(int row, int col);

    /*!
     * \brief Calculate position by given coordinate
     * \sa GetCoordinateByRowCol
     * \param[in] x
     * \param[in] y
     * \param[in] header Optional, header map of raster layer data, the default is m_header
     * \return pair<int row, int col>
     */
    ROW_COL GetPositionByCoordinate(double x, double y, map<string, double>* header = nullptr);

    /*!
     * \brief Copy header information to current Raster data
     * \param[in] refers
     */
    void CopyHeader(const map<string, double>& refers);

private:
    /*!
     * \brief Initialize all raster related variables.
     */
    void InitializeRasterClass();

    /*!
     * \brief Initialize read function for ASC, GDAL, and MongoDB
     */
    void InitializeReadFunction(const string& filename, bool calc_pos = true, clsRasterData<MASK_T>* mask = nullptr,
                                bool use_mask_ext = true, T default_value = static_cast<T>(NODATA_VALUE),
                                STRING_MAP opts = STRING_MAP());

    /*!
     * \brief Constructor of clsRasterData instance from single file of TIFF, ASCII, or other GDAL supported raster file
     * Support 1D and/or 2D raster data
     * \param[in] filename Full path of the raster file
     * \param[in] calc_pos Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<T2> Mask layer
     * \param[in] use_mask_ext Use mask layer extent, even NoDATA exists.
     * \param[in] default_value
     * \param[in] opts Optional key-value stored in metadata
     * \return true if read successfully, otherwise return false.
     */
    bool ConstructFromSingleFile(const string& filename, bool calc_pos = true, clsRasterData<MASK_T>* mask = nullptr,
                                 bool use_mask_ext = true, T default_value = static_cast<T>(NODATA_VALUE),
                                 STRING_MAP opts = STRING_MAP());

    /*!
     * \brief Read raster data from ASC file, the simply usage
     * \param[in] asc_filename \a string
     * \param[out] header Raster header information
     * \param[out] values Raster data matrix
     * \return true if read successfully, otherwise return false.
     */
    bool ReadAscFile(const string& asc_filename, map<string, double>* header, T** values);

    /*!
     * \brief Extract by mask data and calculate position index, if necessary.
     */
    void MaskAndCalculateValidPosition();

    /*!
     * \brief Calculate position index from rectangle grid values, if necessary.
     * To use this function, mask should be nullptr.
     *
     */
    void CalculateValidPositionsFromGridDate();

    /*!
     * \brief Write raster header information into ASC file
     * If the file exists, delete it first.
     * \param[in] filename \a string, output ASC file path
     * \param[in] header header information
     */
    static bool WriteAscHeaders(const string& filename, map<string, double>& header);

#ifdef USE_GDAL
    /*!
    * \brief Read raster data by GDAL, the simply usage
    * \param[in] filename \a string
    * \param[out] header Raster header information
    * \param[out] values Raster data matrix
    * \return true if read successfully, otherwise return false.
    */
    bool ReadRasterFileByGdal(const string& filename, map<string, double>* header, T** values);

    /*!
     * \brief Write single geotiff file
     * If the file exists, delete it first.
     * \param[in] filename \a string, output ASC file path
     * \param[in] header header information
     * \param[in] opts Options, e.g., `srs` - Coordinate system string
     * \param[in] values float raster data array
     */
    static bool WriteSingleGeotiff(const string& filename, map<string, double>& header,
                                   STRING_MAP& opts, float* values);
#endif /* USE_GDAL */

#ifdef USE_MONGODB

    /*!
     * \brief Write full-sized raster data as GridFS file
     *        If the file exists, delete it first.
     * \param[in] gfs
     * \param[in] filename \a string, GridFS file name
     * \param[in] header header information
     * \param[in] values float raster data array
     * \param[in] datalength
     * \param[in] opts (optional) Key-value map for user-specific metadata
     */
    static bool WriteStreamDataAsGridfs(MongoGridFs* gfs, const string& filename,
                                        map<string, double>& header,
                                        T* values, int datalength,
                                        STRING_MAP opts = STRING_MAP());

#endif /* USE_MONGODB */

    /*!
     * \brief Add other layer's rater data to raster_2d_
     * \param[in] row Row number be added on, e.g. 2
     * \param[in] col Column number be added on, e.g. 3
     * \param[in] cellidx Cell index in raster_2d_, e.g. 23
     * \param[in] lyr Layer number which is greater than 1, e.g. 2, 3, ..., n
     * \param[in] lyrheader Header information of current layer
     * \param[in] lyrdata Raster layer data
     */
    void AddOtherLayerRasterData(int row, int col, int cellidx, int lyr,
                                 map<string, double> lyrheader, T* lyrdata);

    /*!
     * \brief If NoDataValue not equal to NODATA_VALUE, while default value do, then change default value.
     */
    void CheckDefaultValue() {
        if (FloatEqual(default_value_, static_cast<T>(NODATA_VALUE)) &&
            !FloatEqual(no_data_value_, static_cast<T>(NODATA_VALUE))) {
            default_value_ = no_data_value_;
        }
    }

private:
    /*!
     * \brief Operator= without implementation
     */
    clsRasterData& operator=(const clsRasterData&) {
    };

private:
    /*! cell number of raster data, i.e. the data length of \sa raster_data_ or \sa raster_2d_
     * 1. all grid cell number, i.e., ncols * nrows, when m_calcPositions is False
     * 2. valid cell number excluding NoDATA, when m_calcPositions is True and m_useMaskExtent is False.
     * 3. including NoDATA when mask is valid and m_useMaskExtent is True.
     */
    int n_cells_;
    //! Layer number of the 2D raster
    int n_lyrs_;
    ///< noDataValue
    T no_data_value_;
    ///< default value when mask by mask data, if not specified, it equals to no_data_value_
    T default_value_;
    ///< raster full path, e.g. "C:/tmp/studyarea.tif"
    string full_path_;
    ///< core raster file name, e.g. "studyarea"
    string core_name_;
    ///< 1D raster data
    T* raster_;
    ///< 2D raster data, [cellIndex][layer]
    T** raster_2d_;
    ///< cell index (row, col) in raster_data_ or the first layer of raster_2d_ (2D array)
    int** raster_pos_data_;
    ///< Key-value options in string format, including spatial reference
    STRING_MAP options_;
    ///< Header information, using double in case of truncation of coordinate value
    map<string, double> headers_;
    ///< Map to store basic statistics values for 1D raster data
    map<string, double> stats_;
    ///< Map to store basic statistics values for 2D raster data
    map<string, double *> stats_2d_;
    ///< mask clsRasterData instance
    clsRasterData<MASK_T>* mask_;
    ///< initial once
    bool initialized_;
    ///< Flag to identify 1D or 2D raster
    bool is_2draster;
    ///< calculate valid positions or not. The default is true.
    bool calc_pos_;
    ///< raster position data is newly allocated array (true), or just a pointer (false)
    bool store_pos_;
    ///< To be consistent with other datesets, keep the extent of Mask layer, even include NoDATA.
    bool use_mask_ext_;
    ///< Statistics calculated?
    bool stats_calculated_;
};

/*******************************************************/
/************* Implementation Code Begin ***************/
/*******************************************************/

/************* Construct functions ***************/

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::InitializeRasterClass() {
    full_path_ = "";
    core_name_ = "";
    n_cells_ = -1;
    no_data_value_ = static_cast<T>(NODATA_VALUE);
    default_value_ = static_cast<T>(NODATA_VALUE);
    raster_ = nullptr;
    raster_pos_data_ = nullptr;
    mask_ = nullptr;
    n_lyrs_ = -1;
    is_2draster = false;
    raster_2d_ = nullptr;
    calc_pos_ = false;
    store_pos_ = false;
    use_mask_ext_ = false;
    stats_calculated_ = false;
    const char* raster_headers[8] = {
        HEADER_RS_NCOLS, HEADER_RS_NROWS, HEADER_RS_XLL, HEADER_RS_YLL, HEADER_RS_CELLSIZE,
        HEADER_RS_NODATA, HEADER_RS_LAYERS, HEADER_RS_CELLSNUM
    };
    for (int i = 0; i < 6; i++) {
#ifdef HAS_VARIADIC_TEMPLATES
        headers_.emplace(raster_headers[i], NODATA_VALUE);
#else
        headers_.insert(make_pair(raster_headers[i], NODATA_VALUE));
#endif
    }
#ifdef HAS_VARIADIC_TEMPLATES
    headers_.emplace(HEADER_RS_LAYERS, -1.);
    headers_.emplace(HEADER_RS_CELLSNUM, -1.);
#else
    headers_.insert(make_pair(HEADER_RS_LAYERS, -1.));
    headers_.insert(make_pair(HEADER_RS_CELLSNUM, -1.));
#endif
    string statsnames[6] = {
        STATS_RS_VALIDNUM, STATS_RS_MIN, STATS_RS_MAX, STATS_RS_MEAN,
        STATS_RS_STD, STATS_RS_RANGE
    };
    for (int i = 0; i < 6; i++) {
#ifdef HAS_VARIADIC_TEMPLATES
        stats_.emplace(statsnames[i], NODATA_VALUE);
        stats_2d_.emplace(statsnames[i], nullptr);
#else
        stats_.insert(make_pair(statsnames[i], NODATA_VALUE));
        stats_2d_.insert(map<string, double *>::value_type(statsnames[i], nullptr));
#endif
    }
    initialized_ = true;
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::InitializeReadFunction(const string& filename, const bool calc_pos /* = true */,
                                                      clsRasterData<MASK_T>* mask /* = nullptr */,
                                                      const bool use_mask_ext /* = true */,
                                                      T default_value /* = static_cast<T>(NODATA_VALUE) */,
                                                      STRING_MAP opts /* = STRING_MAP() */) {
    full_path_ = filename;
    mask_ = mask;
    calc_pos_ = calc_pos;
    use_mask_ext_ = use_mask_ext;
    default_value_ = default_value;
    if (!initialized_) InitializeRasterClass();
    core_name_ = GetCoreFileName(full_path_);
    CopyStringMap(opts, options_);
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::clsRasterData() {
    InitializeRasterClass();
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::clsRasterData(T* data, int cols, int rows, T nodata,
                                        double dx, double xll, double yll,
                                        const string& srs) {
    STRING_MAP opts;
#ifdef HAS_VARIADIC_TEMPLATES
    opts.emplace(HEADER_RS_SRS, srs);
#else
    opts.insert(make_pair(HEADER_RS_SRS, srs));
#endif
    clsRasterData(data, cols, rows, nodata, dx, xll, yll, opts);
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::clsRasterData(T* data, const int cols, const int rows, T nodata,
                                        const double dx, const double xll, const double yll,
                                        STRING_MAP opts /* = STRING_MAP() */) {
    InitializeRasterClass();
    raster_ = data;
    no_data_value_ = nodata;
    CopyStringMap(opts, options_);
    n_cells_ = cols * rows;
    n_lyrs_ = 1;
    headers_[HEADER_RS_NCOLS] = cols;
    headers_[HEADER_RS_NROWS] = rows;
    headers_[HEADER_RS_XLL] = xll;
    headers_[HEADER_RS_YLL] = yll;
    headers_[HEADER_RS_CELLSIZE] = dx;
    headers_[HEADER_RS_NODATA] = nodata;
    headers_[HEADER_RS_LAYERS] = 1;
    headers_[HEADER_RS_CELLSNUM] = n_cells_;
}


template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::clsRasterData(T** data2d, const int cols, const int rows, const int nlayers,
                                        T nodata, const double dx, const double xll, const double yll,
                                        const string& srs) {
    STRING_MAP opts;
#ifdef HAS_VARIADIC_TEMPLATES
    opts.emplace(HEADER_RS_SRS, srs);
#else
    opts.insert(make_pair(HEADER_RS_SRS, srs));
#endif
    clsRasterData(data2d, cols, rows, nlayers, nodata, dx, xll, yll, opts);
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::clsRasterData(T** data2d, const int cols, const int rows, const int nlayers,
                                        T nodata, const double dx, const double xll, const double yll,
                                        STRING_MAP opts /* = STRING_MAP() */) {
    InitializeRasterClass();
    raster_2d_ = data2d;
    is_2draster = true;
    no_data_value_ = nodata;
    CopyStringMap(opts, options_);
    n_cells_ = cols * rows;
    n_lyrs_ = nlayers;
    headers_[HEADER_RS_NCOLS] = cols;
    headers_[HEADER_RS_NROWS] = rows;
    headers_[HEADER_RS_XLL] = xll;
    headers_[HEADER_RS_YLL] = yll;
    headers_[HEADER_RS_CELLSIZE] = dx;
    headers_[HEADER_RS_NODATA] = nodata;
    headers_[HEADER_RS_LAYERS] = nlayers;
    headers_[HEADER_RS_CELLSNUM] = n_cells_;
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::clsRasterData(const string& filename, const bool calc_pos /* = true */,
                                        clsRasterData<MASK_T>* mask /* = nullptr */,
                                        const bool use_mask_ext /* = true */,
                                        T default_value /* = static_cast<T>(NODATA_VALUE) */,
                                        STRING_MAP opts /* = STRING_MAP() */) {
    InitializeRasterClass();
    ReadFromFile(filename, calc_pos, mask, use_mask_ext, default_value, opts);
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>* clsRasterData<T, MASK_T>::Init(const string& filename,
                                                         const bool calc_pos /* = true */,
                                                         clsRasterData<MASK_T>* mask /* = nullptr */,
                                                         const bool use_mask_ext /* = true */,
                                                         T default_value /* = static_cast<T>(NODATA_VALUE) */,
                                                         STRING_MAP opts /* = STRING_MAP() */) {
    if (!CheckRasterFilesExist(filename)) return nullptr;
    return new clsRasterData<T, MASK_T>(filename, calc_pos, mask, use_mask_ext, default_value, opts);
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::clsRasterData(vector<string>& filenames,
                                        const bool calc_pos /* = true */,
                                        clsRasterData<MASK_T>* mask /* = nullptr */,
                                        const bool use_mask_ext /* = true */,
                                        T default_value /* = static_cast<T>(NODATA_VALUE) */,
                                        STRING_MAP opts /* = STRING_MAP() */) {
    InitializeRasterClass();
    if (filenames.empty()) {
        // if filenames is empty
        StatusNoUnitTest("The filenames MUST have at least one raster file path!");
        return;
    }
    if (!CheckRasterFilesExist(filenames)) {
        // if not all filenames are presented
        StatusNoUnitTest("Please make sure all file path existed!");
        return;
    }
    if (filenames.size() == 1) {
        // if filenames has only one file
        if (!ConstructFromSingleFile(filenames[0], calc_pos, mask,
                                     use_mask_ext, default_value, opts)) {
            return;
        }
    } else {
        // construct from multi-layers file
        n_lyrs_ = CVT_INT(filenames.size());
        // 1. firstly, take the first layer as the main input, to calculate position index or
        //    extract by mask if stated.
        if (!ConstructFromSingleFile(filenames[0], calc_pos, mask,
                                     use_mask_ext, default_value, opts)) {
            return;
        }
        // 2. then, change the core file name and file path template which format is: `<file dir>/CoreName_%d.<suffix>`
        core_name_ = SplitString(core_name_, '_').at(0);
        full_path_ = GetPathFromFullName(filenames[0]) + SEP + core_name_ + "_%d." + GetSuffix(filenames[0]);
        // So, to get a given layer's filepath, please use the following code. Definitely, maximum 99 layers is supported now.
        //    string layerFilepath = m_filePathName.replace(m_filePathName.find_last_of("%d") - 1, 2, ValueToString(1));
        // 3. initialize raster_2d_ and read the other layers according to position data if stated,
        //     or just read by row and col
        Initialize2DArray(n_cells_, n_lyrs_, raster_2d_, no_data_value_);
#pragma omp parallel for
        for (int i = 0; i < n_cells_; i++) {
            raster_2d_[i][0] = raster_[i];
        }
        Release1DArray(raster_);
        // take the first layer as mask, and use_mask_ext is true, and no need to calculate position data
        for (int fileidx = 1; fileidx < CVT_INT(filenames.size()); fileidx++) {
            map<string, double> tmpheader;
            T* tmplyrdata = nullptr;
            string curfilename = filenames[fileidx];
            if (StringMatch(GetUpper(GetSuffix(curfilename)), string(ASCIIExtension))) {
                ReadAscFile(curfilename, &tmpheader, &tmplyrdata);
            } else {
#ifdef USE_GDAL
                ReadRasterFileByGdal(curfilename, &tmpheader, &tmplyrdata);
#else
                cout << "Warning: Only ASC format is supported without GDAL!" << endl;
                return;
#endif
            }
            if (calc_pos_) {
#pragma omp parallel for
                for (int i = 0; i < n_cells_; ++i) {
                    int tmp_row = raster_pos_data_[i][0];
                    int tmp_col = raster_pos_data_[i][1];
                    AddOtherLayerRasterData(tmp_row, tmp_col, i, fileidx, tmpheader, tmplyrdata);
                }
            } else {
#pragma omp parallel for
                for (int i = 0; i < CVT_INT(headers_.at(HEADER_RS_NROWS)); i++) {
                    for (int j = 0; j < CVT_INT(headers_.at(HEADER_RS_NCOLS)); j++) {
                        AddOtherLayerRasterData(i, j, i * CVT_INT(headers_.at(HEADER_RS_NCOLS)) + j,
                                                fileidx, tmpheader, tmplyrdata);
                    }
                }
            }
            Release1DArray(tmplyrdata);
        }
        is_2draster = true;
        headers_.at(HEADER_RS_LAYERS) = n_lyrs_; // repair layers count in headers
    }
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>* clsRasterData<T, MASK_T>::Init(vector<string>& filenames,
                                                         bool calc_pos /* = true */,
                                                         clsRasterData<MASK_T>* mask /* = nullptr */,
                                                         bool use_mask_ext /* = true */,
                                                         T default_value /* = static_cast<T>(NODATA_VALUE) */,
                                                         STRING_MAP opts /* = STRING_MAP() */) {
    if (filenames.empty()) {
        // if filenames is empty
        StatusNoUnitTest("The filenames MUST have at least one raster file path!");
        return nullptr;
    }
    if (!CheckRasterFilesExist(filenames)) {
        // if not all filenames are presented
        StatusNoUnitTest("Please make sure all file path existed!");
        return nullptr;
    }
    if (filenames.size() == 1) {
        // if filenames has only one file
        return new clsRasterData<T, MASK_T>(filenames[0], calc_pos, mask, use_mask_ext, default_value, opts);
    }
    // else, construct from multi-layers file
    return new clsRasterData<T, MASK_T>(filenames, calc_pos, mask, use_mask_ext, default_value, opts);
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::clsRasterData(clsRasterData<MASK_T>* mask, T* const values,
                                        STRING_MAP opts /* = STRING_MAP() */) {
    InitializeRasterClass();
    mask_ = mask;
    n_lyrs_ = mask->GetLayers();
    n_cells_ = mask_->GetCellNumber();
    Initialize1DArray(n_cells_, raster_, values); // DO NOT ASSIGN ARRAY DIRECTLY, IN CASE OF MEMORY ERROR!
    CopyHeader(mask_->GetRasterHeader());
#ifdef HAS_VARIADIC_TEMPLATES
    options_.emplace(HEADER_RS_SRS, mask_->GetSrsString());
#else
    options_.insert(make_pair(HEADER_RS_SRS, mask_->GetSrsString()));
#endif
    default_value_ = static_cast<T>(mask_->GetDefaultValue());
    calc_pos_ = false;
    if (mask->PositionsCalculated()) {
        mask->GetRasterPositionData(&n_cells_, &raster_pos_data_);
    }
    use_mask_ext_ = true;
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::clsRasterData(clsRasterData<MASK_T>* mask, T** const values,
                                        const int lyrs, STRING_MAP opts /* = STRING_MAP() */) {
    InitializeRasterClass();
    mask_ = mask;
    n_lyrs_ = lyrs;
    CopyHeader(mask_->GetRasterHeader());
    headers_.at(HEADER_RS_LAYERS) = n_lyrs_;
#ifdef HAS_VARIADIC_TEMPLATES
    options_.emplace(HEADER_RS_SRS, mask_->GetSrsString());
#else
    options_.insert(make_pair(HEADER_RS_SRS, mask_->GetSrsString()));
#endif
    n_cells_ = mask_->GetCellNumber();
    Initialize2DArray(n_cells_, n_lyrs_, raster_2d_, values); // DO NOT ASSIGN ARRAY DIRECTLY!
    default_value_ = static_cast<T>(mask_->GetDefaultValue());
    use_mask_ext_ = true;
    if (mask->PositionsCalculated()) {
        mask->GetRasterPositionData(&n_cells_, &raster_pos_data_);
    }
    is_2draster = true;
}

#ifdef USE_MONGODB

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::clsRasterData(MongoGridFs* gfs, const char* remote_filename,
                                        const bool calc_pos /* = true */,
                                        clsRasterData<MASK_T>* mask /* = nullptr */,
                                        const bool use_mask_ext /* = true */,
                                        T default_value /* = static_cast<T>(NODATA_VALUE) */,
                                        STRING_MAP opts /* = STRING_MAP() */) {
    InitializeRasterClass();
    ReadFromMongoDB(gfs, remote_filename, calc_pos, mask, use_mask_ext, default_value, opts);
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>* clsRasterData<T, MASK_T>::Init(MongoGridFs* gfs, const char* remote_filename,
                                                         const bool calc_pos /* = true */,
                                                         clsRasterData<MASK_T>* mask /* = nullptr */,
                                                         const bool use_mask_ext /* = true */,
                                                         T default_value /* = static_cast<T>(NODATA_VALUE) */,
                                                         STRING_MAP opts /* = STRING_MAP() */) {
    return new clsRasterData<T, MASK_T>(gfs, remote_filename, calc_pos, mask, use_mask_ext, default_value, opts);
};

#endif /* USE_MONGODB */

template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::ConstructFromSingleFile(const string& filename,
                                                       const bool calc_pos /* = true */,
                                                       clsRasterData<MASK_T>* mask /* = nullptr */,
                                                       const bool use_mask_ext /* = true */,
                                                       T default_value /* = static_cast<T>(NODATA_VALUE) */,
                                                       STRING_MAP opts /* = STRING_MAP() */) {
    full_path_ = filename;
    calc_pos_ = calc_pos;
    mask_ = mask;
    use_mask_ext_ = use_mask_ext;
    default_value_ = default_value;
    if (nullptr == mask_) {
        use_mask_ext_ = false;
    }
    core_name_ = GetCoreFileName(full_path_);
    default_value_ = default_value;

    CopyStringMap(opts, options_);

    bool readflag = false;
    if (StringMatch(GetUpper(GetSuffix(filename)), ASCIIExtension)) {
        readflag = ReadAscFile(full_path_, &headers_, &raster_);
    } else {
#ifdef USE_GDAL
        readflag = ReadRasterFileByGdal(full_path_, &headers_, &raster_);
#else
        cout << "Warning: Only ASC format is supported without GDAL!" << endl;
        return false;
#endif /* USE_GDAL */
    }
    // After read raster data from ASCII file or GeoTiff.
    if (options_.find(HEADER_RS_SRS) == options_.end()) {
#ifdef HAS_VARIADIC_TEMPLATES
        options_.emplace(HEADER_RS_SRS, "");
#else
        options_.insert(make_pair(HEADER_RS_SRS, ""));
#endif
    }
    CheckDefaultValue();
    if (readflag) {
        if (n_lyrs_ < 0) n_lyrs_ = 1;
        MaskAndCalculateValidPosition();
        return true;
    }
    return false;
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::~clsRasterData() {
    if (core_name_ != "") StatusMessage(("Release raster: " + core_name_).c_str());
    if (nullptr != raster_) Release1DArray(raster_);
    if (nullptr != raster_pos_data_ && store_pos_) Release2DArray(n_cells_, raster_pos_data_);
    if (nullptr != raster_2d_ && is_2draster) Release2DArray(n_cells_, raster_2d_);
    if (is_2draster && stats_calculated_) ReleaseStatsMap2D();
}

/************* Get information functions ***************/
template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::CalculateStatistics() {
    if (stats_calculated_) return;
    if (is_2draster && nullptr != raster_2d_) {
        double** derivedvs;
        BasicStatistics(raster_2d_, n_cells_, n_lyrs_, &derivedvs, no_data_value_);
        if (stats_2d_.empty()) {
#ifdef HAS_VARIADIC_TEMPLATES
            stats_2d_.emplace(STATS_RS_VALIDNUM, derivedvs[0]);
            stats_2d_.emplace(STATS_RS_MEAN, derivedvs[1]);
            stats_2d_.emplace(STATS_RS_MAX, derivedvs[2]);
            stats_2d_.emplace(STATS_RS_MIN, derivedvs[3]);
            stats_2d_.emplace(STATS_RS_STD, derivedvs[4]);
            stats_2d_.emplace(STATS_RS_RANGE, derivedvs[5]);
#else
            stats_2d_.insert(make_pair(STATS_RS_VALIDNUM, derivedvs[0]));
            stats_2d_.insert(make_pair(STATS_RS_MEAN, derivedvs[1]));
            stats_2d_.insert(make_pair(STATS_RS_MAX, derivedvs[2]));
            stats_2d_.insert(make_pair(STATS_RS_MIN, derivedvs[3]));
            stats_2d_.insert(make_pair(STATS_RS_STD, derivedvs[4]));
            stats_2d_.insert(make_pair(STATS_RS_RANGE, derivedvs[5]));
#endif
        } else {
            stats_2d_.at(STATS_RS_VALIDNUM) = derivedvs[0];
            stats_2d_.at(STATS_RS_MEAN) = derivedvs[1];
            stats_2d_.at(STATS_RS_MAX) = derivedvs[2];
            stats_2d_.at(STATS_RS_MIN) = derivedvs[3];
            stats_2d_.at(STATS_RS_STD) = derivedvs[4];
            stats_2d_.at(STATS_RS_RANGE) = derivedvs[5];
        }
        // 1D array elements of derivedvs will be released by the destructor: releaseStatsMap2D()
        Release1DArray(derivedvs);
    } else {
        double* derivedv = nullptr;
        BasicStatistics(raster_, n_cells_, &derivedv, no_data_value_);
        stats_.at(STATS_RS_VALIDNUM) = derivedv[0];
        stats_.at(STATS_RS_MEAN) = derivedv[1];
        stats_.at(STATS_RS_MAX) = derivedv[2];
        stats_.at(STATS_RS_MIN) = derivedv[3];
        stats_.at(STATS_RS_STD) = derivedv[4];
        stats_.at(STATS_RS_RANGE) = derivedv[5];
        Release1DArray(derivedv);
    }
    stats_calculated_ = true;
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::ReleaseStatsMap2D() {
    for (auto it = stats_2d_.begin(); it != stats_2d_.end();) {
        if (nullptr != it->second) {
            Release1DArray(it->second);
        }
        stats_2d_.erase(it++);
    }
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::UpdateStatistics() {
    if (is_2draster && stats_calculated_) ReleaseStatsMap2D();
    stats_calculated_ = false;
    CalculateStatistics();
}

template <typename T, typename MASK_T>
double clsRasterData<T, MASK_T>::GetStatistics(string sindex, const int lyr /* = 1 */) {
    sindex = GetUpper(sindex);
    if (!ValidateRasterData() || !ValidateLayer(lyr)) {
        StatusNoUnitTest("No available raster statistics!");
        return no_data_value_;
    }
    if (is_2draster && nullptr != raster_2d_) {
        // for 2D raster data
        auto it = stats_2d_.find(sindex);
        if (it != stats_2d_.end()) {
            if (nullptr == it->second) {
                stats_calculated_ = false;
                CalculateStatistics();
            }
            return stats_2d_.at(sindex)[lyr - 1];
        }
        StatusNoUnitTest("WARNING: " + ValueToString(sindex) + " is not supported currently.");
        return no_data_value_;
    }
    // Else, for 1D raster data
    auto it = stats_.find(sindex);
    if (it != stats_.end()) {
        if (FloatEqual(it->second, CVT_DBL(NODATA_VALUE)) || !stats_calculated_) {
            CalculateStatistics();
        }
        return stats_.at(sindex);
    }
    StatusNoUnitTest("WARNING: " + ValueToString(sindex) + " is not supported currently.");
    return no_data_value_;
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::GetStatistics(string sindex, int* lyrnum, double** values) {
    if (!is_2draster || nullptr != raster_2d_) {
        StatusNoUnitTest("Please initialize the raster object first.");
        *values = nullptr;
        return;
    }
    sindex = GetUpper(sindex);
    *lyrnum = n_lyrs_;
    auto it = stats_2d_.find(sindex);
    if (!Is2DRaster()) {
        *values = nullptr;
        return;
    }
    if (it == stats_2d_.end()) {
        *values = nullptr;
        StatusNoUnitTest("WARNING: " + ValueToString(sindex) + " is not supported currently.");
        return;
    }
    if (nullptr == it->second || !stats_calculated_) {
        CalculateStatistics();
    }
    *values = it->second;
}

template <typename T, typename MASK_T>
int clsRasterData<T, MASK_T>::GetPosition(const int row, const int col) {
    if (!ValidateRasterData() || !ValidateRowCol(row, col)) {
        return -2; // means error occurred!
    }
    if (!calc_pos_ || nullptr == raster_pos_data_) {
        return GetCols() * row + col;
    }
    for (int i = 0; i < n_cells_; i++) {
        if (row == raster_pos_data_[i][0] && col == raster_pos_data_[i][1]) {
            return i;
        }
    }
    return -1; // means the location is NODATA
}

template <typename T, typename MASK_T>
int clsRasterData<T, MASK_T>::GetPosition(const float x, const float y) {
    return GetPosition(CVT_DBL(x), CVT_DBL(y));
}

template <typename T, typename MASK_T>
int clsRasterData<T, MASK_T>::GetPosition(const double x, const double y) {
    if (!initialized_) return -2;
    double xll_center = GetXllCenter();
    double yll_center = GetYllCenter();
    double dx = GetCellWidth();
    double dy = GetCellWidth();
    int n_rows = GetRows();
    int n_cols = GetCols();

    if (FloatEqual(xll_center, CVT_DBL(NODATA_VALUE)) || FloatEqual(yll_center, CVT_DBL(NODATA_VALUE)) ||
        FloatEqual(dx, CVT_DBL(NODATA_VALUE)) || n_rows < 0 || n_cols < 0) {
        StatusNoUnitTest("No available header information!");
        return -2;
    }

    double x_min = xll_center - dx / 2.;
    double x_max = x_min + dx * n_cols;
    if (x > x_max || x < xll_center) {
        return -2;
    }

    double y_min = yll_center - dy / 2.;
    double y_max = y_min + dy * n_rows;
    if (y > y_max || y < yll_center) {
        return -2;
    }

    int n_row = CVT_INT((y_max - y) / dy); //calculate from ymax
    int n_col = CVT_INT((x - x_min) / dx); //calculate from xmin

    return GetPosition(n_row, n_col);
}

template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::GetRasterData(int* n_cells, T** data) {
    if (ValidateRasterData() && !is_2draster) {
        *n_cells = n_cells_;
        *data = raster_;
        return true;
    }
    *n_cells = -1;
    *data = nullptr;
    return false;
}

template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::Get2DRasterData(int* n_cells, int* n_lyrs, T*** data) {
    if (ValidateRasterData() && is_2draster) {
        *n_cells = n_cells_;
        *n_lyrs = n_lyrs_;
        *data = raster_2d_;
        return true;
    }
    *n_cells = -1;
    *n_lyrs = -1;
    *data = nullptr;
    return false;
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::GetRasterPositionData(int* datalength, int*** positiondata) {
    if (nullptr != raster_pos_data_) {
        *datalength = n_cells_;
        *positiondata = raster_pos_data_;
    } else {
        // reCalculate position data
        if (!ValidateRasterData()) {
            *datalength = -1;
            *positiondata = nullptr;
            return;
        }
        CalculateValidPositionsFromGridDate();
        *datalength = n_cells_;
        *positiondata = raster_pos_data_;
    }
}

template <typename T, typename MASK_T>
const char* clsRasterData<T, MASK_T>::GetSrs() {
    return GetSrsString().c_str();
}

template <typename T, typename MASK_T>
string clsRasterData<T, MASK_T>::GetSrsString() {
    return GetOption(HEADER_RS_SRS);
}

template <typename T, typename MASK_T>
string clsRasterData<T, MASK_T>::GetOption(const char* key) {
    if (options_.find(key) == options_.end()) {
        StatusMessage((string(key) + " is not existed in the options of the raster data!").c_str());
        return string("");
    }
    return options_.at(key);
}

template <typename T, typename MASK_T>
T clsRasterData<T, MASK_T>::GetValueByIndex(const int cell_index, const int lyr /* = 1 */) {
    if (!ValidateRasterData() || !ValidateIndex(cell_index) || !ValidateLayer(lyr)) {
        return no_data_value_;
    }
    if (is_2draster) {
        return raster_2d_[cell_index][lyr - 1];
    }
    return raster_[cell_index];
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::GetValueByIndex(const int cell_index, int* n_lyrs, T** values) {
    if (!ValidateRasterData() || !ValidateIndex(cell_index)) {
        *n_lyrs = -1;
        *values = nullptr;
        return;
    }
    if (is_2draster) {
        T* cell_values = new T[n_lyrs_];
        for (int i = 0; i < n_lyrs_; i++) {
            cell_values[i] = raster_2d_[cell_index][i];
        }
        *n_lyrs = n_lyrs_;
        *values = cell_values;
    } else {
        *n_lyrs = 1;
        T* cell_values = new T[1];
        cell_values[0] = raster_[cell_index];
        *values = cell_values;
    }
}

template <typename T, typename MASK_T>
T clsRasterData<T, MASK_T>::GetValue(const int row, const int col, const int lyr /* = 1 */) {
    if (!ValidateRasterData() || !ValidateRowCol(row, col) || !ValidateLayer(lyr)) {
        return no_data_value_;
    }
    // get index according to position data if possible
    if (calc_pos_ && nullptr != raster_pos_data_) {
        int valid_cell_index = GetPosition(row, col);
        if (valid_cell_index < 0) return no_data_value_; // error or NODATA
        return GetValueByIndex(valid_cell_index, lyr);
    }
    // get data directly from row and col
    if (is_2draster) {
        return raster_2d_[row * GetCols() + col][lyr - 1];
    }
    return raster_[row * GetCols() + col];
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::GetValue(const int row, const int col, int* n_lyrs, T** values) {
    if (!ValidateRasterData() || !ValidateRowCol(row, col)) {
        *n_lyrs = -1;
        *values = nullptr;
        return;
    }
    if (calc_pos_ && nullptr != raster_pos_data_) {
        int valid_cell_index = GetPosition(row, col);
        if (valid_cell_index == -2) {
            *n_lyrs = -1;
            *values = nullptr; // error
        } else if (valid_cell_index == -1) {
            *n_lyrs = n_lyrs_;
            T* cell_values = new T[n_lyrs_];
            for (int i = 0; i < n_lyrs_; i++) {
                cell_values[i] = no_data_value_;
            }
            *values = cell_values; // NODATA
        } else {
            GetValueByIndex(valid_cell_index, n_lyrs, values);
        }
    } else {
        // get data directly from row and col
        if (is_2draster) {
            T* cell_values = new T[n_lyrs_];
            for (int i = 0; i < n_lyrs_; i++) {
                cell_values[i] = raster_2d_[row * GetCols() + col][i];
            }
            *n_lyrs = n_lyrs_;
            *values = cell_values;
        } else {
            *n_lyrs = 1;
            T* cell_values = new T[1];
            cell_values[0] = raster_[row * GetCols() + col];
            *values = cell_values;
        }
    }
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::SetValue(const int row, const int col, T value, const int lyr /* = 1 */) {
    if (!ValidateRasterData() || !ValidateRowCol(row, col) || !ValidateLayer(lyr)) {
        StatusNoUnitTest("Set value failed!");
        return;
    }
    int idx = GetPosition(row, col);
    if (idx == -1) {
        // the origin value is NODATA
        StatusNoUnitTest("Current version do not support to setting value to NoDATA location!");
    } else {
        if (is_2draster) {
            raster_2d_[idx][lyr - 1] = value;
        } else {
            raster_[idx] = value;
        }
    }
}

/************* Output to file functions ***************/

template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::OutputToFile(const string& filename) {
    if (GetPathFromFullName(filename).empty()) return false;
    string abs_filename = GetAbsolutePath(filename);
    if (!ValidateRasterData()) return false;
    string filetype = GetUpper(GetSuffix(abs_filename));
    if (StringMatch(filetype, ASCIIExtension)) {
        return OutputAscFile(filename);
    }
#ifdef USE_GDAL
    if (StringMatch(filetype, GTiffExtension)) {
        return OutputFileByGdal(filename);
    }
    return OutputFileByGdal(ReplaceSuffix(filename, GTiffExtension));
#else
    cout << "Warning: Without GDAL, ASC file will be exported as default!" << endl;
    return OutputAscFile(ReplaceSuffix(filename, ASCIIExtension));
#endif /* USE_GDAL */
}

template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::WriteAscHeaders(const string& filename, map<string, double>& header) {
    DeleteExistedFile(filename);
    string abs_filename = GetAbsolutePath(filename);
    std::ofstream raster_file(abs_filename.c_str(), std::ios::app | std::ios::out);
    if (!raster_file.is_open()) {
        StatusNoUnitTest("Error opening file: " + abs_filename);
        return false;
    }
    // write file
    int rows = CVT_INT(header.at(HEADER_RS_NROWS));
    int cols = CVT_INT(header.at(HEADER_RS_NCOLS));
    // write header
    raster_file << HEADER_RS_NCOLS << " " << cols << endl;
    raster_file << HEADER_RS_NROWS << " " << rows << endl;
    raster_file << HEADER_RS_XLL << " " << header.at(HEADER_RS_XLL) << endl;
    raster_file << HEADER_RS_YLL << " " << header.at(HEADER_RS_YLL) << endl;
    raster_file << HEADER_RS_CELLSIZE << " " << header.at(HEADER_RS_CELLSIZE) << endl;
    raster_file << HEADER_RS_NODATA << " " << setprecision(6) << header.at(HEADER_RS_NODATA) << endl;
    raster_file.close();
    return true;
}

template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::OutputAscFile(const string& filename) {
    string abs_filename = GetAbsolutePath(filename);
    // 1. Is there need to calculate valid position index?
    int count;
    int** position = nullptr;
    bool outputdirectly = true;
    if (nullptr != raster_pos_data_) {
        GetRasterPositionData(&count, &position);
        outputdirectly = false;
        assert(nullptr != position);
    }
    // 2. Write ASC raster headers first (for 1D raster data only)
    if (!is_2draster) {
        if (!WriteAscHeaders(abs_filename, headers_)) return false;
    }
    // 3. Begin to write raster data
    int rows = CVT_INT(headers_.at(HEADER_RS_NROWS));
    int cols = CVT_INT(headers_.at(HEADER_RS_NCOLS));

    // 3.1 2D raster data
    if (is_2draster) {
        string pre_path = GetPathFromFullName(abs_filename);
        if (StringMatch(pre_path, "")) return false;
        string core_name = GetCoreFileName(abs_filename);
        for (int lyr = 0; lyr < n_lyrs_; lyr++) {
            std::stringstream oss;
            oss << pre_path << core_name << "_" << lyr + 1 << "." << ASCIIExtension;
            string tmpfilename = oss.str();
            if (!WriteAscHeaders(tmpfilename, headers_)) return false;
            // write data
            std::ofstream raster_file(tmpfilename.c_str(), std::ios::app | std::ios::out);
            if (!raster_file.is_open()) {
                StatusNoUnitTest("Error opening file: " + tmpfilename);
                return false;
            }
            int index = 0;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (outputdirectly) {
                        index = i * cols + j;
                        raster_file << setprecision(6) << raster_2d_[index][lyr] << " ";
                        continue;
                    }
                    if (index < n_cells_ && (position[index][0] == i && position[index][1] == j)) {
                        raster_file << setprecision(6) << raster_2d_[index][lyr] << " ";
                        index++;
                    } else { raster_file << setprecision(6) << NODATA_VALUE << " "; }
                }
                raster_file << endl;
            }
            raster_file.close();
        }
    } else {
        // 3.2 1D raster data
        std::ofstream raster_file(filename.c_str(), std::ios::app | std::ios::out);
        if (!raster_file.is_open()) {
            StatusNoUnitTest("Error opening file: " + abs_filename);
            return false;
        }
        int index = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (outputdirectly) {
                    index = i * cols + j;
                    raster_file << setprecision(6) << raster_[index] << " ";
                    continue;
                }
                if (index < n_cells_) {
                    if (position[index][0] == i && position[index][1] == j) {
                        raster_file << setprecision(6) << raster_[index] << " ";
                        index++;
                    } else { raster_file << setprecision(6) << no_data_value_ << " "; }
                } else { raster_file << setprecision(6) << no_data_value_ << " "; }
            }
            raster_file << endl;
        }
        raster_file.close();
    }
    position = nullptr;
    return true;
}

#ifdef USE_GDAL
template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::WriteSingleGeotiff(const string& filename,
                                                  map<string, double>& header,
                                                  STRING_MAP& opts, float* values) {
    // 1. Create GeoTiff file driver
    GDALDriver* po_driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (nullptr == po_driver) return false;
    char** papsz_options = nullptr;
    int n_rows = CVT_INT(header.at(HEADER_RS_NROWS));
    int n_cols = CVT_INT(header.at(HEADER_RS_NCOLS));
    GDALDataset* po_dst_ds = po_driver->Create(filename.c_str(), n_cols, n_rows, 1,
                                               GDT_Float32, papsz_options);
    if (nullptr == po_dst_ds) return false;
    // 2. Write raster data
    GDALRasterBand* po_dst_band = po_dst_ds->GetRasterBand(1);
    CPLErr result = po_dst_band->RasterIO(GF_Write, 0, 0, n_cols, n_rows, values,
                                          n_cols, n_rows, GDT_Float32, 0, 0);
    if (result != CE_None) {
        cout << "RaterIO Error: " << CPLGetLastErrorMsg() << endl;
        return false;
    }
    if (nullptr == po_dst_band) return false;
    po_dst_band->SetNoDataValue(header.at(HEADER_RS_NODATA));
    // 3. Writer header information
    double geo_trans[6];
    geo_trans[0] = header.at(HEADER_RS_XLL) - 0.5 * header.at(HEADER_RS_CELLSIZE);
    geo_trans[1] = header.at(HEADER_RS_CELLSIZE);
    geo_trans[2] = 0.;
    geo_trans[3] = header.at(HEADER_RS_YLL) + (n_rows - 0.5) * header.at(HEADER_RS_CELLSIZE);
    geo_trans[4] = 0.;
    geo_trans[5] = -header.at(HEADER_RS_CELLSIZE);
    po_dst_ds->SetGeoTransform(geo_trans);
    if (opts.find(HEADER_RS_SRS) == opts.end()) {
        po_dst_ds->SetProjection("");
    } else {
        po_dst_ds->SetProjection(opts.at(HEADER_RS_SRS).c_str());
    }
    GDALClose(po_dst_ds);

    return true;
}

template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::OutputFileByGdal(const string& filename) {
    string abs_filename = GetAbsolutePath(filename);
    // 1. Is there need to calculate valid position index?
    int cnt;
    int** pos = nullptr;
    bool outputdirectly = true;
    if (nullptr != raster_pos_data_) {
        GetRasterPositionData(&cnt, &pos);
        outputdirectly = false;
        assert(nullptr != pos);
    }
    // 2. Get raster data
    // 2.1 2D raster data
    int n_rows = CVT_INT(headers_.at(HEADER_RS_NROWS));
    int n_cols = CVT_INT(headers_.at(HEADER_RS_NCOLS));
    if (is_2draster) {
        string pre_path = GetPathFromFullName(abs_filename);
        if (StringMatch(pre_path, "")) return false;
        string core_name = GetCoreFileName(abs_filename);
        for (int lyr = 0; lyr < n_lyrs_; lyr++) {
            std::stringstream oss;
            oss << pre_path << core_name << "_" << lyr + 1 << "." << GTiffExtension;
            string tmpfilename = oss.str();
            float* data_1d = nullptr;
            Initialize1DArray(n_rows * n_cols, data_1d, CVT_FLT(no_data_value_));
            int validnum = 0;
            for (int i = 0; i < n_rows; i++) {
                for (int j = 0; j < n_cols; j++) {
                    int index = i * n_cols + j;
                    if (outputdirectly) {
                        data_1d[index] = CVT_FLT(raster_2d_[index][lyr]);
                        continue;
                    }
                    if (validnum < n_cells_ && (pos[validnum][0] == i && pos[validnum][1] == j)) {
                        data_1d[index] = CVT_FLT(raster_2d_[validnum][lyr]);
                        validnum++;
                    }
                }
            }
            bool outflag = WriteSingleGeotiff(tmpfilename, headers_, options_, data_1d);
            Release1DArray(data_1d);
            if (!outflag) return false;
        }
    } else {
        // 3.2 1D raster data
        float* data_1d = nullptr;
        bool newbuilddata = true;
        if (outputdirectly) {
            if (typeid(T) != typeid(float)) {
                Initialize1DArray(n_cells_, data_1d, raster_);
            } else {
                data_1d = reinterpret_cast<float *>(raster_); // DO NOT USE `data_1d = raster_;`
                newbuilddata = false;
            }
        } else {
            Initialize1DArray(n_rows * n_cols, data_1d, no_data_value_);
        }
        int validnum = 0;
        if (!outputdirectly) {
            for (int i = 0; i < n_rows; ++i) {
                for (int j = 0; j < n_cols; ++j) {
                    int index = i * n_cols + j;
                    if (validnum < n_cells_ && (pos[validnum][0] == i && pos[validnum][1] == j)) {
                        data_1d[index] = CVT_FLT(raster_[validnum]);
                        validnum++;
                    }
                }
            }
        }
        bool outflag = WriteSingleGeotiff(abs_filename, headers_, options_, data_1d);
        if (!newbuilddata) { data_1d = nullptr; } else { Release1DArray(data_1d); }
        if (!outflag) return false;
    }
    pos = nullptr;
    return true;
}
#endif /* USE_GDAL */

#ifdef USE_MONGODB
template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::OutputToMongoDB(const string& filename, MongoGridFs* gfs,
                                               STRING_MAP opts /* = STRING_MAP() */) {
    CopyStringMap(opts, options_);
    // 1. Is there need to calculate valid position index?
    int cnt;
    int** pos = nullptr;
    bool outputdirectly = true;
    if (nullptr != raster_pos_data_) {
        GetRasterPositionData(&cnt, &pos);
        outputdirectly = false;
        // assert(nullptr != pos);  // Avoid assert statement. -LJ.
        if (nullptr == pos) {
            cout << "Position data of the valid raster data are required for saving "
                    << filename << " to MongoDB!" << endl;
            return false;
		}
    }
    // 2. Get raster data
    bool saved;
    int try_times = 1;
    T* data_1d = nullptr;
    T no_data_value = static_cast<T>(headers_.at(HEADER_RS_NODATA));
    int n_rows = CVT_INT(headers_.at(HEADER_RS_NROWS));
    int n_cols = CVT_INT(headers_.at(HEADER_RS_NCOLS));
    int datalength;
    string core_name = GetCoreFileName(filename);
    // 2.1 2D raster data
    if (is_2draster) {
        datalength = n_rows * n_cols * n_lyrs_;
        Initialize1DArray(datalength, data_1d, no_data_value);
        int cnt_idx = 0;
        for (int i = 0; i < n_rows; ++i) {
            for (int j = 0; j < n_cols; ++j) {
                int rowcol_index = i * n_cols + j;
                int data_index = i * n_cols * n_lyrs_ + j * n_lyrs_;
                if (outputdirectly) {
                    for (int k = 0; k < n_lyrs_; k++) {
                        data_1d[data_index + k] = raster_2d_[rowcol_index][k];
                    }
                    continue;
                }
                if (cnt_idx < n_cells_ && (pos[cnt_idx][0] == i && pos[cnt_idx][1] == j)) {
                    for (int k = 0; k < n_lyrs_; k++) {
                        data_1d[data_index + k] = raster_2d_[cnt_idx][k];
                    }
                    cnt_idx++;
                }
            }
        }
    } else {
        // 3.2 1D raster data
        datalength = n_rows * n_cols;
        if (outputdirectly) {
            data_1d = raster_;
        } else {
            Initialize1DArray(datalength, data_1d, no_data_value);
		}
        int validnum = 0;
        if (!outputdirectly) {
            for (int i = 0; i < n_rows; ++i) {
                for (int j = 0; j < n_cols; ++j) {
                    int index = i * n_cols + j;
                    if (validnum < n_cells_ && (pos[validnum][0] == i && pos[validnum][1] == j)) {
                        data_1d[index] = raster_[validnum];
                        validnum++;
                    }
                }
            }
        }
    }
    while (try_times <= 3) { // Try 3 times
        saved = WriteStreamDataAsGridfs(gfs, core_name, headers_, data_1d, datalength, options_);
        if (saved) {
            break;
        } else {
            SleepMs(2); // Sleep 0.002 sec and retry
            try_times++;
        }
    }
    if (!is_2draster && outputdirectly) {
        data_1d = nullptr;
    } else {
        Release1DArray(data_1d);
	}
    return saved;
}

template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::WriteStreamDataAsGridfs(MongoGridFs* gfs,
                                                       const string& filename,
                                                       map<string, double>& header,
                                                       T* values,
                                                       const int datalength,
                                                       STRING_MAP opts /* = STRING_MAP() */) {
    bson_t p = BSON_INITIALIZER;
    for (auto iter = header.begin(); iter != header.end(); ++iter) {
        if (std::fmod(iter->second, 1.) == 0) {
            BSON_APPEND_INT32(&p, iter->first.c_str(), CVT_INT(iter->second));
        } else {
            BSON_APPEND_DOUBLE(&p, iter->first.c_str(), iter->second);
        }
    }
    // Add user-specific key-values into metadata
    if (!opts.empty()) {
        for (auto kviter = opts.begin(); kviter != opts.end(); ++kviter) {
            bool is_dbl = false;
            double dbl_value = IsDouble(kviter->second, is_dbl);
            if (StringMatch("", kviter->second) || !is_dbl) {
                BSON_APPEND_UTF8(&p, kviter->first.c_str(), kviter->second.c_str());
            } else {
                if (std::fmod(dbl_value, 1.) == 0) {
                    BSON_APPEND_INT32(&p, kviter->first.c_str(), CVT_INT(dbl_value));
                } else {
                    BSON_APPEND_DOUBLE(&p, kviter->first.c_str(), dbl_value);
                }
            }
        }
    }
    char* buf = reinterpret_cast<char *>(values);
    int buflength = datalength * sizeof(T);
    bool gstatus = gfs->WriteStreamData(filename, buf, buflength, &p);
    bson_destroy(&p);
    return gstatus;
}

#endif /* USE_MONGODB */

/************* Read functions ***************/
template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::ReadFromFile(const string& filename, const bool calc_pos /* = true */,
                                            clsRasterData<MASK_T>* mask /* = nullptr */,
                                            const bool use_mask_ext /* = true */,
                                            T default_value /* = static_cast<T>(NODATA_VALUE) */,
                                            STRING_MAP opts /* = STRING_MAP() */) {
    if (!CheckRasterFilesExist(filename)) return false;
    if (!initialized_) InitializeRasterClass();
    return ConstructFromSingleFile(filename, calc_pos, mask, use_mask_ext, default_value, opts);
}

#ifdef USE_MONGODB

template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::ReadFromMongoDB(MongoGridFs* gfs,
                                               const string& filename,
                                               const bool calc_pos /* = true */,
                                               clsRasterData<MASK_T>* mask /* = nullptr */,
                                               const bool use_mask_ext /* = true */,
                                               T default_value /* = static_cast<T>(NODATA_VALUE) */,
                                               STRING_MAP opts /* = STRING_MAP() */) {
    // 1. Get stream data and metadata by file name
    char* buf = NULL;
    size_t length;
    if (!gfs->GetStreamData(filename, buf, length, NULL, opts) || 
        NULL == buf) {
        initialized_ = false;
        return false;
    }
    bson_t* bmeta = gfs->GetFileMetadata(filename, NULL, opts);
    InitializeReadFunction(filename, calc_pos, mask, use_mask_ext, default_value, opts);
    // 2. Retrieve raster header values
    const char* raster_headers[8] = {
        HEADER_RS_NCOLS, HEADER_RS_NROWS, HEADER_RS_XLL, HEADER_RS_YLL, HEADER_RS_CELLSIZE,
        HEADER_RS_NODATA, HEADER_RS_LAYERS, HEADER_RS_CELLSNUM
    };
    // Loop the metadata, add to `headers_` or `options_`
    bson_iter_t iter;
    if (NULL != bmeta && bson_iter_init(&iter, bmeta)) {
        while (bson_iter_next(&iter)) {
            const char* key = bson_iter_key(&iter);
            bool is_header = false;
            for (int i = 0; i < 8; i++) {
                if (StringMatch(key, raster_headers[i])) {
                    GetNumericFromBsonIterator(&iter, headers_[raster_headers[i]]);
                    is_header = true;
                    break;
                }
            }
            if (!is_header) {
#ifdef HAS_VARIADIC_TEMPLATES
                options_.emplace(key, GetStringFromBsonIterator(&iter));
#else
                options_.insert(make_pair(key, GetStringFromBsonIterator(&iter)));
#endif
            }
        }
    }
    // Destory bson of metadata immediately after use
    bson_destroy(bmeta);

    // Check if "SRS" is existed in `options_`
    if (options_.find(HEADER_RS_SRS) == options_.end()) {
#ifdef HAS_VARIADIC_TEMPLATES
        options_.emplace(HEADER_RS_SRS, "");
#else
        options_.insert(make_pair(HEADER_RS_SRS, ""));
#endif
    }

    int n_rows = CVT_INT(headers_.at(HEADER_RS_NROWS));
    int n_cols = CVT_INT(headers_.at(HEADER_RS_NCOLS));
    n_cells_ = n_rows * n_cols;
    no_data_value_ = static_cast<T>(headers_.at(HEADER_RS_NODATA));
    n_lyrs_ = CVT_INT(headers_.at(HEADER_RS_LAYERS));

    // CAUTION: Currently data stored in MongoDB is always float. lj
    //  I can not find an elegant way to make it templated.
    assert(n_cells_ == length / sizeof(float) / n_lyrs_);

    int validcount = -1;
    if (headers_.find(HEADER_RS_CELLSNUM) != headers_.end()) {
        validcount = CVT_INT(headers_.at(HEADER_RS_CELLSNUM));
    }
    // 3. Store data.
    // check the valid values count and determine whether can read directly.
    bool re_build_data = true;
    if (validcount <= n_cells_ && calc_pos_ && use_mask_ext_ &&
        nullptr != mask_ && validcount == mask_->GetCellNumber()) {
        re_build_data = false;
        store_pos_ = false;
        mask_->GetRasterPositionData(&n_cells_, &raster_pos_data_);
    }
    // read data directly
    if (n_lyrs_ == 1) {
        float* tmpdata = reinterpret_cast<float *>(buf);
        Initialize1DArray(n_cells_, raster_, no_data_value_);
#pragma omp parallel for
        for (int i = 0; i < n_cells_; i++) {
            int tmpidx = i;
            if (!re_build_data) tmpidx = raster_pos_data_[i][0] * GetCols() + raster_pos_data_[i][1];
            raster_[i] = static_cast<T>(tmpdata[tmpidx]);
        }
        Release1DArray(tmpdata);
        is_2draster = false;
    } else {
        float* tmpdata = reinterpret_cast<float *>(buf);
        Initialize2DArray(n_cells_, n_lyrs_, raster_2d_, no_data_value_);
#pragma omp parallel for
        for (int i = 0; i < n_cells_; i++) {
            int tmpidx = i;
            if (!re_build_data) tmpidx = raster_pos_data_[i][0] * GetCols() + raster_pos_data_[i][1];
            for (int j = 0; j < n_lyrs_; j++) {
                int idx = tmpidx * n_lyrs_ + j;
                raster_2d_[i][j] = static_cast<T>(tmpdata[idx]);
            }
        }
        is_2draster = true;
        Release1DArray(tmpdata);
    }
    buf = nullptr;
    CheckDefaultValue();
    if (re_build_data) {
        MaskAndCalculateValidPosition();
    }
    return true;
}

#endif /* USE_MONGODB */

template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::ReadAscFile(const string& asc_filename, map<string, double>* header,
                                           T** values) {
    StatusMessage(("Read " + asc_filename + "...").c_str());
    std::ifstream raster_file(asc_filename.c_str());
    string tmp, xlls, ylls;
    T no_data;
    double tmp_value;
    int rows, cols;
    map<string, double> tmpheader;
    // read header
#ifdef HAS_VARIADIC_TEMPLATES
    raster_file >> tmp >> cols;
    tmpheader.emplace(HEADER_RS_NCOLS, CVT_DBL(cols));
    raster_file >> tmp >> rows;
    tmpheader.emplace(HEADER_RS_NROWS, CVT_DBL(rows));
    raster_file >> xlls >> tmp_value;
    tmpheader.emplace(HEADER_RS_XLL, tmp_value);
    raster_file >> ylls >> tmp_value;
    tmpheader.emplace(HEADER_RS_YLL, tmp_value);
    raster_file >> tmp >> tmp_value;
    tmpheader.emplace(HEADER_RS_CELLSIZE, tmp_value);
    raster_file >> tmp >> no_data;
    tmpheader.emplace(HEADER_RS_NODATA, no_data);
#else
    raster_file >> tmp >> cols;
    tmpheader.insert(make_pair(HEADER_RS_NCOLS, CVT_DBL(cols)));
    raster_file >> tmp >> rows;
    tmpheader.insert(make_pair(HEADER_RS_NROWS, CVT_DBL(rows)));
    raster_file >> xlls >> tmp_value;
    tmpheader.insert(make_pair(HEADER_RS_XLL, tmp_value));
    raster_file >> ylls >> tmp_value;
    tmpheader.insert(make_pair(HEADER_RS_YLL, tmp_value));
    raster_file >> tmp >> tmp_value;
    tmpheader.insert(make_pair(HEADER_RS_CELLSIZE, tmp_value));
    raster_file >> tmp >> no_data;
    tmpheader.insert(make_pair(HEADER_RS_NODATA, no_data));
#endif
    no_data_value_ = no_data;
    // default is center, if corner, then:
    if (StringMatch(xlls, "XLLCORNER")) tmpheader.at(HEADER_RS_XLL) += 0.5 * tmpheader.at(HEADER_RS_CELLSIZE);
    if (StringMatch(ylls, "YLLCORNER")) tmpheader.at(HEADER_RS_YLL) += 0.5 * tmpheader.at(HEADER_RS_CELLSIZE);
#ifdef HAS_VARIADIC_TEMPLATES
    tmpheader.emplace(HEADER_RS_LAYERS, 1.);
    tmpheader.emplace(HEADER_RS_CELLSNUM, -1.);
#else
    tmpheader.insert(make_pair(HEADER_RS_LAYERS, 1.));
    tmpheader.insert(make_pair(HEADER_RS_CELLSNUM, -1.));
#endif
    // get all raster values (i.e., include NODATA_VALUE, m_excludeNODATA = False)
    T* tmprasterdata = new T[rows * cols];
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            raster_file >> tmp_value;
            tmprasterdata[i * cols + j] = static_cast<T>(tmp_value);
        }
    }
    raster_file.close();
    // returned parameters
    *header = tmpheader;
    *values = tmprasterdata;
    return true;
}

#ifdef USE_GDAL
template <typename T, typename MASK_T>
bool clsRasterData<T, MASK_T>::ReadRasterFileByGdal(const string& filename,
                                                    map<string, double>* header, T** values) {
    StatusMessage(("Read " + filename + "...").c_str());
    GDALDataset* po_dataset = static_cast<GDALDataset *>(GDALOpen(filename.c_str(), GA_ReadOnly));
    if (nullptr == po_dataset) {
        StatusNoUnitTest("Open file " + filename + " failed.");
        return false;
    }
    GDALRasterBand* po_band = po_dataset->GetRasterBand(1);
    map<string, double> tmpheader;
    int n_rows = po_band->GetYSize();
    int n_cols = po_band->GetXSize();
    no_data_value_ = static_cast<T>(po_band->GetNoDataValue());
    double geo_trans[6];
    po_dataset->GetGeoTransform(geo_trans);
#ifdef HAS_VARIADIC_TEMPLATES
    tmpheader.emplace(HEADER_RS_NCOLS, CVT_DBL(n_cols));
    tmpheader.emplace(HEADER_RS_NROWS, CVT_DBL(n_rows));
    tmpheader.emplace(HEADER_RS_NODATA, po_band->GetNoDataValue());
    tmpheader.emplace(HEADER_RS_CELLSIZE, geo_trans[1]);
    tmpheader.emplace(HEADER_RS_XLL, geo_trans[0] + 0.5 * tmpheader.at(HEADER_RS_CELLSIZE));
    tmpheader.emplace(HEADER_RS_YLL,
                      geo_trans[3] + (tmpheader.at(HEADER_RS_NROWS) - 0.5) * geo_trans[5]);
    tmpheader.emplace(HEADER_RS_LAYERS, 1.);
    tmpheader.emplace(HEADER_RS_CELLSNUM, -1.);
    options_.emplace(HEADER_RS_SRS, string(po_dataset->GetProjectionRef()));
#else
    tmpheader.insert(make_pair(HEADER_RS_NCOLS, CVT_DBL(n_cols)));
    tmpheader.insert(make_pair(HEADER_RS_NROWS, CVT_DBL(n_rows)));
    tmpheader.insert(make_pair(HEADER_RS_NODATA, po_band->GetNoDataValue()));
    tmpheader.insert(make_pair(HEADER_RS_CELLSIZE, geo_trans[1]));
    tmpheader.insert(make_pair(HEADER_RS_XLL, geo_trans[0] + 0.5 * tmpheader.at(HEADER_RS_CELLSIZE)));
    tmpheader.insert(make_pair(HEADER_RS_YLL,
                               geo_trans[3] + (tmpheader.at(HEADER_RS_NROWS) - 0.5) * geo_trans[5]));
    tmpheader.insert(make_pair(HEADER_RS_LAYERS, 1.));
    tmpheader.insert(make_pair(HEADER_RS_CELLSNUM, -1.));
    options_.insert(make_pair(HEADER_RS_SRS, string(po_dataset->GetProjectionRef())));
#endif
    // get all raster values (i.e., include NODATA_VALUE)
    int fullsize_n = n_rows * n_cols;
    if (n_cells_ < 0) {
        // if n_cells_ has been assigned
        n_cells_ = fullsize_n;
    }
    T* tmprasterdata = new T[fullsize_n];
    GDALDataType data_type = po_band->GetRasterDataType();
    char* char_data = nullptr;
    unsigned char* uchar_data = nullptr;
    vuint16_t* uint16_data = nullptr; // 16-bit unsigned integer
    vint16_t* int16_data = nullptr;   // 16-bit signed integer
    vuint32_t* uint32_data = nullptr; // 32-bit unsigned integer
    vint32_t* int32_data = nullptr;   // 32-bit signed integer
    float* float_data = nullptr;
    double* double_data = nullptr;
    CPLErr result;
    switch (data_type) {
        case GDT_Byte:
            // For GDAL, GDT_Byte is 8-bit unsigned interger, ranges from 0 to 255.
            // However, ArcGIS use 8-bit signed and unsigned intergers which both will be read as GDT_Byte.
            //   8-bit signed integer ranges from -128 to 127.
            // Since both signed and unsigned integers of n bits in length can represent 2^n different values,
            //   there is no inherent way to distinguish signed integers from unsigned integers simply by looking
            //   at them; the software designer is responsible for using them correctly.
            // So, here I can only assume that a negative nodata indicates a 8-bit signed integer type.
            if (no_data_value_ < 0) {
                // commonly -128
                char_data = static_cast<char *>(CPLMalloc(sizeof(char) * n_cols * n_rows));
                result = po_band->RasterIO(GF_Read, 0, 0, n_cols, n_rows, char_data,
                                           n_cols, n_rows, GDT_Byte, 0, 0);
                if (result != CE_None) {
                    cout << "RaterIO trouble: " << CPLGetLastErrorMsg() << endl;
                } else {
                    Copy2DRasterDataTo1DArray(tmprasterdata, char_data, n_rows, n_cols);
                    CPLFree(char_data);
                }
            } else {
                // commonly 255
                uchar_data = static_cast<unsigned char *>(CPLMalloc(sizeof(unsigned char) * n_cols * n_rows));
                result = po_band->RasterIO(GF_Read, 0, 0, n_cols, n_rows, uchar_data,
                                           n_cols, n_rows, GDT_Byte, 0, 0);
                if (result != CE_None) {
                    cout << "RaterIO trouble: " << CPLGetLastErrorMsg() << endl;
                } else {
                    Copy2DRasterDataTo1DArray(tmprasterdata, uchar_data, n_rows, n_cols);
                    CPLFree(uchar_data);
                }
            }
            break;
        case GDT_UInt16: uint16_data = static_cast<vuint16_t *>(CPLMalloc(sizeof(vuint16_t) * n_cols * n_rows));
            result = po_band->RasterIO(GF_Read, 0, 0, n_cols, n_rows, uint16_data,
                                       n_cols, n_rows, GDT_UInt16, 0, 0);
            if (result != CE_None) {
                cout << "RaterIO trouble: " << CPLGetLastErrorMsg() << endl;
            } else {
                Copy2DRasterDataTo1DArray(tmprasterdata, uint16_data, n_rows, n_cols);
                CPLFree(uint16_data);
            }
            break;
        case GDT_Int16: int16_data = static_cast<vint16_t *>(CPLMalloc(sizeof(vint16_t) * n_cols * n_rows));
            result = po_band->RasterIO(GF_Read, 0, 0, n_cols, n_rows, int16_data,
                                       n_cols, n_rows, GDT_Int16, 0, 0);
            if (result != CE_None) {
                cout << "RaterIO trouble: " << CPLGetLastErrorMsg() << endl;
            } else {
                Copy2DRasterDataTo1DArray(tmprasterdata, int16_data, n_rows, n_cols);
                CPLFree(int16_data);
            }
            break;
        case GDT_UInt32: uint32_data = static_cast<vuint32_t *>(CPLMalloc(sizeof(vuint32_t) * n_cols * n_rows));
            result = po_band->RasterIO(GF_Read, 0, 0, n_cols, n_rows, uint32_data,
                                       n_cols, n_rows, GDT_UInt32, 0, 0);
            if (result != CE_None) {
                cout << "RaterIO trouble: " << CPLGetLastErrorMsg() << endl;
            } else {
                Copy2DRasterDataTo1DArray(tmprasterdata, uint32_data, n_rows, n_cols);
                CPLFree(uint32_data);
            }
            break;
        case GDT_Int32: int32_data = static_cast<vint32_t *>(CPLMalloc(sizeof(vint32_t) * n_cols * n_rows));
            result = po_band->RasterIO(GF_Read, 0, 0, n_cols, n_rows, int32_data,
                                       n_cols, n_rows, GDT_Int32, 0, 0);
            if (result != CE_None) {
                cout << "RaterIO trouble: " << CPLGetLastErrorMsg() << endl;
            } else {
                Copy2DRasterDataTo1DArray(tmprasterdata, int32_data, n_rows, n_cols);
                CPLFree(int32_data);
            }
            break;
        case GDT_Float32: float_data = static_cast<float *>(CPLMalloc(sizeof(float) * n_cols * n_rows));
            result = po_band->RasterIO(GF_Read, 0, 0, n_cols, n_rows, float_data,
                                       n_cols, n_rows, GDT_Float32, 0, 0);
            if (result != CE_None) {
                cout << "RaterIO trouble: " << CPLGetLastErrorMsg() << endl;
            } else {
                Copy2DRasterDataTo1DArray(tmprasterdata, float_data, n_rows, n_cols);
                CPLFree(float_data);
            }
            break;
        case GDT_Float64: double_data = static_cast<double *>(CPLMalloc(sizeof(double) * n_cols * n_rows));
            result = po_band->RasterIO(GF_Read, 0, 0, n_cols, n_rows, double_data,
                                       n_cols, n_rows, GDT_Float64, 0, 0);
            if (result != CE_None) {
                cout << "RaterIO trouble: " << CPLGetLastErrorMsg() << endl;
            } else {
                Copy2DRasterDataTo1DArray(tmprasterdata, double_data, n_rows, n_cols);
                CPLFree(double_data);
            }
            break;
        default: cout << "Unexpected GDALDataType: " << GDALGetDataTypeName(data_type) << endl;
            exit(-1);
    }
    GDALClose(po_dataset);
    // returned parameters
    *header = tmpheader;
    *values = tmprasterdata;
    return true;
}
#endif /* USE_GDAL */

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::AddOtherLayerRasterData(const int row, const int col,
                                                       const int cellidx, const int lyr,
                                                       map<string, double> lyrheader, T* lyrdata) {
    int tmpcols = CVT_INT(lyrheader.at(HEADER_RS_NCOLS));
    XY_COOR tmp_xy = GetCoordinateByRowCol(row, col);
    // get current raster layer's value by XY
    ROW_COL tmp_pos = GetPositionByCoordinate(tmp_xy.first, tmp_xy.second, &lyrheader);
    if (tmp_pos.first == -1 || tmp_pos.second == -1) {
        raster_2d_[cellidx][lyr] = no_data_value_;
    } else {
        raster_2d_[cellidx][lyr] = lyrdata[tmp_pos.first * tmpcols + tmp_pos.second];
    }
}

template <typename T, typename MASK_T>
clsRasterData<T, MASK_T>::clsRasterData(clsRasterData<T, MASK_T>* another) {
    InitializeRasterClass();
    Copy(another);
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::Copy(const clsRasterData<T, MASK_T>* orgraster) {
    if (is_2draster && nullptr != raster_2d_ && n_cells_ > 0) {
        Release2DArray(n_cells_, raster_2d_);
    }
    if (!is_2draster && nullptr != raster_) {
        Release1DArray(raster_);
    }
    if (nullptr != raster_pos_data_) {
        Release2DArray(2, raster_pos_data_);
    }
    if (stats_calculated_) {
        ReleaseStatsMap2D();
        stats_calculated_ = false;
    }
    InitializeRasterClass();
    full_path_ = orgraster->GetFilePath();
    core_name_ = orgraster->GetCoreName();
    n_cells_ = orgraster->GetCellNumber();
    n_lyrs_ = orgraster->GetLayers();
    no_data_value_ = orgraster->GetNoDataValue();
    default_value_ = orgraster->GetDefaultValue();
    if (orgraster->Is2DRaster()) {
        is_2draster = true;
        Initialize2DArray(n_cells_, n_lyrs_, raster_2d_, orgraster->Get2DRasterDataPointer());
    } else {
        raster_ = nullptr;
        Initialize1DArray(n_cells_, raster_, orgraster->GetRasterDataPointer());
    }
    mask_ = orgraster->GetMask();
    calc_pos_ = orgraster->PositionsCalculated();
    if (calc_pos_) {
        store_pos_ = true;
        Initialize2DArray(n_cells_, 2, raster_pos_data_, orgraster->GetRasterPositionDataPointer());
    }
    use_mask_ext_ = orgraster->MaskExtented();
    stats_calculated_ = orgraster->StatisticsCalculated();
    if (stats_calculated_) {
        if (is_2draster) {
            if (!stats_2d_.empty()) ReleaseStatsMap2D();
            map<string, double *> stats2D = orgraster->GetStatistics2D();
            for (auto iter = stats2D.begin(); iter != stats2D.end(); ++iter) {
                double* tmpstatvalues = nullptr;
                Initialize1DArray(n_lyrs_, tmpstatvalues, iter->second);
#ifdef HAS_VARIADIC_TEMPLATES
                stats_2d_.emplace(iter->first, tmpstatvalues);
#else
                stats_2d_.insert(make_pair(iter->first, tmpstatvalues));
#endif
            }
        } else {
            map<string, double> stats = orgraster->GetStatistics();
            for (auto iter = stats.begin(); iter != stats.end(); ++iter) {
                stats_[iter->first] = iter->second;
            }
        }
    }
    CopyHeader(orgraster->GetRasterHeader());
    CopyStringMap(orgraster->GetOptions(), options_);
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::ReplaceNoData(T replacedv) {
    if (is_2draster && nullptr != raster_2d_) {
#pragma omp parallel for
        for (int i = 0; i < n_cells_; i++) {
            for (int lyr = 0; lyr < n_lyrs_; lyr++) {
                if (FloatEqual(raster_2d_[i][lyr], no_data_value_)) {
                    raster_2d_[i][lyr] = replacedv;
                }
            }
        }
    } else if (nullptr != raster_) {
#pragma omp parallel for
        for (int i = 0; i < n_cells_; i++) {
            if (FloatEqual(raster_[i], no_data_value_)) {
                raster_[i] = replacedv;
            }
        }
    }
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::Reclassify(map<int, T> reclass_map) {
    if (is_2draster && nullptr != raster_2d_) {
#pragma omp parallel for
        for (int i = 0; i < n_cells_; i++) {
            for (int lyr = 0; lyr < n_lyrs_; lyr++) {
                auto iter = reclass_map.find(CVT_INT(raster_2d_[i][lyr]));
                if (iter != reclass_map.end()) {
                    raster_2d_[i][lyr] = iter->second;
                } else {
                    raster_2d_[i][lyr] = no_data_value_;
                }
            }
        }
    } else if (nullptr != raster_) {
#pragma omp parallel for
        for (int i = 0; i < n_cells_; i++) {
            auto iter = reclass_map.find(CVT_INT(raster_[i]));
            if (iter != reclass_map.end()) {
                raster_[i] = iter->second;
            } else {
                raster_[i] = no_data_value_;
            }
        }
    }
}

/************* Utility functions ***************/

template <typename T, typename MASK_T>
XY_COOR clsRasterData<T, MASK_T>::GetCoordinateByRowCol(const int row, const int col) {
    return XY_COOR(GetXllCenter() + col * GetCellWidth(),
                   GetYllCenter() + (GetRows() - row - 1) * GetCellWidth());
}

template <typename T, typename MASK_T>
ROW_COL clsRasterData<T, MASK_T>::GetPositionByCoordinate(const double x, const double y,
                                                          map<string, double>* header /* = nullptr */) {
    if (nullptr == header) {
        header = &headers_;
    }
    double xll_center = (*header).at(HEADER_RS_XLL);
    double yll_center = (*header).at(HEADER_RS_YLL);
    double dx = (*header).at(HEADER_RS_CELLSIZE);
    double dy = dx;
    int n_rows = CVT_INT((*header).at(HEADER_RS_NROWS));
    int n_cols = CVT_INT((*header).at(HEADER_RS_NCOLS));

    double x_min = xll_center - dx / 2.;
    double x_max = x_min + dx * n_cols;

    double y_min = yll_center - dy / 2.;
    double y_max = y_min + dy * n_rows;
    if ((x > x_max || x < xll_center) || (y > y_max || y < yll_center)) {
        return ROW_COL(-1, -1);
    }
    return ROW_COL(CVT_INT((y_max - y) / dy), CVT_INT((x - x_min) / dx));
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::CopyHeader(const map<string, double>& refers) {
    for (auto iter = refers.begin(); iter != refers.end(); ++iter) {
        headers_[iter->first] = iter->second;
    }
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::CalculateValidPositionsFromGridDate() {
    int oldcellnumber = n_cells_;
    // initial vectors
    vector<T> values;
    vector<vector<T> > values_2d; // store layer 2~n
    vector<int> pos_rows;
    vector<int> pos_cols;
    int nrows = CVT_INT(headers_.at(HEADER_RS_NROWS));
    int ncols = CVT_INT(headers_.at(HEADER_RS_NCOLS));
    // get all valid values (i.e., exclude NODATA_VALUE)
    for (int i = 0; i < nrows; ++i) {
        for (int j = 0; j < ncols; ++j) {
            int idx = i * ncols + j;
            T tmp_value;
            if (is_2draster) {
                tmp_value = raster_2d_[idx][0];
            } else {
                tmp_value = raster_[idx];
            }
            if (FloatEqual(tmp_value, static_cast<T>(no_data_value_))) continue;
            values.emplace_back(tmp_value);
            if (is_2draster && n_lyrs_ > 1) {
                vector<T> tmpv(n_lyrs_ - 1);
                for (int lyr = 1; lyr < n_lyrs_; lyr++) {
                    tmpv[lyr - 1] = raster_2d_[idx][lyr];
                }
                values_2d.emplace_back(tmpv);
            }
            pos_rows.emplace_back(i);
            pos_cols.emplace_back(j);
        }
    }
    // swap vector to save memory
    vector<T>(values).swap(values);
    if (is_2draster && n_lyrs_ > 1) {
        vector<vector<T> >(values_2d).swap(values_2d);
    }
    vector<int>(pos_rows).swap(pos_rows);
    vector<int>(pos_cols).swap(pos_cols);
    // reCreate raster data array
    n_cells_ = CVT_INT(values.size());
    headers_.at(HEADER_RS_CELLSNUM) = n_cells_;
    if (is_2draster) {
        Release2DArray(oldcellnumber, raster_2d_);
        Initialize2DArray(n_cells_, n_lyrs_, raster_2d_, no_data_value_);
    } else {
        Release1DArray(raster_);
        Initialize1DArray(n_cells_, raster_, no_data_value_);
    }

    // raster_pos_data_ is nullptr till now.
    Initialize2DArray(n_cells_, 2, raster_pos_data_, 0);
    store_pos_ = true;
#pragma omp parallel for
    for (int i = 0; i < n_cells_; ++i) {
        if (is_2draster) {
            raster_2d_[i][0] = values.at(i);
            if (n_lyrs_ > 1) {
                for (int lyr = 1; lyr < n_lyrs_; lyr++) {
                    raster_2d_[i][lyr] = values_2d[i][lyr - 1];
                }
            }
        } else {
            raster_[i] = values.at(i);
        }
        raster_pos_data_[i][0] = pos_rows.at(i);
        raster_pos_data_[i][1] = pos_cols.at(i);
    }
    calc_pos_ = true;
}

template <typename T, typename MASK_T>
void clsRasterData<T, MASK_T>::MaskAndCalculateValidPosition() {
    int oldcellnumber = n_cells_;
    if (nullptr == mask_) {
        if (calc_pos_) {
            CalculateValidPositionsFromGridDate();
        } else {
            n_cells_ = GetRows() * GetCols();
            headers_.at(HEADER_RS_CELLSNUM) = n_cells_;
            // do nothing
        }
        return;
    }
    // Use mask data
    // 1. Get new values and positions according to Mask's position data
    vector<T> values;             ///< store layer 1 data
    vector<vector<T> > values_2d; ///< store layer 2~n data (excluding the first layerS)
    vector<int> pos_rows;
    vector<int> pos_cols;
    int cols = CVT_INT(headers_.at(HEADER_RS_NCOLS));
    int n_valid_mask_number;
    int** valid_pos = nullptr;
    int mask_rows = mask_->GetRows();
    int mask_cols = mask_->GetCols();
    int mask_cells = mask_rows * mask_cols;
    // Get the position data from mask
    mask_->GetRasterPositionData(&n_valid_mask_number, &valid_pos);
    // calculate the interect extent between mask and the raster data
    int max_row = -1;
    int min_row = mask_rows;
    int max_col = -1;
    int min_col = mask_rows;
    // Get the valid data according to coordinate
    for (int i = 0; i < n_valid_mask_number; i++) {
        int tmp_row = valid_pos[i][0];
        int tmp_col = valid_pos[i][1];
        XY_COOR tmp_xy = mask_->GetCoordinateByRowCol(tmp_row, tmp_col);
        // get current raster value by XY
        ROW_COL tmp_pos = GetPositionByCoordinate(tmp_xy.first, tmp_xy.second);
        T tmp_value;
        // If the mask location exceeds the extent of raster data, set to no_data_value_.
        if (tmp_pos.first == -1 || tmp_pos.second == -1) {
            tmp_value = no_data_value_;
            if (is_2draster && n_lyrs_ > 1) {
                vector<T> tmp_values(n_lyrs_ - 1);
                for (int lyr = 1; lyr < n_lyrs_; lyr++) tmp_values[lyr - 1] = no_data_value_;
                values_2d.emplace_back(tmp_values);
            }
            values.emplace_back(tmp_value);
            pos_rows.emplace_back(tmp_row);
            pos_cols.emplace_back(tmp_col);
            continue;
        }
        if (is_2draster) {
            tmp_value = raster_2d_[tmp_pos.first * cols + tmp_pos.second][0];
            if (n_lyrs_ > 1) {
                vector<T> tmp_values(n_lyrs_ - 1);
                for (int lyr = 1; lyr < n_lyrs_; lyr++) {
                    tmp_values[lyr - 1] = raster_2d_[tmp_pos.first * cols + tmp_pos.second][lyr];
                    if (FloatEqual(tmp_values[lyr - 1], no_data_value_)) {
                        tmp_values[lyr - 1] = default_value_;
                    }
                }
                values_2d.emplace_back(tmp_values);
            }
        } else {
            tmp_value = raster_[tmp_pos.first * cols + tmp_pos.second];
        }
        if (FloatEqual(tmp_value, no_data_value_)) {
            tmp_value = default_value_;
        } else {
            // the intersect extents dependent on the valid raster values
            if (max_row < tmp_row) max_row = tmp_row;
            if (min_row > tmp_row) min_row = tmp_row;
            if (max_col < tmp_col) max_col = tmp_col;
            if (min_col > tmp_col) min_col = tmp_col;
        }
        values.emplace_back(tmp_value);
        pos_rows.emplace_back(tmp_row);
        pos_cols.emplace_back(tmp_col);
    }
    // swap vector to save memory
    vector<T>(values).swap(values);
    if (is_2draster && n_lyrs_ > 1) {
        vector<vector<T> >(values_2d).swap(values_2d);
        for (auto iter = values_2d.begin(); iter != values_2d.end(); ++iter) {
            vector<T>(*iter).swap(*iter);
        }
        assert(values_2d.size() == values.size());
    }
    vector<int>(pos_rows).swap(pos_rows);
    vector<int>(pos_cols).swap(pos_cols);
    assert(values.size() == pos_rows.size());
    assert(values.size() == pos_cols.size());

    if (!mask_->PositionsCalculated()) Release2DArray(mask_cells, valid_pos);

    // 2. Handing the header information
    // Is the valid grid extent same as the mask data?
    bool same_extent_with_mask = true;
    int new_rows = max_row - min_row + 1;
    int new_cols = max_col - min_col + 1;
    if (new_rows != mask_->GetRows() || new_cols != mask_->GetCols()) {
        same_extent_with_mask = false;
    }
    // 2.2a Copy header of mask data
    CopyHeader(mask_->GetRasterHeader());
    // 2.2b  ReCalculate the header based on the mask's header
    if (!use_mask_ext_ && !same_extent_with_mask) {
        // ||((m_useMaskExtent || sameExtentWithMask) && m_calcPositions && !m_mask->PositionsCalculated())) {
        headers_.at(HEADER_RS_NCOLS) = CVT_DBL(new_cols);
        headers_.at(HEADER_RS_NROWS) = CVT_DBL(new_rows);
        headers_.at(HEADER_RS_XLL) += min_col * mask_->GetCellWidth();
        headers_.at(HEADER_RS_YLL) += (mask_->GetRows() - 1 - max_row) * mask_->GetCellWidth();
        headers_.at(HEADER_RS_CELLSIZE) = mask_->GetCellWidth();
        // clean redundant values (i.e., NODATA)
        auto rit = pos_rows.begin();
        auto cit = pos_cols.begin();
        auto vit = values.begin();
        auto data2dit = values_2d.begin();
        for (auto it = values.begin(); it != values.end();) {
            int idx = CVT_INT(distance(vit, it));
            int tmpr = pos_rows.at(idx);
            int tmpc = pos_cols.at(idx);

            if (tmpr > max_row || tmpr < min_row || tmpc > max_col || tmpc < min_col
                || FloatEqual(static_cast<T>(*it), no_data_value_)) {
                it = values.erase(it);
                if (is_2draster && n_lyrs_ > 1) {
                    values_2d.erase(data2dit + idx);
                    data2dit = values_2d.begin();
                }
                pos_cols.erase(cit + idx);
                pos_rows.erase(rit + idx);
                // reset the iterators
                rit = pos_rows.begin();
                cit = pos_cols.begin();
                vit = values.begin();
            } else {
                // get new column and row number
                pos_rows[idx] -= min_row;
                pos_cols[idx] -= min_col;
                ++it;
            }
        }
        // swap vector to save memory
        vector<T>(values).swap(values);
        if (is_2draster && n_lyrs_ > 1) {
            vector<vector<T> >(values_2d).swap(values_2d);
        }
        vector<int>(pos_rows).swap(pos_rows);
        vector<int>(pos_cols).swap(pos_cols);
    }
    // 2.3 Handling NoData, SRS, and Layers
    headers_.at(HEADER_RS_NODATA) = no_data_value_; ///< avoid to assign the Mask's NODATA
    ///< use the coordinate system of mask data
    if (options_.find(HEADER_RS_SRS) == options_.end()) {
#ifdef HAS_VARIADIC_TEMPLATES
        options_.emplace(HEADER_RS_SRS, mask_->GetSrsString());
#else
        options_.insert(make_pair(HEADER_RS_SRS, mask_->GetSrsString()));
#endif
    } else {
        options_[HEADER_RS_SRS] = mask_->GetSrsString();
    }
    headers_.at(HEADER_RS_LAYERS) = n_lyrs_;
    // 3. Create new raster data, and handling positions data
    // 3.1 Determine the n_cells_, and whether to allocate new position data space
    bool store_fullsize_array = false;
    if ((use_mask_ext_ || same_extent_with_mask) && calc_pos_) {
        mask_->GetRasterPositionData(&n_cells_, &raster_pos_data_);
        store_pos_ = false;
    } else if ((!use_mask_ext_ && !same_extent_with_mask && !calc_pos_) ||
        (((use_mask_ext_ || same_extent_with_mask) && (!calc_pos_)))) {
        // reStore raster values as fullsize array
        n_cells_ = GetCols() * GetRows();
        store_fullsize_array = true;
        store_pos_ = false;
    } else {
        // reCalculate raster positions data and store
        store_pos_ = true;
        n_cells_ = CVT_INT(values.size());
    }
    headers_.at(HEADER_RS_CELLSNUM) = n_cells_;

    // 3.2 Release the original raster values, and create new
    //     raster array and positions data array (if necessary)
    assert(ValidateRasterData());
    if (is_2draster && nullptr != raster_2d_) {
        // multiple layers
        Release2DArray(oldcellnumber, raster_2d_);
        Initialize2DArray(n_cells_, n_lyrs_, raster_2d_, no_data_value_);
    } else {
        // single layer
        Release1DArray(raster_);
        Initialize1DArray(n_cells_, raster_, no_data_value_);
    }
    if (store_pos_) Initialize2DArray(n_cells_, 2, raster_pos_data_, 0);

    // 3.3 Loop the masked raster values
    int ncols = CVT_INT(headers_.at(HEADER_RS_NCOLS));
    int synthesis_idx = -1;
    for (size_t k = 0; k < pos_rows.size(); ++k) {
        if (store_fullsize_array) {
            synthesis_idx = pos_rows.at(k) * ncols + pos_cols.at(k);
        } else if (store_pos_ && !FloatEqual(values.at(k), no_data_value_)) {
            synthesis_idx++;
            raster_pos_data_[synthesis_idx][0] = pos_rows.at(k);
            raster_pos_data_[synthesis_idx][1] = pos_cols.at(k);
        } else {
            synthesis_idx++;
        }
        if (is_2draster) {
            // multiple layers
            raster_2d_[synthesis_idx][0] = values.at(k);
            if (n_lyrs_ > 1) {
                for (int lyr = 1; lyr < n_lyrs_; lyr++) {
                    raster_2d_[synthesis_idx][lyr] = values_2d[k][lyr - 1];
                }
            }
        } else {
            // single layer
            raster_[synthesis_idx] = values.at(k);
        }
    }
}

} /* namespace: data_raster */
} /* namespace: ccgl */
#endif /* CCGL_DATA_RASTER_H */
