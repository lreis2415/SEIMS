#include "ModelMain.h"

#include "utils_time.h"
#include "text.h"

using namespace ccgl::utils_time;

ModelMain::ModelMain(DataCenterMongoDB* data_center, ModuleFactory* factory) :
    m_dataCenter(data_center), m_factory(factory), m_readFileTime(0.),
    m_firstRunOverland(true), m_firstRunChannel(true) {
    /// Get SettingInput and SettingOutput
    m_input = m_dataCenter->GetSettingInput();
    m_output = m_dataCenter->GetSettingOutput();
    /// Get mask raster data, Output folder name, etc
    m_maskRaster = m_dataCenter->GetMaskData();
    m_outputPath = m_dataCenter->GetOutputScenePath();
    /// Get time-step of daily, hillslope, and channel scales
    m_dtDaily = m_input->getDtDaily();
    m_dtHs = m_input->getDtHillslope();
    m_dtCh = m_input->getDtChannel();
    /// Get module IDs
    m_moduleIDs = m_factory->GetModuleIDs();
    /// Get transferred value inputs
    m_tfValueInputs = m_factory->GetTransferredInputs();
    m_nTFValues = CVT_INT(m_tfValueInputs.size());
    for (int i = 0; i < m_nTFValues; i++) {
        string module_id = m_tfValueInputs[i]->ModuleID;
        auto itID = find(m_moduleIDs.begin(), m_moduleIDs.end(), module_id);
        int idIndex = CVT_INT(distance(m_moduleIDs.begin(), itID));
        m_tfValueToModuleIdxs.emplace_back(idIndex);
        string param_name = m_tfValueInputs[i]->Name;
        if (m_tfValueInputs[i]->DependPara != nullptr) {
            module_id = m_tfValueInputs[i]->DependPara->ModuleID;
            param_name = m_tfValueInputs[i]->DependPara->Name;
        }
        itID = find(m_moduleIDs.begin(), m_moduleIDs.end(), module_id);
        idIndex = CVT_INT(distance(m_moduleIDs.begin(), itID));
        m_tfValueFromModuleIdxs.emplace_back(idIndex);
        m_tfValueNames.emplace_back(param_name);
    }
    /// Create module list
    m_factory->CreateModuleList(m_simulationModules, m_dataCenter->GetThreadNumber());
    /// Load data from MongoDB, including calibration of value, 1D data, and 2D data.
    m_readFileTime = m_dataCenter->LoadDataForModules(m_simulationModules);
    int n = CVT_INT(m_simulationModules.size());
    m_executeTime.resize(n, 0.f);
    for (int i = 0; i < n; i++) {
        SimulationModule* p_module = m_simulationModules[i];
        switch (p_module->GetTimeStepType()) {
            case TIMESTEP_HILLSLOPE: {
                m_hillslopeModules.emplace_back(i);
                break;
            }
            case TIMESTEP_CHANNEL: {
                m_channelModules.emplace_back(i);
                break;
            }
            case TIMESTEP_SIMULATION: {
                m_overallModules.emplace_back(i);
                break;
            }
            default: break;
        }
    }
    /// Check the validation of settings of output files, i.e. available of parameter and time ranges
    CheckAvailableOutput();
    /// Update model data if the scenario has requested.
    m_dataCenter->UpdateParametersByScenario(m_dataCenter->GetSubbasinID());
}

void ModelMain::StepHillSlope(const time_t t, const int year_idx, const int sub_index) {
    m_dataCenter->UpdateInput(m_simulationModules, t);
    if (m_hillslopeModules.empty()) { return; }
    for (auto it = m_hillslopeModules.begin(); it != m_hillslopeModules.end(); ++it) {
        m_simulationModules[*it]->SetDate(t, year_idx);
    }
    for (auto it = m_hillslopeModules.begin(); it != m_hillslopeModules.end(); ++it) {
        SimulationModule* p_module = m_simulationModules[*it];
        // cout << "Executing " << m_moduleIDs[*it] << endl; // for debug
        double sub_t1 = TimeCounting();
        if (m_firstRunOverland) {
            m_factory->GetValueFromDependencyModule(*it, m_simulationModules);
        }
        if (sub_index == 0) {
            p_module->ResetSubTimeStep();
        }
        p_module->Execute();

        double sub_t2 = TimeCounting();
        m_executeTime[*it] += sub_t2 - sub_t1;
    }
    m_firstRunOverland = false;
}

void ModelMain::StepChannel(const time_t t, const int year_idx) {
    if (m_channelModules.empty()) { return; }
    for (auto it = m_channelModules.begin(); it != m_channelModules.end(); ++it) {
        m_simulationModules[*it]->SetDate(t, year_idx);
    }
    for (auto it = m_channelModules.begin(); it != m_channelModules.end(); ++it) {
        SimulationModule* p_module = m_simulationModules[*it];
        // cout << "Executing " << m_moduleIDs[*it] << endl; // for debug
        if (m_firstRunChannel) {
            m_factory->GetValueFromDependencyModule(*it, m_simulationModules);
        }
        double sub_t1 = TimeCounting();
        p_module->Execute();
        double sub_t2 = TimeCounting();
        m_executeTime[*it] += sub_t2 - sub_t1;
    }
    m_firstRunChannel = false;
}

void ModelMain::StepOverall(time_t start_t, time_t end_t) {
    if (m_overallModules.empty()) {
        return;
    }
    for (size_t i = 0; i < m_overallModules.size(); i++) {
        int index = m_overallModules[i];
        SimulationModule* p_module = m_simulationModules[index];
        double sub_t1 = TimeCounting();
        p_module->Execute();
        double sub_t2 = TimeCounting();
        m_executeTime[index] += sub_t2 - sub_t1;
    }
}

void ModelMain::Execute() {
    double t1 = TimeCounting();
    time_t startTime = m_input->getStartTime();
    time_t endTime = m_input->getEndTime();
    int startYear = GetYear(startTime);
    int nHs = CVT_INT(m_dtCh / m_dtHs);
    int preYearIdx = -1;
    for (time_t t = startTime; t < endTime; t += m_dtCh) {
        /// Calculate index of current year of the entire simulation
        int curYear = GetYear(t);
        int yearIdx = curYear - startYear;
        if (preYearIdx != yearIdx) {
            cout << "Simulation year: " << startYear + yearIdx << endl;
        }
        StatusMessage(ConvertToString2(t).c_str());
        for (int i = 0; i < nHs; i++) {
            StepHillSlope(t + i * m_dtHs, yearIdx, i);
        }
        StepChannel(t, yearIdx);
        AppendOutputData(t);
        preYearIdx = yearIdx;
    }
    StepOverall(startTime, endTime);
    double t2 = TimeCounting();
    cout << "[TIMESPAN][COMP][ALL] " << std::fixed << setprecision(3) << t2 - t1 << endl;
    OutputExecuteTime();
}

void ModelMain::GetTransferredValue(float* tfvalues) {
    for (int i = 0; i < m_nTFValues; i++) {
        m_simulationModules[m_tfValueFromModuleIdxs[i]]->GetValue(m_tfValueNames[i].c_str(), &tfvalues[i]);
    }
}

void ModelMain::SetTransferredValue(const int index, float* tfvalues) {
    if (m_firstRunChannel) {
        for (auto it = m_channelModules.begin(); it != m_channelModules.end(); ++it) {
            m_factory->GetValueFromDependencyModule(*it, m_simulationModules);
        }
        m_firstRunChannel = true;
    }
    for (int i = 0; i < m_nTFValues; i++) {
        m_simulationModules[m_tfValueToModuleIdxs[i]]->SetValueByIndex(m_tfValueNames[i].c_str(), index, tfvalues[i]);
    }
}

double ModelMain::Output() {
    double t1 = TimeCounting();
    MongoGridFs* gfs = new MongoGridFs(m_dataCenter->GetMongoClient()->GetGridFs(m_dataCenter->GetModelName(),
                                                                                 DB_TAB_OUT_SPATIAL));
    for (auto it = m_output->m_printInfos.begin(); it != m_output->m_printInfos.end(); ++it) {
        for (auto itemIt = (*it)->m_PrintItems.begin(); itemIt != (*it)->m_PrintItems.end(); ++itemIt) {
            PrintInfoItem* item = *itemIt;
            item->Flush(m_outputPath, gfs, m_maskRaster, (*it)->getOutputTimeSeriesHeader());
        }
    }
    delete gfs;
    double t2 = TimeCounting();
    if (m_dataCenter->GetSubbasinID() == 0) {
        // Only print for OpenMP version
        cout << "[TIMESPAN][IO  ][Output] " << std::fixed << setprecision(3) << t2 - t1 << endl;
    }
    return t2 - t1;
}

void ModelMain::OutputExecuteTime() {
    for (int i = 0; i < CVT_INT(m_simulationModules.size()); i++) {
        cout << "[TIMESPAN][COMP][" << m_factory->GetModuleID(i) << "] " <<
                std::fixed << std::setprecision(3) << m_executeTime[i] << endl;
    }
}

void ModelMain::CheckAvailableOutput() {
    m_output->checkDate(m_input->getStartTime(), m_input->getEndTime());
    for (auto it = m_output->m_printInfos.begin(); it != m_output->m_printInfos.end();) {
        string outputid = (*it)->getOutputID();
        outputid = Trim(outputid);

        //try to find module output which match the outputid
        m_factory->FindOutputParameter(outputid, (*it)->m_moduleIndex, (*it)->m_param);

        if ((*it)->m_moduleIndex < 0) {
            // Don't throw the exception, just print the WARNING message, and delete the printInfos. By LJ
            if (m_dataCenter->GetSubbasinID() <= 1 || m_dataCenter->GetSubbasinID() == 9999) {
                // Print only once
                cout << "WARNING: Can't find output variable for output id : " << outputid << "." << endl;
            }
            it = m_output->m_printInfos.erase(it);
        } else {
            ++it;
        }
    }
}

void ModelMain::AppendOutputData(const time_t time) {
    for (auto it = m_output->m_printInfos.begin(); it < m_output->m_printInfos.end(); ++it) {
        int iModule = (*it)->m_moduleIndex;
        //find the corresponding output variable and module
        ParamInfo* param = (*it)->m_param;
        if (nullptr == param) {
            throw ModelException("ModelMain", "Output",
                                 "Output id " + (*it)->getOutputID() + " does not have corresponding output variable.");
        }
        SimulationModule* module = m_simulationModules[iModule];
        if (nullptr == module) {
            throw ModelException("ModelMain", "Output",
                                 "Output id " + (*it)->getOutputID() + " does not have corresponding module.");
        }

        //process every output file
        for (auto itemIt = (*it)->m_PrintItems.begin(); itemIt < (*it)->m_PrintItems.end(); ++itemIt) {
            PrintInfoItem* item = *itemIt;
            const char* keyName = param->Name.c_str();
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
                    float* data;
                    module->Get1DData(keyName, &n, &data);
                    item->TimeSeriesData[time] = data[index];
                } else if (param->Dimension == DT_Array2D) {
                    //time series data for subbasins
                    //some modules will calculate result for all subbasins or all reaches,
                    //regardless of whether they are output to file or not. In this case,
                    //the 2-D array will contain all the results and the subbasinid or reachid
                    //will be used to locate the result.
                    if (StringMatch(param->BasicName, "RECH") || //discharge of reach
                        StringMatch(param->BasicName, "WABA") || //channel water balance
                        StringMatch(param->BasicName, "RSWB") || //reservoir water balance
                        StringMatch(param->BasicName, "RESB") || //reservoir sediment balance
                        StringMatch(param->BasicName, "CHSB") ||
                        StringMatch(param->BasicName, VAR_GWWB) || // groundwater water balance
                        StringMatch(param->BasicName, VAR_SOWB)    // soil water balance
                    ) {
                        // TODO: more conditions will be added in the future.
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
                        float** data;
                        int nRows, nCols;
                        module->Get2DData(param->BasicName.c_str(), &nRows, &nCols, &data);
                        item->add1DTimeSeriesResult(time, nCols, data[subbasinIndex]);
                    } else {
                        float** data;
                        int nRows, nCols;
                        module->Get2DData(param->BasicName.c_str(), &nRows, &nCols, &data);
                        item->AggregateData2D(time, nRows, nCols, data);
                    }
                } else if (param->Dimension == DT_Raster1D) {
                    //spatial distribution, calculate average,sum,min or max
                    int n;
                    float* data;
                    //cout << keyName << " " << n << endl;
                    module->Get1DData(keyName, &n, &data);
                    item->AggregateData(time, n, data);
                } else if (param->Dimension == DT_Raster2D) {
                    // spatial distribution with layers
                    int n, lyrs;
                    float** data;
                    module->Get2DData(keyName, &n, &lyrs, &data);
                    item->AggregateData2D(time, n, lyrs, data);
                }
            }
        }
    }
}
