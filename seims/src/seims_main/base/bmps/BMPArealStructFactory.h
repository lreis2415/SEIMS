/*!
 * \brief Areal struct BMP factory
 * \author Huiran Gao, Liangjun Zhu
 * \date Feb. 2017
 * \revised lj 2017-7-13  partially rewrite this class, Scenario data only read from MongoDB
 *                                   DataCenter will perform the data updating.
 *          lj 2017-11-29 code style review
 *          lj 2018-4-12 Code reformat
 */
#ifndef SEIMS_BMP_AREALSTRUCT_H
#define SEIMS_BMP_AREALSTRUCT_H

#include "tinyxml.h"
#include "basic.h"
#include "data_raster.h"

#include "BMPFactory.h"
#include "ParamInfo.h"

using namespace ccgl;
using namespace bmps;

namespace bmps {
/*!
 * \class BMPArealStruct, inherited from \sa ParamInfo
 * \ingroup MainBMP
 * \brief Manage areal Structural BMP data
 */
class BMPArealStruct: Interface {
public:
    //! Constructor
    BMPArealStruct(const bson_t*& bsonTab, bson_iter_t& iter);
    //! Destructor
    ~BMPArealStruct();
    //! Get name
    string getBMPName() { return m_name; }
    //! Get suitable landuse
    const vector<int>& getSuitableLanduse() const { return m_landuse; }
    //! Get parameters
    const map<string, ParamInfo*>& getParameters() const { return m_parameters; }
private:
    int m_id; ///< unique BMP ID
    string m_name; ///< name
    string m_desc; ///< description
    string m_refer; ///< references
    vector<int> m_landuse; ///< suitable placement landuse
    /*!
     * \key the parameter name, remember to add subbasin number as prefix when use GridFS file in MongoDB
     * \value the \sa ParamInfo class
     */
    map<string, ParamInfo*> m_parameters;
};

/*!
 * \class BMPArealStructFactory
 * \ingroup MainBMP
 *
 * \brief Initiate Areal Structural BMPs
 *
 */
class BMPArealStructFactory: public BMPFactory {
public:
    /// Constructor
    BMPArealStructFactory(int scenarioId, int bmpId, int subScenario,
                          int bmpType, int bmpPriority, vector<string>& distribution,
                          const string& collection, const string& location);

    /// Destructor
    virtual ~BMPArealStructFactory();

    //! Load BMP parameters from MongoDB
    void loadBMP(MongoClient* conn, const string& bmpDBName) OVERRIDE;

    //! Set raster data if needed
    void setRasterData(map<string, FloatRaster*>& sceneRsMap) OVERRIDE;

    //! Get management fields data
    float* GetRasterData() OVERRIDE { return m_mgtFieldsRs; };

    //! Get effect unit IDs
    const vector<int>& getUnitIDs() const { return m_unitIDs; }

    //! Get areal BMP parameters
    const map<int, BMPArealStruct*>& getBMPsSettings() const { return m_bmpStructMap; }

    //! Output
    void Dump(std::ostream* fs) OVERRIDE;

private:
    //! management units file name
    string m_mgtFieldsName;
    //! management units raster data
    float* m_mgtFieldsRs;
    //! locations
    vector<int> m_unitIDs;
    /*!
     *\key The unique areal BMP ID
     *\value Instance of \sa BMPArealStruct
     */
    map<int, BMPArealStruct*> m_bmpStructMap;
};
}
#endif /* SEIMS_BMP_AREALSTRUCT_H */
