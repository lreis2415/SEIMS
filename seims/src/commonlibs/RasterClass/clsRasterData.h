/*!
 * \ingroup data
 * \brief Define Raster class to handle raster data
 *
 * 1. Using GDAL and MongoDB (currently, mongo-c-driver 1.5.0 is supported)
 * 2. Array1D and Array2D raster data are supported
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date Apr. 2011
 * \revised May. 2016
 * \revised Dec. 2016 Separated from SEIMS to a common library for widely use.
 * 
 */
#ifndef CLS_RASTER_DATA
#define CLS_RASTER_DATA
/// include base headers
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
#include <typeinfo>
/// include GDAL, required
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"
/// include MongoDB, optional
#ifdef USE_MONGODB
#include "MongoUtil.h"
#endif
/// include openmp if supported
#ifdef SUPPORT_OMP
#include <omp.h>
#endif
/// include utility functions
#include "utilities.h"

using namespace std;

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
#define HEADER_RS_CELLSNUM		"CELLSNUM"
#define HEADER_RS_SRS           "SRS"

/*!
 * Define constant strings of statistics index
 */
#define STATS_RS_VALIDNUM		"VALID_CELLNUMBER"
#define STATS_RS_MEAN			"MEAN"
#define STATS_RS_MIN			"MIN"
#define STATS_RS_MAX			"MAX"
#define STATS_RS_STD			"STD"
#define STATS_RS_RANGE			"RANGE"

/*!
 * Files or database constant strings
 */
#define ASCIIExtension          "asc"
#define GTiffExtension          "tif"

/*!
 * \struct Coordinate of row and col
 */
struct RowColCoor{
	int row;
	int col;
};

/*!
 * \class clsRasterData
 *
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
     * Set \a m_rasterPositionData, \a m_rasterData, \a m_mask to \a NULL
     */
    clsRasterData(void);
    /*!
     * \brief Constructor of clsRasterData instance from TIFF, ASCII, or other GDAL supported raster file
     * Support 1D and/or 2D raster data
     * \sa ReadASCFile() ReadFromGDAL()
	 * \param[in] filename Full path of the raster file
	 * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
	 * \param[in] mask \a clsRasterData<MaskT> Mask layer
     * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     *
     */
    clsRasterData(string filename, bool calcPositions = true, clsRasterData<MaskT> *mask = NULL, bool useMaskExtent = true);
	/*!
     * \brief Constructor of clsRasterData instance from TIFF, ASCII, or other GDAL supported raster file
     * Support 1D and/or 2D raster data
     * \sa ReadASCFile() ReadFromGDAL()
	 * \param[in] filenames Full paths vector of the 2D raster data
	 * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
	 * \param[in] mask \a clsRasterData<MaskT> Mask layer
     * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     */
    clsRasterData(vector<string> filenames, bool calcPositions = true, clsRasterData<MaskT> *mask = NULL, bool useMaskExtent = true);
	/*!
     * \brief Constructor an clsRasterData instance by 1D array data and mask
     */
    clsRasterData(clsRasterData<MaskT> *mask, T*& values);
	/*!
     * \brief Constructor an clsRasterData instance by 2D array data and mask
     */
    clsRasterData(clsRasterData<MaskT> *mask, T**& values, int lyrs);
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
    clsRasterData(mongoc_gridfs_t *gfs, const char *remoteFilename, bool calcPositions = true, clsRasterData<MaskT> *mask = NULL, bool useMaskExtent = true);
#endif
	//! Destructor, release \a m_rasterPositionData, \a m_rasterData and \a m_mask if existed.
    ~clsRasterData(void);

    /************* Read functions ***************/

    /*!
     * \brief Read raster data from ASC file, mask data is optional
     * \param[in] filename \a string
	 * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
	 * \param[in] mask \a clsRasterData<MaskT>
	 * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     */
    void ReadASCFile(string filename, bool calcPositions = true, clsRasterData<MaskT> *mask = NULL, bool useMaskExtent = true);
	/*!
     * \brief Read raster data by GDAL, mask data is optional
     * \param[in] filename \a string
	 * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
	 * \param[in] mask \a clsRasterData<MaskT>
	 * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     */
    void ReadByGDAL(string filename, bool calcPositions = true, clsRasterData<MaskT> *mask = NULL, bool useMaskExtent = true);

#ifdef USE_MONGODB
    /*!
     * \brief Read raster data from MongoDB
     * \param[in] gfs \a mongoc_gridfs_t
	 * \param[in] filename \a char*, raster file name
	 * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
	 * \param[in] mask \a clsRasterData<MaskT>
	 * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     */
    void ReadFromMongoDB(mongoc_gridfs_t *gfs, string filename, bool calcPositions = true, clsRasterData<MaskT> *mask = NULL, bool useMaskExtent = true);
#endif
    /************* Write functions ***************/

    /*!
     * \brief Write raster to raster file, if 2D raster, output name will be filename_LyrNum
     * \param filename filename with prefix, e.g. ".asc" and ".tif"
     */
    void outputToFile(string filename);
	/*!
     * \brief Write 1D or 2D raster data into ASC file(s)
     * \param[in] filename \a string, output ASC file path, take the CoreName as prefix
     */
    void outputASCFile(string filename);
	/*!
     * \brief Write 1D or 2D raster data into TIFF file by GDAL
     * \param[in] filename \a string, output TIFF file path
     */
    void outputFileByGDAL(string filename);

#ifdef USE_MONGODB
	/*!
     * \brief Write raster data (matrix raster data) into MongoDB
     * \param[in] filename \a string, output file name
     * \param[in] gfs \a mongoc_gridfs_t
     */
    void outputToMongoDB(string filename, mongoc_gridfs_t *gfs);
#endif
	/************* Get information functions ***************/
	
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
	 */
	double getStatistics(string sindex, int lyr = 1);

	/*! 
	 * \brief Get basic statistics values for 2D raster data.
	 * Mean, Max, Min, STD, Range, etc.
	 * \param[in] sindex \string, case insensitive
	 * \param[out] lyrnum \int, layer number
	 * \param[out] values \double* Statistics array
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
	 * \sa getAverage
	 */
	float getRange(int lyr = 1) { return (float) this->getStatistics(STATS_RS_RANGE, lyr); }

	/*! 
	 * \brief Get the non-NoDATA number of raster data
	 * \sa getAverage
	 */
	int getValidNumber(int lyr = 1) { return (int)this->getStatistics(STATS_RS_VALIDNUM, lyr); }

	/*! 
	 * \brief Get the average of 2D raster data
	 * \param[out] lyrnum \int, layer number
	 * \param[out] values \double* Statistics array
	 */
	void getAverage(int *lyrnum, double **values) { 
		this->getStatistics(STATS_RS_MEAN, lyrnum, values); }

	/*! 
	 * \brief Get the maximum of 2D raster data
	 * \sa getAverage
	 */
	void getMaximum(int *lyrnum, double **values) { 
		this->getStatistics(STATS_RS_MAX, lyrnum, values); }

	/*! 
	 * \brief Get the minimum of 2D raster data
	 * \sa getAverage
	 */
	void getMinimum(int *lyrnum, double **values) { 
		this->getStatistics(STATS_RS_MIN, lyrnum, values); }

	/*! 
	 * \brief Get the standard derivation of 2D raster data
	 * \sa getAverage
	 */
	void getSTD(int *lyrnum, double **values) { 
		this->getStatistics(STATS_RS_STD, lyrnum, values); }

	/*! 
	 * \brief Get the range of 2D raster data
	 * \sa getAverage
	 */
	void getRange(int *lyrnum, double **values) { 
		this->getStatistics(STATS_RS_RANGE, lyrnum, values); }

	/*! 
	 * \brief Get the non-NoDATA number of 2D raster data
	 * \sa getAverage
	 */
	void getValidNumber(int *lyrnum, double **values) { 
		this->getStatistics(STATS_RS_VALIDNUM, lyrnum, values); }

	//! Get stored cell number of raster data
	int getCellNumber() { return m_nCells; }

    //! Get column number of raster data
    int getCols() { return (int) m_headers[HEADER_RS_NCOLS]; }

    //! Get row number of raster data
    int getRows() { return (int) m_headers[HEADER_RS_NROWS]; }

    //! Get cell size of raster data
    float getCellWidth() { return (float) m_headers[HEADER_RS_CELLSIZE]; }

	//! Get X coordinate of left lower corner of raster data
	double getXllCenter() { return m_headers[HEADER_RS_XLL]; }

	//! Get Y coordinate of left lower corner of raster data
	double getYllCenter() { return m_headers[HEADER_RS_YLL]; }

    //! Get the first dimension size of raster data
    int getDataLength() { return m_nCells; }

    int getLayers() { return m_nLyrs; }

    //! Get NoDATA value of raster data
    T getNoDataValue() { return (T) m_headers[HEADER_RS_NODATA]; }

    //! Get position index in 1D raster data for specific row and column, return -1 is error occurs.
    int getPosition(int row, int col);

    //! Get position index in 1D raster data for given coordinate (x,y)
    int getPosition(float x, float y);

    //! Get position index in 1D raster data for given coordinate (x,y)
    int getPosition(double x, double y);

    //! Get raster data, include valid cell number and data
    void getRasterData(int *, T **);

    //! Get 2D raster data, include valid cell number of each layer, layer number, and data
    void get2DRasterData(int *, int *, T ***);

    //! Get raster header information
	map<string, double> *getRasterHeader(void){ return &m_headers; }
	//! Get raster statistics information
	map<string, double> *getStatistics(void){ return &m_statsMap; }
	//! Get full path name
	string getFilePath() { return m_filePathName; }
	//! Get core name
	string getCoreName() { return m_coreFileName; }
	/*!
	 * \brief Get position index data and the data length
	 * \param[out] datalength 
	 * \param[out] positiondata
	 */
    void getRasterPositionData(int& datalength, int**& positiondata);

    //! Get pointer of raster data
    T*& getRasterDataPointer() { return m_rasterData; }
	//! Get pointer of position data
	int**& getRasterPositionDataPointer() { return m_rasterPositionData; }
    //! Get pointer of 2D raster data
    T**& get2DRasterDataPointer() { return m_raster2DData; }

    //! Get the spatial reference
    const char *getSRS() { return m_srs.c_str(); }

    //! Get the spatial reference string
    string getSRSString() { return m_srs; }

    /*! 
	 * \brief Get raster data at the valid cell index
	 * The default lyr is 1, which means the 1D raster data, or the first layer of 2D data.
	 */
    T getValue(int validCellIndex, int lyr = 1);
	/*! 
	 * \brief Get raster data via row and col
	 * The default lyr is 1, which means the 1D raster data, or the first layer of 2D data.
	 */
    T getValue(RowColCoor pos, int lyr = 1);
	/*!
	 * \brief Set value to the given position and layer
	 */
	void setValue(RowColCoor pos, T value, int lyr = 1);
	/*! 
	 * \brief Check if the raster data is NoData via row and col
	 * The default lyr is 1, which means the 1D raster data, or the first layer of 2D data.
	 */
    T isNoData(RowColCoor pos, int lyr = 1);
    /*! 
	 * \brief Get raster data at the valid cell index (both for 1D and 2D raster)
	 * \return a float array with length as nLyrs
	 */
    void getValue(int validCellIndex, int *nLyrs, T** values);

    /*! 
	 * \brief Get raster data (both for 1D and 2D raster) at the row and col
	 * \return a float array with length as nLyrs
	 */
    void getValue(RowColCoor pos, int *nLyrs, T** values);

    //! Is 2D raster data?
    bool is2DRaster() { return m_is2DRaster; }

	//! Calculate positions or not
	bool PositionsCalculated() { return m_calcPositions; }

	//! Use mask extent or not
	bool MaskExtented() { return m_useMaskExtent; }
	bool StatisticsCalculated() { return m_statisticsCalculated; }
	//! Get full filename
	string GetFullFileName() { return m_filePathName; }
	//! Get mask data pointer
	clsRasterData<MaskT>* getMask() { return m_mask; }
	/*!
	 * \brief Copy clsRasterData object
	 */
	void Copy(clsRasterData &orgraster);
	/*!
	 * \brief Replace NoData value by the given value
	 */
	void replaceNoData(T replacedv);
    /************* Utility functions ***************/

    /*!
     * \brief Calculate XY coordinates by given row and col number
	 * \sa getPositionByCoordinate
     * \param[in] row
     * \param[in] col
     * \return double[2]
     */
    double* getCoordinateByRowCol(int row, int col);

    /*!
     * \brief Calculate position by given coordinate
	 * \sa getCoordinateByRowCol
     * \param[in] x
     * \param[in] y
	 * \param[in] header Optional, header map of raster layer data, the default is m_header
     * \return int[2] of row and col index
     */
    int* getPositionByCoordinate(double x, double y, map<string, double> *header = NULL);

    /*!
     * \brief Copy header information to current Raster data
     * \param[in] refers
     */
    void copyHeader(map<string, double> *refers);

private:
	/*!
	 * \brief Initialize all raster related variables.
	 */
	void _initialize_raster_class();
	/*!
	 * \brief Initialize read function for ASC, GDAL, and MongoDB
	 */
	void _initialize_read_function(string filename, bool calcPositions = true, clsRasterData<MaskT> *mask = NULL, bool useMaskExtent = true);
	/*!
	 * \brief check the existence of given raster file
	 * \return True if existed, else false
	 */
	bool _check_raster_file_exists(string);
	/*!
	 * \brief check the existence of given vector of raster files
	 * \return True if all existed, else false
	 */
	bool _check_raster_file_exists(vector<string>&);
	/*!
     * \brief Constructor of clsRasterData instance from single file of TIFF, ASCII, or other GDAL supported raster file
     * Support 1D and/or 2D raster data
     * \sa ReadASCFile() ReadFromGDAL()
	 * \param[in] filename Full path of the raster file
	 * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
	 * \param[in] mask \a clsRasterData<T2> Mask layer
     * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     *
     */
    void _construct_from_single_file(string filename, bool calcPositions = true, clsRasterData<MaskT> *mask = NULL, bool useMaskExtent = true);

	/*!
     * \brief Read raster data from ASC file, the simply usage
     * \param[in] ascFileName \a string
	 * \param[out] header Raster header information
	 * \param[out] values Raster data matrix
     */
    void _read_asc_file(string ascFileName, map<string, double> *header, T**values);
	/*!
     * \brief Read raster data by GDAL, the simply usage
     * \param[in] filename \a string
	 * \param[out] header Raster header information
	 * \param[out] values Raster data matrix
     */
    void _read_raster_file_by_gdal(string filename, map<string, double> *header, T**values, string* srs = NULL);
	/*!
	 * \brief Extract by mask data and calculate position index, if necessary.
	 */
	void _mask_and_calculate_valid_positions();

	/*!
	 * \brief Calculate position index from rectangle grid values, if necessary.
	 * To use this function, mask should be NULL.
	 * 
	 */
	void _calculate_valid_positions_from_grid_data();
	/*!
     * \brief Write raster header information into ASC file
     * If the file exists, delete it first.
     * \param[in] filename \a string, output ASC file path
     * \param[in] header header information
     */
	void _write_ASC_headers(string filename, map<string, double>& header);
	/*!
     * \brief Write single geotiff file
     * If the file exists, delete it first.
     * \param[in] filename \a string, output ASC file path
     * \param[in] header header information
	 * \param[in] srs Coordinate system string
	 * \param[in] values float raster data array
     */
	void _write_single_geotiff(string filename, map<string, double>& header, string srs, float *values);
#ifdef USE_MONGODB
	/*!
     * \brief Write full-sized raster data as GridFS file
     * If the file exists, delete it first.
     * \param[in] filename \a string, GridFS file name
     * \param[in] header header information
	 * \param[in] srs Coordinate system string
	 * \param[in] values float raster data array
     */
	void _write_stream_data_as_gridfs(mongoc_gridfs_t* gfs, string filename, map<string, double>& header, string srs, T *values, int datalength);
#endif
	/*!
	 * \brief Add other layer's rater data to m_raster2DData
	 * \param[in] row Row number be added on, e.g. 2
	 * \param[in] col Column number be added on, e.g. 3
	 * \param[in] cellidx Cell index in m_raster2DData, e.g. 23
	 * \param[in] lyr Layer number which is greater than 1, e.g. 2, 3, ..., n
	 * \param[in] lyrheader Header information of current layer
	 * \param[in] lyrdata Raster layer data
	 */
	void _add_other_layer_raster_data(int row, int col, int cellidx, int lyr, map<string, double> lyrheader, T *lyrdata);
private:
    /*! cell number of raster data, i.e. the data length of \sa m_rasterData or \sa m_raster2DData
	 * 1. all grid cell number, i.e., ncols*nrows, when m_calcPositions is False
	 * 2. valid cell number excluding NoDATA, when m_calcPositions is True and m_useMaskExtent is False.
	 * 3. including NoDATA where mask is valid, when m_useMaskExtent is True.
	 */
    int m_nCells;
    ///< noDataValue
    T m_noDataValue;
    ///< raster full path, e.g. "C:/tmp/studyarea.tif"
    string m_filePathName;
	///< core raster file name, e.g. "studyarea"
	string m_coreFileName;
    ///< calculate valid positions or not. The default is true.
    bool m_calcPositions;
	///< To be consistent with other datesets, keep the extent of Mask layer, even include NoDATA.
	bool m_useMaskExtent;
    ///< raster data (1D array)
    T *m_rasterData;
    ///< cell index (row, col) in m_rasterData (2D array)
    int **m_rasterPositionData;
    ///< Header information, using double in case of truncation of coordinate value
    map<string, double> m_headers;
    ///< mask clsRasterData instance
    clsRasterData<MaskT> *m_mask;
    //! raster data (2D array)
    T **m_raster2DData;
    //! Flag to identify 1D or 2D raster
    bool m_is2DRaster;
    //! Layer number of the 2D raster
    int m_nLyrs;
    //! OGRSpatialReference
    string m_srs;
	//! Statistics calculated?
	bool m_statisticsCalculated;
	//! Map to store basic statistics values for 1D raster data
	map<string, double> m_statsMap;
	//! Map to store basic statistics values for 2D raster data
	map<string, double*> m_statsMap2D;
	//! initial once
	bool m_initialized;
};

#endif