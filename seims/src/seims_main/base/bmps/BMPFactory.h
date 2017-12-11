/*!
 * \brief Base namespace for implementation of BMP configuration
 * \author Liang-Jun Zhu
 */
#ifndef SEIMS_BMP_FACTORY_H
#define SEIMS_BMP_FACTORY_H

#include "seims.h"
#include "utilities.h"
#include "BMPText.h"

#include "MongoUtil.h"
#include "clsRasterData.h"

/*!
 * \brief Base class of all kind of BMPs Factory.
 *        Read from BMP_SCENARIOS collection of MongoDB
 * \ingroup bmps
 */
namespace MainBMP {
/*!
 * \class BMPFactory
 * \ingroup MainBMP
 *
 * \brief Base class to initiate a BMP data
 *
 */
class BMPFactory {
public:
    /// Constructor
    BMPFactory(int scenarioId, int bmpId, int subScenario, int bmpType,
               int bmpPriority, vector<string>& distribution, const string& collection,
               const string& location);

    /// virtual Destructor
    virtual ~BMPFactory() = default;

    /// Load BMP parameters from MongoDB
    virtual void loadBMP(MongoClient *conn, const string &bmpDBName) = 0;

    /*!
     * \brief Set raster data if needed
     * This function is not required for each BMP, so DO NOT define as pure virtual function.
     */
    virtual void setRasterData(map<string, FloatRaster *> &sceneRsMap) {};
    /*!
    * \brief Get raster data if needed
    * This function is not required for each BMP, so DO NOT define as pure virtual function.
    */
    virtual float *getRasterData() { return nullptr; }

    /*!  Get BMP type
       1 - reach BMPs which are attached to specific reaches and will change the character of the reach.
       2 - areal structural BMPs which are corresponding to a specific structure in the watershed and will change the character of subbasins/cells.
       3 - areal non-structure BMPs which are NOT corresponding to a specific structure in the watershed and will change the character of subbasins/cells.
       4 - point structural BMPs
     */
    int bmpType() { return m_bmpType; }

    /// Get BMP priority
    int bmpPriority() { return m_bmpPriority; }

    /// Get subScenario ID
    int GetSubScenarioId() { return m_subScenarioId; }

    /// Output
    virtual void Dump(ostream *fs) = 0;

protected:
    const int m_scenarioId;     ///< Scenario ID
    const int m_bmpId;          ///< BMP ID
    const int m_subScenarioId;  ///< SubScenario ID within one BMP iD
    const int m_bmpType;        ///< BMP Type
    const int m_bmpPriority;    ///< BMP Priority
    /*! Distribution vector of BMP
     *  Origin format is [distribution data type]|[distribution parameter name]|Collection name|...
     */
    vector<string> m_distribution;
    const string m_bmpCollection;  ///< Collection name
    const string m_location;       ///< Define where the BMP will be applied
};
}
#endif /* SEIMS_BMP_FACTORY_H */
