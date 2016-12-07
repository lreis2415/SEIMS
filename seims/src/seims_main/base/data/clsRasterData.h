/*!
 * \ingroup data
 * \brief Define Raster class to handle raster data
 *
 * 1. Using GDAL and MongoDB (currently, mongo-c-driver 1.3.5)
 * 2. Array1D and Array2D raster data are supported
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date Apr. 2011
 * \revised May. 2016
 * 
 */
#pragma once

#include <string>
#include <map>
#include "util.h"
#include "mongoc.h"
#include "MongoUtil.h"

using namespace std;

/*!
 * \ingroup data
 * \class clsRasterData
 *
 * \brief Raster data (1D and 2D) class to get access to raster data in MongoDB.
 */
template<typename T>
class clsRasterData
{
public:
    /*!
     * \brief Constructor of clsRasterData instance from ASCII file
     * By default, 1D raster data
     * \sa ReadASCFile()
     *
     * \param[in] ascFileName ASCII file path
     */
    clsRasterData(string);

    /*!
     * \brief Constructor an empty clsRasterData instance
     * By default, 1D raster data
     * Set \a m_rasterPositionData, \a m_rasterData, \a m_mask to \a NULL
     */
    clsRasterData(void);

    /*!
     * \brief Constructor of clsRasterData instance from  ASCII file and mask clsRasterData
     * By default, 1D raster data
     * \sa ReadASCFile()
     * \param[in] ascFileName \a string
     * \param[in] mask \a clsRasterData
     */
    clsRasterData<T>(string, clsRasterData<T> *);

    /*!
     * \brief Constructor of clsRasterData instance from mongoDB
     * By default, 1D raster data
     * \sa ReadFromMongoDB()
     *
     * \param[in] gfs \a mongoc_gridfs_t
     * \param[in] romoteFilename \a char*
     * \param[in] templateRaster \a clsRasterData, NULL as default
     */
    clsRasterData<T>(mongoc_gridfs_t *gfs, const char *remoteFilename, clsRasterData<T> *templateRaster = NULL);

    //! Destructor, Set \a m_rasterPositionData, \a m_rasterData, \a m_mask to \a NULL
    ~clsRasterData(void);

    //! Get the average of raster data
    float getAverage();

    //! Get the average of the given layer of raster data
    float getAverage(int lyr);

    //! Get column number of raster data
    int getCols() { return (int) m_headers[HEADER_RS_NCOLS]; }

    //! Get row number of raster data
    int getRows() { return (int) m_headers[HEADER_RS_NROWS]; }

    //! Get cell size of raster data
    float getCellWidth() { return (float) m_headers[HEADER_RS_CELLSIZE]; }

    //! Get cell numbers ignore NoData
    int getCellNumber() { return m_nCells; }

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
    map<string, double> *getRasterHeader(void);

    //! Get non-NODATA position index data, include cell number and (row, col)
    void getRasterPositionData(int *, T ***);

    //! Get pointer of raster data
    float *getRasterDataPointer() { return m_rasterData; }

    //! Get pointer of 2D raster data
    float **get2DRasterDataPointer() { return m_raster2DData; }

    //! Get the spatial reference
    const char *getSRS() { return m_srs.c_str(); }
	//! Get the spatial reference string
	string getSRSString() { return m_srs; }
    //! Get raster data at the valid cell index
    T getValue(int validCellIndex);

    //! Get raster data at the valid cell index (both for 1D and 2D raster), return a float array with length as nLyrs
    T *getValue(int validCellIndex, int *nLyrs);

    //! Get raster data at the row and col
    T getValue(int row, int col);

    //! Get raster data (both for 1D and 2D raster) at the row and col, return a float array with length as nLyrs
    T *getValue(int row, int col, int *nLyrs);

    //! Get raster data value at row and column of \a templateRasterData and \a rasterData
    T getValue(clsRasterData *templateRasterData, T *rasterData, int row, int col);

    //! Get raster data value at row and column of \a templateRasterData and \a rasterData
    T *getValue(clsRasterData *templateRasterData, T *rasterData, int row, int col, int *nLyrs);

    //! Get X coordinate of left lower corner of raster data
    double getXllCenter() { return m_headers[HEADER_RS_XLL]; }

    //! Get Y coordinate of left lower corner of raster data
    double getYllCenter() { return m_headers[HEADER_RS_YLL]; }

    //! Is 2D raster data?
    bool is2DRaster() { return m_is2DRaster; }

    //! Write raster to ASC Grid file, if 2D raster, output name will be filename_LyrNum
    void outputASCFile(string &filename);

    /*!
     * \brief Write raster data into ASC file
     *
     * \param[in] header header information
     * \param[in] nRows \a int, valid cell number
     * \param[in] position \a float**, position index
     * \param[in] value \a T*, Raster data
     * \param[in] filename \a string, output ASC file path
     */
    static void outputASCFile(map<string, double>, int, float **, T *, string);

    /*!
     * \brief Write 2D raster data into ASC file
     *
     * \param[in] header header information
     * \param[in] nRows \a int, valid cell number
     * \param[in] position \a float**, position index
     * \param[in] value \a T**, 2D Raster data
     * \param[in] filename \a string, output ASC file path, take the CoreName as prefix
     */
    static void outputASCFile(map<string, double>, int, float **, T **, string);

    /*!
     * \brief Write raster data into ASC file
     *
     * \param[in] templateRasterData
     * \param[in] value \a T*, Raster data
     * \param[in] filename \a string, output ASC file path
     */
    static void outputASCFile(clsRasterData *, T *, string);

    /*!
     * \brief Write 2D raster data into ASC file
     *
     * \param[in] templateRasterData \sa  clsRasterData
     * \param[in] value \a T**, 2D Raster data
     * \param[in] filename \a string, output ASC file path
     */
    static void outputASCFile(clsRasterData *, T **, string);

    //! Write raster to GTIFF Grid file, if 2D raster, output name will be filename_LyrNum
    void outputGTiff(string filename);

    /*!
     * \brief Write raster data into GTIFF file
     *
     * \param[in] templateRasterData \sa  clsRasterData
     * \param[in] value \a T*, Raster data
     * \param[in] rasterName \a string, output GTIFF file path
     */
    static void outputGTiff(clsRasterData *templateRasterData, T *value, string rasterName);

    /*!
     * \brief Write raster data into GTIFF file
     *
     * \param[in] header header information
     * \param[in] srs spatial reference string
     * \param[in] nValidCells \a int, valid cell number
     * \param[in] position \a float**, position index
     * \param[in] value \a T*, Raster data
     * \param[in] filename \a string, output GTIFF file path
     */
    static void outputGTiff(map<string, double> header, string &srs, int nValidCells, float **position, T *value,
                            string filename);

    /*!
     * \brief Write 2D raster data into GTIFF file
     *
     * \param[in] templateRasterData
     * \param[in] value \a T**, Raster data
     * \param[in] rasterName \a string, output GTIFF file path
     */
    static void outputGTiff(clsRasterData *templateRasterData, T **value, string rasterName);

    /*!
     * \brief Write 2D raster data into GTIFF file
     *
     * \param[in] header header information
     * \param[in] srs spatial reference string
     * \param[in] nValidCells \a int, valid cell number
     * \param[in] position \a float**, position index
     * \param[in] value \a T**, Raster data
     * \param[in] filename \a string, output GTIFF file path
     */
    static void outputGTiff(map<string, double> header, string &srs, int nValidCells, float **position, T **value,
                            string filename);
	/*
	 * \brief Write 2D-array raster data into GTIFF file
	 *        Used when valid position data is not available.
	 */
	static void outputGTiff(map<string, double> header, string &srs, T *value, string &filename);
    //! Write raster data to MongoDB, if 2D raster, output name will be filename_LyrNum
    void outputToMongoDB(string remoteFilename, mongoc_gridfs_t *gfs);

    /*!
     * \brief Write raster data into MongoDB
     *
     * \param[in] header header information
     * \param[in] srs spatial reference string
     * \param[in] nValidCells \a int, valid cell number
     * \param[in] position \a float**, position index
	 * \param[in] value \a T*, Raster data
     * \param[in] filename \a string, output file name
     * \param[in] gfs \a mongoc_gridfs_t
     */
    static void outputToMongoDB(map<string, double> header, string &srs, int nValid, float **position, T *value,
                                string remoteFilename, mongoc_gridfs_t *gfs);

    /*!
     * \brief Write raster data into MongoDB
     *
     * \param[in] templateRasterData \a clsRasterData
     * \param[in] value \a T*, Raster data
     * \param[in] filename \a string, output file name
     * \param[in] gfs \a mongoc_gridfs_t
     */
    static void outputToMongoDB(clsRasterData *templateRasterData, T *value, string filename, mongoc_gridfs_t *gfs);

    /*!
     * \brief Write 2D raster data into MongoDB
     *
     * \param[in] header header information
     * \param[in] srs spatial reference string
     * \param[in] nValidCells \a int, valid cell number
     * \param[in] position \a float**, position index
     * \param[in] value \a T** 2D RasterData
     * \param[in] lyrs \a layer number
     * \param[in] filename \a string, output file name
     * \param[in] gfs \a mongoc_gridfs_t
     */
    static void outputToMongoDB(map<string, double> header, string &srs, int nValid, float **position, T **value,
                                int lyrs, string remoteFilename, mongoc_gridfs_t *gfs);

    /*!
     * \brief Write 2D raster data into MongoDB
     *
     * \param[in] templateRasterData \a clsRasterData
     * \param[in] value \a T**, Raster data with multi-layers, e.g., soil properties
     * \param[in] lyrs \a layer number
     * \param[in] filename \a string, output file name
     * \param[in] gfs \a mongoc_gridfs_t
     */
    static void outputToMongoDB(clsRasterData *templateRasterData, T **value, int lyrs, string filename,
                                mongoc_gridfs_t *gfs);

    /*!
     * \brief Write weight file according the weight value
     * \param[in] templateRasterData \a clsRasterData
     * \param[in] nCols \a int i.e., HydroClimate site number
     * \param[in] weight \a float
     * \param[in] filename \a char*, weight file name
     */
    static void outputWeightFile(clsRasterData *, int, float, string);

    /*!
     * \brief Read raster data from ASC file
     * \param[in] ascFileName \a string
     */
    void ReadASCFile(string);

    /*!
     * \brief Read raster data from ASC file, using mask
     * Be aware, this mask should have the same extent with the raster
     * i.e., NROWS and NCOLS are the same!
     * \param[in] ascFileName \a string
     * \param[in] mask \a clsRasterData
     */
    void ReadASCFile(string, clsRasterData *mask);

    /*!
     * \brief Read raster data using GDAL
     *
     * \param[in] filename \a string
     */
    void ReadFromGDAL(string filename);

    /*!
     * \brief Read raster data according the the given mask file using GDAL
     * Be aware, this mask should have the same extent with the raster
     * i.e., NROWS and NCOLS are the same!
     * \param[in] filename \a string
     * \param[in] mask \a clsRasterData
     */
    void ReadFromGDAL(string filename, clsRasterData *mask);

    /*!
     * \brief Read raster data from MongoDB
     * \param[in] gfs \a mongoc_gridfs_t
     * \param[in] remoteFilename \a char*, raster file name
     */
    int ReadFromMongoDB(mongoc_gridfs_t *gfs, const char *remoteFilename);

    /*!
     * \brief Read raster data according the the given mask file from MongoDB
     * Be aware, this mask should have the same extent with the raster
     * i.e., NROWS and NCOLS are the same!
     * \param[in] gfs \a mongoc_gridfs_t
     * \param[in] remoteFilename \a char*, raster file name
     * \param[in] mask \a clsRasterData
     */
    int ReadFromMongoDB(mongoc_gridfs_t *gfs, const char *remoteFilename, clsRasterData *mask);

    /*!
     * \brief Get cell number
     * \sa getCellNumber()
     */
    int Size() { return m_nCells; }

private:
	///< noDataValue
	T m_noDataValue;
    ///< raster file name
    string m_fileName;
    ///< raster data (1D array)
    T *m_rasterData;
    ///< cell index (row, col) in m_rasterData (2D array)
    float **m_rasterPositionData;
    ///< cell number of raster data (exclude NODATA_VALUE)
    int m_nCells;
    ///< Header information, using double in case of truncation of coordinate value
    map<string, double> m_headers;
    ///< mask clsRasterData instance
    clsRasterData<float> *m_mask;

    //! raster data (2D array)
    T **m_raster2DData;
    //! Flag to identify 1D or 2D raster
    bool m_is2DRaster;
    //! Layer number of the 2D raster
    int m_nLyrs;
    //! OGRSpatialReference
    string m_srs;

//private:
//	void DeleteExistingData(void);
//public:
//	bool IsNull(int i, int j);
//	bool IsEmpty(void);
};

