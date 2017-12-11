/*!
 * \brief Scenario class in BMP database
 * \author Liang-Jun Zhu
 * \date 2016-6-16
 *            1. Replaced SQLite by MongoDB to manager BMP scenario data.
 */
#ifndef SEIMS_SCENARIO_H
#define SEIMS_SCENARIO_H

#include "MongoUtil.h"
#include "BMPText.h"
#include "BMPFactory.h"
#include "BMPPlantMgtFactory.h"
#include "BMPPointSourceFactory.h"
#include "BMPArealSourceFactory.h"
#include "BMPArealStructFactory.h"
#include "utilities.h"

using namespace std;

namespace MainBMP {
/*!
 * \class Scenario
 * \ingroup bmps
 *
 * \brief Main class of scenario in BMP database
 *
 * Scenario contains a collection of BMPFactory.
 * Each \sa BMPFactory is corresponding to one type of BMP.
 *
 * Usage:       (1) instantiate the class
 *              (2) invoke setRasterForScenario() in DataCenter
 *              (3) invoke setRasterForEachBMP()
 *              (4) set as an input parameter for module use
 * Revised:
 *              (1) Replaced SQLite by MongoDB, 2016-6-16
 *              (2) Add setRasterForEachBMP() function, 2017-7-12
 */
class Scenario {
public:
    //! Constructor according to BMP database name and scenario ID
    Scenario(MongoClient *conn, const string &dbName, int subbsnID = 0, int scenarioID = 0);

    //! Destructor
    ~Scenario();

    //! Get scenario ID, base scenario iD is 0
    int ID() const { return m_sceneID; }

    //! If this is base scenario
    bool IsBaseScenario() { return m_sceneID == 0; }

    //! Get scenario name
    string Name() { return m_name; };

    //! Get BMPs Factories
    const map<int, BMPFactory *> &GetBMPFactories() const { return m_bmpFactories; }

    //! Write all BMPs information of this scenario to a text file
    void Dump(string &fileName);

    //! Output all BMPs information of this scenario to ostream
    void Dump(ostream *fs);

    //! Load time series data from database for some reach structure, \sa BMPReachFactory
    //void loadTimeSeriesData(string databasePath, time_t startTime, time_t endTime, int interval);

    //! get scenario required raster map. DO NOT DEFINE AS CONST FUNCTION, SINCE m_sceneRsMap WILL BE CHANGED ELSEWHERE!
    map<string, FloatRaster *> &getSceneRasterDataMap() { return m_sceneRsMap; }

    //! set raster data for BMPs
    void setRasterForEachBMP();

private:
    //! MongoDB client object, added by Liangjun
    MongoClient *m_conn;
    //! MongoDB name of BMP
    const string m_bmpDBName;
    //! Collections in BMP database used for data checking
    vector<string> m_bmpCollections;
    //! Scenario ID, e.g., 0
    const int m_sceneID;
    //! Scenario Name, e.g., base scenario
    string m_name;
    //! Subbasin ID, 0 for the entire basin
    const int m_subbsnID;

private:
    /*!
     * \brief Map of BMPs Factory
     *        the Key is unique BMP ID, and the value is \sa BMPFactory
     */
    map<int, BMPFactory *> m_bmpFactories;
    /*!
     * \brief Map of spatial data of scenario data, both 1D and 2D
     */
    map<string, FloatRaster *> m_sceneRsMap;

    /// Load scenario information
    void loadScenario();

    /// Get scenario name
    void loadScenarioName();

    /// Load each BMP in current scenario
    void loadBMPs();

    /// Load a single BMP information via \sa BMPFactory
    void loadBMPDetail();
};

} /* MainBMP */
#endif /* SEIMS_SCENARIO_H */
