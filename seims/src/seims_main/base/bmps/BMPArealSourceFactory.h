/*!
 * \brief Areal source pollution and BMP factory
 * \author Liang-Jun Zhu
 * \date Aug 2016
 *
 */
#ifndef SEIMS_BMP_AREALSOURCE_H
#define SEIMS_BMP_AREALSOURCE_H

#include "utilities.h"
#include "BMPFactory.h"
#include "clsRasterData.cpp"

using namespace MainBMP;

namespace MainBMP {
/*!
 * \class ArealSourceLocations
 * \ingroup MainBMP
 *
 * \brief Base class of point BMP, mainly store location related parameters
 *
 */
class ArealSourceLocations {
public:
    /*!
     * \brief Constructor, read and calculate areal BMP locations related parameters from Raster
     */
    ArealSourceLocations(const bson_t *&bsonTab, bson_iter_t &iter);

    /// Destructor
    ~ArealSourceLocations(void);

    /// load valid cells index
    void SetValidCells(int n, float *mgtFieldIDs);

    /// Output
    void Dump(ostream *fs);

    /// Get point source ID
    int GetArealSourceID(void) {
        return m_arealSrcID;
    }

    /// name
    string GetArealSourceName(void) {
        return m_name;
    }

    /// index of valid cells
    vector<int> &GetCellsIndex(void) {
        return m_cellsIndex;
    }

    /// Located subbasin ID
    int GetValidCells(void) {
        return m_nCells;
    }

    /// size
    float GetSize(void) {
        return m_size;
    }

private:
    /// ID of point source
    int m_arealSrcID;
    /// name
    string m_name;
    /// valid cell number
    int m_nCells;
    /// index of valid cells
    vector<int> m_cellsIndex;
    /// size, used to calculate amount of pollutants
    float m_size;
};

/*!
 * \class PointSourceParameters
 * \ingroup MainBMP
 *
 * \brief Point source management parameters
 *
 */
class ArealSourceMgtParams {
public:
    /*!
     * \brief Constructor, parse areal source management parameters from bson object
     * \param[in] bsonTab Query result from MongoDB
     * \param[in] iter Iterator of bsonTab
     */
    ArealSourceMgtParams(const bson_t *&bsonTab, bson_iter_t &iter);

    /// Destructor
    ~ArealSourceMgtParams(void);

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

    /// Get MinP concentration
    float GetMinP(void) {
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
    ///  Q	Water volume	m3/'size'/day ('Size' may be one chicken or so, depends on ARSRC code)
    float m_waterVolume;
    /// Sed	Sediment concentration	kg/'size'/day
    float m_sedimentConc;
    ///	TN	Total Nitrogen concentration	kg/'size'/day
    float m_TNConc;
    ///	NO3	Nitrate Nitrogen concentration	kg/'size'/day
    float m_NO3Conc;
    /// NH4	Ammonium Nitrogen concentration	kg/'size'/day
    float m_NH4Conc;
    ///	ORGN	Organic Nitrogen concentration	kg/'size'/day
    float m_OrgNConc;
    ///	TP	Total phosphorus concentration	kg/'size'/day
    float m_TPConc;
    ///	MINP	Mineral phosphorus concentration	kg/'size'/day
    float m_SolPConc;
    ///	ORGP	Organic phosphorus concentration	kg/'size'/day
    float m_OrgPConc;
    /// cod
    float m_COD;
};

/*!
 * \class BMPArealSrcFactory
 * \ingroup MainBMP
 *
 * \brief Base class of areal source BMPs.
 * Such as chicken farm
 *
 */
class BMPArealSrcFactory : public BMPFactory {
public:
    /// Constructor
    BMPArealSrcFactory(int scenarioId, int bmpId, int subScenario, int bmpType, int bmpPriority,
                       string distribution, string collection, string location);

    /// Destructor
    ~BMPArealSrcFactory(void);

    /// Load BMP parameters from MongoDB
    void loadBMP(MongoClient* conn, string &bmpDBName);

    /// PreUpdate parameters
    void BMPParametersPreUpdate(map<string, clsRasterData<float>*> rsMap,
                                int nSubbasin, mongoc_gridfs_t *spatialData);
    
    /// Output
    void Dump(ostream *fs);

    /*!
     * \brief Load areal BMP location related parameters from MongoDB
     * \param[in] conn mongoc client instance
     * \param[in] bmpDBName BMP Scenario database
     */
    void ReadArealSourceManagements(MongoClient* conn, string &bmpDBName);

    /*!
     * \brief Load areal BMP location related parameters from MongoDB
     * \param[in] conn mongoc client instance
     * \param[in] bmpDBName BMP Scenario database
     */
    void ReadArealSourceLocations(MongoClient* conn, string &bmpDBName);

    string GetArealSrcDistName(void) {
        return m_arealSrcDistName;
    }

    vector<int> &GetArealSrcMgtSeqs(void) {
        return m_arealSrcMgtSeqs;
    }

    map<int, ArealSourceMgtParams *> &GetArealSrcMgtMap(void) {
        return m_arealSrcMgtMap;
    }

    vector<int> &GetArealSrcIDs(void) {
        return m_arealSrcIDs;
    }

    bool GetLocationLoadStatus(void) {
        return m_loadedMgtFieldIDs;
    }

    void SetArealSrcLocsMap(int n, float *mgtField);

    map<int, ArealSourceLocations *> &GetArealSrcLocsMap(void) {
        return m_arealSrcLocsMap;
    }

private:
    /// areal source code
    int m_arealSrc;
    /// Collection of point source management parameters
    string m_arealSrcMgtTab;
    /// Sequences of point source managements
    vector<int> m_arealSrcMgtSeqs;
    /* Map of areal source management parameters
     * Key: Scheduled sequence number, unique
     * Value: Pointer of ArealSourceMgtParams instance
     */
    map<int, ArealSourceMgtParams *> m_arealSrcMgtMap;
    /// core file name of areal source locations, such as MGT_FIELDS
    string m_arealSrcDistName;
    /// areal source distribution table
    string m_arealSrcDistTab;
    /// Field IDs of areal source of current subScenario
    vector<int> m_arealSrcIDs;
    /// flag
    bool m_loadedMgtFieldIDs;
    /* Map of areal source location related parameters
     * Key: ARSRCID, unique
     * Value: Pointer of ArealBMPLocations instance
     */
    map<int, ArealSourceLocations *> m_arealSrcLocsMap;
};
}
#endif /* SEIMS_BMP_AREALSOURCE_H */
