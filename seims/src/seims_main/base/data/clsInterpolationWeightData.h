/*!
 * \brief Methods for clsInterpolationWeightData class
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date May. 2016
 */
#pragma once
#include "text.h"
#include "utilities.h"
#include "MongoUtil.h"

using namespace std;

/*!
 * \ingroup data
 * \class clsInterpolationWeightData
 *
 * \brief 
 */
class clsInterpolationWeightData {
public:
    //! Constructor
    clsInterpolationWeightData(string weightFileName);

    /*!
     * \brief Overload constructor
     *
     * \param[in] gfs
     * \param[in] remoteFilename
     */
    clsInterpolationWeightData(mongoc_gridfs_t *gfs, const char *remoteFilename);

    //! Deconstructor
    ~clsInterpolationWeightData(void);

    /*!
     * \brief Get the weight data read from mongoDB
     *
     * \param[out] n Rows
     * \param[out] data
     */
    void getWeightData(int *, float **);

    /*!
     * \brief Output the weight data to \a ostream
     *
     * \param[out] fs
     */
    void dump(ostream *fs);

    /*!
     * \brief Output the weight data to file
     *
     * \param[in] fs \a ostream using dump(ostream *fs)
     * \param[out] fileName
     * \sa dump(ostream *fs)
     */
    void dump(string);

private:
    //! file name
    string m_fileName;
    //! weight data array
    float *m_weightData;
    //! row of weight data
    int m_nRows;
    //! column of weight data
    int m_nCols;

private:
    /*!
     * \brief Read GridFS from MongoDB
     *
     * \param[in] gfs
     * \param[in] filename
     */
    void ReadFromMongoDB(mongoc_gridfs_t *gfs, const char *filename);
};

