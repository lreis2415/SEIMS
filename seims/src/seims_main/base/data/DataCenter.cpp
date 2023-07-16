#include "DataCenter.h"

#include "utils_time.h"
#include "text.h"
#include "Logging.h"

using namespace utils_time;

DataCenter::DataCenter(InputArgs* input_args, ModuleFactory* factory, const int subbasin_id /* = 0 */) :
    model_name_(input_args->model_name), model_path_(input_args->model_path),
    fdir_method_(input_args->fdir_mtd), lyr_method_(input_args->lyr_mtd), subbasin_id_(subbasin_id),
    scenario_id_(input_args->scenario_id), calibration_id_(input_args->calibration_id),
    mpi_rank_(factory->m_mpi_rank), mpi_size_(factory->m_mpi_size),
    thread_num_(input_args->thread_num),
    use_scenario_(false),
    output_path_(input_args->output_path),
    n_subbasins_(-1), outlet_id_(-1), factory_(factory),
    input_(nullptr), output_(nullptr), clim_station_(nullptr), scenario_(nullptr),
    reaches_(nullptr), subbasins_(nullptr), mask_raster_(nullptr), is_subbasin_conceptual(false), is_lumped(false) {
    // Nothing to do for now.
}

DataCenter::~DataCenter() {
    CLOG(TRACE, LOG_RELEASE) << "Release DataCenter...";
    if (nullptr != input_) {
        CLOG(TRACE, LOG_RELEASE) << "---release setting input data ...";
        delete input_;
        input_ = nullptr;
    }
    if (nullptr != output_) {
        CLOG(TRACE, LOG_RELEASE) << "---release setting output data ...";
        delete output_;
        output_ = nullptr;
    }
    if (nullptr != clim_station_) {
        CLOG(TRACE, LOG_RELEASE) << "---release climate station data ...";
        delete clim_station_;
        clim_station_ = nullptr;
    }
    if (nullptr != scenario_) {
        CLOG(TRACE, LOG_RELEASE) << "---release bmps scenario data ...";
        delete scenario_;
        scenario_ = nullptr;
    }
    if (nullptr != reaches_) {
        CLOG(TRACE, LOG_RELEASE) << "---release reaches data ...";
        delete reaches_;
        reaches_ = nullptr;
    }
    if (nullptr != subbasins_) {
        CLOG(TRACE, LOG_RELEASE) << "---release subbasins data ...";
        delete subbasins_;
        subbasins_ = nullptr;
    }
    CLOG(TRACE, LOG_RELEASE) << "---release map of all 1D and 2D raster data ...";
    for (auto it = rs_map_.begin(); it != rs_map_.end(); ++it) {
        if (nullptr != it->second) {
            CLOG(TRACE, LOG_RELEASE) << "-----" << it->first << " ...";
            delete it->second;
            it->second = nullptr;
        }
    }
    rs_map_.clear();
    CLOG(TRACE, LOG_RELEASE) << "---release map of all integer 1D and 2D raster data ...";
    for (auto it = rs_int_map_.begin(); it != rs_int_map_.end(); ++it) {
        if (nullptr != it->second) {
            CLOG(TRACE, LOG_RELEASE) << "-----" << it->first << " ...";
            delete it->second;
            it->second = nullptr;
        }
    }
    rs_int_map_.clear();
    CLOG(TRACE, LOG_RELEASE) << "---release map of parameters in MongoDB ...";
    for (auto it = init_params_.begin(); it != init_params_.end(); ++it) {
        if (nullptr != it->second) {
            delete it->second;
            it->second = nullptr;
        }
    }
    init_params_.clear();
    CLOG(TRACE, LOG_RELEASE) << "---release map of integer parameters in MongoDB ...";
    for (auto it = init_params_int_.begin(); it != init_params_int_.end(); ++it) {
        if (nullptr != it->second) {
            delete it->second;
            it->second = nullptr;
        }
    }
    init_params_int_.clear();
    CLOG(TRACE, LOG_RELEASE) << "---release map of 1D array data ...";
    for (auto it = array1d_map_.begin(); it != array1d_map_.end(); ++it) {
        if (nullptr != it->second) {
            CLOG(TRACE, LOG_RELEASE) << "-----" << it->first + " ...";
            Release1DArray(it->second);
        }
    }
    array1d_map_.clear();
    CLOG(TRACE, LOG_RELEASE) << "---release map of integer 1D array data ...";
    for (auto it = array1d_int_map_.begin(); it != array1d_int_map_.end(); ++it) {
        if (nullptr != it->second) {
            CLOG(TRACE, LOG_RELEASE) << "-----" << it->first + " ...";
            Release1DArray(it->second);
        }
    }
    array1d_int_map_.clear();
    CLOG(TRACE, LOG_RELEASE) << "---release map of 2D array data ...";
    for (auto it = array2d_map_.begin(); it != array2d_map_.end(); ++it) {
        if (nullptr != it->second) {
            CLOG(TRACE, LOG_RELEASE) << "-----" << it->first << " ...";
            Release2DArray(it->second);
        }
    }
    array2d_map_.clear();
    CLOG(TRACE, LOG_RELEASE) << "---release map of integer 2D array data ...";
    for (auto it = array2d_int_map_.begin(); it != array2d_int_map_.end(); ++it) {
        if (nullptr != it->second) {
            CLOG(TRACE, LOG_RELEASE) << "-----" << it->first << " ...";
            Release2DArray(it->second);
        }
    }
    array2d_int_map_.clear();
}

bool DataCenter::GetFileInStringVector() {
    if (file_in_strs_.empty()) {
        if (!LoadPlainTextFile(file_in_file_, file_in_strs_)) {
            throw ModelException("DataCenter", "getFileInStringVector", "");
        }
    }
    return true;
}

void DataCenter::SetLapseData(const string& remote_filename, int& rows, int& cols, FLTPT**& data) {
    rows = 12;
    cols = 5;
    Initialize2DArray(rows, cols, data, 0.);
    for (int i = 0; i < rows; i++) {
        data[i][0] = 4.;    // element number
        data[i][1] = 0.03;  // P
        data[i][2] = -0.65; // T
        data[i][3] = 0.;    // PET
        data[i][4] = 0.;    // other Meteorology variables
    }
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
        ParamInfo<FLTPT>* tmp_param = it->second;
        if ((StringMatch(tmp_param->Change, PARAM_CHANGE_RC) && FloatEqual(tmp_param->Impact, 1.)) ||
            (StringMatch(tmp_param->Change, PARAM_CHANGE_AC) && FloatEqual(tmp_param->Impact, 0.)) ||
            (StringMatch(tmp_param->Change, PARAM_CHANGE_VC) && FloatEqual(tmp_param->Impact, NODATA_VALUE)) ||
            StringMatch(tmp_param->Change, PARAM_CHANGE_NC)) {
            continue;
        }
        fs << tmp_param->Name << "," << tmp_param->Impact << "," << tmp_param->Change << endl;
    }
    fs.close();
}

bool DataCenter::CheckAdjustment(const string& para_name) {
    string upper_name = GetUpper(para_name);
    auto find_iter = init_params_.find(upper_name);
    bool adjust_data = false;
    if (find_iter != init_params_.end()) {
        ParamInfo<FLTPT>* tmp_param = find_iter->second;
        if ((StringMatch(tmp_param->Change, PARAM_CHANGE_RC) && !FloatEqual(tmp_param->Impact, 1.)) ||
            (StringMatch(tmp_param->Change, PARAM_CHANGE_AC) && !FloatEqual(tmp_param->Impact, 0.)) ||
            (StringMatch(tmp_param->Change, PARAM_CHANGE_VC) && !FloatEqual(tmp_param->Impact, NODATA_VALUE)) ||
            StringMatch(tmp_param->Change, PARAM_CHANGE_NC)) {
            adjust_data = true;
        }
    }
    return adjust_data;
}

bool DataCenter::CheckAdjustmentInt(const string& para_name) {
    string upper_name = GetUpper(para_name);
    auto find_iter = init_params_int_.find(upper_name);
    bool adjust_data = false;
    if (find_iter != init_params_int_.end()) {
        ParamInfo<int>* tmp_param = find_iter->second;
        if ((StringMatch(tmp_param->Change, PARAM_CHANGE_RC) && !FloatEqual(tmp_param->Impact, 1.)) ||
            (StringMatch(tmp_param->Change, PARAM_CHANGE_AC) && !FloatEqual(tmp_param->Impact, 0.)) ||
            (StringMatch(tmp_param->Change, PARAM_CHANGE_VC) && !FloatEqual(tmp_param->Impact, NODATA_VALUE)) ||
            StringMatch(tmp_param->Change, PARAM_CHANGE_NC)) {
            adjust_data = true;
        }
    }
    return adjust_data;
}

void DataCenter::LoadAdjustRasterData(const string& para_name, const string& remote_filename,
                                      const bool is_optional /* = false */, STRING_MAP* opts /* =nullptr */) {
    FloatRaster* raster = nullptr;
    if (!ReadRasterData(remote_filename, raster, opts) || nullptr == raster) {
        if (is_optional) { return; }
        throw ModelException("DataCenter", "LoadAdjustRasterData",
                             "Load " + remote_filename + " failed!");
    }
    string upper_name = GetUpper(para_name);
    if (!CheckAdjustment(upper_name)) { return; }

    int n, lyrs;
    FLTPT* data = nullptr;
    FLTPT** data2d = nullptr;
    /// 1D or 2D raster data
    if (raster->Is2DRaster()) {
        if (!raster->Get2DRasterData(&n, &lyrs, &data2d)) {
            if (is_optional) { return; }
            throw ModelException("DataCenter", "SetRaster",
                                 "Load " + remote_filename + " failed!");
        }
        if (nullptr != data2d) {
            init_params_[upper_name]->Adjust2DRaster(n, raster->GetLayers(), data2d);
        }
    } else {
        if (!raster->GetRasterData(&n, &data)) {
            if (is_optional) { return; }
            throw ModelException("DataCenter", "SetRaster",
                                 "Load " + remote_filename + " failed!");
        }
        if (nullptr != data) {
            init_params_[upper_name]->Adjust1DRaster(n, data);
        }
    }
}

void DataCenter::LoadAdjustIntRasterData(const string& para_name, const string& remote_filename,
                                         const bool is_optional /* = false */, STRING_MAP* opts /* =nullptr */) {
    IntRaster* raster = nullptr;
    if (!ReadRasterData(remote_filename, raster, opts) || nullptr == raster) {
        if (is_optional) { return; }
        throw ModelException("DataCenter", "LoadAdjustRasterData",
                             "Load " + remote_filename + " failed!");
    }
    string upper_name = GetUpper(para_name);
    if (!CheckAdjustmentInt(upper_name)) { return; }

    int n, lyrs;
    int* data = nullptr;
    int** data2d = nullptr;
    /// 1D or 2D raster data
    if (raster->Is2DRaster()) {
        if (!raster->Get2DRasterData(&n, &lyrs, &data2d)) {
            if (is_optional) { return; }
            throw ModelException("DataCenter", "SetRaster",
                                 "Load " + remote_filename + " failed!");
        }
        if (nullptr != data2d) {
            init_params_int_[upper_name]->Adjust2DRaster(n, raster->GetLayers(), data2d);
        }
    }
    else {
        if (!raster->GetRasterData(&n, &data)) {
            if (is_optional) { return; }
            throw ModelException("DataCenter", "SetRaster",
                                 "Load " + remote_filename + " failed!");
        }
        if (nullptr != data) {
            init_params_int_[upper_name]->Adjust1DRaster(n, data);
        }
    }
}

void DataCenter::LoadAdjust1DArrayData(const string& para_name, const string& remote_filename,
                                       const bool is_optional /* = false */, STRING_MAP* opts /* =nullptr */) {
    int n;
    FLTPT* data = nullptr;
    FLTPT* tmpdata = nullptr;
    string upper_name = GetUpper(para_name);
    if (StringMatch(upper_name, Tag_Elevation_Meteorology)) { // Meteorology sites data
        if (clim_station_->NumberOfSites(DataType_Meteorology, n) &&
            clim_station_->GetElevation(DataType_Meteorology, tmpdata)) {
            Initialize1DArray(n, data, tmpdata);
        } else {
            throw ModelException("DataCenter", "LoadAdjust1DArrayData",
                                 "Cannot find Meteorology site!");
        }
    } else if (StringMatch(upper_name, Tag_Elevation_Precipitation)) { // Precipitation sites data
        if (clim_station_->NumberOfSites(DataType_Precipitation, n) &&
            clim_station_->GetElevation(DataType_Precipitation, tmpdata)) {
            Initialize1DArray(n, data, tmpdata);
        } else {
            throw ModelException("DataCenter", "LoadAdjust1DArrayData",
                                 "Cannot find Precipitation site!");
        }
    } else if (StringMatch(upper_name, Tag_Latitude_Meteorology)) { // Latitude of sites
        if (clim_station_->NumberOfSites(DataType_Meteorology, n) &&
            clim_station_->GetLatitude(DataType_Meteorology, tmpdata)) {
            Initialize1DArray(n, data, tmpdata);
        } else {
            throw ModelException("DataCenter", "LoadAdjust1DArrayData",
                                 "Cannot find Latitude_M site!");
        }
    } else { // any other 1D arrays, such as Heat units of all simulation years (HUTOT)
        Read1DArrayData(remote_filename, n, data, opts);
    }
    if (nullptr != data) {
        // Adjust data according to calibration parameters
        if (CheckAdjustment(upper_name)) {
            init_params_[upper_name]->Adjust1DArray(n, data);
        }
#ifdef HAS_VARIADIC_TEMPLATES
        array1d_map_.emplace(remote_filename, data);
        array1d_len_map_.emplace(remote_filename, n);
#else
        array1d_map_.insert(make_pair(remote_filename, data));
        array1d_len_map_.insert(make_pair(remote_filename, n));
#endif
    }
}

void DataCenter::LoadAdjustInt1DArrayData(const string& para_name, const string& remote_filename,
                                          const bool is_optional /* = false */, STRING_MAP* opts /* =nullptr */) {
    int n;
    int* data = nullptr;
    string upper_name = GetUpper(para_name);
    Read1DArrayData(remote_filename, n, data, opts);
    if (nullptr != data) {
        // Adjust data according to calibration parameters
        if (CheckAdjustmentInt(upper_name)) {
            init_params_int_[upper_name]->Adjust1DArray(n, data);
        }
#ifdef HAS_VARIADIC_TEMPLATES
        array1d_int_map_.emplace(remote_filename, data);
        array1d_int_len_map_.emplace(remote_filename, n);
#else
        array1d_int_map_.insert(make_pair(remote_filename, data));
        array1d_int_len_map_.insert(make_pair(remote_filename, n));
#endif
    }
}

void DataCenter::LoadAdjust2DArrayData(const string& para_name, const string& remote_filename, STRING_MAP* opts /* =nullptr */) {
    int n_rows = 0;
    int n_cols = 1;
    FLTPT** data = nullptr;
    string upper_name = GetUpper(para_name);
    /// Load data from DataCenter
    if (StringMatch(upper_name, TAG_OUT_OL_IUH)) {
        // Overland flow IUH
        ReadIuhData(remote_filename, n_rows, data, opts);
        n_cols = 1;
    } else if (StringMatch(upper_name, Tag_LapseRate)) {
        /// Match to the format of DT_Array2D, By LJ.
        SetLapseData(remote_filename, n_rows, n_cols, data);
    } else if (StringMatch(upper_name, Tag_Weight[0])) {
        ReadItpWeightData(remote_filename, n_rows, n_cols, data, opts);
    } else {
        // Including: ROUTING_LAYERS,
        //            FLOWIN_INDEX, FLOWIN_FRACTION,
        //            FLOWOUT_INDEX, FLOWOUT_FRACTION
        Read2DArrayData(remote_filename, n_rows, n_cols, data, opts);
    }
    if (nullptr != data) {
        // Adjust data according to calibration parameters
        if (CheckAdjustment(upper_name)) {
            init_params_[upper_name]->Adjust2DArray(n_rows, data);
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
}

void DataCenter::LoadAdjustInt2DArrayData(const string& para_name, const string& remote_filename, STRING_MAP* opts /* =nullptr */) {
    int n_rows = 0;
    int n_cols = 1;
    int** data = nullptr;
    string upper_name = GetUpper(para_name);
    // Including: ROUTING_LAYERS,
    //            FLOWIN_INDEX,
    //            FLOWOUT_INDEX,
    Read2DArrayData(remote_filename, n_rows, n_cols, data, opts);
    if (nullptr != data) {
        // Adjust data according to calibration parameters
        if (CheckAdjustmentInt(upper_name)) {
            init_params_int_[upper_name]->Adjust2DArray(n_rows, data);
        }
        /// insert to corresponding maps
#ifdef HAS_VARIADIC_TEMPLATES
        array2d_int_map_.emplace(remote_filename, data);
        array2d_int_rows_map_.emplace(remote_filename, n_rows);
        array2d_int_cols_map_.emplace(remote_filename, n_cols);
#else
        array2d_int_map_.insert(make_pair(remote_filename, data));
        array2d_int_rows_map_.insert(make_pair(remote_filename, n_rows));
        array2d_int_cols_map_.insert(make_pair(remote_filename, n_cols));
#endif
    }
}

double DataCenter::LoadParametersForModules(vector<SimulationModule *>& modules) {
    double t1 = TimeCounting();
    vector<string>& module_ids = factory_->GetModuleIDs();
    // module_settings
    map<string, SEIMSModuleSetting *>& module_settings = factory_->GetModuleSettings();
    map<string, Information> module_informations=factory_->GetModuleInformations();
    // floating point number
    map<string, vector<ParamInfo<FLTPT>*> >& module_parameters = factory_->GetModuleParams();
    // integer parameter
    map<string, vector<ParamInfo<int>*> >& module_parameters_int = factory_->GetModuleParamsInt();


    for (size_t i = 0; i < module_ids.size(); i++) {
        string id = module_ids[i];
        vector<ParamInfo<FLTPT>*>& parameters = module_parameters[id];

        STRING_MAP opts;
        bool is_module_conceptual = StringMatch(module_informations[id].ModuleAbstractionType, PARAM_ABSTRACTION_TYPE_CONEPTUAL);
        if (is_subbasin_conceptual && is_module_conceptual) {
            opts.emplace(HEADER_RS_PARAM_ABSTRACTION_TYPE, module_informations[id].ModuleAbstractionType);
        }else if (is_subbasin_conceptual && !is_module_conceptual){
            throw ModelException("DataCenter", "LoadParametersForModules",
                "Conceptual Subbasin must use conceptual modules. While module `"+id+"` is physical only.");
        }else {
            opts.emplace(HEADER_RS_PARAM_ABSTRACTION_TYPE, PARAM_ABSTRACTION_TYPE_PHYSICAL);
        }

        for (size_t j = 0; j < parameters.size(); j++) {
            ParamInfo<FLTPT>* param = parameters[j];
            if (StringMatch(param->Name, Tag_VerticalInterpolation[0])) { continue; }
            SetData(module_settings[id], param, modules[i], &opts);
        }
        vector<ParamInfo<int>*>& parameters_int = module_parameters_int[id];
        for (size_t j = 0; j < parameters_int.size(); j++) {
            ParamInfo<int>* param = parameters_int[j];
            if (StringMatch(param->Name, Tag_VerticalInterpolation[0])) {
                modules[i]->SetValue(param->Name.c_str(), param->Value);
                continue;
            }
            SetData(module_settings[id], param, modules[i], &opts);
        }
    }
    double timeconsume = TimeCounting() - t1;
    CLOG(TRACE, LOG_INIT) << "Loading data for modules, TIMESPAN " << timeconsume << " sec.";
    return timeconsume;
}

void DataCenter::SetData(SEIMSModuleSetting* setting, ParamInfo<FLTPT>* param,
                         SimulationModule* p_module, STRING_MAP* opts /* =nullptr */) {
    double stime = TimeCounting();
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
    if (StringMatch(name, Tag_Weight[0])) {
        if (setting->dataTypeString() == DataType_Precipitation) {
            oss << "_P";
        } else {
            oss << "_M";
        }
    }
    string remote_filename = oss.str();

    // If the parameters from Database is optional.
    bool is_opt = false;
    if (StringMatch(param->Source, Source_ParameterDB_Optional)) is_opt = true;

    switch (param->Dimension) {
        case DT_Unknown: throw ModelException("ModuleFactory", "SetData",
                                              "Type of " + param->Name + " is unknown.");
        case DT_Single: SetValue(param, p_module);
            break;
        case DT_Array1D: Set1DData(name, remote_filename, p_module, is_opt, opts);
            break;
        case DT_Array2D: Set2DData(name, remote_filename, p_module, is_opt, opts);
            break;
        case DT_Array1DDateValue:
            break;
        case DT_Raster1D: SetRaster(name, remote_filename, p_module, is_opt, opts);
            break;
        case DT_Raster2D: SetRaster(name, remote_filename, p_module, is_opt, opts);
            break;
        case DT_Scenario: SetScenario(p_module, is_opt);
            break;
        case DT_Reach: SetReaches(p_module);
            break;
        case DT_Subbasin: SetSubbasins(p_module);
            break;
		//case DT_CH_DEPTH:SetReachDepthData(p_module);
		//	break;
        default: break;
    }
    double timeconsume = TimeCounting() - stime;
    CLOG(TRACE, LOG_INIT) << "Set " << name << ": " << remote_filename <<
    " done, TIMESPAN " << timeconsume << " sec.";
}

void DataCenter::SetData(SEIMSModuleSetting* setting, ParamInfo<int>* param,
                         SimulationModule* p_module, STRING_MAP* opts /* =nullptr */) {
    double stime = TimeCounting();
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
    }
    else {
        oss << name;
    }
    if (StringMatch(name, Tag_Weight[0])) {
        if (setting->dataTypeString() == DataType_Precipitation) {
            oss << "_P";
        }
        else {
            oss << "_M";
        }
    }
    string remote_filename = oss.str();

    // If the parameters from Database is optional.
    bool is_opt = false;
    if (StringMatch(param->Source, Source_ParameterDB_Optional)) is_opt = true;

    switch (param->Dimension) {
        case DT_Unknown: throw ModelException("ModuleFactory", "SetData",
                                              "Type of " + param->Name + " is unknown.");
        case DT_SingleInt: SetValue(param, p_module);
            break;
        case DT_Array1DInt: Set1DDataInt(name, remote_filename, p_module, is_opt, opts);
            break;
        case DT_Array2DInt: Set2DDataInt(name, remote_filename, p_module, is_opt, opts);
            break;
        case DT_Array1DDateValue:
            break;
        case DT_Raster1DInt: SetRasterInt(name, remote_filename, p_module, is_opt, opts);
            break;
        case DT_Raster2DInt: SetRasterInt(name, remote_filename, p_module, is_opt, opts);
            break;
        case DT_Scenario: SetScenario(p_module, is_opt);
            break;
        case DT_Reach: SetReaches(p_module);
            break;
        case DT_Subbasin: SetSubbasins(p_module);
            break;
        default: break;
    }
    double timeconsume = TimeCounting() - stime;
    CLOG(TRACE, LOG_INIT) << "Set " << name << ": " << remote_filename <<
    " done, TIMESPAN " << timeconsume << " sec.";
}

void DataCenter::SetValue(ParamInfo<FLTPT>* param, SimulationModule* p_module) {
    if (StringMatch(param->Name, Tag_DataType)) {
        // the data type is got from config.fig
        return;
    }
    if (StringMatch(param->Name, Tag_CellWidth[0])) {
        param->Value = CVT_FLT(mask_raster_->GetCellWidth()); //cell size
    } else {
        if (init_params_.find(GetUpper(param->Name)) != init_params_.end()) {
            param->Value = init_params_[GetUpper(param->Name)]->GetAdjustedValue();
        } else {
            if (!StringMatch(param->Source, Source_ParameterDB_Optional)) {
                throw ModelException("ModuleFactory", "SetValue",
                                     param->Name + " must be specified!");
            }
            return; // if optional, just return without assignment
        }
    }
    p_module->SetValue(param->Name.c_str(), param->Value);
}


void DataCenter::SetValue(ParamInfo<int>* param, SimulationModule* p_module) {
    if (StringMatch(param->Name, Tag_DataType)) {
        // the data type is got from config.fig
        return;
    }
    if (StringMatch(param->Name, Tag_CellSize[0])) {
        // valid cells number, do not be confused with Tag_CellWidth
        param->Value = mask_raster_->GetCellNumber(); // old code is ->Size();  they have the same function
    } else if (StringMatch(param->Name, Tag_SubbasinId)) {
        param->Value = subbasin_id_;
    } else if (StringMatch(param->Name, VAR_SUBBSNID_NUM[0])) {
        param->Value = n_subbasins_;
    } else if (StringMatch(param->Name, Tag_TimeStep[0])) {
        param->Value = CVT_INT(input_->getDtDaily()); // return 86400 secs
    } else if (StringMatch(param->Name, Tag_HillSlopeTimeStep[0])) {
        param->Value = CVT_INT(input_->getDtHillslope());
    } else if (StringMatch(param->Name, Tag_ChannelTimeStep[0])) {
        param->Value = CVT_INT(input_->getDtChannel());
    } else if (StringMatch(param->Name, Tag_LayeringMethod[0])) {
        param->Value = lyr_method_;
    } else if (StringMatch(param->Name, Tag_FlowDirectionMethod[0])) {
        param->Value = fdir_method_;
    }
    else {
        if (init_params_int_.find(GetUpper(param->Name)) != init_params_int_.end()) {
            param->Value = init_params_int_[GetUpper(param->Name)]->GetAdjustedValue();
        }
        else {
            if (!StringMatch(param->Source, Source_ParameterDB_Optional)) {
                throw ModelException("ModuleFactory", "SetValue",
                                     param->Name + " must be specified!");
            }
            return; // if optional, just return without assignment
        }
    }
    p_module->SetValue(param->Name.c_str(), param->Value);
}

void DataCenter::Set1DData(const string& para_name, const string& remote_filename,
                           SimulationModule* p_module, const bool is_optional /* = false */,
                           STRING_MAP* opts /* =nullptr */) {
    FLTPT* data = nullptr;
    /// If the data has not been loaded
    if (array1d_map_.find(remote_filename) == array1d_map_.end()) {
        LoadAdjust1DArrayData(para_name, remote_filename, is_optional, opts);
    }
    /// If the data has been read and stored in `array1d_map_` successfully
    if (array1d_map_.find(remote_filename) != array1d_map_.end()) {
        data = array1d_map_.at(remote_filename);
        p_module->Set1DData(para_name.c_str(), array1d_len_map_.at(remote_filename), data);
        return;
    }
    if (!is_optional) {
        throw ModelException("ModuleFactory", "Set1DData",
                             "Failed reading file " + remote_filename);
    }
}

void DataCenter::Set1DDataInt(const string& para_name, const string& remote_filename,
                              SimulationModule* p_module, const bool is_optional /* = false */,
                              STRING_MAP* opts /* =nullptr */) {
    int* data = nullptr;
    /// If the data has not been loaded
    if (array1d_int_map_.find(remote_filename) == array1d_int_map_.end()) {
        LoadAdjustInt1DArrayData(para_name, remote_filename, is_optional, opts);
    }
    /// If the data has been read and stored in `array1d_map_` successfully
    if (array1d_int_map_.find(remote_filename) != array1d_int_map_.end()) {
        data = array1d_int_map_.at(remote_filename);
        p_module->Set1DData(para_name.c_str(), array1d_len_map_.at(remote_filename), data);
        return;
    }
    if (!is_optional) {
        throw ModelException("ModuleFactory", "Set1DData",
                             "Failed reading file " + remote_filename);
    }
}

void DataCenter::Set2DData(const string& para_name, const string& remote_filename,
                           SimulationModule* p_module, const bool is_optional /* = false */,
                           STRING_MAP* opts /* =nullptr */) {
    int n_rows = 0;
    int n_cols = 1;
    FLTPT** data = nullptr;
    string real_filename = remote_filename;
    if (StringMatch(para_name, Tag_FLOWIN_FRACTION[0]) || StringMatch(para_name, Tag_FLOWOUT_FRACTION[0])) {
        /// Get FLOWIN/FLOWOUT_FRACTION's real file name according to flow direction algorithm except D8
        if (fdir_method_ == D8) { return; }
        real_filename.append(FlowDirMethodString[fdir_method_]);
    }
    if (array2d_map_.find(real_filename) == array2d_map_.end()) {
        LoadAdjust2DArrayData(para_name, real_filename, opts);
    }
    /// Check if the data is already loaded
    if (array2d_map_.find(real_filename) != array2d_map_.end()) {
        data = array2d_map_.at(real_filename);
        n_rows = array2d_rows_map_.at(real_filename);
        n_cols = array2d_cols_map_.at(real_filename);

        p_module->Set2DData(para_name.c_str(), n_rows, n_cols, data);
        return;
    }
    if (!is_optional) {
        throw ModelException("DataCenter", "Set2DData",
                             "Failed reading file " + remote_filename);
    }
}

void DataCenter::Set2DDataInt(const string& para_name, const string& remote_filename,
                              SimulationModule* p_module, const bool is_optional /* = false */,
                              STRING_MAP* opts /* =nullptr */) {
    int n_rows = 0;
    int n_cols = 1;
    int** data = nullptr;
    string real_filename = remote_filename;
    if (StringMatch(para_name, Tag_ROUTING_LAYERS[0])) {
        /// Get ROUTING_LAYERS's real file name according to Layering method and flow direction algorithm
        real_filename.append(LayeringMethodString[lyr_method_]);
        real_filename.append(FlowDirMethodString[fdir_method_]);
    }
    else if (StringMatch(para_name, Tag_FLOWIN_INDEX[0]) || StringMatch(para_name, Tag_FLOWOUT_INDEX[0])) {
        /// Get FLOWIN/FLOWOUT_INDEX's real file name according to flow direction algorithm
        real_filename.append(FlowDirMethodString[fdir_method_]);
    }
    if (array2d_int_map_.find(real_filename) == array2d_int_map_.end()) {
        LoadAdjustInt2DArrayData(para_name, real_filename, opts);
    }
    /// Check if the data is already loaded
    if (array2d_int_map_.find(real_filename) != array2d_int_map_.end()) {
        data = array2d_int_map_.at(real_filename);
        n_rows = array2d_int_rows_map_.at(real_filename);
        n_cols = array2d_int_cols_map_.at(real_filename);

        p_module->Set2DData(para_name.c_str(), n_rows, n_cols, data);
        return;
    }
    if (!is_optional) {
        throw ModelException("DataCenter", "Set2DData",
                             "Failed reading file " + remote_filename);
    }
}

void DataCenter::SetRaster(const string& para_name, const string& remote_filename,
                           SimulationModule* p_module, const bool is_optional /* = false */,
                           STRING_MAP* opts /* =nullptr */) {
    int n, lyrs;
    FLTPT* data = nullptr;
    FLTPT** data2d = nullptr;
    FloatRaster* raster = nullptr;
    if (rs_map_.find(remote_filename) == rs_map_.end()) {
        if (StringMatch(para_name.c_str(), Type_RasterPositionData)) {
            if (!rs_map_.empty()) {
                raster = rs_map_.begin()->second;
                int** positions = raster->GetRasterPositionDataPointer();
                int rows = raster->GetRows();
                int cols = raster->GetCols();
                p_module->SetValue(HEADER_RS_NROWS, rows);
                p_module->SetValue(HEADER_RS_NCOLS, cols);
                p_module->SetRasterPositionDataPointer(para_name.c_str(), positions);
                return;
            }
        }
        else {
            LoadAdjustRasterData(para_name, remote_filename, is_optional, opts);
        }
    }
    if (rs_map_.find(remote_filename) == rs_map_.end()) {
        return; // when encounter optional parameters
    }
    raster = rs_map_.at(remote_filename);
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

void DataCenter::SetRasterInt(const string& para_name, const string& remote_filename,
                              SimulationModule* p_module, const bool is_optional /* = false */,
                              STRING_MAP* opts /* =nullptr */) {
    int n, lyrs;
    int* data = nullptr;
    int** data2d = nullptr;
    IntRaster* raster = nullptr;
    if (rs_int_map_.find(remote_filename) == rs_int_map_.end()) {
        LoadAdjustIntRasterData(para_name, remote_filename, is_optional, opts);
    }
    if (rs_int_map_.find(remote_filename) == rs_int_map_.end()) {
        return; // when encounter optional parameters
    }
    raster = rs_int_map_.at(remote_filename);
    if (raster->Is2DRaster()) {
        if (!raster->Get2DRasterData(&n, &lyrs, &data2d)) {
            throw ModelException("DataCenter", "SetRasterInt", "Load " + remote_filename + " failed!");
        }
        p_module->Set2DData(para_name.c_str(), n, lyrs, data2d);
    }
    else {
        if (!raster->GetRasterData(&n, &data)) {
            throw ModelException("DataCenter", "SetRasterInt", "Load " + remote_filename + " failed!");
        }
        p_module->Set1DData(para_name.c_str(), n, data);
    }
}

void DataCenter::SetScenario(SimulationModule* p_module, const bool is_optional /* = false */) {
    if (nullptr == scenario_ && nullptr == GetScenarioData()) {
        if (!is_optional) {
            throw ModelException("DataCenter", "SetScenario", "Scenarios has not been set!");
        }
    }
    p_module->SetScenario(scenario_);
}

void DataCenter::SetReaches(SimulationModule* p_module) {
    if (nullptr == reaches_ && nullptr == GetReachesData()) {
        throw ModelException("DataCenter", "SetReaches", "Reaches has not been set!");
    }
    p_module->SetReaches(reaches_);
}

void DataCenter::SetSubbasins(SimulationModule* p_module) {
    if (nullptr == subbasins_ && nullptr == GetSubbasinData()) {
        throw ModelException("DataCenter", "SetSubbasins", "Subbasins data has not been initialized!");
    }
    p_module->SetSubbasins(subbasins_);
}

void DataCenter::UpdateOutputDate(time_t start_time, time_t end_time) {
    for (auto it = origin_out_items_.begin(); it < origin_out_items_.end(); ++it) {
        if ((*it).sTimet < start_time || (*it).sTimet >= end_time) {
            CLOG(TRACE, LOG_INIT) << "The start time of output " << (*it).outFileName
            << " will be changed to " << ConvertToString2(start_time);
            (*it).sTimet = start_time;
        }
        if ((*it).eTimet > end_time || (*it).eTimet <= start_time) {
            (*it).eTimet = end_time;
            CLOG(TRACE, LOG_INIT) << "The end time of output " << (*it).outFileName
            << " will be changed to " << ConvertToString2(end_time);
            (*it).eTimet = end_time;
        }
        CLOG(TRACE, LOG_INIT) << "Output Info of subbasin: " << subbasin_id_ << ": "
        << (*it).outputID << ": " << (*it).aggType << ", "
        << ConvertToString((*it).sTimet) << " -- " << ConvertToString2((*it).eTimet)
        << ", " << (*it).subBsn;
    }
}

void DataCenter::UpdateInput(vector<SimulationModule *>& modules, const time_t t) {
    vector<string>& module_ids = factory_->GetModuleIDs();
    map<string, SEIMSModuleSetting *>& module_settings = factory_->GetModuleSettings();
    map<string, vector<ParamInfo<FLTPT>*> >& module_inputs = factory_->GetModuleInputs();
    size_t n = module_ids.size();
    for (size_t i = 0; i < n; i++) {
        string id = module_ids[i];
        //cout << "processing module:" << id << endl;
        SimulationModule* p_module = modules[i];
        vector<ParamInfo<FLTPT>*>& inputs = module_inputs[id];
        string data_type = module_settings[id]->dataTypeString();
        for (size_t j = 0; j < inputs.size(); j++) {
            ParamInfo<FLTPT>* param = inputs[j];
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
                FLTPT* data;
                clim_station_->GetTimeSeriesData(t, data_type, &datalen, &data);
                if (StringMatch(param->Name.c_str(), DataType_PotentialEvapotranspiration)) {
                    for (int i_data = 0; i_data < datalen; i_data++) {
                        data[i_data] *= init_params_[VAR_K_PET[0]]->GetAdjustedValue();
                    }
                }
                p_module->Set1DData(DataType_Prefix_TS, datalen, data);
            }
        }
    }
}

void DataCenter::UpdateScenarioParametersStable(const int subbsn_id) {
    if (nullptr == scenario_) {
        return;
    }
    map<int, BMPFactory *> bmp_factories = scenario_->GetBMPFactories();
    for (auto iter = bmp_factories.begin(); iter != bmp_factories.end(); ++iter) {
        /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
        if (iter->first / 100000 != BMP_TYPE_AREALSTRUCT) {
            continue;
        }

        // only update BMP without variable parameters
        if (iter->second->IsEffectivenessChangeable()) continue;

        cout << "Update by Scenario parameters stable" << endl;
        BMPArealStructFactory* tmp_bmp_areal_struct_factory = static_cast<BMPArealStructFactory *>(iter->second);
        map<int, BMPArealStruct *> arealbmps = tmp_bmp_areal_struct_factory->getBMPsSettings();
        int* mgtunits = tmp_bmp_areal_struct_factory->GetRasterData();
        vector<int> sel_ids = tmp_bmp_areal_struct_factory->getUnitIDs();
        /// Get landuse data of current subbasin ("0_" for the whole basin)
        string lur = GetUpper(ValueToString(subbsn_id) + "_" + VAR_LANDUSE[0]);
        int nsize = -1;
        int* ludata = nullptr;
        rs_int_map_[lur]->GetRasterData(&nsize, &ludata);

        for (auto iter2 = arealbmps.begin(); iter2 != arealbmps.end(); ++iter2) {

            cout << "  - SubScenario ID: " << iter->second->GetSubScenarioId() << ", BMP name: "
                    << iter2->second->getBMPName() << endl;
            vector<int>& suitablelu = iter2->second->getSuitableLanduse();
            map<string, ParamInfo<FLTPT>*>& updateparams = iter2->second->getParameters();
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
                ParamInfo<FLTPT>* tmpparam = init_params_[paraname];
                if (iter3->second->Change.empty()) {
                    iter3->second->Change = tmpparam->Change;
                }
                iter3->second->Maximum = tmpparam->Maximum;
                iter3->second->Minimum = tmpparam->Minimum;
                // Perform update
                string remote_filename = GetUpper(ValueToString(subbsn_id) + "_" + paraname);
                if (rs_map_.find(remote_filename) == rs_map_.end()) {
                    cout << "      Warning: the parameter name: " << remote_filename <<
                            " is not loaded as 1D or 2D raster, and "
                            " will not work as expected." << endl;
                    continue;
                }
                int count = 0;
                if (rs_map_[remote_filename]->Is2DRaster()) {
                    int lyr = -1;
                    FLTPT** data2d = nullptr;
                    rs_map_[remote_filename]->Get2DRasterData(&nsize, &lyr, &data2d);
                    count = iter3->second->Adjust2DRaster(nsize, lyr, data2d, mgtunits,
                                                          sel_ids, ludata, suitablelu);
                }
                else {
                    FLTPT* data = nullptr;
                    rs_map_[remote_filename]->GetRasterData(&nsize, &data);
                    count = iter3->second->Adjust1DRaster(nsize, data, mgtunits, sel_ids,
                                                          ludata, suitablelu);
                }
                cout << "      A total of " << count << " has been updated for " <<
                        remote_filename << endl;
            }
        }
    }
}

bool DataCenter::UpdateScenarioParametersDynamic(const int subbsn_id, time_t t) {
    bool hasUpdated = false;
    if (nullptr == scenario_) {
        return hasUpdated;
    }

    map<int, BMPFactory *> bmp_factories = scenario_->GetBMPFactories();
    for (auto iter = bmp_factories.begin(); iter != bmp_factories.end(); ++iter) {
        /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
        if (iter->first / 100000 != BMP_TYPE_AREALSTRUCT) continue;

        // only update BMP without variable parameters
        if (!iter->second->IsEffectivenessChangeable()) continue;

        //DEBUG: only fengjin
        //if (iter->second->GetSubScenarioId() != 1) continue;

        BMPArealStructFactory* tmp_bmp_areal_struct_factory = static_cast<BMPArealStructFactory *>(iter->second);
        map<int, BMPArealStruct *> arealbmps = tmp_bmp_areal_struct_factory->getBMPsSettings();
        for (auto iter2 = arealbmps.begin(); iter2 != arealbmps.end(); ++iter2) {
            if (tmp_bmp_areal_struct_factory->getSeriesIndex() == tmp_bmp_areal_struct_factory->GetChangeTimes()){
                continue;
            }

            time_t lastUpdateTime = iter2->second->getLastUpdateTime();
            time_t changeFrequency = iter->second->GetChangeFrequency();

            // update condition: long enough
            time_t needUpdateTime = -1;
            if (lastUpdateTime == -1) {//first time
                //Add a configuration item later!
                time_t warmUpPeriod = 31536000;// 1 year
                needUpdateTime = input_->getStartTime() + warmUpPeriod + changeFrequency;
                //needUpdateTime = input_->getStartTime() + 1;//debug only
            }
            else{
                needUpdateTime = lastUpdateTime + changeFrequency;
                //needUpdateTime = lastUpdateTime + 2;// changeFrequency;
            }

            if (t >= needUpdateTime){
                cout << "Update scenario parameters dynamically." << endl;
                int* mgtunits = tmp_bmp_areal_struct_factory->GetRasterData();
                vector<int> sel_ids = tmp_bmp_areal_struct_factory->getUnitIDsByIndex();
                map<int, int> unitUpdateTimes = tmp_bmp_areal_struct_factory->getUpdateTimesByIndex();
                // some spatial units need to update
                if (!sel_ids.empty()){
                    /// Get landuse data of current subbasin ("0_" for the whole basin)
                    string lur = GetUpper(ValueToString(subbsn_id) + "_" + VAR_LANDUSE[0]);
                    int nsize = -1;
                    int* ludata = nullptr;
                    rs_int_map_[lur]->GetRasterData(&nsize, &ludata);

                    cout << "  - SubScenario ID: " << iter->second->GetSubScenarioId() << ", BMP name: "
                            << iter2->second->getBMPName() << endl;
                    vector<int>& suitablelu = iter2->second->getSuitableLanduse();
                    map<string, ParamInfo<FLTPT> *>& updateparams = iter2->second->getParameters();
                    for (auto iter3 = updateparams.begin(); iter3 != updateparams.end(); ++iter3) {
                        string paraname = iter3->second->Name;

                        //DEBUG: only conductivity
                        //if (paraname != "CONDUCTIVITY")continue;

                        cout << "   -- Parameter ID: " << paraname << endl;
                        /// Check whether the parameter is existed in m_parametersInDB.
                        ///   If existed, update the missing values, otherwise, print warning message and continue.
                        if (init_params_.find(paraname) == init_params_.end()) {
                            cout << "      Warning: the parameter is not defined in PARAMETER table, and "
                                    " will not work as expected." << endl;
                            continue;
                        }
                        ParamInfo<FLTPT>* tmpparam = init_params_[paraname];
                        if (iter3->second->Change.empty()) {
                            iter3->second->Change = tmpparam->Change;
                        }
                        iter3->second->Maximum = tmpparam->Maximum;
                        iter3->second->Minimum = tmpparam->Minimum;

                        // Perform update
                        string remote_filename = GetUpper(ValueToString(subbsn_id) + "_" + paraname);
                        if (rs_map_.find(remote_filename) == rs_map_.end()) {
                            cout << "      Warning: the parameter name: " << remote_filename <<
                                    " is not loaded as 1D or 2D raster, and "
                                    " will not work as expected." << endl;
                            continue;
                        }
#ifdef _DEBUG
                        // DEBUG: output the modified data
                        CLOG(INFO, LOG_OUTPUT) << t << "  - SubScenario ID: " << iter->second->GetSubScenarioId() << ", BMP name: "
                        << iter2->second->getBMPName() << " param: " << remote_filename;
                        vector<string> output_params;
                        output_params.push_back("0_CONDUCTIVITY"); //"0_DENSITY", "0_CONDUCTIVITY"
#endif // _DEBUG
                        int count = 0;
                        if (rs_map_[remote_filename]->Is2DRaster()) {
                            int lyr = -1;
                            FLTPT** data2d = nullptr;
                            rs_map_[remote_filename]->Get2DRasterData(&nsize, &lyr, &data2d);
                            count = iter3->second->Adjust2DRasterWithImpactIndexes(nsize, lyr, data2d, mgtunits,
                                sel_ids, unitUpdateTimes, ludata, suitablelu);
                        }
                        else {
                            FLTPT* data = nullptr;
                            rs_map_[remote_filename]->GetRasterData(&nsize, &data);
                            count = iter3->second->Adjust1DRasterWithImpactIndexes(nsize, data, mgtunits, sel_ids,
                                unitUpdateTimes, ludata, suitablelu);
                        }
                        cout << "      A total of " << count << " has been updated for " <<
                                remote_filename << endl;
                    }
                }
                tmp_bmp_areal_struct_factory->increaseSeriesIndex();//use next location array
                iter2->second->setLastUpdateTime(t);
                hasUpdated = true;
            }
        }
    }
    return hasUpdated;
}
