/*!
 * \brief Areal struct BMP factory
 * \author GAO Huiran
 * \date Feb. 2017
 * \revised Liangjun Zhu, 2017-7-13  partially rewrite this class, Scenario data only read from MongoDB
 *                                   DataCenter will perform the data updating.
 */
#ifndef SEIMS_BMP_AREALSTRUCT_H
#define SEIMS_BMP_AREALSTRUCT_H

#include "text.h"
#include "utilities.h"
#include "tinyxml.h"
#include "BMPFactory.h"
#include "clsRasterData.cpp"

using namespace MainBMP;

namespace MainBMP
{
/*!
 * \struct ParamEffect
 */
struct ParamEffect
{
    string  paramID;
    string  paramDesc;
    string  change;
    float   impact;
};
/*!
 * \class BMPArealStruct
 * \ingroup MainBMP
 * \brief Manage areal Structural BMP data 
 */
class BMPArealStruct
{
public:
    //! Constructor
    BMPArealStruct(const bson_t *&bsonTab, bson_iter_t &iter);
    //! Destructor
    ~BMPArealStruct();
private:
    int             m_id;          ///< unique BMP ID
    string          m_name;        ///< name
    string          m_desc;        ///< description
    string          m_refer;       ///< references
    vector<int>     m_landuse;     ///< suitable placement landuse
    /*!
     * key is the parameter name, remember to add subbasin number as prefix when use GridFS file in MongoDB
     * value is the \sa ParamEffect data struct
     */
    map<string, ParamEffect> m_parameters;
};
/*!
 * \class BMPArealStructFactory
 * \ingroup MainBMP
 *
 * \brief Initiate Areal Structural BMPs
 *
 */
class BMPArealStructFactory: public BMPFactory
{
public:
    /// Constructor
    BMPArealStructFactory(const int scenarioId, const int bmpId, const int subScenario,
                          const int bmpType, const int bmpPriority, vector<string> &distribution,
                          const string collection, const string location);
    
    /// Destructor
	~BMPArealStructFactory(void);

	//vector<string> m_BMPparam;

	//struct Param
	//{
	//	char BMPName[30];
	//	int  BMPID;
	//	char ParamName[30];
	//	char Method;
	//	double Value;
	//};
	////vector, storage of every parameter value
	//vector<struct Param> Params;
	////! read Xml file
	//bool ReadXmlFile(string& szFileName,string moduleName);

	//! Load BMP parameters from MongoDB
    void loadBMP(MongoClient* conn, const string &bmpDBName);

    //! Set raster data if needed
    void setRasterData(map<string, FloatRaster*> &sceneRsMap);

	//! Output
	void Dump(ostream *fs);

    /// preUpdate parameters. In my view, this operation should be executed in DataCenter.
	void BMPParametersPreUpdate(map<string, clsRasterData<float>*> rsMap, int nSubbasin,
                                mongoc_gridfs_t *spatialData);

private:
    map<int, BMPArealStruct*> m_bmpStructMap;
private:
	///// field index for where to apply the subScenario
	//string m_bmpDBName;
	///// cell number
	//int m_nCell;
	///// field number
	//int m_nField;

 //   /*
 //    * Key is the subScenario Id
 //    * Value i
	// * string m_bmpDBName;       
 //    */
 //   map<int, vector<int>>  m_bmpArealStrOps;

	///// BMP index in each field
	//int* m_fieldBMPid;
    string m_mgtFieldsName;
	FloatRaster* m_mgtFieldsRs;

	///// field raster data
	//float* m_fieldmap;

	///*!
 //   * Get BMP id in each field.
 //   * !!!This function is Not used!!!
 //   */
	//void getFieldBMPid(map<int, vector<int>>  m_bmpArealStrOps);

	///*!
 //   * Get field raster data.
 //   *
 //   */
	//void readFieldRasterfile(int nSubbasin, mongoc_gridfs_t* spatialData,
 //                            clsRasterData<float>* templateRaster);

	///*!
	//* Update 1D parameters. 
	//* 
	//*/
	//void Update(string paraName, int n, float* data, int subScenarioid,
 //               vector<int> location);

	///*!
	//* Update 2D parameters. 
	//* 
	//*/
	//void Update2D(string paraName, int n, int lyr, float** data2D, int subScenarioid,
 //                 vector<int> location);
};
}
#endif /* SEIMS_BMP_AREALSTRUCT_H */
