#include "DataCenter.h"

#include "utils_time.h"
#include "text.h"

using namespace utils_time;

DataCenter::DataCenter(InputArgs* input_args, ModuleFactory* factory, const int subbasin_id /* = 0 */) :
    model_name_(input_args->model_name), model_path_(input_args->model_path),
    lyr_method_(input_args->lyr_mtd), subbasin_id_(subbasin_id),
    scenario_id_(input_args->scenario_id), calibration_id_(input_args->calibration_id),
    thread_num_(input_args->thread_num),
    use_scenario_(false), output_scene_(DB_TAB_OUT_SPATIAL),
    output_path_(""),
    model_mode_(""), n_subbasins_(-1), outlet_id_(-1), factory_(factory),
    input_(nullptr), output_(nullptr), clim_station_(nullptr), scenario_(nullptr),
    reaches_(nullptr), subbasins_(nullptr), mask_raster_(nullptr) {
    /// Clean output folder
    if (scenario_id_ >= 0) {
        // -1 means no BMPs scenario will be simulated
        output_scene_ += ValueToString(scenario_id_);
        /// Be aware, m_useScenario will be updated in checkModelPreparedData().
    }
    if (calibration_id_ >= 0) {
        // -1 means no calibration setting will be used.
        output_scene_ += "-" + ValueToString(calibration_id_);
    }
    output_path_ = model_path_ + SEP + output_scene_ + SEP;
    if (subbasin_id_ <= 1) CleanDirectory(output_path_); // avoid repeat operation in mpi version
}

DataCenter::~DataCenter() {
    StatusMessage("Release DataCenter...");
    if (nullptr != input_) {
        StatusMessage("---release setting input data ...");
        delete input_;
        input_ = nullptr;
    }
    if (nullptr != output_) {
        StatusMessage("---release setting output data ...");
        delete output_;
        output_ = nullptr;
    }
    if (nullptr != clim_station_) {
        StatusMessage("---release climate station data ...");
        delete clim_station_;
        clim_station_ = nullptr;
    }
    if (nullptr != scenario_) {
        StatusMessage("---release bmps scenario data ...");
        delete scenario_;
        scenario_ = nullptr;
    }
    if (nullptr != reaches_) {
        StatusMessage("---release reaches data ...");
        delete reaches_;
        reaches_ = nullptr;
    }
    if (nullptr != subbasins_) {
        StatusMessage("---release subbasins data ...");
        delete subbasins_;
        subbasins_ = nullptr;
    }
    StatusMessage("---release map of all 1D and 2D raster data ...");
    for (auto it = rs_map_.begin(); it != rs_map_.end();) {
        if (nullptr != it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            delete it->second;
            it->second = nullptr;
        }
        rs_map_.erase(it++);
    }
    rs_map_.clear();
    StatusMessage("---release map of parameters in MongoDB ...");
    for (auto it = init_params_.begin(); it != init_params_.end();) {
        if (nullptr != it->second) {
            delete it->second;
            it->second = nullptr;
        }
        init_params_.erase(it++);
    }
    init_params_.clear();
    StatusMessage("---release map of 1D array data ...");
    for (auto it = array1d_map_.begin(); it != array1d_map_.end();) {
        if (nullptr != it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            Release1DArray(it->second);
        }
        array1d_map_.erase(it++);
    }
    array1d_map_.clear();
    StatusMessage("---release map of 2D array data ...");
    for (auto it = array2d_map_.begin(); it != array2d_map_.end();) {
        if (nullptr != it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            Release2DArray(array2d_rows_map_[it->first], it->second);
        }
        array2d_map_.erase(it++);
    }
    array2d_map_.clear();
    StatusMessage("---release Interpolation weight data ...");
    for (auto it = weight_data_map_.begin(); it != weight_data_map_.end();) {
        if (nullptr != it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            delete it->second;
            it->second = nullptr;
        }
        weight_data_map_.erase(it++);
    }
    weight_data_map_.clear();
}

bool DataCenter::GetFileInStringVector() {
    if (file_in_strs_.empty()) {
        if (!LoadPlainTextFile(file_in_file_, file_in_strs_)) {
            throw ModelException("DataCenter", "getFileInStringVector", "");
        }
    }
    return true;
}

void DataCenter::SetLapseData(const string& remote_filename, int& rows, int& cols, float**& data) {
    int n_rows = 12;
    int n_cols = 5;
    data = new float *[n_rows];
    for (int i = 0; i < n_rows; i++) {
        data[i] = new float[n_cols];
        data[i][0] = 4.f;    /// element number
        data[i][1] = 0.03f;  // P
        data[i][2] = -0.65f; // T
        data[i][3] = 0.f;    // PET
        data[i][4] = 0.f;    // other Meteorology variables
    }
    /// insert to corresponding maps
#ifdef HAS_VARIADIC_TEMPLATES
    array2d_map_.emplace(remote_filename, data);
    array2d_rows_map_.emplace(remote_filename, n_rows);
    array2d_cols_map_.emplace(remote_filename, n_cols);
#else
    array2d_map_.insert(make_pair(remote_filename, data));
    array2d_rows_map_.insert(make_pair(remote_filename, n_rows));
    array2d_cols_map_.insert(make_pair(remote_filename, n_cols));
#endif
}

void DataCenter::DumpCaliParametersInDB() {
    if (init_params_.empty()) return;
    if (subbasin_id_ > 1) return; // only dump at omp version(subbasin ID is 0) or subbasin 1 of mpi version
    string file_name = output_path_ + SEP + "param.cali";
    std::ofstream fs;
    fs.open(file_name.c_str(), std::ios::ate);
    if (!fs.is_open()) return;

    if (calibration_id_ >= 0) fs << "# Calibration ID:" << calibration_id_ << endl;
    fs << "# All calibrated parameters" << endl;
    for (auto it = init_params_.begin(); it != init_params_.end(); ++it) {
        if (nullptr == it->second) continue;
        ParamInfo* tmp_param = it->second;
        if ((StringMatch(tmp_param->Change, PARAM_CHANGE_RC) && FloatEqual(tmp_param->Impact, 1.f)) ||
            (StringMatch(tmp_param->Change, PARAM_CHANGE_AC) && FloatEqual(tmp_param->Impact, 0.f)) ||
            (StringMatch(tmp_param->Change, PARAM_CHANGE_VC) && FloatEqual(tmp_param->Impact, NODATA_VALUE)) ||
            StringMatch(tmp_param->Change, PARAM_CHANGE_NC)) {
            continue;
        }
        fs << tmp_param->Name << "," << tmp_param->Impact << "," << tmp_param->Change << endl;
    }

    fs.close();
}

double DataCenter::LoadDataForModules(vector<SimulationModule *>& modules) {
    double t1 = TimeCounting();
    vector<string>& module_ids = factory_->GetModuleIDs();
    map<string, SEIMSModuleSetting *>& module_settings = factory_->GetModuleSettings();
    map<string, vector<ParamInfo*> >& module_parameters = factory_->GetModuleParameters();
    for (size_t i = 0; i < module_ids.size(); i++) {
        string id = module_ids[i];
        vector<ParamInfo*>& parameters = module_parameters[id];
        bool vertical_interpolation = true;
        /// Special operation for ITP module
        if (id.find(MID_ITP) != string::npos) {
            modules[i]->SetClimateDataType(module_settings[id]->dataType());
            for (size_t j = 0; j < parameters.size(); j++) {
                ParamInfo* param = parameters[j];
                if (StringMatch(param->Name, Tag_VerticalInterpolation)) {
                    vertical_interpolation = param->Value > 0.f;
                    break;
                }
            }
        }
        for (size_t j = 0; j < parameters.size(); j++) {
            ParamInfo* param = parameters[j];
            SetData(module_settings[id], param, modules[i], vertical_interpolation);
        }
    }
    double timeconsume = TimeCounting() - t1;
    StatusMessage(("Loading data for modules, TIMESPAN " + ValueToString(timeconsume) + " sec.").c_str());
    return timeconsume;
}

void DataCenter::SetData(SEIMSModuleSetting* setting, ParamInfo* param,
                         SimulationModule* p_module, const bool vertital_itp) {
#ifdef _DEBUG
    double stime = TimeCounting();
#endif
    string name = param->BasicName;
    if (setting->dataTypeString().empty()
        && !StringMatch(param->BasicName, CONS_IN_ELEV)
        && !StringMatch(param->BasicName, CONS_IN_LAT)
        && !StringMatch(param->BasicName, CONS_IN_XPR)
        && !StringMatch(param->BasicName, CONS_IN_YPR)) {
        name = param->Name;
    }
    std::ostringstream oss;
    size_t tmp = name.find("LOOKUP");
    if (tmp == string::npos) {
        oss << subbasin_id_ << "_" << name;
    } else {
        oss << name;
    }
    if (StringMatch(name, Tag_Weight)) {
        if (setting->dataTypeString() == DataType_Precipitation) {
            oss << "_P";
        } else {
            oss << "_M";
        }
    }
    string remote_filename = oss.str();

    switch (param->Dimension) {
        case DT_Unknown: throw ModelException("ModuleFactory", "SetData", "Type of " + param->Name + " is unknown.");
        case DT_Single: SetValue(param, p_module);
            break;
        case DT_Array1D: Set1DData(name, remote_filename, p_module, vertital_itp);
            break;
        case DT_Array2D: Set2DData(name, remote_filename, p_module);
            break;
        case DT_Array1DDateValue: break;
        case DT_Raster1D: SetRaster(name, remote_filename, p_module);
            break;
        case DT_Raster2D: SetRaster(name, remote_filename, p_module);
            break;
        case DT_Scenario: SetScenario(p_module);
            break;
        case DT_Reach: SetReaches(p_module);
            break;
        case DT_Subbasin: SetSubbasins(p_module);
            break;
        default: break;
    }
#ifdef _DEBUG
    double timeconsume = TimeCounting() - stime;
    StatusMessage(("Set " + name + ": " + remote_filename + " done, TIMESPAN " +
                      ValueToString(timeconsume) + " sec.").c_str());
#endif
}

void DataCenter::SetValue(ParamInfo* param, SimulationModule* p_module) {
    if (StringMatch(param->Name, Tag_DataType)) {
        // the data type is got from config.fig
        return;
    }
    if (StringMatch(param->Name, Tag_SubbasinId)) {
        param->Value = CVT_FLT(subbasin_id_);
    } else if (StringMatch(param->Name, Tag_CellSize)) {
        // valid cells number, do not be confused with Tag_CellWidth
        param->Value = CVT_FLT(mask_raster_->GetCellNumber()); // old code is ->Size();  they have the same function
    } else if (StringMatch(param->Name, Tag_CellWidth)) {
        //cell size
        param->Value = CVT_FLT(mask_raster_->GetCellWidth());
    } else if (StringMatch(param->Name, Tag_TimeStep)) {
        param->Value = CVT_FLT(input_->getDtDaily()); // return 86400 secs
    } else if (StringMatch(param->Name, Tag_HillSlopeTimeStep)) {
        param->Value = CVT_FLT(input_->getDtHillslope());
    } else if (StringMatch(param->Name, Tag_ChannelTimeStep)) {
        param->Value = CVT_FLT(input_->getDtChannel());
    } else if (StringMatch(param->Name, Tag_LayeringMethod)) {
        param->Value = CVT_FLT(lyr_method_);
    } else {
        if (init_params_.find(GetUpper(param->Name)) != init_params_.end()) {
            param->Value = init_params_[GetUpper(param->Name)]->GetAdjustedValue();
        } else {
            // cout << "WARNING: Parameter " << param->Name << " is not existed in DB!" << endl;
            // param->Value = NODATA_VALUE; // NOT existed parameters will be intialized in modules.
        }
    }

    p_module->SetValue(param->Name.c_str(), param->Value);
}

void DataCenter::Set1DData(const string& para_name, const string& remote_filename,
                           SimulationModule* p_module, const bool vertital_itp) {
    int n;
    float* data = nullptr;
    /// the data has been read before, which stored in m_1DArrayMap
    if (array1d_map_.find(remote_filename) != array1d_map_.end()) {
        data = array1d_map_.at(remote_filename);
        p_module->Set1DData(para_name.c_str(), array1d_len_map_.at(remote_filename), data);
        return;
    }
    if (weight_data_map_.find(remote_filename) != weight_data_map_.end()) {
        ItpWeightData* weight_data = weight_data_map_.at(remote_filename);
        weight_data->GetWeightData(&n, &data);
        p_module->Set1DData(para_name.c_str(), n, data);
        return;
    }
    /// the data has not been read yet.
    /// 1. IF FLOWOUT_INDEX_D8
    if (StringMatch(para_name, Tag_FLOWOUT_INDEX_D8)) {
        Read1DArrayData(para_name, remote_filename, n, data);
        if (mask_raster_->GetCellNumber() != n) {
            throw ModelException("DataCenter", "Set1DData",
                                 "The data length derived from Read1DArray in " + remote_filename +
                                 " is not the same as the template.");
        }
    }
        /// 2. IF Weight data
    else if (StringMatch(para_name, Tag_Weight)) {
        ReadItpWeightData(remote_filename, n, data);
    }
        /// 3. IF Meteorology sites data
    else if (StringMatch(para_name, Tag_Elevation_Meteorology)) {
        if (vertital_itp) {
            n = clim_station_->NumberOfSites(DataType_Meteorology);
            data = clim_station_->GetElevation(DataType_Meteorology);
        } else {
            return;
        }
    }
        /// 4. IF Precipitation sites data
    else if (StringMatch(para_name, Tag_Elevation_Precipitation)) {
        if (vertital_itp) {
            n = clim_station_->NumberOfSites(DataType_Precipitation);
            data = clim_station_->GetElevation(DataType_Precipitation);
        } else {
            return;
        }
    }
        /// 5. IF Latitude of sites
    else if (StringMatch(para_name, Tag_Latitude_Meteorology)) {
        if (vertital_itp) {
            n = clim_station_->NumberOfSites(DataType_Meteorology);
            data = clim_station_->GetLatitude(DataType_Meteorology);
        } else {
            return;
        }
    }
        /// 6. IF any other 1D arrays, such as Heat units of all simulation years (HUTOT)
    else {
        Read1DArrayData(para_name, remote_filename, n, data);
    }
    if (nullptr == data) {
        throw ModelException("ModuleFactory", "Set1DData", "Failed reading file " + remote_filename);
    }
    p_module->Set1DData(para_name.c_str(), n, data);
}

void DataCenter::Set2DData(const string& para_name, const string& remote_filename, SimulationModule* p_module) {
    int n_rows = 0;
    int n_cols = 1;
    float** data;
    /// Get ROUTING_LAYERS real file name
    string real_filename = remote_filename;
    if (StringMatch(para_name, Tag_ROUTING_LAYERS)) {
        real_filename += lyr_method_ == UP_DOWN ? "_UP_DOWN" : "_DOWN_UP";
    }
    /// Check if the data is already loaded
    if (array2d_map_.find(real_filename) != array2d_map_.end()) {
        data = array2d_map_.at(real_filename);
        n_rows = array2d_rows_map_.at(real_filename);
        n_cols = array2d_cols_map_.at(real_filename);

        p_module->Set2DData(para_name.c_str(), n_rows, n_cols, data);
        return;
    }
    /// Load data from DataCenter
    if (StringMatch(para_name, Tag_ROUTING_LAYERS) || StringMatch(para_name, Tag_ROUTING_LAYERS_DINF)) {
        // Routing layering data based on different flow model
        Read2DArrayData(real_filename, n_rows, n_cols, data);
    } else if (StringMatch(para_name, Tag_FLOWIN_INDEX_D8) || StringMatch(para_name, Tag_FLOWIN_INDEX_DINF)
        || StringMatch(para_name, Tag_FLOWIN_PERCENTAGE_DINF) || StringMatch(para_name, Tag_FLOWOUT_INDEX_DINF)) {
        // Flow in and flow out data based on different flow model
        Read2DArrayData(real_filename, n_rows, n_cols, data);
    } else if (StringMatch(para_name, TAG_OUT_OL_IUH)) {
        // Overland flow IUH
        ReadIuhData(remote_filename, n_rows, data);
    } else if (StringMatch(para_name, Tag_LapseRate)) {
        /// Match to the format of DT_Array2D, By LJ.
        SetLapseData(real_filename, n_rows, n_cols, data);
    } else {
        Read2DArrayData(real_filename, n_rows, n_cols, data);
    }

    p_module->Set2DData(para_name.c_str(), n_rows, n_cols, data);
}

void DataCenter::SetRaster(const string& para_name, const string& remote_filename, SimulationModule* p_module) {
    int n, lyrs;
    float* data = nullptr;
    float** data2d = nullptr;
    FloatRaster* raster = nullptr;
    if (rs_map_.find(remote_filename) == rs_map_.end()) {
        raster = ReadRasterData(remote_filename);
        if (nullptr == raster) {
            throw ModelException("DataCenter", "SetRaster", "Load " + remote_filename + " failed!");
        }
        string upper_name = GetUpper(para_name);
        auto find_iter = init_params_.find(upper_name);
        bool adjust_data = false;
        if (find_iter != init_params_.end()) {
            ParamInfo* tmp_param = find_iter->second;
            if ((StringMatch(tmp_param->Change, PARAM_CHANGE_RC) && !FloatEqual(tmp_param->Impact, 1.f)) ||
                (StringMatch(tmp_param->Change, PARAM_CHANGE_AC) && !FloatEqual(tmp_param->Impact, 0.f)) ||
                (StringMatch(tmp_param->Change, PARAM_CHANGE_VC) && !FloatEqual(tmp_param->Impact, NODATA_VALUE)) ||
                StringMatch(tmp_param->Change, PARAM_CHANGE_NC)) {
                adjust_data = true;
            }
        }
        /// 1D or 2D raster data
        if (raster->Is2DRaster()) {
            if (!raster->Get2DRasterData(&n, &lyrs, &data2d)) {
                throw ModelException("DataCenter", "SetRaster", "Load " + remote_filename + " failed!");
            }
            if (nullptr != data2d && adjust_data) {
                init_params_[upper_name]->Adjust2DRaster(n, raster->GetLayers(), data2d);
            }
        } else {
            if (!raster->GetRasterData(&n, &data)) {
                throw ModelException("DataCenter", "SetRaster", "Load " + remote_filename + " failed!");
            }
            if (nullptr != data && adjust_data) {
                init_params_[upper_name]->Adjust1DRaster(n, data);
            }
        }
    } else {
        raster = rs_map_.at(remote_filename);
        //cout << remoteFileName << endl;
    }
    if (raster->Is2DRaster()) {
        if (!raster->Get2DRasterData(&n, &lyrs, &data2d)) {
            throw ModelException("DataCenter", "SetRaster", "Load " + remote_filename + " failed!");
        }
        p_module->Set2DData(para_name.c_str(), n, lyrs, data2d);
    } else {
        if (!raster->GetRasterData(&n, &data)) {
            throw ModelException("DataCenter", "SetRaster", "Load " + remote_filename + " failed!");
        }
        p_module->Set1DData(para_name.c_str(), n, data);
    }
}

/// Added by Liang-Jun Zhu, 2016-6-22
void DataCenter::SetScenario(SimulationModule* p_module) {
    if (nullptr == scenario_ && nullptr == GetScenarioData()) {
        throw ModelException("DataCenter", "SetScenario", "Scenarios has not been set!");;
    }
    p_module->SetScenario(scenario_);
}

/// Added by Liang-Jun Zhu, 2016-7-2
void DataCenter::SetReaches(SimulationModule* p_module) {
    if (nullptr == reaches_ && nullptr == GetReachesData()) {
        throw ModelException("DataCenter", "SetReaches", "Reaches has not been set!");
    }
    p_module->SetReaches(reaches_);
}

/// Added by Liang-Jun Zhu, 2016-7-28
void DataCenter::SetSubbasins(SimulationModule* p_module) {
    if (nullptr == subbasins_ && nullptr == GetSubbasinData()) {
        throw ModelException("DataCenter", "SetSubbasins", "Subbasins data has not been initialized!");
    }
    p_module->SetSubbasins(subbasins_);
}

void DataCenter::UpdateInput(vector<SimulationModule *>& modules, const time_t t) {
    vector<string>& module_ids = factory_->GetModuleIDs();
    map<string, SEIMSModuleSetting *>& module_settings = factory_->GetModuleSettings();
    map<string, vector<ParamInfo*> >& module_inputs = factory_->GetModuleInputs();
    size_t n = module_ids.size();
    for (size_t i = 0; i < n; i++) {
        string id = module_ids[i];
        SimulationModule* p_module = modules[i];
        vector<ParamInfo*>& inputs = module_inputs[id];
        string data_type = module_settings[id]->dataTypeString();
        for (size_t j = 0; j < inputs.size(); j++) {
            ParamInfo* param = inputs[j];
            if (param->DependPara != nullptr) {
                continue;
            } //the input which comes from other modules will not change when the date is change.
            if (StringMatch(param->Name.c_str(), CONS_IN_ELEV)
                || StringMatch(param->Name.c_str(), CONS_IN_LAT)
                || StringMatch(param->Name.c_str(), CONS_IN_XPR)
                || StringMatch(param->Name.c_str(), CONS_IN_YPR))
                continue;
            if (data_type.length() > 0) {
                int datalen;
                float* data;
                clim_station_->GetTimeSeriesData(t, data_type, &datalen, &data);
                if (StringMatch(param->Name.c_str(), DataType_PotentialEvapotranspiration)) {
                    for (int i_data = 0; i_data < datalen; i_data++) {
                        data[i_data] *= init_params_[VAR_K_PET]->GetAdjustedValue();
                    }
                }
                p_module->Set1DData(DataType_Prefix_TS, datalen, data);
            }
        }
    }
}

/// added by Huiran GAO, Feb. 2017
/// redesigned by Liangjun Zhu, 08/16/17
void DataCenter::UpdateParametersByScenario(const int subbsn_id) {
    if (nullptr == scenario_) {
        return;
    }
    map<int, BMPFactory *> bmp_factories = scenario_->GetBMPFactories();
    for (auto iter = bmp_factories.begin(); iter != bmp_factories.end(); ++iter) {
        /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
        if (iter->first / 100000 != BMP_TYPE_AREALSTRUCT) {
            continue;
        }
        cout << "Update parameters by Scenario settings." << endl;
        BMPArealStructFactory* tmp_bmp_areal_struct_factory = static_cast<BMPArealStructFactory *>(iter->second);
        map<int, BMPArealStruct *> arealbmps = tmp_bmp_areal_struct_factory->getBMPsSettings();
        float* mgtunits = tmp_bmp_areal_struct_factory->GetRasterData();
        vector<int> sel_ids = tmp_bmp_areal_struct_factory->getUnitIDs();
        /// Get landuse data of current subbasin ("0_" for the whole basin)
        string lur = GetUpper(ValueToString(subbsn_id) + "_" + VAR_LANDUSE);
        int nsize = -1;
        float* ludata = nullptr;
        rs_map_[lur]->GetRasterData(&nsize, &ludata);

        for (auto iter2 = arealbmps.begin(); iter2 != arealbmps.end(); ++iter2) {
            cout << "  - SubScenario ID: " << iter->second->GetSubScenarioId() << ", BMP name: "
                    << iter2->second->getBMPName() << endl;
            vector<int>& suitablelu = iter2->second->getSuitableLanduse();
            map<string, ParamInfo *>& updateparams = iter2->second->getParameters();
            for (auto iter3 = updateparams.begin(); iter3 != updateparams.end(); ++iter3) {
                string paraname = iter3->second->Name;
                cout << "   -- Parameter ID: " << paraname << endl;
                /// Check whether the parameter is existed in m_parametersInDB.
                ///   If existed, update the missing values, otherwise, print warning message and continue.
                if (init_params_.find(paraname) == init_params_.end()) {
                    cout << "      Warning: the parameter is not defined in PARAMETER table, and "
                            " will not work as expected." << endl;
                    continue;
                }
                ParamInfo* tmpparam = init_params_[paraname];
                if (iter3->second->Change.empty()) {
                    iter3->second->Change = tmpparam->Change;
                }
                iter3->second->Maximum = tmpparam->Maximum;
                iter3->second->Minimun = tmpparam->Minimun;
                // Perform update
                string remote_filename = GetUpper(ValueToString(subbsn_id) + "_" + paraname);
                if (rs_map_.find(remote_filename) == rs_map_.end()) {
                    cout << "      Warning: the parameter name: " << remote_filename <<
                            " is not loaded as 1D or 2D raster, and "
                            " will not work as expected." << endl;
                    continue;
                }
                if (rs_map_[remote_filename]->Is2DRaster()) {
                    int lyr = -1;
                    float** data2d = nullptr;
                    rs_map_[remote_filename]->Get2DRasterData(&nsize, &lyr, &data2d);
                    iter3->second->Adjust2DRaster(nsize, lyr, data2d, mgtunits, sel_ids, ludata, suitablelu);
                } else {
                    float* data = nullptr;
                    rs_map_[remote_filename]->GetRasterData(&nsize, &data);
                    iter3->second->Adjust1DRaster(nsize, data, mgtunits, sel_ids, ludata, suitablelu);
                }
            }
        }
    }
}
