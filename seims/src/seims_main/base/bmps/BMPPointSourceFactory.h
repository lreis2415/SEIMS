/*!
 * \file BMPPointSourceFactory.h
 * \brief Point source pollution and BMP factory
 * \author Liang-Jun Zhu
 * \date July 2016
 */
#ifndef SEIMS_BMP_POINTSOURCE_H
#define SEIMS_BMP_POINTSOURCE_H

#include "basic.h"
#include "BMPFactory.h"

using namespace ccgl;
using namespace bmps;

namespace bmps {
/*!
 * \class bmps::PointSourceLocations
 * \brief Base class of point BMP, mainly store location related parameters
 *
 */
class PointSourceLocations: Interface {
public:
    /*!
     * \brief Constructor, parse point BMP location related parameters from bson object
     * \param[in] bsonTable Query result from MongoDB
     * \param[in] iter Iterator of bsonTab
     */
    PointSourceLocations(const bson_t*& bsonTable, bson_iter_t& iter);

    /// Destructor
    //~PointSourceLocations() = default;

    /// Output
    void Dump(std::ostream* fs);

    /// Get point source ID
    int GetPointSourceID() { return m_pointSrcID; }

    /// name
    string GetPointSourceName() { return m_name; }

    /// Lat
    FLTPT GetLat() { return m_lat; }

    /// Lon
    FLTPT GetLon() { return m_lon; }

    /// localX
    FLTPT GetLocalX() { return m_localX; }

    /// localY
    FLTPT GetLocalY() { return m_localY; }

    /// Located subbasin ID
    int GetSubbasinID() { return m_subbasinID; }

    /// size
    FLTPT GetSize() { return m_size; }

    /// Distance to the downstream reach
    FLTPT GetDistanceDown() { return m_distDown; }

private:
    /// ID of point source
    int m_pointSrcID;
    /// name
    string m_name;
    /// Lat
    FLTPT m_lat;
    /// Lon
    FLTPT m_lon;
    /// localX
    FLTPT m_localX;
    /// localY
    FLTPT m_localY;
    /// Located subbasin ID
    int m_subbasinID;
    /// size
    FLTPT m_size;
    /// Distance to the downstream reach
    FLTPT m_distDown;
};

/*!
 * \class bmps::PointSourceMgtParams
 * \brief Point source management parameters
 *
 */
class PointSourceMgtParams: Interface {
public:
    /*!
     * \brief Constructor, parse point source management parameters from bson object
     * \param[in] bsonTable Query result from MongoDB
     * \param[in] iter Iterator of bsonTab
     */
    PointSourceMgtParams(const bson_t*& bsonTable, bson_iter_t& iter);

    /// Destructor
    //~PointSourceMgtParams() = default;

    /// Output
    void Dump(std::ostream* fs);

    /// Get start date of the current management operation
    time_t GetStartDate() { return m_startDate; }

    /// Get end date
    time_t GetEndDate() { return m_endDate; }

    /// Get sequence number
    int GetSequence() { return m_seqence; }

    /// Get subScenario name
    string GetSubScenarioName() { return m_name; }

    /// Get water volume
    FLTPT GetWaterVolume() { return m_waterVolume; }

    /// Get sediment concentration
    FLTPT GetSedment() { return m_sedimentConc; }

    /// Get sediment concentration
    FLTPT GetTN() { return m_TNConc; }

    /// Get NO3 concentration
    FLTPT GetNO3() { return m_NO3Conc; }

    /// Get NH4 concentration
    FLTPT GetNH4() { return m_NH4Conc; }

    /// Get OrgN concentration
    FLTPT GetOrgN() { return m_OrgNConc; }

    /// Get TP concentration
    FLTPT GetTP() { return m_TPConc; }

    /// Get SolP concentration
    FLTPT GetSolP() { return m_SolPConc; }

    /// Get OrgP concentration
    FLTPT GetOrgP() { return m_OrgPConc; }

    /// Get COD concentration
    FLTPT GetCOD() { return m_COD; }

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
    FLTPT m_waterVolume;
    /// Sed	Sediment concentration	g/cm3, or Mg/m3
    FLTPT m_sedimentConc;
    ///	TN	Total Nitrogen concentration mg/L
    FLTPT m_TNConc;
    ///	NO3	Nitrate Nitrogen concentration	mg/L
    FLTPT m_NO3Conc;
    /// NH4	Ammonium Nitrogen concentration	mg/L
    FLTPT m_NH4Conc;
    ///	ORGN	Organic Nitrogen concentration	mg/L
    FLTPT m_OrgNConc;
    ///	TP	Total phosphorus concentration	mg/L
    FLTPT m_TPConc;
    ///	SOLP	Soluble phosphorus concentration	mg/L
    FLTPT m_SolPConc;
    ///	ORGP	Organic phosphorus concentration	mg/L
    FLTPT m_OrgPConc;
    /// COD  mg/L
    FLTPT m_COD;
};

/*!
 * \class bmps::BMPPointSrcFactory
 * \brief Base class of point source BMPs.
 * Actually, include point pollution sources, such as sewage outlet of animal farm.
 *
 */
class BMPPointSrcFactory: public BMPFactory {
public:
    /// Constructor
    BMPPointSrcFactory(int scenarioId, int bmpId, int subScenario,
                       int bmpType, int bmpPriority, vector<string>& distribution,
                       const string& collection, const string& location);
    /// Destructor
    ~BMPPointSrcFactory();

    /// Load BMP parameters from MongoDB
    void loadBMP(MongoClient* conn, const string& bmpDBName) OVERRIDE;

    /// Output
    void Dump(std::ostream* fs) OVERRIDE;

    /*!
     * \brief Load point BMP location related parameters from MongoDB
     * \param[in] conn MongoClient instance
     * \param[in] bmpDBName BMP Scenario database
     */
    void ReadPointSourceManagements(MongoClient* conn, const string& bmpDBName);

    /*!
     * \brief Load point BMP location related parameters from MongoDB
     * \param[in] conn MongoClient instance
     * \param[in] bmpDBName BMP Scenario database
     */
    void ReadPointSourceLocations(MongoClient* conn, const string& bmpDBName);

    vector<int>& GetPointSrcMgtSeqs() { return m_pointSrcMgtSeqs; }

    map<int, PointSourceMgtParams *>& GetPointSrcMgtMap() { return m_pointSrcMgtMap; }

    vector<int>& GetPointSrcIDs() { return m_pointSrcIDs; }

    map<int, PointSourceLocations *>& GetPointSrcLocsMap() { return m_pointSrcLocsMap; }

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
