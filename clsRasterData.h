/*!
 * \brief Define Raster class to handle raster data
 *
 *        1. Using GDAL and MongoDB (currently, mongo-c-driver 1.5.0+ is supported)
 *        2. Array1D and Array2D raster data are supported
 *        3. C++11 supported
 *        4. Unit Tests based Google Test.
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.1
 * \date Apr. 2011
 * \revised May. 2016
 *          Dec. 2016 lj Separated from SEIMS to a common library for widely use.
 *          Mar. 2017 lj VLD check, bug fixed, function enhanced.
 *          Apr. 2017 lj Avoid try...catch block
 *          May. 2017 lj Use MongoDB wrapper
 *          Nov. 2017 lj Code review based on C++11 and use a single header file.
 *          Dec. 2017 lj Add Unittest based on Google Test.
 */
#ifndef CLS_RASTER_DATA
#define CLS_RASTER_DATA

/// include MongoDB, optional
#ifdef USE_MONGODB

#include "MongoUtil.h"

#endif /* USE_MONGODB */
/// include utility functions
#include "utilities.h"
/// include GDAL, required
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"
/// include openmp if supported
#ifdef SUPPORT_OMP

#include <omp.h>

#endif /* SUPPORT_OMP */
/// include base headers
#include <cstdint>
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
#include <typeinfo>

using namespace std;

/* Ignore warning on Windows MSVC compiler caused by GDAL.
 * refers to http://blog.csdn.net/liminlu0314/article/details/8227518
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable: 4100 4190 4251 4275 4305 4309 4819 4996)
#endif /* Ignore warnings of GDAL */

/*!
 * Define Raster related constant strings used for raster headers
 */
#define HEADER_RS_NODATA        "NODATA_VALUE"
#define HEADER_RS_XLL           "XLLCENTER"  /// or XLLCORNER
#define HEADER_RS_YLL           "YLLCENTER"  /// or YLLCORNER
#define HEADER_RS_NROWS         "NROWS"
#define HEADER_RS_NCOLS         "NCOLS"
#define HEADER_RS_CELLSIZE      "CELLSIZE"
#define HEADER_RS_LAYERS        "LAYERS"
#define HEADER_RS_CELLSNUM      "CELLSNUM"
#define HEADER_RS_SRS           "SRS"

/*!
 * Define constant strings of statistics index
 */
#define STATS_RS_VALIDNUM        "VALID_CELLNUMBER"
#define STATS_RS_MEAN            "MEAN"
#define STATS_RS_MIN             "MIN"
#define STATS_RS_MAX             "MAX"
#define STATS_RS_STD             "STD"
#define STATS_RS_RANGE           "RANGE"

/*!
 * Files or database constant strings
 */
#define ASCIIExtension          "asc"
#define GTiffExtension          "tif"

typedef pair<int, int> RowCol;
typedef pair<double, double> XYCoor;

/** Common functions independent to clsRasterData **/
inline void print_status(string status_str) {
#ifndef UNITTEST
    cout << status_str << endl;
#endif
}

/*!
 * \brief check the existence of given raster file
 */
inline bool _check_raster_files_exist(const string &filename) {
    if (!FileExists(filename)) {
        print_status("The raster file " + filename + " does not exist or has not read permission.");
        return false;
    }
    return true;
}

/*!
 * \brief check the existence of given vector of raster files
 * \return True if all existed, else false
 */
inline bool _check_raster_files_exist(vector<string> &filenames) {
    for (auto it = filenames.begin(); it != filenames.end(); it++) {
        if (!_check_raster_files_exist(*it)) {
            return false;
        }
    }
    return true;
}

template<typename T1, typename T2>
inline void _get_data_from_gdal(T1 *dst, T2 *src, int nr, int nc) {
#pragma omp parallel for
    for (int i = 0; i < nr; ++i) {
        for (int j = 0; j < nc; ++j) {
            int index = i * nc + j;
            dst[index] = (T1)src[index];
        }
    }
}
/*!
 * \class clsRasterData
 * \ingroup data
 * \brief Raster data (1D and 2D) I/O class
 * Support I/O between TIFF, ASCII file or/and MongoBD database.
 */
template<typename T, typename MaskT = T>
class clsRasterData {
public:
    /************* Construct functions ***************/
    /*!
     * \brief Constructor an empty clsRasterData instance
     * By default, 1D raster data
     * Set \a m_rasterPositionData, \a m_rasterData, \a m_mask to \a nullptr
     */
    clsRasterData();

    /*!
     * \brief Constructor of clsRasterData instance from TIFF, ASCII, or other GDAL supported raster file
     * \param[in] filename Full path of the raster file
     * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<MaskT> Mask layer
     * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     * \param[in] defalutValue Default value when mask data exceeds the raster extend.
     *
     */
    explicit clsRasterData(const string &filename,
                           bool calcPositions = true,
                           clsRasterData<MaskT> *mask = nullptr,
                           bool useMaskExtent = true,
                           T defalutValue = (T) NODATA_VALUE);

    /*!
     * \brief Validation check before the constructor of clsRasterData,
     *        i.e., the raster file is existed and supported.
     *        This is the recommended way to construct an instance of clsRasterData.
     * \usage
     *       clsRasterData<T, MaskT> *rs = clsRasterData<T, MaskT>::Init(filename)
     *       if (nullptr == rs) {
     *           throw exception("clsRasterData initialization failed!");
     *           // or other error handling code.
     *       }
     */
    static clsRasterData<T, MaskT> *Init(const string &filename,
                                         bool calcPositions = true,
                                         clsRasterData<MaskT> *mask = nullptr,
                                         bool useMaskExtent = true,
                                         T defalutValue = (T) NODATA_VALUE);

    /*!
     * \brief Constructor of clsRasterData instance from TIFF, ASCII, or other GDAL supported raster file
     * Support 1D and/or 2D raster data
     * \sa ReadASCFile() ReadByGDAL()
     * \param[in] filenames Full paths vector of the 2D raster data
     * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<MaskT> Mask layer
     * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     */
    explicit clsRasterData(vector<string> &filenames,
                           bool calcPositions = true,
                           clsRasterData<MaskT> *mask = nullptr,
                           bool useMaskExtent = true,
                           T defalutValue = (T) NODATA_VALUE);

    /*!
     * \brief Validation check before the constructor of clsRasterData,
     *        i.e., the raster files are all existed and supported.
     */
    static clsRasterData<T, MaskT> *Init(vector<string> &filenames,
                                         bool calcPositions = true,
                                         clsRasterData<MaskT> *mask = nullptr,
                                         bool useMaskExtent = true,
                                         T defalutValue = (T) NODATA_VALUE);

    /*!
     * \brief Construct an clsRasterData instance by 1D array data and mask
     */
    clsRasterData(clsRasterData<MaskT> *mask, const T *values);

    /*!
     * \brief Construct an clsRasterData instance by 2D array data and mask
     */
    clsRasterData(clsRasterData<MaskT> *mask, const T * const *values, int lyrs);

#ifdef USE_MONGODB

    /*!
     * \brief Constructor based on mongoDB
     * \sa ReadFromMongoDB()
     *
     * \param[in] gfs \a mongoc_gridfs_t
     * \param[in] romoteFilename \a char*
     * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<MaskT> Mask layer
     * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     */
    clsRasterData(MongoGridFS *gfs, const char *remoteFilename, bool calcPositions = true,
                  clsRasterData<MaskT> *mask = nullptr, bool useMaskExtent = true,
                  T defalutValue = (T) NODATA_VALUE);

    /*!
     * \brief Validation check before the constructor of clsRasterData.
     */
    static clsRasterData<T, MaskT> *Init(MongoGridFS *gfs, const char *remoteFilename,
                                         bool calcPositions = true,
                                         clsRasterData<MaskT> *mask = nullptr,
                                         bool useMaskExtent = true,
                                         T defalutValue = (T) NODATA_VALUE);

#endif

    /*!
     * \brief Copy constructor
     * \usage
     *         Method 1: clsRasterData<T> newraster(baseraster);
     *
     *         Method 2: clsRasterData<T> newraster;
     *                   newraster.Copy(baseraster);
     *
     *         Method 3: clsRasterData<T>* newraster = new clsRasterData<T>(baseraster);
     *                   delete newraster;
     */
    explicit clsRasterData(clsRasterData<T, MaskT> *another);

    //! Destructor
    ~clsRasterData();

    /************* Read functions ***************/
    /*!
     * \brief Read raster data from file, mask data is optional
     * \param[in] filename \a string
     * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<MaskT>
     * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     */
    bool ReadFromFile(string filename, bool calcPositions = true, clsRasterData<MaskT> *mask = nullptr,
                      bool useMaskExtent = true, T defalutValue = (T) NODATA_VALUE);

#ifdef USE_MONGODB

    /*!
     * \brief Read raster data from MongoDB
     * \param[in] gfs \a mongoc_gridfs_t
     * \param[in] filename \a char*, raster file name
     * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<MaskT>
     * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     */
    bool ReadFromMongoDB(MongoGridFS *gfs,
                         string filename,
                         bool calcPositions = true,
                         clsRasterData<MaskT> *mask = nullptr,
                         bool useMaskExtent = true,
                         T defalutValue = (T) NODATA_VALUE);

#endif /* USE_MONGODB */
    /************* Write functions ***************/

    /*!
     * \brief Write raster to raster file, if 2D raster, output name will be filename_LyrNum
     * \param filename filename with prefix, e.g. ".asc" and ".tif"
     */
    bool outputToFile(string filename);

    /*!
     * \brief Write 1D or 2D raster data into ASC file(s)
     * \param[in] filename \a string, output ASC file path, take the CoreName as prefix
     */
    bool outputASCFile(string filename);

    /*!
     * \brief Write 1D or 2D raster data into TIFF file by GDAL
     * \param[in] filename \a string, output TIFF file path
     */
    bool outputFileByGDAL(string filename);

#ifdef USE_MONGODB

    /*!
     * \brief Write raster data (matrix raster data) into MongoDB
     * \param[in] filename \a string, output file name
     * \param[in] gfs \a mongoc_gridfs_t
     */
    void outputToMongoDB(string filename, MongoGridFS *gfs);

#endif /* USE_MONGODB */

    /************************************************************************/
    /*    Set information functions                                         */
    /************************************************************************/

    //! Set new core file name
    void setCoreName(string name) { m_coreFileName = name; }

    /*!
     * \brief Set value to the given position and layer
     */
    void setValue(int row, int col, T value, int lyr = 1);

    /************************************************************************/
    /*    Get information functions                                         */
    /************************************************************************/

    /*!
     * \brief Calculate basic statistics values in one time
     * Mean, Max, Min, STD, Range, etc.
     */
    void calculateStatistics();

    /*!
     * \brief Force to update basic statistics values
     * Mean, Max, Min, STD, Range, etc.
     */
    void updateStatistics();

    /*!
     * \brief Release statistics map of 2D raster data
     */
    void releaseStatsMap2D();

    /*!
     * \brief Get basic statistics value
     * Mean, Max, Min, STD, Range, etc.
     * \param[in] sindex \string, case insensitive
     * \param[in] lyr optional for 1D and the first layer of 2D raster data.
     * \param[out] Statistics value or NODATA
     */
    double getStatistics(string sindex, int lyr = 1);

    /*!
     * \brief Get basic statistics values for 2D raster data.
     * Mean, Max, Min, STD, Range, etc.
     * \param[in] sindex \string, case insensitive
     * \param[out] lyrnum \int, layer number
     * \param[out] values \double* Statistics array or nullptr
     */
    void getStatistics(string sindex, int *lyrnum, double **values);

    /*! 
     * \brief Get the average of raster data
     * \param[in] lyr optional for 1D and the first layer of 2D raster data.
     */
    float getAverage(int lyr = 1) { return (float) this->getStatistics(string(STATS_RS_MEAN), lyr); }

    /*!
     * \brief Get the maximum of raster data
     * \sa getAverage
     */
    float getMaximum(int lyr = 1) { return (float) this->getStatistics(STATS_RS_MAX, lyr); }

    /*!
     * \brief Get the minimum of raster data
     * \sa getAverage
     */
    float getMinimum(int lyr = 1) { return (float) this->getStatistics(STATS_RS_MIN, lyr); }

    /*!
     * \brief Get the stand derivation of raster data
     * \sa getAverage
     */
    float getSTD(int lyr = 1) { return (float) this->getStatistics(STATS_RS_STD, lyr); }

    /*!
     * \brief Get the range of raster data
     * \sa getMaximum, getMinimum
     */
    float getRange(int lyr = 1) { return (float) this->getStatistics(STATS_RS_RANGE, lyr); }

    /*!
     * \brief Get the average of 2D raster data
     * \param[out] lyrnum \int, layer number
     * \param[out] values \double* Statistics array
     */
    void getAverage(int *lyrnum, double **values) { this->getStatistics(STATS_RS_MEAN, lyrnum, values); }

    /*!
     * \brief Get the maximum of 2D raster data
     * \sa getAverage
     */
    void getMaximum(int *lyrnum, double **values) { this->getStatistics(STATS_RS_MAX, lyrnum, values); }

    /*!
     * \brief Get the minimum of 2D raster data
     * \sa getAverage
     */
    void getMinimum(int *lyrnum, double **values) { this->getStatistics(STATS_RS_MIN, lyrnum, values); }

    /*!
     * \brief Get the standard derivation of 2D raster data
     * \sa getAverage
     */
    void getSTD(int *lyrnum, double **values) { this->getStatistics(STATS_RS_STD, lyrnum, values); }

    /*!
     * \brief Get the range of 2D raster data
     * \sa getAverage
     */
    void getRange(int *lyrnum, double **values) { this->getStatistics(STATS_RS_RANGE, lyrnum, values); }

    /*!
     * \brief Get the non-NoDATA number of 2D raster data
     * \sa getAverage
     */
    void getValidNumber(int *lyrnum, double **values) { this->getStatistics(STATS_RS_VALIDNUM, lyrnum, values); }

    /*!
     * \brief Get the non-NoDATA cells number of the given raster layer data
     * \sa getCellNumber, getDataLength
     */
    int getValidNumber(int lyr = 1) { return (int) this->getStatistics(STATS_RS_VALIDNUM, lyr); }

    //! Get stored cell number of raster data
    int getCellNumber() const { return m_nCells; }

    //! Get the first dimension size of raster data
    //! TODO, check out if this function is need? by lj.
    int getDataLength() const { return m_nCells; }

    //! Get column number of raster data
    int getCols() const { return (int) m_headers.at(HEADER_RS_NCOLS); }

    //! Get row number of raster data
    int getRows() const { return (int) m_headers.at(HEADER_RS_NROWS); }

    //! Get cell size of raster data
    float getCellWidth() const { return (float) m_headers.at(HEADER_RS_CELLSIZE); }

    //! Get X coordinate of left lower corner of raster data
    double getXllCenter() const { return m_headers.at(HEADER_RS_XLL); }

    //! Get Y coordinate of left lower corner of raster data
    double getYllCenter() const { return m_headers.at(HEADER_RS_YLL); }

    inline int getLayers() const {
        assert(m_nLyrs == (int) m_headers.at(HEADER_RS_LAYERS));
        return m_nLyrs;
    }

    //! Get NoDATA value of raster data
    T getNoDataValue() const { return (T) m_headers.at(HEADER_RS_NODATA); }

    //! Get NoDATA value of raster data
    T getDefaultValue() const { return m_defaultValue; }

    /*!
     * \brief Get position index in 1D raster data for specific row and column
     * \return -1 --- the position is nodata
     *         -2 --- the position is out of the extent, which indicates an error
     */
    int getPosition(int row, int col);

    //! Get position index in 1D raster data for given coordinate (x,y)
    int getPosition(float x, float y);

    //! Get position index in 1D raster data for given coordinate (x,y)
    int getPosition(double x, double y);

    /*! \brief Get raster data, include valid cell number and data
     * \return true if the raster data has been initialized, otherwise return false and print error info.
     */
    bool getRasterData(int *nCells, T **data);

    /*! \brief Get 2D raster data, include valid cell number of each layer, layer number, and data
     * \return true if the 2D raster has been initialized, otherwise return false and print error info.
     */
    bool get2DRasterData(int *nCells, int *nLyrs, T ***data);

    //! Get raster header information
    const map<string, double> &getRasterHeader() const { return m_headers; }

    //! Get raster statistics information
    const map<string, double> &getStatistics() const { return m_statsMap; }

    //! Get raster statistics information of 2D raster
    const map<string, double *> &getStatistics2D() const { return m_statsMap2D; };

    //! Get full path name
    string getFilePath() const { return m_filePathName; }

    //! Get core name
    string getCoreName() const { return m_coreFileName; }

    /*!
     * \brief Get position index data and the data length
     * \param[out] datalength
     * \param[out] positiondata, the pointer of 2D array (pointer)
     */
    void getRasterPositionData(int *datalength, int ***positiondata);

    //! Get pointer of raster data
    T *getRasterDataPointer() const { return m_rasterData; }

    //! Get pointer of position data
    int **getRasterPositionDataPointer() const { return m_rasterPositionData; }

    //! Get pointer of 2D raster data
    T **get2DRasterDataPointer() const { return m_raster2DData; }

    //! Get the spatial reference
    const char *getSRS() { return m_srs.c_str(); }

    //! Get the spatial reference string
    string getSRSString() { return m_srs; }

    /*!
     * \brief Get raster data at the valid cell index
     * The default lyr is 1, which means the 1D raster data, or the first layer of 2D data.
     */
    T getValueByIndex(int cellIndex, int lyr = 1);

    /*!
     * \brief Get raster data at the valid cell index (both for 1D and 2D raster)
     * \return a float array with length as nLyrs
     */
    void getValueByIndex(int cellIndex, int *nLyrs, T **values);

    /*!
     * \brief Get raster data via row and col
     * The default lyr is 1, which means the 1D raster data, or the first layer of 2D data.
     */
    T getValue(int row, int col, int lyr = 1);

    /*!
     * \brief Get raster data (both for 1D and 2D raster) at the (row, col)
     * \return a float array with length as nLyrs
     */
    void getValue(int row, int col, int *nLyrs, T **values);

    /*!
     * \brief Check if the raster data is NoData via row and col
     * The default lyr is 1, which means the 1D raster data, or the first layer of 2D data.
     */
    inline T isNoData(int row, int col, int lyr = 1) {
        return FloatEqual(this->getValue(row, col, lyr), m_noDataValue);
    }

    //! Is 2D raster data?
    bool is2DRaster() const { return m_is2DRaster; }

    //! Calculate positions or not
    bool PositionsCalculated() const { return m_calcPositions; }

    //! raster position data is stored as array (true), or just a pointer
    bool PositionsAllocated() const { return m_storePositions; }

    //! Use mask extent or not
    bool MaskExtented() const { return m_useMaskExtent; }

    //! Basic statistics has been calculated or not
    bool StatisticsCalculated() const { return m_statisticsCalculated; }

    //! The instance of clsRasterData has been initialized or not
    bool Initialized() const { return m_initialized; }

    /*!
     * \brief Validate the available of raster data, both 1D and 2D data
     */
    inline bool validate_raster_data() {
        if ((!m_is2DRaster && nullptr != m_rasterData) ||  // Valid 1D raster
            (m_is2DRaster && nullptr != m_raster2DData)) { // Valid 2D raster
            return true;
        } else {
            print_status("Please initialize the raster object first.");
            return false;
        }
    }

    /*!
     * \brief Validate the input row and col
     */
    inline bool validate_row_col(int row, int col) {
        if ((row < 0 || row >= this->getRows()) || (col < 0 || col >= this->getCols())) {
            print_status("The row must between 0 and " + ValueToString(this->getRows() - 1) +
                ", and the col must between 0 and " + ValueToString(this->getCols() - 1));
            return false;
        } else { return true; }
    }

    /*!
     * \brief Validate the input layer number
     */
    inline bool validate_layer(int lyr) {
        if (lyr <= 0 || lyr > m_nLyrs) {
            print_status("The layer must be 1 ");
            if (m_nLyrs > 1) print_status(" or between 1 and " + ValueToString(m_nLyrs));
            return false;
        } else { return true; }
    }

    /*!
     * \brief Validate the input index
     */
    inline bool validate_index(int idx) {
        if (idx < 0 || idx >= m_nCells) {
            print_status("The index must between 0 and " + ValueToString(m_nCells - 1));
            return false;
        } else { return true; }
    }

    //! Get full filename
    string GetFullFileName() const { return m_filePathName; }

    //! Get mask data pointer
    clsRasterData<MaskT> *getMask() const { return m_mask; }

    /*!
     * \brief Copy clsRasterData object
     */
    void Copy(const clsRasterData<T, MaskT> *orgraster);

    /*!
     * \brief Replace NoData value by the given value
     */
    void replaceNoData(T replacedv);

    /*!
     * \brief classify raster
     */
    void reclassify(map<int, T> reclassMap);

    /************* Utility functions ***************/

    /*!
     * \brief Calculate XY coordinates by given row and col number
     * \sa getPositionByCoordinate
     * \param[in] row
     * \param[in] col
     * \return pair<double x, double y>
     */
    XYCoor getCoordinateByRowCol(int row, int col);

    /*!
     * \brief Calculate position by given coordinate
     * \sa getCoordinateByRowCol
     * \param[in] x
     * \param[in] y
     * \param[in] header Optional, header map of raster layer data, the default is m_header
     * \return pair<int row, int col>
     */
    RowCol getPositionByCoordinate(double x, double y, map<string, double> *header = nullptr);

    /*!
     * \brief Copy header information to current Raster data
     * \param[in] refers
     */
    void copyHeader(const map<string, double> &refers);

private:
    /*!
     * \brief Initialize all raster related variables.
     */
    void _initialize_raster_class();

    /*!
     * \brief Initialize read function for ASC, GDAL, and MongoDB
     */
    void _initialize_read_function(string filename, bool calcPositions = true, clsRasterData<MaskT> *mask = nullptr,
                                   bool useMaskExtent = true, T defalutValue = (T) NODATA_VALUE);

    /*!
     * \brief Constructor of clsRasterData instance from single file of TIFF, ASCII, or other GDAL supported raster file
     * Support 1D and/or 2D raster data
     * \param[in] filename Full path of the raster file
     * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] mask \a clsRasterData<T2> Mask layer
     * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     * \return true if read successfully, otherwise return false.
     */
    bool _construct_from_single_file(string filename, bool calcPositions = true, clsRasterData<MaskT> *mask = nullptr,
                                     bool useMaskExtent = true, T defalutValue = (T) NODATA_VALUE);

    /*!
     * \brief Read raster data from ASC file, the simply usage
     * \param[in] ascFileName \a string
     * \param[out] header Raster header information
     * \param[out] values Raster data matrix
     * \return true if read successfully, otherwise return false.
     */
    bool _read_asc_file(string ascFileName, map<string, double> *header, T **values);

    /*!
     * \brief Read raster data by GDAL, the simply usage
     * \param[in] filename \a string
     * \param[out] header Raster header information
     * \param[out] values Raster data matrix
     * \return true if read successfully, otherwise return false.
     */
    bool _read_raster_file_by_gdal(string filename, map<string, double> *header,
                                   T **values, string *srs = nullptr);

    /*!
     * \brief Extract by mask data and calculate position index, if necessary.
     */
    void _mask_and_calculate_valid_positions();

    /*!
     * \brief Calculate position index from rectangle grid values, if necessary.
     * To use this function, mask should be nullptr.
     *
     */
    void _calculate_valid_positions_from_grid_data();

    /*!
     * \brief Write raster header information into ASC file
     * If the file exists, delete it first.
     * \param[in] filename \a string, output ASC file path
     * \param[in] header header information
     */
    bool _write_ASC_headers(string filename, map<string, double> &header);

    /*!
     * \brief Write single geotiff file
     * If the file exists, delete it first.
     * \param[in] filename \a string, output ASC file path
     * \param[in] header header information
     * \param[in] srs Coordinate system string
     * \param[in] values float raster data array
     */
    bool _write_single_geotiff(string filename, map<string, double> &header, string srs, float *values);

#ifdef USE_MONGODB

    /*!
     * \brief Write full-sized raster data as GridFS file
     * If the file exists, delete it first.
     * \param[in] filename \a string, GridFS file name
     * \param[in] header header information
     * \param[in] srs Coordinate system string
     * \param[in] values float raster data array
     */
    void _write_stream_data_as_gridfs(MongoGridFS *gfs,
                                      string filename,
                                      map<string, double> &header,
                                      string srs,
                                      T *values,
                                      size_t datalength);

#endif /* USE_MONGODB */

    /*!
     * \brief Add other layer's rater data to m_raster2DData
     * \param[in] row Row number be added on, e.g. 2
     * \param[in] col Column number be added on, e.g. 3
     * \param[in] cellidx Cell index in m_raster2DData, e.g. 23
     * \param[in] lyr Layer number which is greater than 1, e.g. 2, 3, ..., n
     * \param[in] lyrheader Header information of current layer
     * \param[in] lyrdata Raster layer data
     */
    void _add_other_layer_raster_data(int row, int col, int cellidx, int lyr,
                                      map<string, double> lyrheader, T *lyrdata);

    inline void _check_default_value() {
        if (FloatEqual(m_defaultValue, (T) NODATA_VALUE) && !FloatEqual(m_noDataValue, (T) NODATA_VALUE)) {
            m_defaultValue = m_noDataValue;
        }
    }

private:
    /*!
     * \brief Operator= without implementation
     */
    clsRasterData &operator=(const clsRasterData &another);

private:
    /*! cell number of raster data, i.e. the data length of \sa m_rasterData or \sa m_raster2DData
     * 1. all grid cell number, i.e., ncols * nrows, when m_calcPositions is False
     * 2. valid cell number excluding NoDATA, when m_calcPositions is True and m_useMaskExtent is False.
     * 3. including NoDATA when mask is valid and m_useMaskExtent is True.
     */
    int m_nCells;
    //! Layer number of the 2D raster
    int m_nLyrs;
    ///< noDataValue
    T m_noDataValue;
    ///< default value when mask by mask data, if not specified, it equals to m_noDataValue
    T m_defaultValue;
    ///< raster full path, e.g. "C:/tmp/studyarea.tif"
    string m_filePathName;
    ///< core raster file name, e.g. "studyarea"
    string m_coreFileName;
    ///< OGRSpatialReference
    string m_srs;
    ///< 1D raster data
    T *m_rasterData;
    ///< 2D raster data, [cellIndex][layer]
    T **m_raster2DData;
    ///< cell index (row, col) in m_rasterData or the first layer of m_raster2DData (2D array)
    int **m_rasterPositionData;
    ///< Header information, using double in case of truncation of coordinate value
    map<string, double> m_headers;
    //! Map to store basic statistics values for 1D raster data
    map<string, double> m_statsMap;
    //! Map to store basic statistics values for 2D raster data
    map<string, double *> m_statsMap2D;
    ///< mask clsRasterData instance
    clsRasterData<MaskT> *m_mask;
    ///< initial once
    bool m_initialized;
    ///< Flag to identify 1D or 2D raster
    bool m_is2DRaster;
    ///< calculate valid positions or not. The default is true.
    bool m_calcPositions;
    ///< raster position data is newly allocated array (true), or just a pointer (false)
    bool m_storePositions;
    ///< To be consistent with other datesets, keep the extent of Mask layer, even include NoDATA.
    bool m_useMaskExtent;
    ///< Statistics calculated?
    bool m_statisticsCalculated;
};

/*******************************************************/
/************* Implementation Code Begin ***************/
/*******************************************************/

/************* Construct functions ***************/

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_initialize_raster_class() {
    m_filePathName = "";
    m_coreFileName = "";
    m_nCells = -1;
    m_noDataValue = (T) NODATA_VALUE;
    m_defaultValue = (T) NODATA_VALUE;
    m_rasterData = nullptr;
    m_rasterPositionData = nullptr;
    m_mask = nullptr;
    m_nLyrs = -1;
    m_is2DRaster = false;
    m_raster2DData = nullptr;
    m_calcPositions = false;
    m_storePositions = false;
    m_useMaskExtent = false;
    m_statisticsCalculated = false;
    const char *RASTER_HEADERS[8] = {HEADER_RS_NCOLS, HEADER_RS_NROWS, HEADER_RS_XLL, HEADER_RS_YLL, HEADER_RS_CELLSIZE,
                                     HEADER_RS_NODATA, HEADER_RS_LAYERS, HEADER_RS_CELLSNUM};
    for (int i = 0; i < 6; i++) {
        m_headers.insert(make_pair(RASTER_HEADERS[i], NODATA_VALUE));
    }
    m_headers.insert(make_pair(HEADER_RS_LAYERS, -1.));
    m_headers.insert(make_pair(HEADER_RS_CELLSNUM, -1.));
    string statsnames[6] = {STATS_RS_VALIDNUM, STATS_RS_MIN, STATS_RS_MAX, STATS_RS_MEAN,
                            STATS_RS_STD, STATS_RS_RANGE};
    for (int i = 0; i < 6; i++) {
        m_statsMap.insert(make_pair(statsnames[i], NODATA_VALUE));
        m_statsMap2D.insert(map<string, double *>::value_type(statsnames[i], nullptr));
    }
    m_initialized = true;
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_initialize_read_function(string filename, bool calcPositions /* = true */,
                                                        clsRasterData<MaskT> *mask /* = nullptr */,
                                                        bool useMaskExtent /* = true */,
                                                        T defalutValue /* = (T) NODATA_VALUE */) {
    if (!m_initialized) this->_initialize_raster_class();
    if (nullptr != mask) m_mask = mask;
    m_filePathName = filename; // full path
    m_coreFileName = GetCoreFileName(m_filePathName);
    m_calcPositions = calcPositions;
    m_useMaskExtent = useMaskExtent;
    m_defaultValue = defalutValue;
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData() {
    this->_initialize_raster_class();
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(const string &filename, bool calcPositions /* = true */,
                                       clsRasterData<MaskT> *mask /* = nullptr */,
                                       bool useMaskExtent /* = true */,
                                       T defalutValue /* = (T) NODATA_VALUE */) {
    this->ReadFromFile(filename, calcPositions, mask, useMaskExtent, defalutValue);
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT> *clsRasterData<T, MaskT>::Init(const string &filename,
                                                       bool calcPositions /* = true */,
                                                       clsRasterData<MaskT> *mask /* = nullptr */,
                                                       bool useMaskExtent /* = true */,
                                                       T defalutValue /* = (T) NODATA_VALUE */) {
    if (!_check_raster_files_exist(filename)) return nullptr;
    return new clsRasterData<T, MaskT>(filename, calcPositions, mask, useMaskExtent, defalutValue);
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(vector<string> &filenames,
                                       bool calcPositions /* = true */,
                                       clsRasterData<MaskT> *mask /* = nullptr */,
                                       bool useMaskExtent /* = true */,
                                       T defalutValue /* = (T) NODATA_VALUE */) {
    this->_initialize_raster_class();
    if (filenames.empty()) { /// if filenames is empty
        print_status("The filenames MUST have at least one raster file path!");
        return;
    }
    if (!_check_raster_files_exist(filenames)) {  /// if not all filenames are presented
        print_status("Please make sure all file path existed!");
        return;
    }
    if (filenames.size() == 1) {  /// if filenames has only one file
        if (!this->_construct_from_single_file(filenames[0], calcPositions, mask,
                                               useMaskExtent, defalutValue)) {
            return;
        }
    } else {  /// construct from multi-layers file
        m_nLyrs = (int) filenames.size();
        /// 1. firstly, take the first layer as the main input, to calculate position index or
        ///    extract by mask if stated.
        if (!this->_construct_from_single_file(filenames[0], calcPositions, mask,
                                               useMaskExtent, defalutValue)) {
            return;
        }
        /// 2. then, change the core file name and file path template which format is: <file dir>/CoreName_%d.<suffix>
        m_coreFileName = SplitString(m_coreFileName, '_')[0];
        m_filePathName = GetPathFromFullName(filenames[0]) + SEP + m_coreFileName + "_%d." + GetSuffix(filenames[0]);
        /// So, to get a given layer's filepath, please use the following code. Definitely, maximum 99 layers is supported now.
        ///    string layerFilepath = m_filePathName.replace(m_filePathName.find_last_of("%d") - 1, 2, ValueToString(1));
        /// 3. initialize m_raster2DData and read the other layers according to position data if stated,
        ///     or just read by row and col
        Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, m_noDataValue);
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            m_raster2DData[i][0] = m_rasterData[i];
        }
        Release1DArray(m_rasterData);
        /// take the first layer as mask, and useMaskExtent is true, and no need to calculate position data
        //for (vector<string>::iterator iter = filenames.begin(); iter != filenames.end(); iter++){
        for (size_t fileidx = 1; fileidx < filenames.size(); fileidx++) {
            map<string, double> tmpheader;
            T *tmplyrdata = nullptr;
            string curfilename = filenames[fileidx];
            if (StringMatch(GetUpper(GetSuffix(curfilename)), string(ASCIIExtension))) {
                this->_read_asc_file(curfilename, &tmpheader, &tmplyrdata);
            } else {
                this->_read_raster_file_by_gdal(curfilename, &tmpheader, &tmplyrdata, &m_srs);
            }
            if (m_calcPositions) {
#pragma omp parallel for
                for (int i = 0; i < m_nCells; ++i) {
                    int tmpRow = m_rasterPositionData[i][0];
                    int tmpCol = m_rasterPositionData[i][1];
                    this->_add_other_layer_raster_data(tmpRow, tmpCol, i, (int) fileidx,
                                                       tmpheader, tmplyrdata);
                }
            } else {
#pragma omp parallel for
                for (int i = 0; i < (int) m_headers.at(HEADER_RS_NROWS); ++i) {
                    for (int j = 0; j < (int) m_headers.at(HEADER_RS_NCOLS); ++j) {
                        this->_add_other_layer_raster_data(i, j, i * (int) m_headers.at(HEADER_RS_NCOLS) + j,
                                                           (int) fileidx, tmpheader, tmplyrdata);
                    }
                }
            }
            Release1DArray(tmplyrdata);
        }
        m_is2DRaster = true;
        m_headers.at(HEADER_RS_LAYERS) = m_nLyrs;  // repair layers count in headers
    }
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT> *clsRasterData<T, MaskT>::Init(vector<string> &filenames,
                                                       bool calcPositions /* = true */,
                                                       clsRasterData<MaskT> *mask /* = nullptr */,
                                                       bool useMaskExtent /* = true */,
                                                       T defalutValue /* = (T) NODATA_VALUE */) {
    if (filenames.empty()) { /// if filenames is empty
        print_status("The filenames MUST have at least one raster file path!");
        return nullptr;
    }
    if (!_check_raster_files_exist(filenames)) {  /// if not all filenames are presented
        print_status("Please make sure all file path existed!");
        return nullptr;
    }
    if (filenames.size() == 1) {  /// if filenames has only one file
        return new clsRasterData<T, MaskT>(filenames[0], calcPositions, mask, useMaskExtent, defalutValue);
    } else {  /// construct from multi-layers file
        return new clsRasterData<T, MaskT>(filenames, calcPositions, mask, useMaskExtent, defalutValue);
    }
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(clsRasterData<MaskT> *mask, const T *values) {
    this->_initialize_raster_class();
    m_mask = mask;
    m_nLyrs = mask->getLayers();
    m_nCells = m_mask->getCellNumber();
    Initialize1DArray(m_nCells, m_rasterData, values); // DO NOT ASSIGN ARRAY DIRECTLY, IN CASE OF MEMORY ERROR!
    this->copyHeader(m_mask->getRasterHeader());
    m_srs = m_mask->getSRSString();
    m_defaultValue = m_mask->getDefaultValue();
    m_calcPositions = false;
    if (mask->PositionsCalculated()) {
        mask->getRasterPositionData(&m_nCells, &m_rasterPositionData);
    }
    m_useMaskExtent = true;
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(clsRasterData<MaskT> *mask, const T * const *values, int lyrs) {
    this->_initialize_raster_class();
    m_mask = mask;
    m_nLyrs = lyrs;
    this->copyHeader(m_mask->getRasterHeader());
    m_nCells = m_mask->getCellNumber();
    Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, values); // DO NOT ASSIGN ARRAY DIRECTLY!
    m_defaultValue = m_mask->getDefaultValue();
    m_useMaskExtent = true;
    if (mask->PositionsCalculated()) {
        mask->getRasterPositionData(&m_nCells, &m_rasterPositionData);
    }
    m_is2DRaster = true;
}

#ifdef USE_MONGODB

template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(MongoGridFS *gfs, const char *remoteFilename,
                                       bool calcPositions /* = true */,
                                       clsRasterData<MaskT> *mask /* = nullptr */,
                                       bool useMaskExtent /* = true */,
                                       T defalutValue /* = (T) NODATA_VALUE */) {
    this->_initialize_raster_class();
    this->ReadFromMongoDB(gfs, remoteFilename, calcPositions, mask, useMaskExtent, defalutValue);
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT> *clsRasterData<T, MaskT>::Init(MongoGridFS *gfs, const char *remoteFilename,
                                                       bool calcPositions /* = true */,
                                                       clsRasterData<MaskT> *mask /* = nullptr */,
                                                       bool useMaskExtent /* = true */,
                                                       T defalutValue /* = (T) NODATA_VALUE */) {
    return new clsRasterData<T, MaskT>(gfs, remoteFilename, calcPositions, mask, useMaskExtent, defalutValue);
};

#endif /* USE_MONGODB */

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::_construct_from_single_file(string filename, bool calcPositions /* = true */,
                                                          clsRasterData<MaskT> *mask /* = nullptr */,
                                                          bool useMaskExtent /* = true */,
                                                          T defalutValue /* = (T) NODATA_VALUE */) {
    if (nullptr != mask) { m_mask = mask; }
    else { useMaskExtent = false; }
    m_filePathName = filename; // full path
    m_coreFileName = GetCoreFileName(m_filePathName);
    m_calcPositions = calcPositions;
    m_useMaskExtent = useMaskExtent;
    m_defaultValue = defalutValue;

    bool readflag = false;
    if (StringMatch(GetUpper(GetSuffix(filename)), ASCIIExtension)) {
        readflag = _read_asc_file(m_filePathName, &m_headers, &m_rasterData);
    } else {
        readflag = _read_raster_file_by_gdal(m_filePathName, &m_headers, &m_rasterData, &m_srs);
    }
    this->_check_default_value();
    if (readflag) {
        if (m_nLyrs < 0) m_nLyrs = 1;
        this->_mask_and_calculate_valid_positions();
        return true;
    } else { return false; }
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT>::~clsRasterData() {
    StatusMessage(("Release raster: " + m_coreFileName).c_str());
    if (nullptr != m_rasterData) Release1DArray(m_rasterData);
    if (nullptr != m_rasterPositionData && m_storePositions) Release2DArray(m_nCells, m_rasterPositionData);
    if (nullptr != m_raster2DData && m_is2DRaster) Release2DArray(m_nCells, m_raster2DData);
    if (m_is2DRaster && m_statisticsCalculated) this->releaseStatsMap2D();
}

/************* Get information functions ***************/
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::calculateStatistics() {
    if (this->m_statisticsCalculated) return;
    if (m_is2DRaster && nullptr != m_raster2DData) {
        double **derivedvs;
        basicStatistics(m_raster2DData, m_nCells, m_nLyrs, &derivedvs, m_noDataValue);
        if (m_statsMap2D.empty()) {
            m_statsMap2D.insert(map<string, double *>::value_type(STATS_RS_VALIDNUM, derivedvs[0]));
            m_statsMap2D.insert(map<string, double *>::value_type(STATS_RS_MEAN, derivedvs[1]));
            m_statsMap2D.insert(map<string, double *>::value_type(STATS_RS_MAX, derivedvs[2]));
            m_statsMap2D.insert(map<string, double *>::value_type(STATS_RS_MIN, derivedvs[3]));
            m_statsMap2D.insert(map<string, double *>::value_type(STATS_RS_STD, derivedvs[4]));
            m_statsMap2D.insert(map<string, double *>::value_type(STATS_RS_RANGE, derivedvs[5]));
        } else {
            m_statsMap2D.at(STATS_RS_VALIDNUM) = derivedvs[0];
            m_statsMap2D.at(STATS_RS_MEAN) = derivedvs[1];
            m_statsMap2D.at(STATS_RS_MAX) = derivedvs[2];
            m_statsMap2D.at(STATS_RS_MIN) = derivedvs[3];
            m_statsMap2D.at(STATS_RS_STD) = derivedvs[4];
            m_statsMap2D.at(STATS_RS_RANGE) = derivedvs[5];
        }
        /// 1D array elements of derivedvs will be released by the destructor: releaseStatsMap2D()
        delete[] derivedvs;
        derivedvs = nullptr;
    } else {
        double *derivedv = nullptr;
        basicStatistics(m_rasterData, m_nCells, &derivedv, m_noDataValue);
        m_statsMap.at(STATS_RS_VALIDNUM) = derivedv[0];
        m_statsMap.at(STATS_RS_MEAN) = derivedv[1];
        m_statsMap.at(STATS_RS_MAX) = derivedv[2];
        m_statsMap.at(STATS_RS_MIN) = derivedv[3];
        m_statsMap.at(STATS_RS_STD) = derivedv[4];
        m_statsMap.at(STATS_RS_RANGE) = derivedv[5];
        Release1DArray(derivedv);
    }
    this->m_statisticsCalculated = true;
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::releaseStatsMap2D() {
    for (auto it = m_statsMap2D.begin(); it != m_statsMap2D.end();) {
        if (nullptr != it->second) {
            Release1DArray(it->second);
        }
        m_statsMap2D.erase(it++);
    }
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::updateStatistics() {
    if (m_is2DRaster && this->m_statisticsCalculated) this->releaseStatsMap2D();
    this->m_statisticsCalculated = false;
    this->calculateStatistics();
}

template<typename T, typename MaskT>
double clsRasterData<T, MaskT>::getStatistics(string sindex, int lyr /* = 1 */) {
    sindex = GetUpper(sindex);
    if (!this->validate_raster_data() || !this->validate_layer(lyr)) {
        print_status("No available raster statistics!");
        return m_noDataValue;
    }
    if (this->m_is2DRaster && nullptr != m_raster2DData)  // for 2D raster data
    {
        map<string, double *>::iterator it = m_statsMap2D.find(sindex);
        if (it != m_statsMap2D.end()) {
            if (nullptr == it->second) {
                m_statisticsCalculated = false;
                this->calculateStatistics();
            }
            return m_statsMap2D.at(sindex)[lyr - 1];
        } else {
            print_status("WARNING: " + ValueToString(sindex) + " is not supported currently.");
            return m_noDataValue;
        }
    } else  // for 1D raster data
    {
        map<string, double>::iterator it = m_statsMap.find(sindex);
        if (it != m_statsMap.end()) {
            if (FloatEqual(it->second, (double) NODATA_VALUE) || !this->m_statisticsCalculated) {
                this->calculateStatistics();
            }
            return m_statsMap.at(sindex);
        } else {
            print_status("WARNING: " + ValueToString(sindex) + " is not supported currently.");
            return m_noDataValue;
        }
    }
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::getStatistics(string sindex, int *lyrnum, double **values) {
    if (!m_is2DRaster || nullptr != m_raster2DData) {
        print_status("Please initialize the raster object first.");
        *values = nullptr;
        return;
    }
    sindex = GetUpper(sindex);
    *lyrnum = m_nLyrs;
    map<string, double *>::iterator it = m_statsMap2D.find(sindex);
    if (!this->is2DRaster()) {
        *values = nullptr;
        return;
    }
    if (it == m_statsMap2D.end()) {
        *values = nullptr;
        print_status("WARNING: " + ValueToString(sindex) + " is not supported currently.");
        return;
    }
    if (nullptr == it->second || !this->m_statisticsCalculated) {
        this->calculateStatistics();
    }
    *values = it->second;
}

template<typename T, typename MaskT>
int clsRasterData<T, MaskT>::getPosition(int row, int col) {
    if (!this->validate_raster_data() || !this->validate_row_col(row, col)) {
        return -2;  // means error occurred!
    }
    if (!m_calcPositions || nullptr == m_rasterPositionData) {
        return this->getCols() * row + col;
    }
    for (int i = 0; i < m_nCells; i++) {
        if (row == m_rasterPositionData[i][0] && col == m_rasterPositionData[i][1]) {
            return i;
        }
    }
    return -1;  // means the location is NODATA
}

template<typename T, typename MaskT>
int clsRasterData<T, MaskT>::getPosition(float x, float y) {
    return getPosition((double) x, (double) y);
}

template<typename T, typename MaskT>
int clsRasterData<T, MaskT>::getPosition(double x, double y) {
    if (!m_initialized) return -2;
    double xllCenter = this->getXllCenter();
    double yllCenter = this->getYllCenter();
    float dx = this->getCellWidth();
    float dy = this->getCellWidth();
    int nRows = this->getRows();
    int nCols = this->getCols();

    if (FloatEqual(xllCenter, (double) NODATA_VALUE) || FloatEqual(yllCenter, (double) NODATA_VALUE) ||
        FloatEqual(dx, NODATA_VALUE) || nRows < 0 || nCols < 0) {
        print_status("No available header information!");
        return -2;
    }

    double xmin = xllCenter - dx / 2.;
    double xMax = xmin + dx * nCols;
    if (x > xMax || x < xllCenter) {
        return -2;
    }

    double ymin = yllCenter - dy / 2.;
    double yMax = ymin + dy * nRows;
    if (y > yMax || y < yllCenter) {
        return -2;
    }

    int nRow = (int) ((yMax - y) / dy); //calculate from ymax
    int nCol = (int) ((x - xmin) / dx); //calculate from xmin

    return getPosition(nRow, nCol);
}

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::getRasterData(int *nCells, T **data) {
    if (this->validate_raster_data() && !m_is2DRaster) {
        *nCells = m_nCells;
        *data = m_rasterData;
        return true;
    }
    *nCells = -1;
    *data = nullptr;
    return false;
}

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::get2DRasterData(int *nCells, int *nLyrs, T ***data) {
    if (this->validate_raster_data() && m_is2DRaster) {
        *nCells = m_nCells;
        *nLyrs = m_nLyrs;
        *data = m_raster2DData;
        return true;
    }
    *nCells = -1;
    *nLyrs = -1;
    *data = nullptr;
    return false;
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::getRasterPositionData(int *datalength, int ***positiondata) {
    if (nullptr != m_rasterPositionData) {
        *datalength = m_nCells;
        *positiondata = m_rasterPositionData;
    } else {// reCalculate position data
        if (!this->validate_raster_data()) {
            *datalength = -1;
            *positiondata = nullptr;
            return;
        }
        _calculate_valid_positions_from_grid_data();
        *datalength = m_nCells;
        *positiondata = m_rasterPositionData;
    }
}

template<typename T, typename MaskT>
T clsRasterData<T, MaskT>::getValueByIndex(int cellIndex, int lyr /* = 1 */) {
    if (!this->validate_raster_data() || !this->validate_index(cellIndex) || !this->validate_layer(lyr)) {
        return m_noDataValue;
    }
    if (m_is2DRaster) {
        return m_raster2DData[cellIndex][lyr - 1];
    } else {
        return m_rasterData[cellIndex];
    }
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::getValueByIndex(int cellIndex, int *nLyrs, T **values) {
    if (!this->validate_raster_data() || !this->validate_index(cellIndex)) {
        *nLyrs = -1;
        *values = nullptr;
        return;
    }
    if (m_is2DRaster) {
        T *cellValues = new T[m_nLyrs];
        for (int i = 0; i < m_nLyrs; i++) {
            cellValues[i] = m_raster2DData[cellIndex][i];
        }
        *nLyrs = m_nLyrs;
        *values = cellValues;
    } else {
        *nLyrs = 1;
        T *cellValues = new T[1];
        cellValues[0] = m_rasterData[cellIndex];
        *values = cellValues;
    }
    return;
}

template<typename T, typename MaskT>
T clsRasterData<T, MaskT>::getValue(int row, int col, int lyr /* = 1 */) {
    if (!this->validate_raster_data() || !this->validate_row_col(row, col) || !this->validate_layer(lyr)) {
        return m_noDataValue;
    }
    /// get index according to position data if possible
    if (m_calcPositions && nullptr != m_rasterPositionData) {
        int validCellIndex = this->getPosition(row, col);
        if (validCellIndex < 0) return m_noDataValue;  // error or NODATA
        return this->getValueByIndex(validCellIndex, lyr);
    } else { // get data directly from row and col
        if (m_is2DRaster) {
            return m_raster2DData[row * this->getCols() + col][lyr - 1];
        } else {
            return m_rasterData[row * this->getCols() + col];
        }
    }
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::getValue(int row, int col, int *nLyrs, T **values) {
    if (!this->validate_raster_data() || !this->validate_row_col(row, col)) {
        *nLyrs = -1;
        *values = nullptr;
        return;
    }
    if (m_calcPositions && nullptr != m_rasterPositionData) {
        int validCellIndex = this->getPosition(row, col);
        if (validCellIndex == -2) {
            *nLyrs = -1;
            *values = nullptr;  // error
            return;
        } else if (validCellIndex == -1) {
            *nLyrs = m_nLyrs;
            T *cellValues = new T[m_nLyrs];
            for (int i = 0; i < m_nLyrs; i++) {
                cellValues[i] = m_noDataValue;
            }
            *values = cellValues;  // NODATA
            return;
        } else { return this->getValueByIndex(validCellIndex, nLyrs, values); }
    } else { // get data directly from row and col
        if (m_is2DRaster) {
            T *cellValues = new T[m_nLyrs];
            for (int i = 0; i < m_nLyrs; i++) {
                cellValues[i] = m_raster2DData[row * this->getCols() + col][i];
            }
            *nLyrs = m_nLyrs;
            *values = cellValues;
        } else {
            *nLyrs = 1;
            T *cellValues = new T[1];
            cellValues[0] = m_rasterData[row * this->getCols() + col];
            *values = cellValues;
        }
    }
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::setValue(int row, int col, T value, int lyr /* = 1 */) {
    if (!this->validate_raster_data() || !this->validate_row_col(row, col) || !this->validate_layer(lyr)) {
        print_status("Set value failed!");
        return;
    }
    int idx = this->getPosition(row, col);
    if (idx == -1) {  // the origin value is NODATA
        print_status("Current version do not support to setting value to NoDATA location!");
    } else {
        if (m_is2DRaster) {
            m_raster2DData[idx][lyr - 1] = value;
        } else {
            m_rasterData[idx] = value;
        }
    }
    return;
}

/************* Output to file functions ***************/

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::outputToFile(string filename) {
    if (GetPathFromFullName(filename) == "") return false;
    filename = GetAbsolutePath(filename);
    if (!this->validate_raster_data()) return false;
    string filetype = GetUpper(GetSuffix(filename));
    if (StringMatch(filetype, ASCIIExtension)) {
        return outputASCFile(filename);
    } else if (StringMatch(filetype, GTiffExtension)) {
        return outputFileByGDAL(filename);
    } else {
        return outputFileByGDAL(ReplaceSuffix(filename, string(GTiffExtension)));
    }
}

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::_write_ASC_headers(string filename, map<string, double> &header) {
    DeleteExistedFile(filename);
    ofstream rasterFile(filename.c_str(), ios::app | ios::out);
    if (!rasterFile.is_open()) {
        print_status("Error opening file: " + filename);
        return false;
    }
    //write file
    int rows = int(header.at(HEADER_RS_NROWS));
    int cols = int(header.at(HEADER_RS_NCOLS));
    /// write header
    rasterFile << HEADER_RS_NCOLS << " " << cols << endl;
    rasterFile << HEADER_RS_NROWS << " " << rows << endl;
    rasterFile << HEADER_RS_XLL << " " << header.at(HEADER_RS_XLL) << endl;
    rasterFile << HEADER_RS_YLL << " " << header.at(HEADER_RS_YLL) << endl;
    rasterFile << HEADER_RS_CELLSIZE << " " << (float) header.at(HEADER_RS_CELLSIZE) << endl;
    rasterFile << HEADER_RS_NODATA << " " << setprecision(6) << header.at(HEADER_RS_NODATA) << endl;
    rasterFile.close();
    return true;
}

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::outputASCFile(string filename) {
    filename = GetAbsolutePath(filename);
    /// 1. Is there need to calculate valid position index?
    int count;
    int **position = nullptr;
    bool outputdirectly = true;
    if (nullptr != m_rasterPositionData) {
        this->getRasterPositionData(&count, &position);
        outputdirectly = false;
        assert(nullptr != position);
    }
    /// 2. Write ASC raster headers first (for 1D raster data only)
    if (!m_is2DRaster) {
        if (!this->_write_ASC_headers(filename, m_headers)) return false;
    }
    /// 3. Begin to write raster data
    int rows = int(m_headers.at(HEADER_RS_NROWS));
    int cols = int(m_headers.at(HEADER_RS_NCOLS));

    /// 3.1 2D raster data
    if (m_is2DRaster) {
        string prePath = GetPathFromFullName(filename);
        if (StringMatch(prePath, "")) return false;
        string coreName = GetCoreFileName(filename);
        for (int lyr = 0; lyr < m_nLyrs; lyr++) {
            stringstream oss;
            oss << prePath << coreName << "_" << (lyr + 1) << "." << ASCIIExtension;
            string tmpfilename = oss.str();
            if (!this->_write_ASC_headers(tmpfilename, m_headers)) return false;
            // write data
            ofstream rasterFile(tmpfilename.c_str(), ios::app | ios::out);
            if (!rasterFile.is_open()) {
                print_status("Error opening file: " + tmpfilename);
                return false;
            }
            int index = 0;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (outputdirectly) {
                        index = i * cols + j;
                        rasterFile << setprecision(6) << m_raster2DData[index][lyr] << " ";
                        continue;
                    }
                    if (index < m_nCells && (position[index][0] == i && position[index][1] == j)) {
                        rasterFile << setprecision(6) << m_raster2DData[index][lyr] << " ";
                        index++;
                    } else { rasterFile << setprecision(6) << NODATA_VALUE << " "; }
                }
                rasterFile << endl;
            }
            rasterFile.close();
        }
    } else {  /// 3.2 1D raster data
        ofstream rasterFile(filename.c_str(), ios::app | ios::out);
        if (!rasterFile.is_open()) {
            print_status("Error opening file: " + filename);
            return false;
        }
        int index = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (outputdirectly) {
                    index = i * cols + j;
                    rasterFile << setprecision(6) << m_rasterData[index] << " ";
                    continue;
                }
                if (index < m_nCells) {
                    if (position[index][0] == i && position[index][1] == j) {
                        rasterFile << setprecision(6) << m_rasterData[index] << " ";
                        index++;
                    } else { rasterFile << setprecision(6) << m_noDataValue << " "; }
                } else { rasterFile << setprecision(6) << m_noDataValue << " "; }
            }
            rasterFile << endl;
        }
        rasterFile.close();
    }
    position = nullptr;
    return true;
}

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::_write_single_geotiff(string filename,
                                                    map<string, double> &header,
                                                    string srs, float *values) {
    /// 1. Create GeoTiff file driver
    GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (nullptr == poDriver) return false;
    char **papszOptions = nullptr;
    int nRows = int(header.at(HEADER_RS_NROWS));
    int nCols = int(header.at(HEADER_RS_NCOLS));
    GDALDataset *poDstDS = poDriver->Create(filename.c_str(), nCols, nRows, 1, GDT_Float32, papszOptions);
    if (nullptr == poDstDS) return false;
    /// 2. Write raster data
    GDALRasterBand *poDstBand = poDstDS->GetRasterBand(1);
    poDstBand->RasterIO(GF_Write, 0, 0, nCols, nRows, values, nCols, nRows, GDT_Float32, 0, 0);
    if (nullptr == poDstBand) return false;
    poDstBand->SetNoDataValue(header.at(HEADER_RS_NODATA));
    /// 3. Writer header information
    double geoTrans[6];
    geoTrans[0] = header.at(HEADER_RS_XLL) - 0.5 * header.at(HEADER_RS_CELLSIZE);
    geoTrans[1] = header.at(HEADER_RS_CELLSIZE);
    geoTrans[2] = 0.;
    geoTrans[3] = header.at(HEADER_RS_YLL) + (nRows - 0.5) * header.at(HEADER_RS_CELLSIZE);
    geoTrans[4] = 0.;
    geoTrans[5] = -header.at(HEADER_RS_CELLSIZE);
    poDstDS->SetGeoTransform(geoTrans);
    poDstDS->SetProjection(srs.c_str());
    GDALClose(poDstDS);

    return true;
}

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::outputFileByGDAL(string filename) {
    filename = GetAbsolutePath(filename);
    /// 1. Is there need to calculate valid position index?
    int count;
    int **position = nullptr;
    bool outputdirectly = true;
    if (nullptr != m_rasterPositionData) {
        this->getRasterPositionData(&count, &position);
        outputdirectly = false;
        assert(nullptr != position);
    }
    /// 2. Get raster data
    /// 2.1 2D raster data
    int nRows = int(m_headers.at(HEADER_RS_NROWS));
    int nCols = int(m_headers.at(HEADER_RS_NCOLS));
    if (m_is2DRaster) {
        string prePath = GetPathFromFullName(filename);
        if (StringMatch(prePath, "")) return false;
        string coreName = GetCoreFileName(filename);
        for (int lyr = 0; lyr < m_nLyrs; lyr++) {
            stringstream oss;
            oss << prePath << coreName << "_" << (lyr + 1) << "." << GTiffExtension;
            string tmpfilename = oss.str();
            float *rasterdata1D = nullptr;
            Initialize1DArray(nRows * nCols, rasterdata1D, (float) m_noDataValue);
            int validnum = 0;
            for (int i = 0; i < nRows; ++i) {
                for (int j = 0; j < nCols; ++j) {
                    int index = i * nCols + j;
                    if (outputdirectly) {
                        rasterdata1D[index] = m_raster2DData[index][lyr];
                        continue;
                    }
                    if (validnum < m_nCells && (position[validnum][0] == i && position[validnum][1] == j)) {
                        rasterdata1D[index] = m_raster2DData[validnum][lyr];
                        validnum++;
                    }
                }
            }
            bool outflag = this->_write_single_geotiff(tmpfilename, m_headers, m_srs, rasterdata1D);
            Release1DArray(rasterdata1D);
            if (!outflag) return false;
        }
    } else {  /// 3.2 1D raster data
        float *rasterdata1D = nullptr;
        bool newbuilddata = true;
        if (outputdirectly) {
            if (typeid(T) != typeid(float)) {
                /// copyArray() should be an common used function
                rasterdata1D = new float[m_nCells];
                for (int i = 0; i < m_nCells; i++) {
                    rasterdata1D[i] = (float) m_rasterData[i];
                }
            } else {
                rasterdata1D = (float *) m_rasterData;
                newbuilddata = false;
            }
        } else {
            Initialize1DArray(nRows * nCols, rasterdata1D, (float) m_noDataValue);
        }
        int validnum = 0;
        if (!outputdirectly) {
            for (int i = 0; i < nRows; ++i) {
                for (int j = 0; j < nCols; ++j) {
                    int index = i * nCols + j;
                    if (validnum < m_nCells && (position[validnum][0] == i && position[validnum][1] == j)) {
                        rasterdata1D[index] = m_rasterData[validnum];
                        validnum++;
                    }
                }
            }
        }
        bool outflag = this->_write_single_geotiff(filename, m_headers, m_srs, rasterdata1D);
        if (!newbuilddata) { rasterdata1D = nullptr; }
        else { Release1DArray(rasterdata1D); }
        if (!outflag) return false;
    }
    position = nullptr;
    return true;
}

#ifdef USE_MONGODB

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::outputToMongoDB(string filename, MongoGridFS *gfs) {
    /// 1. Is there need to calculate valid position index?
    int count;
    int **position;
    bool outputdirectly = true;
    if (nullptr != m_rasterPositionData) {
        this->getRasterPositionData(&count, &position);
        outputdirectly = false;
        assert(nullptr != position);
    }
    /// 2. Get raster data
    /// 2.1 2D raster data
    T noDataValue = (T) m_headers.at(HEADER_RS_NODATA);
    int nRows = int(m_headers.at(HEADER_RS_NROWS));
    int nCols = int(m_headers.at(HEADER_RS_NCOLS));
    int datalength;
    if (m_is2DRaster) {
        string coreName = GetCoreFileName(filename);
        T *rasterdata1D = nullptr;
        datalength = nRows * nCols * m_nLyrs;
        Initialize1DArray(datalength, rasterdata1D, noDataValue);
        int countindex = 0;
        for (int i = 0; i < nRows; ++i) {
            for (int j = 0; j < nCols; ++j) {
                int rowcolindex = i * nCols + j;
                int dataIndex = i * nCols * m_nLyrs + j * m_nLyrs;
                if (outputdirectly) {
                    for (int k = 0; k < m_nLyrs; k++) {
                        rasterdata1D[dataIndex + k] = m_raster2DData[rowcolindex][k];
                    }
                    continue;
                }
                if (countindex < m_nCells && (position[countindex][0] == i && position[countindex][1] == j)) {
                    for (int k = 0; k < m_nLyrs; k++) {
                        rasterdata1D[dataIndex + k] = m_raster2DData[countindex][k];
                    }
                    countindex++;
                }
            }
        }
        this->_write_stream_data_as_gridfs(gfs, filename, m_headers, m_srs, rasterdata1D, datalength);
        Release1DArray(rasterdata1D);
    } else {  /// 3.2 1D raster data
        float *rasterdata1D = nullptr;
        datalength = nRows * nCols;
        if (outputdirectly) {
            rasterdata1D = m_rasterData;
        } else
            Initialize1DArray(datalength, rasterdata1D, noDataValue);
        int validnum = 0;
        if (!outputdirectly) {
            for (int i = 0; i < nRows; ++i) {
                for (int j = 0; j < nCols; ++j) {
                    int index = i * nCols + j;
                    if (validnum < m_nCells && (position[validnum][0] == i && position[validnum][1] == j)) {
                        rasterdata1D[index] = m_rasterData[validnum];
                        validnum++;
                    }
                }
            }
        }
        this->_write_stream_data_as_gridfs(gfs, filename, m_headers, m_srs, rasterdata1D, datalength);
        if (outputdirectly) { rasterdata1D = nullptr; }
        else
            Release1DArray(rasterdata1D);
    }
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_write_stream_data_as_gridfs(MongoGridFS *gfs,
                                                           string filename,
                                                           map<string, double> &header,
                                                           string srs,
                                                           T *values,
                                                           size_t datalength) {
    bson_t p = BSON_INITIALIZER;
    for (map<string, double>::iterator iter = header.begin(); iter != header.end(); iter++) {
        BSON_APPEND_DOUBLE(&p, iter->first.c_str(), iter->second);
    }
    BSON_APPEND_UTF8(&p, HEADER_RS_SRS, srs.c_str());
    char *buf = (char *) values;
    size_t buflength = datalength * sizeof(T);
    gfs->writeStreamData(filename, buf, buflength, &p);
    bson_destroy(&p);
}

#endif /* USE_MONGODB */

/************* Read functions ***************/
template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::ReadFromFile(string filename, bool calcPositions /* = true */,
                                           clsRasterData<MaskT> *mask /* = nullptr */,
                                           bool useMaskExtent /* = true */,
                                           T defalutValue /* = (T) NODATA_VALUE */) {
    if (!_check_raster_files_exist(filename)) return false;
    this->_initialize_raster_class();
    return this->_construct_from_single_file(filename, calcPositions, mask, useMaskExtent, defalutValue);
}

#ifdef USE_MONGODB

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::ReadFromMongoDB(MongoGridFS *gfs,
                                              string filename,
                                              bool calcPositions /* = true */,
                                              clsRasterData<MaskT> *mask /* = nullptr */,
                                              bool useMaskExtent /* = true */,
                                              T defalutValue /* = (T) NODATA_VALUE */) {
    this->_initialize_read_function(filename, calcPositions, mask, useMaskExtent, defalutValue);
    /// 1. Get stream data and metadata by file name
    char *buf;
    size_t length;
    gfs->getStreamData(filename, buf, length);
    bson_t *bmeta = gfs->getFileMetadata(filename);
    /// 2. Retrieve raster header values
    const char *RASTER_HEADERS[8] = {HEADER_RS_NCOLS, HEADER_RS_NROWS, HEADER_RS_XLL, HEADER_RS_YLL, HEADER_RS_CELLSIZE,
                                     HEADER_RS_NODATA, HEADER_RS_LAYERS, HEADER_RS_CELLSNUM};
    for (int i = 0; i < 8; i++) {
        GetNumericFromBson(bmeta, RASTER_HEADERS[i], m_headers[RASTER_HEADERS[i]]);
    }
    m_srs = GetStringFromBson(bmeta, HEADER_RS_SRS);

    int nRows = (int) m_headers.at(HEADER_RS_NROWS);
    int nCols = (int) m_headers.at(HEADER_RS_NCOLS);
    m_nCells = nRows * nCols;
    m_noDataValue = (T) m_headers.at(HEADER_RS_NODATA);
    m_nLyrs = (int) m_headers.at(HEADER_RS_LAYERS);

    /// TODO (by LJ), currently data stored in MongoDB is always float.
    ///  I can not find an elegant way to make it templated.
    assert(m_nCells == length / sizeof(float) / m_nLyrs);

    int validcount = -1;
    if (m_headers.find(HEADER_RS_CELLSNUM) != m_headers.end()) {
        validcount = (int) m_headers.at(HEADER_RS_CELLSNUM);
    }
    /// 3. Store data.
    /// check the valid values count and determine whether can read directly.
    bool reBuildData = true;
    if (validcount != m_nCells && m_calcPositions && m_useMaskExtent &&
        nullptr != m_mask && validcount == m_mask->getCellNumber()) {
        reBuildData = false;
        m_storePositions = false;
        m_mask->getRasterPositionData(&m_nCells, &m_rasterPositionData);
    }
    /// read data directly
    if (m_nLyrs == 1) {
        float *tmpdata = (float *) buf;
        Initialize1DArray(m_nCells, m_rasterData, m_noDataValue);
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            int tmpidx = i;
            if (!reBuildData) tmpidx = m_rasterPositionData[i][0] * this->getCols() + m_rasterPositionData[i][1];
            m_rasterData[i] = (T) tmpdata[tmpidx];
        }
        Release1DArray(tmpdata);
        m_is2DRaster = false;
    } else {
        float *tmpdata = (float *) buf;
        Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, m_noDataValue);
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            int tmpidx = i;
            if (!reBuildData) tmpidx = m_rasterPositionData[i][0] * this->getCols() + m_rasterPositionData[i][1];
            for (int j = 0; j < m_nLyrs; j++) {
                int idx = tmpidx * m_nLyrs + j;
                m_raster2DData[i][j] = (T) tmpdata[idx];
            }
        }
        m_is2DRaster = true;
        Release1DArray(tmpdata);
    }
    buf = nullptr;
    this->_check_default_value();
    if (reBuildData) {
        this->_mask_and_calculate_valid_positions();
    }
    return true;
}

#endif /* USE_MONGODB */

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::_read_asc_file(string ascFileName, map<string, double> *header, T **values) {
    StatusMessage(("Read " + ascFileName + "...").c_str());
    ifstream rasterFile(ascFileName.c_str());
    string tmp, xlls, ylls;
    T noData;
    double tempFloat;
    int rows, cols;
    map<string, double> tmpheader;
    /// read header
    rasterFile >> tmp >> cols;
    tmpheader.insert(make_pair(HEADER_RS_NCOLS, double(cols)));
    rasterFile >> tmp >> rows;
    tmpheader.insert(make_pair(HEADER_RS_NROWS, double(rows)));
    rasterFile >> xlls >> tempFloat;
    tmpheader.insert(make_pair(HEADER_RS_XLL, tempFloat));
    rasterFile >> ylls >> tempFloat;
    tmpheader.insert(make_pair(HEADER_RS_YLL, tempFloat));
    rasterFile >> tmp >> tempFloat;
    tmpheader.insert(make_pair(HEADER_RS_CELLSIZE, tempFloat));
    rasterFile >> tmp >> noData;
    tmpheader.insert(make_pair(HEADER_RS_NODATA, noData));
    m_noDataValue = noData;
    /// default is center, if corner, then:
    if (StringMatch(xlls, "XLLCORNER")) tmpheader.at(HEADER_RS_XLL) += 0.5 * tmpheader.at(HEADER_RS_CELLSIZE);
    if (StringMatch(ylls, "YLLCORNER")) tmpheader.at(HEADER_RS_YLL) += 0.5 * tmpheader.at(HEADER_RS_CELLSIZE);

    tmpheader.insert(make_pair(HEADER_RS_LAYERS, 1.));
    tmpheader.insert(make_pair(HEADER_RS_CELLSNUM, -1.));
    /// get all raster values (i.e., include NODATA_VALUE, m_excludeNODATA = False)
    T *tmprasterdata = new T[rows * cols];
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            rasterFile >> tempFloat;
            tmprasterdata[i * cols + j] = (T) tempFloat;
        }
    }
    rasterFile.close();
    /// returned parameters
    *header = tmpheader;
    *values = tmprasterdata;
    return true;
}

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::_read_raster_file_by_gdal(string filename, map<string, double> *header,
                                                        T **values, string *srs /* = nullptr */) {
    StatusMessage(("Read " + filename + "...").c_str());
    GDALDataset *poDataset = (GDALDataset *) GDALOpen(filename.c_str(), GA_ReadOnly);
    if (nullptr == poDataset) {
        print_status("Open file " + filename + " failed.");
        return false;
    }
    GDALRasterBand *poBand = poDataset->GetRasterBand(1);
    map<string, double> tmpheader;
    int nRows = poBand->GetYSize();
    int nCols = poBand->GetXSize();
    tmpheader.insert(make_pair(HEADER_RS_NCOLS, (double) nCols));
    tmpheader.insert(make_pair(HEADER_RS_NROWS, (double) nRows));
    tmpheader.insert(make_pair(HEADER_RS_NODATA, poBand->GetNoDataValue()));
    m_noDataValue = (T) poBand->GetNoDataValue();
    double adfGeoTransform[6];
    poDataset->GetGeoTransform(adfGeoTransform);
    tmpheader.insert(make_pair(HEADER_RS_CELLSIZE, adfGeoTransform[1]));
    tmpheader.insert(make_pair(HEADER_RS_XLL, adfGeoTransform[0] + 0.5 * tmpheader.at(HEADER_RS_CELLSIZE)));
    tmpheader.insert(make_pair(HEADER_RS_YLL,
                               adfGeoTransform[3] + (tmpheader.at(HEADER_RS_NROWS) - 0.5) * adfGeoTransform[5]));
    tmpheader.insert(make_pair(HEADER_RS_LAYERS, 1.));
    tmpheader.insert(make_pair(HEADER_RS_CELLSNUM, -1.));
    string tmpsrs = string(poDataset->GetProjectionRef());
    /// get all raster values (i.e., include NODATA_VALUE)
    int fullsize_nCells = nRows * nCols;
    if (m_nCells < 0) { /// if m_nCells has been assigned
        m_nCells = fullsize_nCells;
    }
    T *tmprasterdata = new T[fullsize_nCells];
    GDALDataType dataType = poBand->GetRasterDataType();
    char *char_data = nullptr;
    unsigned char *uchar_data = nullptr;
    unsigned short *ushort_data = nullptr;
    short *short_data = nullptr;
    unsigned int *uint_data = nullptr;
    int *int_data = nullptr;
    float *float_data = nullptr;
    double *double_data = nullptr;
    switch (dataType) {
    case GDT_Byte:
        /// For GDAL, GDT_Byte is 8-bit unsigned interger, ranges from 0 to 255.
        /// However, ArcGIS use 8-bit signed and unsigned intergers which both will be read as GDT_Byte.
        ///   8-bit signed integer ranges from -128 to 127.
        /// Since both signed and unsigned integers of n bits in length can represent 2^n different values,
        ///   there is no inherent way to distinguish signed integers from unsigned integers simply by looking
        ///   at them; the software designer is responsible for using them correctly.
        /// So, here I can only assume that a negative nodata indicates a 8-bit signed integer type.
        if (m_noDataValue < 0) {  // commonly -128
            char_data = (char *)CPLMalloc(sizeof(char) * nCols * nRows);
            poBand->RasterIO(GF_Read, 0, 0, nCols, nRows, char_data, nCols, nRows, GDT_Byte, 0, 0);
            _get_data_from_gdal(tmprasterdata, char_data, nRows, nCols);
            CPLFree(char_data);
        } else {  // commonly 255
            uchar_data = (unsigned char *)CPLMalloc(sizeof(unsigned char) * nCols * nRows);
            poBand->RasterIO(GF_Read, 0, 0, nCols, nRows, uchar_data, nCols, nRows, GDT_Byte, 0, 0);
            _get_data_from_gdal(tmprasterdata, uchar_data, nRows, nCols);
            CPLFree(uchar_data);
        }
        break;
    case GDT_UInt16:
        ushort_data = (unsigned short *)CPLMalloc(sizeof(unsigned short) * nCols * nRows);
        poBand->RasterIO(GF_Read, 0, 0, nCols, nRows, ushort_data, nCols, nRows, GDT_UInt16, 0, 0);
        _get_data_from_gdal(tmprasterdata, ushort_data, nRows, nCols);
        CPLFree(ushort_data);
        break;
    case GDT_Int16:
        short_data = (short *)CPLMalloc(sizeof(short) * nCols * nRows);
        poBand->RasterIO(GF_Read, 0, 0, nCols, nRows, short_data, nCols, nRows, GDT_Int16, 0, 0);
        _get_data_from_gdal(tmprasterdata, short_data, nRows, nCols);
        CPLFree(short_data);
        break;
    case GDT_UInt32:
        uint_data = (unsigned int *)CPLMalloc(sizeof(unsigned int) * nCols * nRows);
        poBand->RasterIO(GF_Read, 0, 0, nCols, nRows, uint_data, nCols, nRows, GDT_UInt32, 0, 0);
        _get_data_from_gdal(tmprasterdata, uint_data, nRows, nCols);
        CPLFree(uint_data);
        break;
    case GDT_Int32:
        int_data = (int *)CPLMalloc(sizeof(int) * nCols * nRows);
        poBand->RasterIO(GF_Read, 0, 0, nCols, nRows, int_data, nCols, nRows, GDT_Int32, 0, 0);
        _get_data_from_gdal(tmprasterdata, int_data, nRows, nCols);
        CPLFree(int_data);
        break;
    case GDT_Float32:
        float_data = (float *)CPLMalloc(sizeof(float) * nCols * nRows);
        poBand->RasterIO(GF_Read, 0, 0, nCols, nRows, float_data, nCols, nRows, GDT_Float32, 0, 0);
        _get_data_from_gdal(tmprasterdata, float_data, nRows, nCols);
        CPLFree(float_data);
        break;
    case GDT_Float64:
        double_data = (double *)CPLMalloc(sizeof(double) * nCols * nRows);
        poBand->RasterIO(GF_Read, 0, 0, nCols, nRows, double_data, nCols, nRows, GDT_Float64, 0, 0);
        _get_data_from_gdal(tmprasterdata, double_data, nRows, nCols);
        CPLFree(double_data);
        break;
    default:
        cout << "Unexpected GDALDataType: " << GDALGetDataTypeName(dataType) << endl;
        exit(-1);
    }
    GDALClose(poDataset);
    /// returned parameters
    *header = tmpheader;
    *values = tmprasterdata;
    *srs = tmpsrs;
    return true;
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_add_other_layer_raster_data(int row, int col, int cellidx, int lyr,
                                                           map<string, double> lyrheader, T *lyrdata) {
    int tmpcols = (int) lyrheader.at(HEADER_RS_NCOLS);
    XYCoor tmpXY = this->getCoordinateByRowCol(row, col);
    /// get current raster layer's value by XY
    RowCol tmpPosition = this->getPositionByCoordinate(tmpXY.first, tmpXY.second, &lyrheader);
    if (tmpPosition.first == -1 || tmpPosition.second == -1) {
        m_raster2DData[cellidx][lyr] = m_noDataValue;
    } else {
        m_raster2DData[cellidx][lyr] = lyrdata[tmpPosition.first * tmpcols + tmpPosition.second];
    }
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(clsRasterData<T, MaskT> *another) {
    this->_initialize_raster_class();
    this->Copy(another);
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::Copy(const clsRasterData<T, MaskT> *orgraster) {
    if (m_is2DRaster && nullptr != m_raster2DData && m_nCells > 0) {
        Release2DArray(m_nCells, m_raster2DData);
    }
    if (!m_is2DRaster && nullptr != m_rasterData) {
        Release1DArray(m_rasterData);
    }
    if (nullptr != m_rasterPositionData) {
        Release2DArray(2, m_rasterPositionData);
    }
    if (m_statisticsCalculated) {
        releaseStatsMap2D();
        m_statisticsCalculated = false;
    }
    this->_initialize_raster_class();
    m_filePathName = orgraster->getFilePath();
    m_coreFileName = orgraster->getCoreName();
    m_nCells = orgraster->getCellNumber();
    m_nLyrs = orgraster->getLayers();
    m_noDataValue = orgraster->getNoDataValue();
    m_defaultValue = orgraster->getDefaultValue();
    if (orgraster->is2DRaster()) {
        m_is2DRaster = true;
        Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, orgraster->get2DRasterDataPointer());
    } else {
        m_rasterData = nullptr;
        Initialize1DArray(m_nCells, m_rasterData, orgraster->getRasterDataPointer());
    }
    m_mask = orgraster->getMask();
    m_calcPositions = orgraster->PositionsCalculated();
    if (m_calcPositions) {
        m_storePositions = true;
        Initialize2DArray(m_nCells, 2, m_rasterPositionData, orgraster->getRasterPositionDataPointer());
    }
    m_useMaskExtent = orgraster->MaskExtented();
    m_statisticsCalculated = orgraster->StatisticsCalculated();
    if (m_statisticsCalculated) {
        if (m_is2DRaster) {
            if (!m_statsMap2D.empty()) releaseStatsMap2D();
            map<string, double *> stats2D = orgraster->getStatistics2D();
            for (auto iter = stats2D.begin(); iter != stats2D.end(); iter++) {
                double *tmpstatvalues = nullptr;
                Initialize1DArray(m_nLyrs, tmpstatvalues, iter->second);
                m_statsMap2D.insert(make_pair(iter->first, tmpstatvalues));
            }
        } else {
            map<string, double> stats = orgraster->getStatistics();
            for (auto iter = stats.begin(); iter != stats.end(); iter++) {
                m_statsMap[iter->first] = iter->second;
            }
        }
    }
    this->copyHeader(orgraster->getRasterHeader());
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::replaceNoData(T replacedv) {
    if (m_is2DRaster && nullptr != m_raster2DData) {
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            for (int lyr = 0; lyr < m_nLyrs; lyr++) {
                if (FloatEqual(m_raster2DData[i][lyr], m_noDataValue)) {
                    m_raster2DData[i][lyr] = replacedv;
                }
            }
        }
    } else if (nullptr != m_rasterData) {
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            if (FloatEqual(m_rasterData[i], m_noDataValue)) {
                m_rasterData[i] = replacedv;
            }
        }
    }
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::reclassify(map<int, T> reclassMap) {
    if (m_is2DRaster && nullptr != m_raster2DData) {
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            for (int lyr = 0; lyr < m_nLyrs; lyr++) {
                map<int, float>::iterator iter = reclassMap.find((int) m_raster2DData[i][lyr]);
                if (iter != reclassMap.end()) {
                    m_raster2DData[i][lyr] = iter->second;
                } else {
                    m_raster2DData[i][lyr] = m_noDataValue;
                }
            }
        }
    } else if (nullptr != m_rasterData) {
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            map<int, float>::iterator iter = reclassMap.find((int) m_rasterData[i]);
            if (iter != reclassMap.end()) {
                m_rasterData[i] = iter->second;
            } else {
                m_rasterData[i] = m_noDataValue;
            }
        }
    }
}

/************* Utility functions ***************/

template<typename T, typename MaskT>
XYCoor clsRasterData<T, MaskT>::getCoordinateByRowCol(int row, int col) {
    double xllCenter = this->getXllCenter();
    double yllCenter = this->getYllCenter();
    double cs = this->getCellWidth();
    double nrows = this->getRows();
    return XYCoor(xllCenter + col * cs, yllCenter + (nrows - row - 1) * cs);
}

template<typename T, typename MaskT>
RowCol clsRasterData<T, MaskT>::getPositionByCoordinate(double x, double y,
                                                        map<string, double> *header /* = nullptr */) {
    if (nullptr == header) {
        header = &m_headers;
    }
    double xllCenter = (*header).at(HEADER_RS_XLL);
    double yllCenter = (*header).at(HEADER_RS_YLL);
    float dx = (float) (*header).at(HEADER_RS_CELLSIZE);
    float dy = dx;
    int nRows = (int) (*header).at(HEADER_RS_NROWS);
    int nCols = (int) (*header).at(HEADER_RS_NCOLS);

    double xmin = xllCenter - dx / 2.;
    double xMax = xmin + dx * nCols;

    double ymin = yllCenter - dy / 2.;
    double yMax = ymin + dy * nRows;
    if ((x > xMax || x < xllCenter) || (y > yMax || y < yllCenter)) {
        return RowCol(-1, -1);
    } else {
        return RowCol((int) ((yMax - y) / dy), (int) ((x - xmin) / dx));
    }
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::copyHeader(const map<string, double> &refers) {
    for (auto iter = refers.begin(); iter != refers.end(); iter++) {
        m_headers[iter->first] = iter->second;
    }
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_calculate_valid_positions_from_grid_data() {
    int oldcellnumber = m_nCells;
    /// initial vectors
    vector<T> values;
    vector<vector<T> > values2D; /// store layer 2~n
    vector<int> positionRows;
    vector<int> positionCols;
    int nrows = (int) m_headers.at(HEADER_RS_NROWS);
    int ncols = (int) m_headers.at(HEADER_RS_NCOLS);
    /// get all valid values (i.e., exclude NODATA_VALUE)
    for (int i = 0; i < nrows; ++i) {
        for (int j = 0; j < ncols; ++j) {
            int idx = i * ncols + j;
            T tempFloat;
            if (m_is2DRaster) {
                tempFloat = m_raster2DData[idx][0];
            } else {
                tempFloat = m_rasterData[idx];
            }
            if (FloatEqual(double(tempFloat), double(m_noDataValue))) continue;
            values.emplace_back(tempFloat);
            if (m_is2DRaster && m_nLyrs > 1) {
                vector<T> tmpv(m_nLyrs - 1);
                for (int lyr = 1; lyr < m_nLyrs; lyr++) {
                    tmpv[lyr - 1] = m_raster2DData[idx][lyr];
                }
                values2D.emplace_back(tmpv);
            }
            positionRows.emplace_back(i);
            positionCols.emplace_back(j);
        }
    }
    /// swap vector to save memory
    vector<T>(values).swap(values);
    if (m_is2DRaster && m_nLyrs > 1) {
        vector<vector<T> >(values2D).swap(values2D);
    }
    vector<int>(positionRows).swap(positionRows);
    vector<int>(positionCols).swap(positionCols);
    /// reCreate raster data array
    m_nCells = (int) values.size();
    m_headers.at(HEADER_RS_CELLSNUM) = m_nCells;
    if (m_is2DRaster) {
        Release2DArray(oldcellnumber, m_raster2DData);
        Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, m_noDataValue);
    } else {
        Release1DArray(m_rasterData);
        Initialize1DArray(m_nCells, m_rasterData, m_noDataValue);
    }

    /// m_rasterPositionData is nullptr till now.
    //m_rasterPositionData = new int *[m_nCells];
    Initialize2DArray(m_nCells, 2, m_rasterPositionData, 0);
    m_storePositions = true;
#pragma omp parallel for
    for (int i = 0; i < m_nCells; ++i) {
        if (m_is2DRaster) {
            m_raster2DData[i][0] = values.at(i);
            if (m_nLyrs > 1) {
                for (int lyr = 1; lyr < m_nLyrs; lyr++) {
                    m_raster2DData[i][lyr] = values2D[i][lyr - 1];
                }
            }
        } else {
            m_rasterData[i] = values.at(i);
        }
        //m_rasterPositionData[i] = new int[2];
        m_rasterPositionData[i][0] = positionRows.at(i);
        m_rasterPositionData[i][1] = positionCols.at(i);
    }
    m_calcPositions = true;
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_mask_and_calculate_valid_positions() {
    int oldcellnumber = m_nCells;
    if (nullptr == m_mask) {
        if (m_calcPositions) {
            _calculate_valid_positions_from_grid_data();
        } else {
            m_nCells = this->getRows() * this->getCols();
            m_headers.at(HEADER_RS_CELLSNUM) = m_nCells;
            // do nothing
        }
        return;
    }
    /// Use mask data
    /// 1. Get new values and positions according to Mask's position data
    vector<T> values;  /// store layer 1 data
    vector<vector<T> > values2D; /// store layer 2~n data (excluding the first layerS)
    vector<int> positionRows;
    vector<int> positionCols;
    int cols = (int) m_headers.at(HEADER_RS_NCOLS);
    int nValidMaskNumber;
    int **validPosition = nullptr;
    int maskRows = m_mask->getRows();
    int maskCols = m_mask->getCols();
    int maskCells = maskRows * maskCols;
    /// Get the position data from mask
    m_mask->getRasterPositionData(&nValidMaskNumber, &validPosition);
    /// calculate the interect extent between mask and the raster data
    int max_row = -1;
    int min_row = maskRows;
    int max_col = -1;
    int min_col = maskRows;
    /// Get the valid data according to coordinate
    for (int i = 0; i < nValidMaskNumber; ++i) {
        int tmpRow = validPosition[i][0];
        int tmpCol = validPosition[i][1];
        XYCoor tmpXY = m_mask->getCoordinateByRowCol(tmpRow, tmpCol);
        /// get current raster value by XY
        RowCol tmpPosition = this->getPositionByCoordinate(tmpXY.first, tmpXY.second);
        T tmpValue;
        /// If the mask location exceeds the extent of raster data, set to m_noDataValue.
        if (tmpPosition.first == -1 || tmpPosition.second == -1) {
            tmpValue = m_noDataValue;
            if (m_is2DRaster && m_nLyrs > 1) {
                vector<T> tmpValues(m_nLyrs - 1);
                for (int lyr = 1; lyr < m_nLyrs; lyr++) tmpValues[lyr - 1] = m_noDataValue;
                values2D.emplace_back(tmpValues);
            }
            values.emplace_back(tmpValue);
            positionRows.emplace_back(tmpRow);
            positionCols.emplace_back(tmpCol);
            continue;
        }
        if (m_is2DRaster) {
            tmpValue = m_raster2DData[tmpPosition.first * cols + tmpPosition.second][0];
            if (m_nLyrs > 1) {
                vector<T> tmpValues(m_nLyrs - 1);
                for (int lyr = 1; lyr < m_nLyrs; lyr++) {
                    tmpValues[lyr - 1] = m_raster2DData[tmpPosition.first * cols + tmpPosition.second][lyr];
                    if (FloatEqual(tmpValues[lyr - 1], m_noDataValue)) {
                        tmpValues[lyr - 1] = m_defaultValue;
                    }
                }
                values2D.emplace_back(tmpValues);
            }
        } else {
            tmpValue = m_rasterData[tmpPosition.first * cols + tmpPosition.second];
        }
        if (FloatEqual(tmpValue, m_noDataValue)) {
            tmpValue = m_defaultValue;
        } else {  /// the intersect extents dependent on the valid raster values
            if (max_row < tmpRow) max_row = tmpRow;
            if (min_row > tmpRow) min_row = tmpRow;
            if (max_col < tmpCol) max_col = tmpCol;
            if (min_col > tmpCol) min_col = tmpCol;
        }
        values.emplace_back(tmpValue);
        positionRows.emplace_back(tmpRow);
        positionCols.emplace_back(tmpCol);
    }
    /// swap vector to save memory
    vector<T>(values).swap(values);
    if (m_is2DRaster && m_nLyrs > 1) {
        vector<vector<T> >(values2D).swap(values2D);
        for (typename vector<vector<T> >::iterator iter = values2D.begin();
             iter != values2D.end(); iter++) {
            vector<T>(*iter).swap(*iter);
        }
        assert(values2D.size() == values.size());
    }
    vector<int>(positionRows).swap(positionRows);
    vector<int>(positionCols).swap(positionCols);
    assert(values.size() == positionRows.size());
    assert(values.size() == positionCols.size());

    if (!m_mask->PositionsCalculated()) Release2DArray(maskCells, validPosition);

    /// 2. Handing the header information
    /// Is the valid grid extent same as the mask data?
    bool sameExtentWithMask = true;
    int newRows = max_row - min_row + 1;
    int newCols = max_col - min_col + 1;
    if (newRows != m_mask->getRows() || newCols != m_mask->getCols()) {
        sameExtentWithMask = false;
    }
    /// 2.2a Copy header of mask data
    this->copyHeader(m_mask->getRasterHeader());
    /// 2.2b  ReCalculate the header based on the mask's header
    if (!m_useMaskExtent && !sameExtentWithMask) {
        // ||((m_useMaskExtent || sameExtentWithMask) && m_calcPositions && !m_mask->PositionsCalculated())) {
        m_headers.at(HEADER_RS_NCOLS) = double(newCols);
        m_headers.at(HEADER_RS_NROWS) = double(newRows);
        m_headers.at(HEADER_RS_XLL) += min_col * m_mask->getCellWidth();
        m_headers.at(HEADER_RS_YLL) += (m_mask->getRows() - 1 - max_row) * m_mask->getCellWidth();
        m_headers.at(HEADER_RS_CELLSIZE) = m_mask->getCellWidth();
        /// clean redundant values (i.e., NODATA)
        vector<int>::iterator rit = positionRows.begin();
        vector<int>::iterator cit = positionCols.begin();
        typename vector<T>::iterator vit = values.begin();
        typename vector<vector<T> >::iterator data2dit = values2D.begin();
        for (typename vector<T>::iterator it = values.begin(); it != values.end();) {
            int64_t idx = distance(vit, it);
            int tmpr = positionRows.at(idx);
            int tmpc = positionCols.at(idx);

            if (tmpr > max_row || tmpr < min_row || tmpc > max_col || tmpc < min_col
                || FloatEqual((T) *it, m_noDataValue)) {
                it = values.erase(it);
                if (m_is2DRaster && m_nLyrs > 1) {
                    values2D.erase(data2dit + idx);
                    data2dit = values2D.begin();
                }
                positionCols.erase(cit + idx);
                positionRows.erase(rit + idx);
                /// reset the iterators
                rit = positionRows.begin();
                cit = positionCols.begin();
                vit = values.begin();
            } else {
                /// get new column and row number
                positionRows[idx] -= min_row;
                positionCols[idx] -= min_col;
                ++it;
            }
        }
        /// swap vector to save memory
        vector<T>(values).swap(values);
        if (m_is2DRaster && m_nLyrs > 1) {
            vector<vector<T> >(values2D).swap(values2D);
        }
        vector<int>(positionRows).swap(positionRows);
        vector<int>(positionCols).swap(positionCols);
    }
    /// 2.3 Handling NoData, SRS, and Layers
    m_headers.at(HEADER_RS_NODATA) = m_noDataValue;  /// avoid to assign the Mask's NODATA
    m_srs = string(m_mask->getSRS());  /// use the coordinate system of mask data
    m_headers.at(HEADER_RS_LAYERS) = m_nLyrs;
    /// 3. Create new raster data, and handling positions data
    /// 3.1 Determine the m_nCells, and whether to allocate new position data space
    bool store_fullsize_array = false;
    if ((m_useMaskExtent || sameExtentWithMask) && m_calcPositions) {
        m_mask->getRasterPositionData(&m_nCells, &m_rasterPositionData);
        m_storePositions = false;
    } else if ((!m_useMaskExtent && !sameExtentWithMask && !m_calcPositions) ||
        ((m_useMaskExtent || sameExtentWithMask) && !m_calcPositions)) {
        // reStore raster values as fullsize array
        m_nCells = this->getCols() * this->getRows();
        store_fullsize_array = true;
        m_storePositions = false;
    } else {  // reCalculate raster positions data and store
        m_storePositions = true;
        m_nCells = (int) values.size();
    }
    m_headers.at(HEADER_RS_CELLSNUM) = m_nCells;

    /// 3.2 Release the original raster values, and create new
    ///     raster array and positions data array (if necessary)
    assert(this->validate_raster_data());
    if (m_is2DRaster && nullptr != m_raster2DData) {  // multiple layers
        Release2DArray(oldcellnumber, m_raster2DData);
        Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, m_noDataValue);
    } else {  // single layer
        Release1DArray(m_rasterData);
        Initialize1DArray(m_nCells, m_rasterData, m_noDataValue);
    }
    if (m_storePositions) Initialize2DArray(m_nCells, 2, m_rasterPositionData, 0);

    /// 3.3 Loop the masked raster values
    int ncols = (int) m_headers.at(HEADER_RS_NCOLS);
    int synthesisIdx = -1;
    for (size_t k = 0; k < positionRows.size(); ++k) {
        if (store_fullsize_array) {
            synthesisIdx = positionRows.at(k) * ncols + positionCols.at(k);
        } else if (m_storePositions && !FloatEqual(values.at(k), m_noDataValue)) {
            synthesisIdx++;
            m_rasterPositionData[synthesisIdx][0] = positionRows.at(k);
            m_rasterPositionData[synthesisIdx][1] = positionCols.at(k);
        } else {
            synthesisIdx++;
        }
        if (m_is2DRaster) {  // multiple layers
            m_raster2DData[synthesisIdx][0] = values.at(k);
            if (m_nLyrs > 1) {
                for (int lyr = 1; lyr < m_nLyrs; lyr++) {
                    m_raster2DData[synthesisIdx][lyr] = values2D[k][lyr - 1];
                }
            }
        } else {  // single layer
            m_rasterData[synthesisIdx] = values.at(k);
        }
    }
}

#endif /* CLS_RASTER_DATA */
