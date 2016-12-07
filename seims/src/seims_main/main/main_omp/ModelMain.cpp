/*!
 * \brief Control the simulation of SEIMS
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date May 2016
 *
 * 
 */
#include "ModelMain.h"
#include "utils.h"
#include "util.h"
#include "ClimateParams.h"
#include "ModelException.h"
#include "PrintInfo.h"
#include "MongoUtil.h"
#include <ctime>
#include <sstream>
#include "clsRasterData.cpp"

ModelMain::ModelMain(mongoc_client_t *conn, string dbName, string projectPath, SettingsInput *input,
                     ModuleFactory *factory, int subBasinID, int scenarioID, int numThread,
                     LayeringMethod layeringMethod)
        : m_conn(conn), m_dbName(dbName), m_outputGfs(NULL), m_projectPath(projectPath), m_input(input), m_factory(factory),
          m_subBasinID(subBasinID), m_scenarioID(scenarioID), m_threadNum(numThread), m_layeringMethod(layeringMethod),
          m_templateRasterData(NULL), m_readFileTime(0.f), m_firstRunChannel(true), m_firstRunOverland(true),
          m_initialized(false), m_output(NULL)
{
    mongoc_gridfs_t *spatialData;
    bson_error_t *err = NULL;
    spatialData = mongoc_client_get_gridfs(m_conn, m_dbName.c_str(), DB_TAB_SPATIAL, err);
    if (err != NULL)
        throw ModelException("MainMongoDB", "ModelMain", "Failed to get GridFS: " + string(DB_TAB_SPATIAL) + ".\n");
	string outputScene = string(DB_TAB_OUT_SPATIAL) + ValueToString(m_scenarioID);
    m_outputGfs = mongoc_client_get_gridfs(m_conn, m_dbName.c_str(), outputScene.c_str(), err);
    if (err != NULL)
        throw ModelException("MainMongoDB", "ModelMain",
                             "Failed to create output GridFS: " + outputScene + ".\n");

    m_dtDaily = m_input->getDtDaily();
    m_dtHs = m_input->getDtHillslope();
    m_dtCh = m_input->getDtChannel();
	/// Load Setting Output from file.out, which is deprecated now! By LJ, 2016-7-11
    /// m_output = new SettingsOutput(m_subBasinID, m_projectPath + File_Output, m_conn, m_dbName, m_outputGfs);
	m_output = new SettingsOutput(m_subBasinID, m_conn, dbName, m_outputGfs);
    CheckOutput(spatialData);

    m_readFileTime = factory->CreateModuleList(m_dbName, m_subBasinID, m_threadNum, m_layeringMethod,
                                               m_templateRasterData, m_input, m_simulationModules);
    //cout << "Read file time: " << m_readFileTime << endl;
    size_t n = m_simulationModules.size();
    m_executeTime.resize(n, 0.f);
    for (size_t i = 0; i < n; i++)
    {
        SimulationModule *pModule = m_simulationModules[i];
        switch (pModule->GetTimeStepType())
        {
            case TIMESTEP_HILLSLOPE:
                m_hillslopeModules.push_back(i);
                break;
            case TIMESTEP_CHANNEL:
                m_channelModules.push_back(i);
                break;
            case TIMESTEP_ECOLOGY:
                m_ecoModules.push_back(i);
            case TIMESTEP_SIMULATION:
                m_overallModules.push_back(i);
        }
    }

    CheckOutput();
    mongoc_gridfs_destroy(spatialData);
}

ModelMain::~ModelMain(void)
{
    StatusMessage("Start to release ModelMain ...");
    try
    {
        if (this->m_templateRasterData != NULL) delete this->m_templateRasterData;
        for (map<string, ParamInfo *>::iterator it = m_parameters.begin(); it != m_parameters.end();)
        {
			if(it->second != NULL)
				delete it->second;
            it->second = NULL;
            it = m_parameters.erase(it);
        }
        m_parameters.clear();
        if (this->m_input != NULL)
			delete this->m_input;
        if (this->m_output != NULL)
            delete this->m_output;
        if (m_outputGfs != NULL)
            ModelMain::CloseGridFS();
        if (!m_conn)
            mongoc_client_destroy(m_conn);
    }
    catch (exception e)
    {
        cout << e.what() << endl;
    }
}

//void ModelMain::Step(time_t time)
//{
//	m_factory->UpdateInput(m_simulationModules, m_input, time);
//	
//	for (size_t i = 0; i < m_simulationModules.size(); i++)
//	{
//		SimulationModule *pModule = m_simulationModules[i];
//
//		if (m_firstRun)
//			m_factory->GetValueFromDependencyModule(i, m_simulationModules);
//		//cout << m_factory->GetModuleID(i) << endl;
//		clock_t sub_t1 = clock();
//		
//		try
//		{
//			pModule->SetDate(time);
//			pModule->Execute();
//		}
//		catch (ModelException e)
//		{
//			cout << e.toString();
//			exit(-1);
//		}
//
//		clock_t sub_t2 = clock();
//		
//		m_executeTime[i] += (sub_t2 - sub_t1);
//	}
//	
//	Output(time);
//	m_firstRun = false;
//	m_firstRunOverland = false;
//	m_firstRunChannel = false;
//	//cout << m_subBasinID << "Step\n";
//}

float ModelMain::GetQOutlet()
{
    if (m_channelModules.size() == 0)
        return 0.f;
    float value;
    int index = m_channelModules[0];
    m_simulationModules[index]->GetValue(TAG_OUT_QOUTLET, &value);
    return value;
}

//void ModelMain::StepOverland(time_t t)
//{
//	m_factory->UpdateInput(m_simulationModules, m_input, t);
//	Step(t, m_hillslopeModules, m_firstRunOverland);
//	m_firstRunOverland = false;
//	//cout << m_subBasinID << "StepOverland\n";
//}

void ModelMain::StepHillSlope(time_t t, int yearIdx, int subIndex)
{
    m_factory->UpdateInput(m_simulationModules, m_input, t);

    for (size_t i = 0; i < m_hillslopeModules.size(); i++)
    {
        int index = m_hillslopeModules[i];
        SimulationModule *pModule = m_simulationModules[index];
        //clock_t sub_t1 = clock();
		double sub_t1 = TimeCounting();
        if (m_firstRunOverland)
            m_factory->GetValueFromDependencyModule(index, m_simulationModules);
        if (subIndex == 0)
            pModule->ResetSubTimeStep();
        pModule->SetDate(t, yearIdx);
        //cout << "\tHillslope process:" << i << endl;
        pModule->Execute();

        //clock_t sub_t2 = clock();
		double sub_t2 = TimeCounting();
        m_executeTime[index] += (sub_t2 - sub_t1);
    }
    m_firstRunOverland = false;
}

void ModelMain::StepChannel(time_t t, int yearIdx)
{
    //cout << m_subBasinID << "StepChannelBegin\n";
    //cout << "Size of channel modules: " << m_channelModules.size() << endl;
    Step(t, yearIdx, m_channelModules, m_firstRunChannel);
    m_firstRunChannel = false;
}

void ModelMain::StepOverall(time_t startT, time_t endT)
{
    for (size_t i = 0; i < m_overallModules.size(); i++)
    {
        int index = m_overallModules[i];
        SimulationModule *pModule = m_simulationModules[index];
        //clock_t sub_t1 = clock();
		double sub_t1 = TimeCounting();
        pModule->Execute();
        //clock_t sub_t2 = clock();
		double sub_t2 = TimeCounting();
        m_executeTime[index] += (sub_t2 - sub_t1);
    }
}

void ModelMain::Step(time_t t, int yearIdx, vector<int> &moduleIndex, bool firstRun)
{
    for (size_t i = 0; i < moduleIndex.size(); i++)
    {
        int index = moduleIndex[i];
        SimulationModule *pModule = m_simulationModules[index];

        if (m_firstRunChannel)
            m_factory->GetValueFromDependencyModule(index, m_simulationModules);

        //clock_t sub_t1 = clock();
		double sub_t1 = TimeCounting();
        pModule->SetDate(t, yearIdx);
        pModule->Execute();
        //clock_t sub_t2 = clock();
		double sub_t2 = TimeCounting();
        m_executeTime[index] += (sub_t2 - sub_t1);
    }
}

void ModelMain::Execute()
{
    utils util;
    //clock_t t1 = clock();
	double t1 = TimeCounting();
    time_t startTime = m_input->getStartTime();
    time_t endTime = m_input->getEndTime();
    int startYear = GetYear(startTime);
    int nHs = 0;
	
    for (time_t t = startTime; t < endTime; t += m_dtCh)
    {
        cout << util.ConvertToString2(&t) << endl;
        /// Calculate index of current year of the entire simulation
        int curYear = GetYear(t);
        int yearIdx = curYear - startYear;
        nHs = int(m_dtCh / m_dtHs);
        for (int i = 0; i < nHs; i++)
            StepHillSlope(t + i * m_dtHs, yearIdx, i);
        StepChannel(t, yearIdx);
        Output(t);
    }
    StepOverall(startTime, endTime);
    //clock_t t2 = clock();
	double t2 = TimeCounting();
    //cout << "time(ms):  " << t2-t1 << endl;
    cout << "[TIMESPAN][COMPUTING]\tALL\t" << fixed << setprecision(3) << (t2 - t1) << endl;
    OutputExecuteTime();
}

void ModelMain::Output()
{
	//clock_t t1 = clock();
	double t1 = TimeCounting();
	string outputPath = m_projectPath + DB_TAB_OUT_SPATIAL + ValueToString(m_scenarioID);
#ifndef linux
	if (::GetFileAttributes(outputPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		LPSECURITY_ATTRIBUTES att = NULL;
		::CreateDirectory(outputPath.c_str(), att);
	}
	else
	{
		vector<string> existedFiles;
		FindFiles(outputPath.c_str(),"*.*",existedFiles);
		for(vector<string>::iterator it = existedFiles.begin(); it != existedFiles.end(); it++)
			remove((*it).c_str());
	}
#else
	if(access(outputPath.c_str(), F_OK) != 0)
	{
		mkdir(outputPath.c_str(), 0777);
	}
	else
	{
		vector<string> existedFiles;
		FindFiles(outputPath.c_str(),".*",existedFiles);
		//cout<<existedFiles.size()<<endl;
		for(vector<string>::iterator it = existedFiles.begin(); it != existedFiles.end(); it++){
			//cout<<*it<<endl;
			remove((*it).c_str());
		}
	}
#endif
    vector<PrintInfo *>::iterator it;
    for (it = this->m_output->m_printInfos.begin(); it < m_output->m_printInfos.end(); it++)
    {
        vector<PrintInfoItem *>::iterator itemIt;
        for (itemIt = (*it)->m_PrintItems.begin(); itemIt < (*it)->m_PrintItems.end(); itemIt++)
        {
            PrintInfoItem *item = *itemIt;
            item->Flush(m_projectPath + DB_TAB_OUT_SPATIAL + ValueToString(m_scenarioID) + SEP, 
				m_templateRasterData, (*it)->getOutputTimeSeriesHeader());
        }
    }
	//clock_t t2 = clock();
	double t2 = TimeCounting();
	cout << "[TIMESPAN][OUTPUTING]\tALL\t" << fixed << setprecision(3) << (t2 - t1) << endl;
}

void ModelMain::CheckOutput(mongoc_gridfs_t *gfs)
{
    if (this->m_input == NULL) return;
    if (this->m_output == NULL) return;

    this->m_output->checkDate(m_input->getStartTime(), m_input->getEndTime());

    ostringstream oss;
#ifdef USE_MONGODB
	// Read Mask raster data and add to m_rsMap in m_factory, by LJ.
    oss << m_subBasinID << "_" << GetUpper(NAME_MASK);
    m_templateRasterData = new clsRasterData<float>(gfs, oss.str().c_str());
	m_factory->AddMaskRaster(oss.str(), m_templateRasterData);
#endif
}

void ModelMain::OutputExecuteTime()
{
    for (size_t i = 0; i < m_simulationModules.size(); i++)
    {
        cout << "[TIMESPAN][COMPUTING]\t" << m_factory->GetModuleID(i) << "\t" << fixed << setprecision(3) <<
        m_executeTime[i] << endl;
    }
}

void ModelMain::CheckOutput()
{
    vector<PrintInfo *>::iterator it;
    for (it = m_output->m_printInfos.begin(); it < m_output->m_printInfos.end();)
    {
        string outputid = (*it)->getOutputID();
        outputid = trim(outputid);

        //try to find module output which match the outputid
        m_factory->FindOutputParameter(outputid, (*it)->m_moduleIndex, (*it)->m_param);

        if ((*it)->m_moduleIndex < 0){
			// Don't throw the exception, just print the WARNING message, and delete the printInfos. By LJ
			cout<<"WARNING: Can't find output variable for output id : "<<outputid<<"."<<endl;
			it = m_output->m_printInfos.erase(it);
            //throw ModelException("ModelMain", "CheckOutput", "Can't find output variable for output id " + outputid +
            //                                                 ". Please check config.fig, file.out and module's metadata.");
		}
		else
			it++;

        //find site index
        //if((*it)->m_param->Dimension == DT_Array1D)
        //{
        //	vector<PrintInfoItem *>::iterator itemIt;
        //	for(itemIt=(*it)->m_PrintItems.begin();itemIt<(*it)->m_PrintItems.end();itemIt++)
        //	{
        //		PrintInfoItem * item = *itemIt;

        //		if(item->SubbasinID != -1) continue;

        //		if(item->SiteID == -1) throw  ModelException("SettingsConfig","CheckOutput","The site id can't be -1 in output id "+outputid+".");

        //		//because different stations can have different types of data, so when looking for the station index,
        //		//the data type should be used.
        //		string dataType = (*it)->getOutputID();
        //		dataType = dataType.substr(2,dataType.length() -2);
        //		int siteIndex = input->StationData()->getSiteIndexFromID(item->SiteID,dataType);

        //		if(siteIndex == -1)
        //		{
        //			char s[20];
        //			strprintf(s,20,"%d",item->SiteID);
        //			throw ModelException("SettingsConfig","CheckOutput","Can't find site "+string(s) + " in input sites.");
        //		}

        //		item->SiteIndex = siteIndex;
        //	}
        //}

        //for output subbasin information
        //if((*it)->m_param->Dimension == DT_Array2D)
        //{
        //	int subbasinSeletedCount = -1;
        //	float* subbasinSelected = NULL;
        //	(*it)->getSubbasinSelected(&subbasinSeletedCount,&subbasinSelected);

        //	if(subbasinSeletedCount <= 0 || subbasinSelected == NULL)
        //		throw ModelException("SettingsConfig","CheckOutput","Can't find subbasin information of output id "+outputid+".");

        //	(*it)->m_param->SubbasinSelected = subbasinSelected;
        //	(*it)->m_param->SubbasinSelectedCount = subbasinSeletedCount;

        //	if(module!=NULL) module->initialSubbasin(subbasinSeletedCount,subbasinSelected); //initial the subbasinselected parameter
        //}
    }
}

/// Revised LiangJun Zhu
/// Fix Output code of DT_Raster2D
/// 5-27-2016
void ModelMain::Output(time_t time)
{
    vector<PrintInfo *>::iterator it;
    for (it = m_output->m_printInfos.begin(); it < m_output->m_printInfos.end(); it++)
    {
        int iModule = (*it)->m_moduleIndex;

        //find the corresponding output variable and module
        ParamInfo *param = (*it)->m_param;
        if (param == NULL){
			throw ModelException("ModelMain", "Output",
				"Output id " + (*it)->getOutputID() + " does not have corresponding output variable.");
		}

        SimulationModule *module = m_simulationModules[iModule];
        if (module == NULL)
            throw ModelException("ModelMain", "Output",
                                 "Output id " + (*it)->getOutputID() + " does not have corresponding module.");

        //process every output file
        vector<PrintInfoItem *>::iterator itemIt;
        for (itemIt = (*it)->m_PrintItems.begin(); itemIt < (*it)->m_PrintItems.end(); itemIt++)
        {
            PrintInfoItem *item = *itemIt;
            const char *keyName = param->Name.c_str();
            //time_t t1 = item->getStartTime();
            //time_t t2 = item->getEndTime();
            if (time >= item->getStartTime() && time <= item->getEndTime())
            {
                if (param->Dimension == DT_Single)
                {
                    float value;
                    module->GetValue(keyName, &value);
                    item->TimeSeriesData[time] = value;
                }
				//time series data for sites or some time series data for subbasins, such as T_SBOF,T_SBIF
                else if (param->Dimension == DT_Array1D)  
                {
                    int index = -1;
                    if (index < 0)
                        index = item->SubbasinID;        //time series data for some time series data for subbasins
                    if (index < 0)
                        index = 0;

                    int n;
                    float *data;
                    module->Get1DData(keyName, &n, &data);
                    item->TimeSeriesData[time] = data[index];
                }
                else if (param->Dimension == DT_Array2D)  //time series data for subbasins
                {
                    //some modules will calculate result for all subbasins or all reaches,
                    //regardless of whether they are output to file or not. In this case,
                    //the 2-D array will contain all the results and the subbasinid or reachid
                    //will be used to locate the result.
                    if (StringMatch(param->BasicName, "RECH") ||    //discharge of reach
                        StringMatch(param->BasicName, "WABA") ||    //channel water balance
                        StringMatch(param->BasicName, "RSWB") ||    //reservoir water balance
                        StringMatch(param->BasicName, "RESB") ||  //reservoir sediment balance
                        StringMatch(param->BasicName, "CHSB") ||
						StringMatch(param->BasicName, VAR_GWWB) ||  // groundwater water balance
						StringMatch(param->BasicName, VAR_SOWB)  // soil water balance
                        )// TODO: more conditions will be added in the future.
                    {
                        //for modules in which only the results of output subbasins are calculated.
                        //In this case, the 2-D array just contain the results of selected subbasins in file.out.
                        //So, the index of Subbasin in file.out will be used to locate the result.
                        int subbasinIndex = item->SubbasinIndex;

                        subbasinIndex = item->SubbasinID;

                        if (subbasinIndex == -1)
                        {
                            char s[20];
                            strprintf(s, 20, "%d", item->SubbasinID);
                            throw ModelException("SettingsConfig", "Output",
                                                 "Can't find subbasin " + string(s) + " in input sites.");
                        }

                        float **data;
                        int nRows, nCols;
                        module->Get2DData(param->BasicName.c_str(), &nRows, &nCols, &data);
                        item->add1DTimeSeriesResult(time, nCols, data[subbasinIndex]);
                    }
                    else
                    {
                        float **data;
                        int nRows, nCols;
                        module->Get2DData(param->BasicName.c_str(), &nRows, &nCols, &data);
                        item->AggregateData2D(time, nRows, nCols, data);
                    }
                }
                else if (param->Dimension == DT_Raster1D) //spatial distribution, calculate average,sum,min or max
                {
                    int n;
                    float *data;
                    //cout << keyName << " " << n << endl;
                    module->Get1DData(keyName, &n, &data);
                    item->AggregateData(time, n, data);
                }
                else if (param->Dimension == DT_Raster2D) // spatial distribution with layers
                {
                    int n, lyrs;
                    float **data;
                    module->Get2DData(keyName, &n, &lyrs, &data);
                    item->AggregateData2D(time, n, lyrs, data);
                }
            }
        }
    }
}

void ModelMain::SetChannelFlowIn(float value)
{
	int index = m_channelModules[0];
	m_simulationModules[index]->SetValue(VAR_QUPREACH, value);
}
