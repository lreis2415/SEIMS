/*!
 * \brief Scenario class in BMP database
 * \revised Liang-Jun Zhu
 * \date 2016-6-16
 *            1. Replaced SQLite by MongoDB to manager BMP scenario data.
 */
#pragma once
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <ostream>
#include <iomanip>
#include "utils.h"
#include "ModelException.h"
#include "MongoUtil.h"
#include "BMPText.h"
#include "BMPFactory.h"
#include "BMPPlantMgtFactory.h"
#include "BMPPointSourceFactory.h"
#include "BMPArealSourceFactory.h"
using namespace std;

namespace MainBMP
{
        /*!
         * \class Scenario
         * \ingroup bmps
         *
         * \brief Main class of scenario in BMP database
         *
         * Scenario contains a collection of BMPFactory.
         * Each \sa BMPFactory is corresponding to one type of BMP.
         *
         * Usage:		    (1) instantiate the class
         *              (2)
         * Revised:
         *              (1) Replaced SQLite by MongoDB, 2016-6-16
         */
        class Scenario
        {
public:
                //! Constructor according to BMP database name and scenario ID
                Scenario(mongoc_client_t *conn, string dbName, int scenarioID = 0);

                //! Destructor
                ~Scenario(void);

                //! Get scenario ID, base scenario iD is 0
                int ID() {
                        return this->m_id;
                }

                //! If this is base scenario
                bool IsBaseScenario() {
                        return this->m_id == 0;
                }

                //! Get scenario name
                string Name();

                //! Get BMPs Factories
                map<int, BMPFactory *> *GetBMPFactories() {
                        return &m_bmpFactories;
                }

                //! Write all BMPs information of this scenario to a text file
                void Dump(string fileName);

                //! Output all BMPs information of this scenario to ostream
                void Dump(ostream *fs);

                //! Load time series data from database for some reach structure, \sa BMPReachFactory
                void loadTimeSeriesData(string databasePath, time_t startTime, time_t endTime, int interval);

public:
                ////! Get reach structure as \sa BMPReachFlowDiversion
                //BMPReachFlowDiversion*	getFlowDiversion(int reach);
                ////! Get reach structure as \sa BMPReachPointSource
                //BMPReachPointSource*		getPointSource(int reach);
                ////! Get reach structure as \sa BMPReachReservoir
                //BMPReachReservoir*			getReservoir(int reach);

                //! Get maximum reservoir id
                int getMaxReservoirId();

                //-----------------------------------------------
                //areal non-structural bmp information

                /*/// Get planting operation at current time
                   NonStructural::ManagementOperationPlant* getOperationPlant(
                    int validCellIndex,int startYear,time_t currentTime){
                        return (NonStructural::ManagementOperationPlant*)getOperation(validCellIndex,startYear,currentTime,BMP_TYPE_CROP);};

                   /// Get harvest operation at current time
                   NonStructural::ManagementOperationHarvest* getOperationHarvest(
                    int validCellIndex,int startYear,time_t currentTime)
                   {return (NonStructural::ManagementOperationHarvest*)getOperation(validCellIndex,startYear,currentTime,BMP_TYPE_CROP,true);};

                   /// Get fertilizer operation at current time
                   NonStructural::ManagementOperationFertilizer* getOperationFertilizer(
                    int validCellIndex,int startYear,time_t currentTime)
                   {return (NonStructural::ManagementOperationFertilizer*)getOperation(validCellIndex,startYear,currentTime,BMP_TYPE_FERTILIZER);};

                   /// Get tillage operation at current time
                   NonStructural::ManagementOperationTillage* getOperationTillage(
                    int validCellIndex,int startYear,time_t currentTime)
                   {return (NonStructural::ManagementOperationTillage*)getOperation(validCellIndex,startYear,currentTime,BMP_TYPE_TILLAGE);};
                 */
                //-----------------------------------------------

private:
                /// Get reach structural BMP information according to the given reach ID and reach BMP ID
                //BMPReach* getReachStructure(int reach,int reachBMPid);
                /*!
                 * \brief Get crop/landcover management operation BMP information
                 *
                 * \param[in] validCellIndex location index
                 * \param[in] startYear starting year of current growth period
                 * \param[in] currentTime current time date
                 * \param[in] BMPid BMP ID which should be found in BMP database
                 * \param[in] useSecond Use second opertion or not
                 * \return \sa MnagemetnOperation pointer
                 */
                //NonStructural::ManagementOperation* getOperation(
                //	int validCellIndex,int startYear,time_t currentTime,int BMPid,bool useSecond = false);
private:
                //! MongoDB client object, added by Liangjun
                mongoc_client_t *m_conn;
                /// MongoDB name of BMP
                string m_bmpDBName;
                /// Collections in BMP database used for data checking
                vector<string> m_bmpCollections;
                /// Scenario ID, e.g., 0
                int m_id;
                /// Scenario Name, e.g., base scenario
                string m_name;

                /*!
                 * Model path, used to find BMP.db3 in previous version
                 * \deprecated now because of the use of MongoDB
                 */
                //string m_projectPath;
private:
                /*!
                 * Map of BMPs Factory
                 * the Key is unique BMP ID, and the value is \sa BMPFactory
                 */
                map<int, BMPFactory *> m_bmpFactories;

                /// Load scenario information
                void loadScenario();

                /// Get scenario name
                void loadScenarioName();

                /// Load each BMP in current scenario
                void loadBMPs();

                /// Load a single BMP information via \sa BMPFactory
                void loadBMPDetail();
        };
}
