#include "BMPArealStructFactory.h"

using namespace MainBMP;

BMPArealStructFactory::BMPArealStructFactory(const int scenarioId, const int bmpId, const int subScenario,
                                             const int bmpType, const int bmpPriority,
                                             vector<string> &distribution, const string collection,
                                             const string location)
	: BMPFactory(scenarioId, bmpId, subScenario, bmpType, bmpPriority, distribution, collection, location), 
	m_fieldBMPid(NULL), m_fieldmap(NULL), m_nCell(-1), m_nField(-1)
{

}

BMPArealStructFactory::~BMPArealStructFactory()
{
/*
	map<int, vector<int>>::iterator iter;
	for (iter = m_bmpArealStrOps.begin(); iter != m_bmpArealStrOps.end(); )
	{
		if(!iter->second.empty())
			delete (*iter).second;
		//iter = m_bmpArealStrOps.erase(iter);
        m_bmpArealStrOps.erase(iter++);
	}
	m_bmpArealStrOps.clear();*/
	if(m_fieldmap != NULL) Release1DArray(m_fieldmap);
	if(m_fieldBMPid != NULL) Release1DArray(m_fieldBMPid);
	if(m_mgtFieldsRs != NULL) delete m_mgtFieldsRs;
}

void BMPArealStructFactory::loadBMP(MongoClient* conn, string &bmpDBName)
{
	m_bmpDBName = bmpDBName;
	/*
	if (m_bmpArealStrOps.find(m_subScenarioId) == m_bmpArealStrOps.end())
	{
		m_bmpArealStrOps.insert(map<int, vector<int>> :: value_type(m_subScenarioId, m_location));
	}
	*/

	// initial m_BMPparam
	string fileName = "D:\\SEIMS_model\\Model_data\\youwuzhen\\model_youwuzhen_10m_longterm\\bmpParam.xml";
	ReadXmlFile(fileName,m_bmpDBName);

	m_BMPparam.push_back(Params.begin()->ParamName);
	for (vector<struct Param>::iterator iter=Params.begin();iter!=Params.end();++iter)
	{
		vector<string>::iterator paramIter=m_BMPparam.begin();
		for(paramIter;paramIter!=m_BMPparam.end();++paramIter)
		{
			if(*paramIter==iter->ParamName)
				break;
		}
		if(paramIter==m_BMPparam.end ())
			m_BMPparam.push_back(iter->ParamName);
	}

}

void BMPArealStructFactory::setRasterData(map<string, FloatRaster*> &sceneRsMap) {
    if (sceneRsMap.find(m_mgtFieldsName) != sceneRsMap.end()) {
        m_mgtFieldsRs = sceneRsMap.at(m_mgtFieldsName);
    }
    else{
        // raise Exception?
    }
}

void BMPArealStructFactory::BMPParametersPreUpdate(map<string, clsRasterData<float>*> rsMap, int nSubbasin,  mongoc_gridfs_t *spatialData)
{
	int subScenarioid = this->m_subScenarioId;
	vector<int> locationArr = SplitStringForInt(this->m_location, ',');

	ostringstream oss;
    oss << nSubbasin << "_" << GetUpper(VAR_MGT_FIELD);
	m_mgtFieldsRs = new clsRasterData<float>(spatialData,oss.str().c_str());
	readFieldRasterfile(nSubbasin, spatialData, m_mgtFieldsRs);
	//getFieldBMPid(m_bmpArealStrOps);
	
	vector<string>::iterator it;
	for (it = m_BMPparam.begin(); it!=m_BMPparam.end(); it++)
	{
		int n;
		clsRasterData<float> *raster = NULL;
		string name = *it;
		ostringstream oss;
		oss << nSubbasin << "_" << GetUpper(name);
		string remoteFileName = oss.str();
		// cout << "remoteFileName:" << remoteFileName << endl;

		if (rsMap.find(remoteFileName) != rsMap.end())
		{
			raster = rsMap[remoteFileName];
			if(raster->is2DRaster())
			{
				int lyr;
				float** data2D = NULL;
				raster->get2DRasterData(&n, &lyr, &data2D);
				Update2D(name, n, lyr, data2D, subScenarioid, locationArr);
			}
			else
			{
				float* data = NULL;
				raster->getRasterData(&n, &data);
				Update(name, n, data, subScenarioid, locationArr);
			}
			//cout << "Parameter name: " << name << endl;
		}
	}
}

void BMPArealStructFactory::Update(string paraName, int n, float* data, int subScenarioid, vector<int> location)
{
	for(vector<int>::iterator iter_loc = location.begin(); iter_loc != location.end(); iter_loc++)
	{
		for(int i = 0; i < n; i++)
		{
			if(*iter_loc == i)
			{
				for (vector<struct Param>::iterator iter=Params.begin();iter!=Params.end();++iter)
				{
					if(subScenarioid==iter->BMPID && StringMatch(paraName,iter->ParamName))
					{
						switch (iter->Method)
						{
						case '*':
							data[i] *= iter->Value;
							break;
						case '+':
							data[i] += iter->Value;
							break;
						case '=':
							data[i] = iter->Value;
							break;
						}
						break;
					}
				}
			}
		}
	}
	
}

void BMPArealStructFactory::Update2D(string paraName, int n, int lyr, float** data2D, int subScenarioid, vector<int> location)
{
	for(vector<int>::iterator iter_loc = location.begin(); iter_loc != location.end(); iter_loc++)
	{
		for(int i = 0; i < n; i++)
		{
			if(*iter_loc == i)
			{
				for (vector<struct Param>::iterator iter=Params.begin();iter!=Params.end();++iter)
				{
					if(subScenarioid==iter->BMPID && StringMatch(paraName,iter->ParamName))
					{
						for(int k = 0; k < lyr; k++)
						{
							switch (iter->Method)
							{
							case '*':
								data2D[i][k] *= iter->Value;
								break;
							case '+':
								data2D[i][k] += iter->Value;
								break;
							case '=':
								data2D[i][k] = iter->Value;
								break;
							}
						}
						break;
					}
				}
			}
		}
	}
}

void BMPArealStructFactory::readFieldRasterfile(int nSubbasin, mongoc_gridfs_t* spatialData,
                                                clsRasterData<float>* templateRaster)
{
	// read spatial data from MongoDB
	int n = 0;
	clsRasterData<float>*raster = NULL;
	ostringstream oss;
	try
	{
        oss << nSubbasin << "_" << GetUpper(VAR_MGT_FIELD);
        raster = new clsRasterData<float>(m_spatialGridFS, oss.str().c_str(), true, templateRaster, true);
	}
	catch (ModelException e)
	{
		cout << e.toString() << endl;
		return;
	}
	raster->getRasterData(&n, &m_fieldmap);
	m_nCell = n;

	// get field number
	m_nField = m_fieldmap[0];
	float nField_start = m_fieldmap[0];
	for(int i = 1; i < m_nCell; i++)
	{
		if(m_nField < m_fieldmap[i]) m_nField = m_fieldmap[i];
		if(nField_start > m_fieldmap[i]) nField_start = m_fieldmap[i];
	}
	if(nField_start == 0.) m_nField += 1.;
}

bool BMPArealStructFactory::ReadXmlFile(string& xmlFilePath,string moduleName)
{
	try
	{
		//create a XML object
		TiXmlDocument *myDocument = new TiXmlDocument(xmlFilePath.c_str());
		myDocument->LoadFile();
		//get BMPs
		TiXmlElement *RootElement = myDocument->RootElement();
		//get BMP nodes
		TiXmlElement *BMP = RootElement->FirstChildElement();

		//get BMP attributes
		while(BMP)
		{
			TiXmlAttribute *IDAttribute = BMP->FirstAttribute();
			TiXmlElement *PorpertyElement = BMP->FirstChildElement();
			TiXmlElement *BMPName = PorpertyElement->FirstChildElement();
			TiXmlElement *Description = BMPName->NextSiblingElement();
			TiXmlElement *Reference = Description->NextSiblingElement();

			//cout<<"   "<<BMPName->FirstChild()->Value()<< endl;

			TiXmlElement *Parameter = PorpertyElement->NextSiblingElement();

			////storage of BMPs
			while(Parameter)
			{
				TiXmlElement *Name = Parameter->FirstChildElement();
				TiXmlElement *Description = Name->NextSiblingElement();
				TiXmlElement *Method = Description->NextSiblingElement();
				TiXmlElement *Value = Method->NextSiblingElement();

				Param param;
				strcpy_s(param.BMPName,BMPName->FirstChild()->Value());
				param.BMPID=atoi(IDAttribute->Value());
				strcpy_s(param.ParamName,Name->FirstChild()->Value());
				param.Method=*(Method->FirstChild()->Value());
				param.Value =atof(Value->FirstChild()->Value());

				Params.push_back(param);
				Parameter = Parameter->NextSiblingElement();
			}
			BMP =BMP->NextSiblingElement();
		}
	}
	catch (string& e)
	{
		return false;
	}
	return true;
}

void BMPArealStructFactory::getFieldBMPid(map<int, vector<int>>  m_bmpArealStrOps)
{
	if(m_nField <= 0)
		throw ModelException("BMPArealStructFactory", "getFieldBMPid", "Field number should not be less than 0");
	m_fieldBMPid = new int(m_nField);
	for(int i = 0; i < m_nField; i++)
	{
		m_fieldBMPid[i] = 0;
		//cout << "id:" << i << endl;
	}
	//update m_fieldBMPid
	for (map<int, vector<int>>::iterator it = m_bmpArealStrOps.begin(); it != m_bmpArealStrOps.end(); it++)
	{
		cout << "BMP:" << it->first << endl;
		for(vector<int>::iterator it_loc = it->second.begin(); it_loc != it->second.end(); it_loc++)
		{
			m_fieldBMPid[*it_loc] = it->first;
			cout << m_fieldBMPid[*it_loc] << ": " << it->first << endl;
		}
	}
}

void BMPArealStructFactory::Dump(ostream *fs)
{
	if (fs == NULL) return;
	*fs << "Areal-Struct Management Factory: " << endl <<
		"    SubScenario ID: " << m_subScenarioId << endl;
}
