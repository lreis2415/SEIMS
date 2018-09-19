/*!
 * \file clsInterpolationWeightData.h
 * \brief Methods for clsITPWeightData class
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date May. 2017
 */
#ifndef SEIMS_ITP_WEIGHTDATA_H
#define SEIMS_ITP_WEIGHTDATA_H

#include "basic.h"
#include "db_mongoc.h"

using namespace ccgl;
using namespace db_mongoc;

/*!
 * \ingroup data
 * \class ItpWeightData
 *
 * \brief Read weight data of each observe stations from database
 */
class ItpWeightData: Interface {
public:
    /*!
     * \brief Overload constructor
     *
     * \param[in] gfs MongoGridFs
     * \param[in] filename
     */
    ItpWeightData(MongoGridFs* gfs, const string& filename);

    //! Destructor
    ~ItpWeightData();

    /*!
     * \brief Get the weight data read from mongoDB
     *
     * \param[out] n Rows
     * \param[out] data
     */
    void GetWeightData(int* n, float** data);

    /*!
     * \brief Output the weight data to \a ostream
     *
     * \param[out] fs
     */
    void Dump(std::ostream* fs);

    /*!
     * \brief Output the weight data to file
     * \param[in] filename
     * \sa Dump(std::ostream *fs)
     */
    void Dump(const string& filename);

    //! Initialized successful?
    bool Initialized() { return initialized_; }

private:
    /*!
     * \brief Read GridFS from MongoDB
     *
     * \param[in] gfs MongoGridFs
     * \param[in] filename
     */
    bool ReadFromMongoDB(MongoGridFs* gfs, const string& filename);

private:
    //! file name
    string filename_;
    //! iterpolation weight data array
    float* itp_weight_data_;
    //! row of weight data
    int n_rows_;
    //! column of weight data, i.e., number of stations
    int n_cols_;
    //! load data success?
    bool initialized_;
};
#endif /* SEIMS_ITP_WEIGHTDATA_H */
