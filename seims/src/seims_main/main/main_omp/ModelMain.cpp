#include "ModelMain.h"

ModelMain::ModelMain(DataCenterMongoDB* dataCenter, ModuleFactory* factory) :
m_dataCenter(dataCenter), m_factory(factory), m_readFileTime(0.f),
m_firstRunOverland(true), m_firstRunChannel(true)
{
    /// Get SettingInput and SettingOutput
    m_input = m_dataCenter->getSettingInput();
    m_output = m_dataCenter->getSettingOutput();
    /// Get mask raster data, Output folder name, etc
    m_maskRaster = m_dataCenter->getMaskData();
    m_outputPath = m_dataCenter->getOutputScenePath();
    /// Get time-step of daily, hillslope, and channel scales
    m_dtDaily = m_input->getDtDaily();
    m_dtHs = m_input->getDtHillslope();
    m_dtCh = m_input->getDtChannel();

    /// Create module list and load data from MongoDB, including calibration of value, 1D data, and 2D data.
    m_readFileTime = m_factory->CreateModuleList(m_simulationModules);
    size_t n = m_simulationModules.size();
    m_executeTime.resize(n, 0.f);
    for (int i = 0; i < n; i++) {
        SimulationModule *pModule = m_simulationModules[i];
        switch (pModule->GetTimeStepType()) {
        case TIMESTEP_HILLSLOPE: {
            m_hillslopeModules.push_back(i);
            break;
        }
        case TIMESTEP_CHANNEL: {
            m_channelModules.push_back(i);
            break;
        }
        case TIMESTEP_ECOLOGY: {
            m_ecoModules.push_back(i);
            break;
        }
        case TIMESTEP_SIMULATION: {
            m_overallModules.push_back(i);
            break;
        }
        default:
            break;
        }
    }
    /// Check the validation of settings of output files, i.e. available of parameter and time ranges
    CheckAvailableOutput();
    /// Update model data if the scenario has requested.
    m_factory->updateParametersByScenario(m_dataCenter->getSubbasinID()); /// TODO
}

ModelMain::~ModelMain(void) {
    StatusMessage("Start to release ModelMain ...");
    if (m_factory != NULL) {
        delete m_factory;
        m_factory = NULL;
    }
    if (m_dataCenter != NULL) {
        delete m_dataCenter;
        m_dataCenter = NULL;
    }
}

void ModelMain::StepHillSlope(time_t t, int yearIdx, int subIndex) {
    m_factory->UpdateInput(m_simulationModules, t);
    if (m_hillslopeModules.empty()) {
        return;
    }
    for (size_t i = 0; i < m_hillslopeModules.size(); i++) {
        int index = m_hillslopeModules[i];
        SimulationModule *pModule = m_simulationModules[index];
        double sub_t1 = TimeCounting();
        if (m_firstRunOverland) {
            m_factory->GetValueFromDependencyModule(index, m_simulationModules);
        }
        if (subIndex == 0) {
            pModule->ResetSubTimeStep();
        }
        pModule->SetDate(t, yearIdx);
        //cout << "\tHillslope process:" << i << endl;
        pModule->Execute();

        double sub_t2 = TimeCounting();
        m_executeTime[index] += (sub_t2 - sub_t1);
    }
    m_firstRunOverland = false;
}

void ModelMain::StepChannel(time_t t, int yearIdx) {
    if (m_channelModules.empty()) {
        return;
    }
    for (size_t i = 0; i < m_channelModules.size(); i++) {
        int index = m_channelModules[i];
        SimulationModule *pModule = m_simulationModules[index];
        if (m_firstRunChannel) {
            m_factory->GetValueFromDependencyModule(index, m_simulationModules);
        }
        double sub_t1 = TimeCounting();
        pModule->SetDate(t, yearIdx);
        pModule->Execute();
        double sub_t2 = TimeCounting();
        m_executeTime[index] += (sub_t2 - sub_t1);
    }
    m_firstRunChannel = false;
}

void ModelMain::StepOverall(time_t startT, time_t endT) {
    if (m_overallModules.empty()) {
        return;
    }
    for (size_t i = 0; i < m_overallModules.size(); i++) {
        int index = m_overallModules[i];
        SimulationModule *pModule = m_simulationModules[index];
        double sub_t1 = TimeCounting();
        pModule->Execute();
        double sub_t2 = TimeCounting();
        m_executeTime[index] += (sub_t2 - sub_t1);
    }
}

void ModelMain::Execute() {
    double t1 = TimeCounting();
    time_t startTime = m_input->getStartTime();
    time_t endTime = m_input->getEndTime();
    int startYear = GetYear(startTime);
    int nHs = 0;

    for (time_t t = startTime; t < endTime; t += m_dtCh) {
        cout << ConvertToString2(&t) << endl;
        /// Calculate index of current year of the entire simulation
        int curYear = GetYear(t);
        int yearIdx = curYear - startYear;
        nHs = int(m_dtCh / m_dtHs);
        for (int i = 0; i < nHs; i++) {
            StepHillSlope(t + i * m_dtHs, yearIdx, i);
        }
        StepChannel(t, yearIdx);
        AppendOutputData(t);
    }
    StepOverall(startTime, endTime);
    double t2 = TimeCounting();
    cout << "[TIMESPAN][COMPUTING]\tALL\t" << fixed << setprecision(3) << (t2 - t1) << endl;
    OutputExecuteTime();
}

void ModelMain::Output(void) {
    double t1 = TimeCounting();
    vector<PrintInfo *>::iterator it;
    for (it = this->m_output->m_printInfos.begin(); it < m_output->m_printInfos.end(); it++) {
        vector<PrintInfoItem *>::iterator itemIt;
        for (itemIt = (*it)->m_PrintItems.begin(); itemIt < (*it)->m_PrintItems.end(); itemIt++) {
            PrintInfoItem *item = *itemIt;
            item->Flush(m_outputPath, m_maskRaster, (*it)->getOutputTimeSeriesHeader());
        }
    }
    double t2 = TimeCounting();
    cout << "[TIMESPAN][OUTPUTING]\tALL\t" << fixed << setprecision(3) << (t2 - t1) << endl;
}

void ModelMain::OutputExecuteTime(void) {
    for (int i = 0; i < m_simulationModules.size(); ++i) {
        cout << "[TIMESPAN][COMPUTING]\t" << m_factory->GetModuleID(i) << "\t" << fixed << setprecision(3) <<
             m_executeTime[i] << endl;
    }
}

void ModelMain::CheckAvailableOutput() {
    m_output->checkDate(m_input->getStartTime(), m_input->getEndTime());
    vector<PrintInfo *>::iterator it;
    for (it = m_output->m_printInfos.begin(); it != m_output->m_printInfos.end();) {
        string outputid = (*it)->getOutputID();
        outputid = trim(outputid);

        //try to find module output which match the outputid
        m_factory->FindOutputParameter(outputid, (*it)->m_moduleIndex, (*it)->m_param);

        if ((*it)->m_moduleIndex < 0) {
            // Don't throw the exception, just print the WARNING message, and delete the printInfos. By LJ
            cout << "WARNING: Can't find output variable for output id : " << outputid << "." << endl;
            it = m_output->m_printInfos.erase(it);
        } else {
            it++;
        }
    }
}

void ModelMain::AppendOutputData(const time_t time) {
    vector<PrintInfo *>::iterator it;
    for (it = m_output->m_printInfos.begin(); it < m_output->m_printInfos.end(); it++) {
        int iModule = (*it)->m_moduleIndex;

        //find the corresponding output variable and module
        ParamInfo *param = (*it)->m_param;
        if (param == NULL) {
            throw ModelException("ModelMain", "Output",
                                 "Output id " + (*it)->getOutputID() + " does not have corresponding output variable.");
        }

        SimulationModule *module = m_simulationModules[iModule];
        if (module == NULL) {
            throw ModelException("ModelMain", "Output",
                                 "Output id " + (*it)->getOutputID() + " does not have corresponding module.");
        }

        //process every output file
        vector<PrintInfoItem *>::iterator itemIt;
        for (itemIt = (*it)->m_PrintItems.begin(); itemIt < (*it)->m_PrintItems.end(); itemIt++) {
            PrintInfoItem *item = *itemIt;
            const char *keyName = param->Name.c_str();
            //time_t t1 = item->getStartTime();
            //time_t t2 = item->getEndTime();
            if (time >= item->getStartTime() && time <= item->getEndTime()) {
                if (param->Dimension == DT_Single) {
                    float value;
                    module->GetValue(keyName, &value);
                    item->TimeSeriesData[time] = value;
                }
                //time series data for sites or some time series data for subbasins, such as T_SBOF,T_SBIF
                else if (param->Dimension == DT_Array1D) {
                    int index = item->SubbasinID;
                    //time series data for some time series data for subbasins
                    if (index < 0) index = 0;

                    int n;
                    float *data;
                    module->Get1DData(keyName, &n, &data);
                    item->TimeSeriesData[time] = data[index];
                } else if (param->Dimension == DT_Array2D)  //time series data for subbasins
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
                        if (subbasinIndex == -1) {
                            char s[20];
                            strprintf(s, 20, "%d", item->SubbasinID);
                            throw ModelException("ModelMain", "Output",
                                                 "Can't find subbasin " + string(s) + " in input sites.");
                        }
                        float **data;
                        int nRows, nCols;
                        module->Get2DData(param->BasicName.c_str(), &nRows, &nCols, &data);
                        item->add1DTimeSeriesResult(time, nCols, data[subbasinIndex]);
                    } else {
                        float **data;
                        int nRows, nCols;
                        module->Get2DData(param->BasicName.c_str(), &nRows, &nCols, &data);
                        item->AggregateData2D(time, nRows, nCols, data);
                    }
                } else if (param->Dimension == DT_Raster1D) //spatial distribution, calculate average,sum,min or max
                {
                    int n;
                    float *data;
                    //cout << keyName << " " << n << endl;
                    module->Get1DData(keyName, &n, &data);
                    item->AggregateData(time, n, data);
                } else if (param->Dimension == DT_Raster2D) // spatial distribution with layers
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
