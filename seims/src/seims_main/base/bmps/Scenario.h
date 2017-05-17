/*!
 * \brief Scenario class in BMP database
 * \revised Liang-Jun Zhu
 * \date 2016-6-16
 *            1. Replaced SQLite by MongoDB to manager BMP scenario data.
 */
#ifndef SEIMS_SCENARIO_H
#define SEIMS_SCENARIO_H

#include "utilities.h"
#include "MongoUtil.h"
#include "BMPText.h"
#include "BMPFactory.h"
#include "BMPPlantMgtFactory.h"
#include "BMPPointSourceFactory.h"
#include "BMPArealSourceFactory.h"

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
 *              (2)
 * Revised:
 *              (1) Replaced SQLite by MongoDB, 2016-6-16
 */
class Scenario {
public:
    //! Constructor according to BMP database name and scenario ID
    Scenario(MongoClient *conn, string dbName, int scenarioID = 0);

    //! Destructor
    ~Scenario(void);

    //! Get scenario ID, base scenario iD is 0
    int ID(void) const { return m_id; }

    //! If this is base scenario
    bool IsBaseScenario(void) const { return m_id == 0; }

    //! Get scenario name
    string Name(void);

    //! Get BMPs Factories
    map<int, BMPFactory *>& GetBMPFactories(void) {
        return m_bmpFactories;
    }

    //! Write all BMPs information of this scenario to a text file
    void Dump(string fileName);

    //! Output all BMPs information of this scenario to ostream
    void Dump(ostream *fs);

    //! Load time series data from database for some reach structure, \sa BMPReachFactory
    void loadTimeSeriesData(string databasePath, time_t startTime, time_t endTime, int interval);

private:
    //! MongoDB client object, added by Liangjun
    MongoClient* m_conn;
    /// MongoDB name of BMP
    string m_bmpDBName;
    /// Collections in BMP database used for data checking
    vector<string> m_bmpCollections;
    /// Scenario ID, e.g., 0
    int m_id;
    /// Scenario Name, e.g., base scenario
    string m_name;
private:
    /*!
     * \brief Map of BMPs Factory
     *        the Key is unique BMP ID, and the value is \sa BMPFactory
     */
    map<int, BMPFactory *> m_bmpFactories;

    /// Load scenario information
    void loadScenario(void);

    /// Get scenario name
    void loadScenarioName(void);

    /// Load each BMP in current scenario
    void loadBMPs(void);

    /// Load a single BMP information via \sa BMPFactory
    void loadBMPDetail(void);
};
}
#endif /* SEIMS_SCENARIO_H */
