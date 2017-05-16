/*!
 * \brief Methods for clsITPWeightData class
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date May. 2017
 */
#ifndef SEIMS_ITP_WEIGHTDATA_H
#define SEIMS_ITP_WEIGHTDATA_H

#include "text.h"
#include "utilities.h"
#include "MongoUtil.h"

using namespace std;

/*!
 * \ingroup data
 * \class clsITPWeightData
 *
 * \brief 
 */
class clsITPWeightData {
public:
    //! Constructor
    //clsITPWeightData(string weightFileName);

    /*!
     * \brief Overload constructor
     *
     * \param[in] gfs MongoGridFS
     * \param[in] remoteFilename
     */
    clsITPWeightData(MongoGridFS* gfs, const char *remoteFilename);

    //! Destructor
    ~clsITPWeightData(void);

    /*!
     * \brief Get the weight data read from mongoDB
     *
     * \param[out] n Rows
     * \param[out] data
     */
    void getWeightData(int* Rows, float** data);

    /*!
     * \brief Output the weight data to \a ostream
     *
     * \param[out] fs
     */
    void dump(ostream *fs);

    /*!
     * \brief Output the weight data to file
     * \param[in] fileName
     * \sa dump(ostream *fs)
     */
    void dump(string fineName);

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
     * \param[in] gfs MongoGridFS
     * \param[in] filename
     */
    void ReadFromMongoDB(MongoGridFS* gfs, const char *filename);
};
#endif /* SEIMS_ITP_WEIGHTDATA_H */
