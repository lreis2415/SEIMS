/*!
 * \brief Point source pollution and BMP factory
 * \author Liang-Jun Zhu
 * \date July 2016
 *
 */
#ifndef SEIMS_BMP_POINTSOURCE_H
#define SEIMS_BMP_POINTSOURCE_H

#include "utilities.h"
#include "BMPFactory.h"

using namespace MainBMP;

namespace MainBMP {
/*!
 * \class PointSourceLocations
 * \ingroup MainBMP
 *
 * \brief Base class of point BMP, mainly store location related parameters
 *
 */
class PointSourceLocations {
public:
    /*!
     * \brief Constructor, parse point BMP location related parameters from bson object
     * \param[in] bsonTab Query result from MongoDB
     * \param[in] iter Iterator of bsonTab
     */
    PointSourceLocations(const bson_t *&bsonTab, bson_iter_t &iter);

    /// Destructor
    ~PointSourceLocations(void);

    /// Output
    void Dump(ostream *fs);

    /// Get point source ID
    int GetPointSourceID(void) {
        return m_pointSrcID;
    }

    /// name
    string GetPointSourceName(void) {
        return m_name;
    }

    /// Lat
    float GetLat(void) {
        return m_lat;
    }

    /// Lon
    float GetLon(void) {
        return m_lon;
    }

    /// localX
    float GetLocalX(void) {
        return m_localX;
    }

    /// localY
    float GetLocalY(void) {
        return m_localY;
    }

    /// Located subbasin ID
    int GetSubbasinID(void) {
        return m_subbasinID;
    }

    /// size
    float GetSize(void) {
        return m_size;
    }

    /// Distance to the downstream reach
    float GetDistanceDown(void) {
        return m_distDown;
    }

private:
    /// ID of point source
    int m_pointSrcID;
    /// name
    string m_name;
    /// Lat
    float m_lat;
    /// Lon
    float m_lon;
    /// localX
    float m_localX;
    /// localY
    float m_localY;
    /// Located subbasin ID
    int m_subbasinID;
    /// size
    float m_size;
    /// Distance to the downstream reach
    float m_distDown;
};

/*!
 * \class PointSourceParameters
 * \ingroup MainBMP
 *
 * \brief Point source management parameters
 *
 */
class PointSourceMgtParams {
public:
    /*!
     * \brief Constructor, parse point source management parameters from bson object
     * \param[in] bsonTab Query result from MongoDB
     * \param[in] iter Iterator of bsonTab
     */
    PointSourceMgtParams(const bson_t *&bsonTab, bson_iter_t &iter);

    /// Destructor
    ~PointSourceMgtParams(void);

    /// Output
    void Dump(ostream *fs);

    /// Get start date of the current management operation
    time_t GetStartDate(void) {
        return m_startDate;
    }

    /// Get end date
    time_t GetEndDate(void) {
        return m_endDate;
    }

    /// Get sequence number
    int GetSequence(void) {
        return m_seqence;
    }

    /// Get subScenario name
    string GetSubScenarioName(void) {
        return m_name;
    }

    /// Get water volume
    float GetWaterVolume(void) {
        return m_waterVolume;
    }

    /// Get sediment concentration
    float GetSedment(void) {
        return m_sedimentConc;
    }

    /// Get sediment concentration
    float GetTN(void) {
        return m_TNConc;
    }

    /// Get NO3 concentration
    float GetNO3(void) {
        return m_NO3Conc;
    }

    /// Get NH4 concentration
    float GetNH4(void) {
        return m_NH4Conc;
    }

    /// Get OrgN concentration
    float GetOrgN(void) {
        return m_OrgNConc;
    }

    /// Get TP concentration
    float GetTP(void) {
        return m_TPConc;
    }

    /// Get SolP concentration
    float GetSolP(void) {
        return m_SolPConc;
    }

    /// Get OrgP concentration
    float GetOrgP(void) {
        return m_OrgPConc;
    }

    /// Get COD concentration
    float GetCOD(void) {
        return m_COD;
    }

private:
    /// subSecenario name
    string m_name;
    /// Sequence number of management
    int m_seqence;
    /// Start date
    time_t m_startDate;
    /// End date
    time_t m_endDate;
    ///  Q	Water volume	m3/'size'/day ('Size' may be one cattle or one pig, depends on PTSRC code)
    float m_waterVolume;
    /// Sed	Sediment concentration	g/cm3, or Mg/m3
    float m_sedimentConc;
    ///	TN	Total Nitrogen concentration mg/L
    float m_TNConc;
    ///	NO3	Nitrate Nitrogen concentration	mg/L
    float m_NO3Conc;
    /// NH4	Ammonium Nitrogen concentration	mg/L
    float m_NH4Conc;
    ///	ORGN	Organic Nitrogen concentration	mg/L
    float m_OrgNConc;
    ///	TP	Total phosphorus concentration	mg/L
    float m_TPConc;
    ///	SOLP	Soluble phosphorus concentration	mg/L
    float m_SolPConc;
    ///	ORGP	Organic phosphorus concentration	mg/L
    float m_OrgPConc;
    /// COD  mg/L
    float m_COD;
};

/*!
 * \class BMPPointSrcFactory
 * \ingroup MainBMP
 *
 * \brief Base class of point source BMPs.
 * Actually, include point pollution sources, such as sewage outlet of animal farm.
 *
 */
class BMPPointSrcFactory : public BMPFactory {
public:
    /// Constructor
    BMPPointSrcFactory(int scenarioId, int bmpId, int subScenario, int bmpType, int bmpPriority,
                       string distribution, string collection, string location);

    /// Destructor
    ~BMPPointSrcFactory(void);

    /// Load BMP parameters from MongoDB
    void loadBMP(MongoClient* conn, string &bmpDBName);

    /// Output
    void Dump(ostream *fs);

    /*!
     * \brief Load point BMP location related parameters from MongoDB
     * \param[in] conn MongoClient instance
     * \param[in] bmpDBName BMP Scenario database
     */
    void ReadPointSourceManagements(MongoClient* conn, string &bmpDBName);

    /*!
     * \brief Load point BMP location related parameters from MongoDB
     * \param[in] conn MongoClient instance
     * \param[in] bmpDBName BMP Scenario database
     */
    void ReadPointSourceLocations(MongoClient* conn, string &bmpDBName);

    vector<int> &GetPointSrcMgtSeqs(void) {
        return m_pointSrcMgtSeqs;
    }

    map<int, PointSourceMgtParams *> &GetPointSrcMgtMap(void) {
        return m_pointSrcMgtMap;
    }

    vector<int> &GetPointSrcIDs(void) {
        return m_pointSrcIDs;
    }

    map<int, PointSourceLocations *> &GetPointSrcLocsMap(void) {
        return m_pointSrcLocsMap;
    }

private:
    /// Code of point source
    int m_pointSrc;
    /// Collection of point source management parameters
    string m_pointSrcMgtTab;
    /// Sequences of point source managements
    vector<int> m_pointSrcMgtSeqs;
    /*!
     * Map of point source management parameters
     * Key: Scheduled sequence number, unique
     * Value: Pointer of PointBMPParamters instance
     */
    map<int, PointSourceMgtParams *> m_pointSrcMgtMap;
    /// Collection of point source locations
    string m_pointSrcDistTab;
    /// IDs of point source of current subScenario
    vector<int> m_pointSrcIDs;
    /*!
     * Map of point source BMP location related parameters
     * Key: PTSRCID, unique
     * Value: Pointer of PointBMPParamters instance
     */
    map<int, PointSourceLocations *> m_pointSrcLocsMap;
};
}
#endif /* SEIMS_BMP_POINTSOURCE_H */
