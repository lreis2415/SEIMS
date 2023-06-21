/*!
 * \file clsInterpolationWeightData.h
 * \brief Methods for clsITPWeightData class
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.1
 * \date Aug, 2022
 */
#ifndef SEIMS_ITP_WEIGHTDATA_H
#define SEIMS_ITP_WEIGHTDATA_H
#include "basic.h"
#include "db_mongoc.h"

#include <seims.h>

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
     * \param[in] filename file name
     */
    ItpWeightData(MongoGridFs* gfs, const string& filename, STRING_MAP& opts);

    //! Destructor
    ~ItpWeightData();

    /*!
    * \brief Get the weight data read from mongoDB in form of 2DArray
    *
    * \param[out] n Rows
    * \param[out] n_stations Cols
    * \param[out] data data
    */
    void GetWeightData2D(int* n, int* n_stations, FLTPT*** data);
	// xdw modify, to support multi-stations itp weight data, we have to get the length of itp weight data array so that initialize it
	void GetWeightData(int* n, FLTPT** data, int *itp_weight_data_length);
    /*!
     * \brief Output the weight data to \a ostream
     */
    void Dump(std::ostream* fs);

    /*!
     * \brief Output the weight data to file
     * \param[in] filename file name
     * \sa Dump(std::ostream *fs)
     */
    void Dump(const string& filename);

    //! Initialized successful?
    bool Initialized() { return initialized_; }

private:
    /*!
     * \brief Read GridFS from MongoDB
     * \param[in] gfs MongoGridFs
     * \param[in] filename file name
     */
    bool ReadFromMongoDB(MongoGridFs* gfs, const string& filename, STRING_MAP& opts);

private:
    //! file name
    string filename_;
    //! interpolation weight data array
    FLTPT* itp_weight_data_;
    //! interpolation weight data array (2DArray)
    FLTPT** itp_weight_data2d_;
    //! row of weight data
    int n_rows_;
    //! column of weight data, i.e., number of stations
    int n_cols_;
    //! load data success?
    bool initialized_;
};
#endif /* SEIMS_ITP_WEIGHTDATA_H */
