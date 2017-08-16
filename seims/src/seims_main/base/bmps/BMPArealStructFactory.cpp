#include "BMPArealStructFactory.h"

using namespace MainBMP;

BMPArealStruct::BMPArealStruct(const bson_t *&bsonTable, bson_iter_t &iter):
m_name(""), m_desc(""), m_refer(""), m_id(-1)
{
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_SUB)) {
        GetNumericFromBsonIterator(&iter, m_id);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_NAME)) {
        m_name = GetStringFromBsonIterator(&iter);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_ARSTRUCT_FLD_DESC)) {
        m_desc = GetStringFromBsonIterator(&iter);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_ARSTRUCT_FLD_REF)) {
        m_refer = GetStringFromBsonIterator(&iter);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_ARSTRUCT_FLD_LANDUSE)) {
        string landuse_str = GetStringFromBsonIterator(&iter);
        m_landuse = SplitStringForInt(landuse_str, '-');
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_ARSTRUCT_FLD_PARAMS)) {
        string params_str = GetStringFromBsonIterator(&iter);
        vector<string> params_strs = SplitString(params_str, '-');
        for (vector<string>::iterator it = params_strs.begin(); it != params_strs.end(); it++) {
            vector<string> tmp_param_items = SplitString(*it, ':');
            assert(tmp_param_items.size() == 4);
            ParamEffect tmp_pe;
            tmp_pe.paramID = tmp_param_items[0];  /// tmp_param_items is already uppercased during preprocess.
            tmp_pe.paramDesc = tmp_param_items[1];
            tmp_pe.change = tmp_param_items[2];
            tmp_pe.impact = (float)atof(tmp_param_items[3].c_str());
            if (!m_parameters.insert(make_pair(tmp_pe.paramID, tmp_pe)).second) {
                cout << "WARNING: Read BMPArealStruct failed: BMPID: " << m_id << ", param_name: "
                    << tmp_pe.paramID << endl;
            }
        }
    }
}

BMPArealStruct::~BMPArealStruct() {
    /// currently, nothing need to be done.
}

BMPArealStructFactory::BMPArealStructFactory(const int scenarioId, const int bmpId, const int subScenario,
                                             const int bmpType, const int bmpPriority,
                                             vector<string> &distribution, const string collection,
                                             const string location): 
    BMPFactory(scenarioId, bmpId, subScenario, bmpType, bmpPriority, distribution, collection, location), 
    m_mgtFieldsRs(NULL)
{
    if (m_distribution.size() >= 2 && StringMatch(m_distribution[0], FLD_SCENARIO_DIST_RASTER)) {
        m_mgtFieldsName = m_distribution[1];
    }
    else {
        throw ModelException("BMPArealStructFactory", "Initialization",
            "The distribution field must follow the format: "
            "RASTER|CoreRasterName.\n");
    }
}

BMPArealStructFactory::~BMPArealStructFactory()
{
	//if(m_mgtFieldsRs != NULL) delete m_mgtFieldsRs;  // will be released in DataCenter
    for (map<int, BMPArealStruct*>::iterator it = m_bmpStructMap.begin(); it != m_bmpStructMap.end(); ) {
        if (NULL != it->second) {
            delete it->second;
            it->second = NULL;
        }
        m_bmpStructMap.erase(it++);
    }
    m_bmpStructMap.clear();
}

void BMPArealStructFactory::loadBMP(MongoClient* conn, const string &bmpDBName)
{
    bson_t *b = bson_new();
    bson_t *child1 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
    BSON_APPEND_INT32(child1, BMP_FLD_SUB, m_subScenarioId);
    bson_append_document_end(b, child1);
    bson_destroy(child1);

    unique_ptr<MongoCollection> collection(new MongoCollection(conn->getCollection(bmpDBName, m_bmpCollection)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);

    bson_iter_t iter;
    const bson_t *bsonTable;

    /// Use count to counting sequence number, in case of discontinuous or repeat of SEQUENCE in database.
    while (mongoc_cursor_next(cursor, &bsonTable)) {
        if (!m_bmpStructMap.insert(make_pair(m_subScenarioId, new BMPArealStruct(bsonTable, iter))).second) {
            cout << "WARNING: Read Areal Structural BMP failed: subScenarioID: " << m_subScenarioId << endl;
        }
    }
    bson_destroy(b);
    mongoc_cursor_destroy(cursor);
}

void BMPArealStructFactory::setRasterData(map<string, FloatRaster*> &sceneRsMap) {
    if (sceneRsMap.find(m_mgtFieldsName) != sceneRsMap.end()) {
        int n;
        sceneRsMap.at(m_mgtFieldsName)->getRasterData(&n, &m_mgtFieldsRs);
    }
    else{
        // raise Exception?
    }
}

void BMPArealStructFactory::BMPParametersPreUpdate(map<string, clsRasterData<float>*> rsMap, int nSubbasin,  mongoc_gridfs_t *spatialData)
{
	//int subScenarioid = this->m_subScenarioId;
	//vector<int> locationArr = SplitStringForInt(this->m_location, '-');

	//ostringstream oss;
 //   oss << nSubbasin << "_" << GetUpper(VAR_MGT_FIELD);
	//m_mgtFieldsRs = new clsRasterData<float>(spatialData,oss.str().c_str());
	//readFieldRasterfile(nSubbasin, spatialData, m_mgtFieldsRs);
	////getFieldBMPid(m_bmpArealStrOps);
	//
	//vector<string>::iterator it;
	//for (it = m_BMPparam.begin(); it!=m_BMPparam.end(); it++)
	//{
	//	int n;
	//	clsRasterData<float> *raster = NULL;
	//	string name = *it;
	//	ostringstream oss;
	//	oss << nSubbasin << "_" << GetUpper(name);
	//	string remoteFileName = oss.str();
	//	// cout << "remoteFileName:" << remoteFileName << endl;

	//	if (rsMap.find(remoteFileName) != rsMap.end())
	//	{
	//		raster = rsMap[remoteFileName];
	//		if(raster->is2DRaster())
	//		{
	//			int lyr;
	//			float** data2D = NULL;
	//			raster->get2DRasterData(&n, &lyr, &data2D);
	//			Update2D(name, n, lyr, data2D, subScenarioid, locationArr);
	//		}
	//		else
	//		{
	//			float* data = NULL;
	//			raster->getRasterData(&n, &data);
	//			Update(name, n, data, subScenarioid, locationArr);
	//		}
	//		//cout << "Parameter name: " << name << endl;
	//	}
	//}
}

//void BMPArealStructFactory::Update(string paraName, int n, float* data, int subScenarioid, vector<int> location)
//{
//	for(vector<int>::iterator iter_loc = location.begin(); iter_loc != location.end(); iter_loc++)
//	{
//		for(int i = 0; i < n; i++)
//		{
//			if(*iter_loc == i)
//			{
//				for (vector<struct Param>::iterator iter=Params.begin();iter!=Params.end();++iter)
//				{
//					if(subScenarioid==iter->BMPID && StringMatch(paraName,iter->ParamName))
//					{
//						switch (iter->Method)
//						{
//						case '*':
//							data[i] *= iter->Value;
//							break;
//						case '+':
//							data[i] += iter->Value;
//							break;
//						case '=':
//							data[i] = iter->Value;
//							break;
//						}
//						break;
//					}
//				}
//			}
//		}
//	}
//	
//}
//
//void BMPArealStructFactory::Update2D(string paraName, int n, int lyr, float** data2D, int subScenarioid, vector<int> location)
//{
//	for(vector<int>::iterator iter_loc = location.begin(); iter_loc != location.end(); iter_loc++)
//	{
//		for(int i = 0; i < n; i++)
//		{
//			if(*iter_loc == i)
//			{
//				for (vector<struct Param>::iterator iter=Params.begin();iter!=Params.end();++iter)
//				{
//					if(subScenarioid==iter->BMPID && StringMatch(paraName,iter->ParamName))
//					{
//						for(int k = 0; k < lyr; k++)
//						{
//							switch (iter->Method)
//							{
//							case '*':
//								data2D[i][k] *= iter->Value;
//								break;
//							case '+':
//								data2D[i][k] += iter->Value;
//								break;
//							case '=':
//								data2D[i][k] = iter->Value;
//								break;
//							}
//						}
//						break;
//					}
//				}
//			}
//		}
//	}
//}
//
//void BMPArealStructFactory::readFieldRasterfile(int nSubbasin, mongoc_gridfs_t* spatialData,
//                                                clsRasterData<float>* templateRaster)
//{
//	// read spatial data from MongoDB
//	int n = 0;
//	clsRasterData<float>*raster = NULL;
//	ostringstream oss;
//	try
//	{
//        oss << nSubbasin << "_" << GetUpper(VAR_MGT_FIELD);
//        raster = new clsRasterData<float>(m_spatialGridFS, oss.str().c_str(), true, templateRaster, true);
//	}
//	catch (ModelException e)
//	{
//		cout << e.toString() << endl;
//		return;
//	}
//	raster->getRasterData(&n, &m_fieldmap);
//	m_nCell = n;
//
//	// get field number
//	m_nField = m_fieldmap[0];
//	float nField_start = m_fieldmap[0];
//	for(int i = 1; i < m_nCell; i++)
//	{
//		if(m_nField < m_fieldmap[i]) m_nField = m_fieldmap[i];
//		if(nField_start > m_fieldmap[i]) nField_start = m_fieldmap[i];
//	}
//	if(nField_start == 0.) m_nField += 1.;
//}
//
//bool BMPArealStructFactory::ReadXmlFile(string& xmlFilePath,string moduleName)
//{
//	try
//	{
//		//create a XML object
//		TiXmlDocument *myDocument = new TiXmlDocument(xmlFilePath.c_str());
//		myDocument->LoadFile();
//		//get BMPs
//		TiXmlElement *RootElement = myDocument->RootElement();
//		//get BMP nodes
//		TiXmlElement *BMP = RootElement->FirstChildElement();
//
//		//get BMP attributes
//		while(BMP)
//		{
//			TiXmlAttribute *IDAttribute = BMP->FirstAttribute();
//			TiXmlElement *PorpertyElement = BMP->FirstChildElement();
//			TiXmlElement *BMPName = PorpertyElement->FirstChildElement();
//			TiXmlElement *Description = BMPName->NextSiblingElement();
//			TiXmlElement *Reference = Description->NextSiblingElement();
//
//			//cout<<"   "<<BMPName->FirstChild()->Value()<< endl;
//
//			TiXmlElement *Parameter = PorpertyElement->NextSiblingElement();
//
//			////storage of BMPs
//			while(Parameter)
//			{
//				TiXmlElement *Name = Parameter->FirstChildElement();
//				TiXmlElement *Description = Name->NextSiblingElement();
//				TiXmlElement *Method = Description->NextSiblingElement();
//				TiXmlElement *Value = Method->NextSiblingElement();
//
//				Param param;
//				strcpy_s(param.BMPName,BMPName->FirstChild()->Value());
//				param.BMPID=atoi(IDAttribute->Value());
//				strcpy_s(param.ParamName,Name->FirstChild()->Value());
//				param.Method=*(Method->FirstChild()->Value());
//				param.Value =atof(Value->FirstChild()->Value());
//
//				Params.push_back(param);
//				Parameter = Parameter->NextSiblingElement();
//			}
//			BMP =BMP->NextSiblingElement();
//		}
//	}
//	catch (string& e)
//	{
//		return false;
//	}
//	return true;
//}
//
//void BMPArealStructFactory::getFieldBMPid(map<int, vector<int>>  m_bmpArealStrOps)
//{
//	if(m_nField <= 0)
//		throw ModelException("BMPArealStructFactory", "getFieldBMPid", "Field number should not be less than 0");
//	m_fieldBMPid = new int(m_nField);
//	for(int i = 0; i < m_nField; i++)
//	{
//		m_fieldBMPid[i] = 0;
//		//cout << "id:" << i << endl;
//	}
//	//update m_fieldBMPid
//	for (map<int, vector<int>>::iterator it = m_bmpArealStrOps.begin(); it != m_bmpArealStrOps.end(); it++)
//	{
//		cout << "BMP:" << it->first << endl;
//		for(vector<int>::iterator it_loc = it->second.begin(); it_loc != it->second.end(); it_loc++)
//		{
//			m_fieldBMPid[*it_loc] = it->first;
//			cout << m_fieldBMPid[*it_loc] << ": " << it->first << endl;
//		}
//	}
//}

void BMPArealStructFactory::Dump(ostream *fs)
{
	if (fs == NULL) return;
	*fs << "Areal Structural BMP Management Factory: " << endl <<
		"    SubScenario ID: " << m_subScenarioId << endl;
}
