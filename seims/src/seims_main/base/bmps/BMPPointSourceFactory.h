/*!
 * \brief Point source pollution and BMP factory
 * \author Liang-Jun Zhu
 * \date July 2016
 *
 */
#ifndef SEIMS_BMP_POINTSOURCE_H
#define SEIMS_BMP_POINTSOURCE_H

#include "BMPFactory.h"
#include "utilities.h"

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
    ~PointSourceLocations() = default;

    /// Output
    void Dump(ostream *fs);

    /// Get point source ID
    int GetPointSourceID() { return m_pointSrcID; }

    /// name
    string GetPointSourceName() { return m_name; }

    /// Lat
    float GetLat() { return m_lat; }

    /// Lon
    float GetLon() { return m_lon; }

    /// localX
    float GetLocalX() { return m_localX; }

    /// localY
    float GetLocalY() { return m_localY; }

    /// Located subbasin ID
    int GetSubbasinID() { return m_subbasinID; }

    /// size
    float GetSize() { return m_size; }

    /// Distance to the downstream reach
    float GetDistanceDown() { return m_distDown; }

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
    ~PointSourceMgtParams() = default;

    /// Output
    void Dump(ostream *fs);

    /// Get start date of the current management operation
    time_t GetStartDate() { return m_startDate; }

    /// Get end date
    time_t GetEndDate() { return m_endDate; }

    /// Get sequence number
    int GetSequence() { return m_seqence; }

    /// Get subScenario name
    string GetSubScenarioName() { return m_name; }

    /// Get water volume
    float GetWaterVolume() { return m_waterVolume; }

    /// Get sediment concentration
    float GetSedment() { return m_sedimentConc; }

    /// Get sediment concentration
    float GetTN() { return m_TNConc; }

    /// Get NO3 concentration
    float GetNO3() { return m_NO3Conc; }

    /// Get NH4 concentration
    float GetNH4() { return m_NH4Conc; }

    /// Get OrgN concentration
    float GetOrgN() { return m_OrgNConc; }

    /// Get TP concentration
    float GetTP() { return m_TPConc; }

    /// Get SolP concentration
    float GetSolP() { return m_SolPConc; }

    /// Get OrgP concentration
    float GetOrgP() { return m_OrgPConc; }

    /// Get COD concentration
    float GetCOD() { return m_COD; }

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
    BMPPointSrcFactory(int scenarioId, int bmpId, int subScenario,
                       int bmpType, int bmpPriority, vector<string> &distribution,
                       const string &collection, const string &location);
    /// Destructor
    virtual ~BMPPointSrcFactory();

    /// Load BMP parameters from MongoDB
    virtual void loadBMP(MongoClient *conn, const string &bmpDBName);

    /// Output
    virtual void Dump(ostream *fs);

    /*!
     * \brief Load point BMP location related parameters from MongoDB
     * \param[in] conn MongoClient instance
     * \param[in] bmpDBName BMP Scenario database
     */
    void ReadPointSourceManagements(MongoClient *conn, const string &bmpDBName);

    /*!
     * \brief Load point BMP location related parameters from MongoDB
     * \param[in] conn MongoClient instance
     * \param[in] bmpDBName BMP Scenario database
     */
    void ReadPointSourceLocations(MongoClient *conn, const string &bmpDBName);

    const vector<int> &GetPointSrcMgtSeqs() const { return m_pointSrcMgtSeqs; }

    const map<int, PointSourceMgtParams *> &GetPointSrcMgtMap() const { return m_pointSrcMgtMap; }

    const vector<int> &GetPointSrcIDs() const { return m_pointSrcIDs; }

    const map<int, PointSourceLocations *> &GetPointSrcLocsMap() const { return m_pointSrcLocsMap; }

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
