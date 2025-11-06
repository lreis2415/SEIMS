#include "DataCenterMongoDB.h"
#include "text.h"
#include "Logging.h"

const int MAIN_DB_TABS_REQ_NUM = 5;
const char* MAIN_DB_TABS_REQ[] = {
    DB_TAB_FILE_OUT, DB_TAB_SITELIST,
    DB_TAB_PARAMETERS, DB_TAB_REACH, DB_TAB_SPATIAL
};

const int METEO_VARS_NUM = 6;
const char* METEO_VARS[] = {
    DataType_MeanTemperature, DataType_MaximumTemperature,
    DataType_MinimumTemperature, DataType_SolarRadiation,
    DataType_WindSpeed, DataType_RelativeAirMoisture
};

const int SOILWATER_VARS_NUM = 5;
const char* SOILWATER_VARS[] = {
    VAR_SOL_WPMM[0], VAR_SOL_AWC[0], VAR_SOL_UL[0],
    VAR_SOL_SUMAWC[0], VAR_SOL_SUMSAT[0]
};

DataCenterMongoDB::DataCenterMongoDB(InputArgs* input_args, MongoClient* client,
                                     MongoGridFs* spatial_gfs_in, MongoGridFs* spatial_gfs_out,
                                     SettingsInput* simu_in,
                                     ModuleFactory* factory, 
                                     const int subbasin_id /* = 0 */) :
    DataCenter(input_args, factory, subbasin_id), mongodb_ip_(input_args->host.c_str()),
    mongodb_port_(input_args->port),
    mongo_client_(client), main_database_(nullptr),
    spatial_gridfs_(spatial_gfs_in), spatial_gfs_out_(spatial_gfs_out) {
    //spatial_gridfs_ = new MongoGridFs(mongo_client_->GetGridFs(model_name_, DB_TAB_SPATIAL));
    //spatial_gfs_out_ = new MongoGridFs(mongo_client_->GetGridFs(model_name_, DB_TAB_OUT_SPATIAL));
    /// Check and get the main model database
    vector<string> existed_dbnames;
    mongo_client_->GetDatabaseNames(existed_dbnames);
    if (!ValueInVector(string(model_name_), existed_dbnames)) {
        LOG(ERROR) << "The main model database does not existed: " << model_name_;
        throw ModelException("DataCenterMongoDB", "Constructor",
                             "The main model database does not existed!");
    }
    main_database_ = new MongoDatabase(mongo_client_->GetDatabase(model_name_));
    /// Check the existence of FILE_IN, FILE_OUT, PARAMETERS, REACHES, SITELIST, SPATIAL, etc
    vector<string> existed_main_db_tabs;
    main_database_->GetCollectionNames(existed_main_db_tabs);
    for (int i = 0; i < MAIN_DB_TABS_REQ_NUM; ++i) {
        if (!ValueInVector(string(MAIN_DB_TABS_REQ[i]), existed_main_db_tabs)) {
            LOG(ERROR) << "Table " << MAIN_DB_TABS_REQ[i] << " must be existed in " << model_name_;
            throw ModelException("DataCenterMongoDB", "Constructor",
                                 "Table " + string(MAIN_DB_TABS_REQ[i]) + " does not existed!");
        }
    }
    if (scenario_id_ >= 0 && ValueInVector(string(DB_TAB_SCENARIO), existed_main_db_tabs)) {
        use_scenario_ = true; // we can first believe the scenario will be used.
    }
    if (nullptr != simu_in) {
        input_ = simu_in;
        model_mode_ = SimuModeToString(input_->getModelMode());
    }
    else {
        if (DataCenterMongoDB::GetFileInStringVector()) {
            input_ = SettingsInput::Init(file_in_strs_);
            if (nullptr == input_) {
                throw ModelException("DataCenterMongoDB", "Constructor",
                                     "Failed to initialize input settings of simulation!");
            }
        } else {
            throw ModelException("DataCenterMongoDB", "Constructor", "Failed to query FILE_IN!");
        }
    }
    outlet_id_ = DataCenterMongoDB::ReadIntParameterInDB(VAR_OUTLETID[0]);
    n_subbasins_ = DataCenterMongoDB::ReadIntParameterInDB(VAR_SUBBSNID_NUM[0]);
    if (outlet_id_ < 0 || n_subbasins_ < 0) {
        throw ModelException("DataCenterMongoDB", "Constructor", "Query subbasin number and outlet ID failed!");
    }
    if (!DataCenterMongoDB::GetInitialFileOutMap()) {
        throw ModelException("DataCenterMongoDB", "Constructor", "Read initial output items failed!");
    }
    if (!DataCenterMongoDB::GetSelectedFileOutVector()) { // Query FILE_OUT_SPEC first
        // then try to read from file.out directly
        selected_out_items_ = SettingsOutput::ReadFileOutFile(input_args, origin_out_items_);
    }
    // The start and end time of output items should be checked and updated here! -LJ. 09/28/2020
    UpdateOutputDate(input_->getStartTime(), input_->getEndTime());
    output_ = SettingsOutput::Init(n_subbasins_, outlet_id_, subbasin_id_, selected_out_items_,
                                   scenario_id_, calibration_id_, mpi_rank_, mpi_size_);
    if (nullptr == output_) {
        throw ModelException("DataCenterMongoDB", "Constructor",
                             "Failed to query FILE_OUT_SPEC from MongoDB or read from file.out!");
    }
    /// Check the existence of all required and optional data
    if (!DataCenterMongoDB::CheckModelPreparedData()) {
        throw ModelException("DataCenterMongoDB", "checkModelPreparedData", "Model data has not been set up!");
    }
}

DataCenterMongoDB::~DataCenterMongoDB() {
    CLOG(TRACE, LOG_RELEASE) << "Release DataCenterMongoDB...";
    /*if (spatial_gridfs_ != nullptr) {
        delete spatial_gridfs_;
        spatial_gridfs_ = nullptr;
    }
    if (spatial_gfs_out_ != nullptr) {
        delete spatial_gfs_out_;
        spatial_gfs_out_ = nullptr;
    }*/
    if (main_database_ != nullptr) {
        delete main_database_;
        main_database_ = nullptr;
    }
}

bool DataCenterMongoDB::CheckModelPreparedData() {
    /// Read climate site information from Climate database
    clim_station_ = new InputStation(mongo_client_, input_->getDtHillslope(), input_->getDtChannel());
    ReadClimateSiteList();

    /// Read initial parameters
    if (!ReadParametersInDB()) {
        return false;
    }
    if (!ReadCalibrateParametersInDB()) {
        return false;
    }
    DumpCaliParametersInDB();

    /// Read Mask raster data
    std::ostringstream oss;
    oss << subbasin_id_ << "_" << VAR_SUBBSN[0]; // Tag_Mask[0];
    string mask_filename = GetUpper(oss.str());
    mask_raster_ = IntRaster::Init(spatial_gridfs_, mask_filename.c_str());
    assert(nullptr != mask_raster_);
#ifdef HAS_VARIADIC_TEMPLATES
    rs_int_map_.emplace(mask_filename, mask_raster_);
#else
    rs_int_map_.insert(make_pair(mask_filename, mask_raster_));
#endif

    /// Constructor Subbasin data. Subbasin and slope data are required!
    oss.str("");
    oss << subbasin_id_ << "_" << VAR_SLOPE[0];
    LoadAdjustRasterData(VAR_SLOPE[0], GetUpper(oss.str()));

    subbasins_ = clsSubbasins::Init(rs_int_map_, rs_map_, subbasin_id_);
    assert(nullptr != subbasins_);

    /// Read Reaches data, all reaches will be read for both MPI and OMP version
    reaches_ = new clsReaches(mongo_client_, model_name_, DB_TAB_REACH, lyr_method_);
    reaches_->Update(init_params_, mask_raster_);
    /// Check if Scenario will be applied, Get scenario database if necessary
    if (scenario_id_ >= 0 && use_scenario_) {
        bson_t* query = bson_new();
        scenario_dbname_ = QueryDatabaseName(query, DB_TAB_SCENARIO);
        if (!scenario_dbname_.empty()) {
            scenario_ = new Scenario(mongo_client_, scenario_dbname_, subbasin_id_, scenario_id_,
                                     input_->getStartTime(), input_->getEndTime());
            if (SetRasterForScenario()) {
                scenario_->setRasterForEachBMP();
            }
        } else {
            use_scenario_ = false;
        }
    }
    return true;
}

string DataCenterMongoDB::QueryDatabaseName(bson_t* query, const char* tabname) {
    std::unique_ptr<MongoCollection>
            collection(new MongoCollection(mongo_client_->GetCollection(model_name_, tabname)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(query);
    const bson_t* doc;
    string dbname;
    while (mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, "DB")) {
            dbname = GetStringFromBsonIterator(&iter);
            break;
        }
        LOG(ERROR) << "The DB field does not exist in " << string(tabname);
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    return dbname;
}

bool DataCenterMongoDB::GetFileInStringVector() {
    if (file_in_strs_.empty()) {
        // todo: Move "SUB_MODEL" and "TASK" to text.h
        bson_t* b = BCON_NEW("SUB_MODEL", BCON_UTF8(model_cfgname_.c_str()),
                             "TASK", BCON_UTF8(task_name_.c_str()));
        std::unique_ptr<MongoCollection>
                collection(new MongoCollection(mongo_client_->GetCollection(model_name_, DB_TAB_FILE_IN)));
        mongoc_cursor_t* cursor = collection->ExecuteQuery(b);
        bson_error_t err;
        if (mongoc_cursor_error(cursor, &err)) {
            LOG(ERROR) << "Nothing found in the collection: " << DB_TAB_FILE_IN << ".";
            return false;
        }
        bson_iter_t it;
        const bson_t* bson_table;
        file_in_strs_.resize(4); // currently, only four TAG-Value are supported!
        while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bson_table)) {
            // Move "MODE", "INTERVAL", "STARTTIME", and "ENDTIME" to text.h
            if (bson_iter_init_find(&it, bson_table, "MODE")) {
                model_mode_ = GetStringFromBsonIterator(&it);
                file_in_strs_[0] = "MODE|" + model_mode_; // keep the interface consistent
            }
            if (bson_iter_init_find(&it, bson_table, "INTERVAL")) {
                file_in_strs_[1] = "INTERVAL|" + GetStringFromBsonIterator(&it);
            }
            if (bson_iter_init_find(&it, bson_table, "STARTTIME")) {
                file_in_strs_[2] = "STARTTIME|" + GetStringFromBsonIterator(&it);
            }
            if (bson_iter_init_find(&it, bson_table, "ENDTIME")) {
                file_in_strs_[3] = "ENDTIME|" + GetStringFromBsonIterator(&it);
            }
        }
        bson_destroy(b);
        mongoc_cursor_destroy(cursor);
    }
    if (!file_in_strs_.empty()) {
        for (auto it = file_in_strs_.begin(); it != file_in_strs_.end(); ++it) {
            CLOG(TRACE, LOG_INIT) << "FILE_IN Info: " << *it;
        }
    }
    return true;
}

bool DataCenterMongoDB::GetInitialFileOutMap() {
    if (!origin_out_items_.empty()) {
        return true;
    }
    // read all available output items from FILE_OUT
    bson_t* b = bson_new();
    std::unique_ptr<MongoCollection>
            collection(new MongoCollection(mongo_client_->GetCollection(model_name_, DB_TAB_FILE_OUT)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);
    bson_error_t err;
    if (mongoc_cursor_error(cursor, &err)) {
        LOG(ERROR) << "Nothing found in the collection: " << DB_TAB_FILE_OUT << " for current modeling.";
        bson_destroy(b);
        mongoc_cursor_destroy(cursor);
        return false;
    }
    bson_iter_t itertor;
    const bson_t* bson_table;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bson_table)) {
        OrgOutItem tmp_output_item;
        if (bson_iter_init_find(&itertor, bson_table, Tag_OutputUSE)) {
            GetNumericFromBsonIterator(&itertor, tmp_output_item.use);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_MODCLS)) {
            tmp_output_item.modCls = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_OutputID)) {
            tmp_output_item.outputID = GetUpper(GetStringFromBsonIterator(&itertor));
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_OutputDESC)) {
            tmp_output_item.descprition = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_FileName)) {
            tmp_output_item.outFileName = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_AggType)) {
            tmp_output_item.aggType = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_OutputUNIT)) {
            tmp_output_item.unit = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_OutputSubbsn)) {
            tmp_output_item.subBsn = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_StartTime)) {
            /// TODO: Currently we only accept "%d-%d-%d %d:%d:%d" for UTC TIME! -LJ. 09/28/2020
            tmp_output_item.sTimet = ConvertToTime(GetStringFromBsonIterator(&itertor),
                                                   "%d-%d-%d %d:%d:%d", true);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_EndTime)) {
            tmp_output_item.eTimet = ConvertToTime(GetStringFromBsonIterator(&itertor),
                                                   "%d-%d-%d %d:%d:%d", true);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_Interval)) {
            GetNumericFromBsonIterator(&itertor, tmp_output_item.interval);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_IntervalUnit)) {
            tmp_output_item.intervalUnit = GetStringFromBsonIterator(&itertor);
        }
#ifdef HAS_VARIADIC_TEMPLATES
        origin_out_items_.emplace(tmp_output_item.outputID, tmp_output_item);
#else
        origin_out_items_.insert(make_pair(tmp_output_item.outputID, tmp_output_item));
#endif
    }
    /// destroy
    bson_destroy(b);
    mongoc_cursor_destroy(cursor);
    return !origin_out_items_.empty();
}

bool DataCenterMongoDB::GetSelectedFileOutVector() {
    if (!selected_out_items_.empty()) {
        return true;
    }
    if (origin_out_items_.empty()) {
        if (!GetInitialFileOutMap()) return false;
    }
    // read selected output items from FILE_OUT_SPEC
    bson_t* b = BCON_NEW("SUB_MODEL", BCON_UTF8(model_cfgname_.c_str()),
                         "TASK", BCON_UTF8(task_name_.c_str()));
    std::unique_ptr<MongoCollection>
        collection(new MongoCollection(mongo_client_->GetCollection(model_name_, "FILE_OUT_SPEC")));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);
    bson_error_t err;
    if (mongoc_cursor_error(cursor, &err)) {
        LOG(ERROR) << "Nothing found in the collection: " << "FILE_OUT_SPEC" << " for current modeling.";
        bson_destroy(b);
        mongoc_cursor_destroy(cursor);
        return false;
    }
    bson_iter_t itertor;
    const bson_t* bson_table;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bson_table)) {
        string cur_outid = "";
        if (bson_iter_init_find(&itertor, bson_table, Tag_OutputID)) {
            cur_outid = GetUpper(GetStringFromBsonIterator(&itertor));
        }
        if (cur_outid.empty()) {
            continue;
        }
        if (origin_out_items_.find(cur_outid) == origin_out_items_.end()) {
            LOG(WARNING) << "The specified output is not supported: " << cur_outid;
            continue;
        }
        OrgOutItem& tmp_item = origin_out_items_.at(cur_outid);
        tmp_item.outputID = cur_outid;
        tmp_item.use = 1;
        if (bson_iter_init_find(&itertor, bson_table, Tag_FileName)) {
            tmp_item.outFileName = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_AggType)) {
            tmp_item.aggType = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_OutputUNIT)) {
            tmp_item.unit = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_OutputSubbsn)) {
            tmp_item.subBsn = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_StartTime)) {
            /// TODO: Currently we only accept "%d-%d-%d %d:%d:%d" for UTC TIME! -LJ. 09/28/2020
            tmp_item.sTimet = ConvertToTime(GetStringFromBsonIterator(&itertor),
                "%d-%d-%d %d:%d:%d", true);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_EndTime)) {
            tmp_item.eTimet = ConvertToTime(GetStringFromBsonIterator(&itertor),
                "%d-%d-%d %d:%d:%d", true);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_Interval)) {
            GetNumericFromBsonIterator(&itertor, tmp_item.interval);
        }
        if (bson_iter_init_find(&itertor, bson_table, Tag_IntervalUnit)) {
            tmp_item.intervalUnit = GetStringFromBsonIterator(&itertor);
        }
        selected_out_items_.emplace_back(tmp_item);
    }
    vector<OrgOutItem>(selected_out_items_).swap(selected_out_items_);
    // selected_out_items_.shrink_to_fit();
    return !selected_out_items_.empty();
}

int DataCenterMongoDB::ReadIntParameterInDB(const char* param_name) {
    bson_t* filter = BCON_NEW(PARAM_FLD_NAME, BCON_UTF8(param_name));
    CLOG(TRACE, LOG_INIT) << "Query for " << param_name << ": " << bson_as_json(filter, NULL);
    std::unique_ptr<MongoCollection>
            collection(new MongoCollection(mongo_client_->GetCollection(model_name_, DB_TAB_PARAMETERS)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(filter);
    bson_error_t err;
    if (mongoc_cursor_error(cursor, &err)) {
        LOG(ERROR) << "ReadIntParameterInDB: " << "Nothing found for " << param_name;
        /// destroy
        bson_destroy(filter);
        mongoc_cursor_destroy(cursor);
        return -9999;
    }
    bson_iter_t iter;
    const bson_t* bson_table;
    int num_tmp = -1;
    while (mongoc_cursor_next(cursor, &bson_table)) {
        if (bson_iter_init_find(&iter, bson_table, PARAM_FLD_VALUE)) {
            GetNumericFromBsonIterator(&iter, num_tmp);
        }
    }
    bson_destroy(filter);
    mongoc_cursor_destroy(cursor);
    return num_tmp;
}

void DataCenterMongoDB::ReadClimateSiteList() {
//    bson_t* query = bson_new();
//    BSON_APPEND_INT32(query, Tag_SubbasinId, subbasin_id_); // subbasin id
//    BSON_APPEND_UTF8(query, Tag_Mode, input_->getModelMode().c_str()); // mode

    bson_t* query = BCON_NEW(Tag_SubbasinId, BCON_INT32(subbasin_id_),
                             Tag_Mode, BCON_UTF8(SimuModeToString(input_->getModelMode())));
    CLOG(TRACE, LOG_INIT) << "ReadClimateSiteList: " << bson_as_json(query, NULL);
    std::unique_ptr<MongoCollection> collection(new MongoCollection(mongo_client_->GetCollection(model_name_,
                                                                        DB_TAB_SITELIST)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(query);

    const bson_t* doc;
    while (mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;
        string clim_type = "";
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, MONG_SITELIST_DB)) {
            clim_dbname_ = GetStringFromBsonIterator(&iter);
        } else {
            throw ModelException("DataCenterMongoDB", "ReadClimateSiteList",
                                 "The DB field does not exist in SiteList table.");
        }
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, MONG_HYDRO_SITE_TYPE)) {
            clim_type = GetStringFromBsonIterator(&iter);
        }
        string site_list;
        if (StringMatch(clim_type, SITELIST_TABLE_M) && \
            (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE))) {
            site_list = GetStringFromBsonIterator(&iter);
            for (int i = 0; i < METEO_VARS_NUM; ++i) {
                clim_station_->ReadSitesData(clim_dbname_, site_list, METEO_VARS[i],
                                             input_->getStartTime(), input_->getEndTime(), input_->isStormMode());
            }
        }
        if (StringMatch(clim_type, SITELIST_TABLE_P) && \
            (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE))) {
            site_list = GetStringFromBsonIterator(&iter);
            clim_station_->ReadSitesData(clim_dbname_, site_list, DataType_Precipitation,
                                         input_->getStartTime(), input_->getEndTime(), input_->isStormMode());
        }
        if (StringMatch(clim_type, SITELIST_TABLE_PET) && \
            (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE))) {
            site_list = GetStringFromBsonIterator(&iter);
            clim_station_->ReadSitesData(clim_dbname_, site_list, DataType_PotentialEvapotranspiration,
                                         input_->getStartTime(), input_->getEndTime(), input_->isStormMode());
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
}

bool DataCenterMongoDB::ReadParametersInDB() {
    if (!init_params_.empty()) {
        return true;
    }
    // query all available parameters from PARAMETERS
    bson_t* filter = bson_new();
    std::unique_ptr<MongoCollection>
            collection(new MongoCollection(mongo_client_->GetCollection(model_name_, DB_TAB_PARAMETERS)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(filter);
    bson_error_t err;
    const bson_t* info;
    if (mongoc_cursor_error(cursor, &err)) {
        LOG(ERROR) << "Nothing found in the collection: " << DB_TAB_PARAMETERS << ".";
        /// destroy
        bson_destroy(filter);
        mongoc_cursor_destroy(cursor);
        return false;
    }
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &info)) {
        //ParamInfo<FLTPT>* p = new ParamInfo<FLTPT>();
        bson_iter_t iter;
        string name = "";
        string desc = "";
        string unit = "";
        string module = "";
        FLTPT value = MISSINGFLOAT;
        string change = "";
        FLTPT impact = MISSINGFLOAT;
        FLTPT maximum = MISSINGFLOAT;
        FLTPT minimum = MISSINGFLOAT;
        bool isint = false;
        if (bson_iter_init_find(&iter, info, PARAM_FLD_NAME)) {
            name = GetUpper(GetStringFromBsonIterator(&iter));
        }
        if (bson_iter_init_find(&iter, info, PARAM_FLD_DESC)) {
            desc = GetStringFromBsonIterator(&iter);
        }
        if (bson_iter_init_find(&iter, info, PARAM_FLD_UNIT)) {
            unit = GetStringFromBsonIterator(&iter);
        }
        if (bson_iter_init_find(&iter, info, PARAM_FLD_MIDS)) {
            module = GetStringFromBsonIterator(&iter);
        }
        if (bson_iter_init_find(&iter, info, PARAM_FLD_VALUE)) {
            GetNumericFromBsonIterator(&iter, value);
        }
        if (bson_iter_init_find(&iter, info, PARAM_FLD_IMPACT)) {
            GetNumericFromBsonIterator(&iter, impact);
        }
        if (bson_iter_init_find(&iter, info, PARAM_FLD_CHANGE)) {
            change = GetStringFromBsonIterator(&iter);
        }
        if (bson_iter_init_find(&iter, info, PARAM_FLD_MAX)) {
            GetNumericFromBsonIterator(&iter, maximum);
        }
        if (bson_iter_init_find(&iter, info, PARAM_FLD_MIN)) {
            GetNumericFromBsonIterator(&iter, minimum);
        }
        if (bson_iter_init_find(&iter, info, PARAM_FLD_DTYPE)) {
            isint = StringMatch(GetStringFromBsonIterator(&iter), "INT") ||
                StringMatch(GetStringFromBsonIterator(&iter), "INT32");
        }
        // Check if the item is illegal, name and change MUST not be empty,
        //   value and impact MUST not be MISSINGFLOAT
        if (name.empty() || change.empty() || FloatEqual(impact, MISSINGFLOAT) ||
            FloatEqual(value, MISSINGFLOAT)) {
            continue;
        }
        if (isint) {
            ParamInfo<int>* intp = new ParamInfo<int>(name, desc, unit, module, CVT_INT(value),
                                                      change, CVT_INT(impact), CVT_INT(maximum),
                                                      CVT_INT(minimum), isint);
#ifdef HAS_VARIADIC_TEMPLATES
            if (!init_params_int_.emplace(name, intp).second) {
#else
            if (!init_params_int_.insert(make_pair(name, intp)).second) {
#endif
                LOG(WARNING) << "Load parameter: " << name << " failed! We will ignore it!";
                continue;
            }
        }
        else {
            ParamInfo<FLTPT>* p = new ParamInfo<FLTPT>(name, desc, unit, module, value,
                                                       change, impact, maximum, minimum, isint);
#ifdef HAS_VARIADIC_TEMPLATES
            if (!init_params_.emplace(name, p).second) {
#else
            if (!init_params_.insert(make_pair(name, p)).second) {
#endif
                LOG(WARNING) << "Load parameter: " << name << " failed! We will ignore it!";
                continue;
            }
            /// Special handling code for soil water capcity parameters
            /// e.g., SOL_AWC, SOL_UL, WILTINGPOINT. By ljzhu, 2018-1-11
            if (StringMatch(name, VAR_SW_CAP[0])) {
                for (int si = 0; si < SOILWATER_VARS_NUM; si++) {
                    ParamInfo<FLTPT>* tmpp = new ParamInfo<FLTPT>(*p);
                    tmpp->Name = SOILWATER_VARS[si];
#ifdef HAS_VARIADIC_TEMPLATES
                    init_params_.emplace(GetUpper(tmpp->Name), tmpp);
#else
                    init_params_.insert(make_pair(GetUpper(tmpp->Name), tmpp));
#endif
                }
            }
        }
    }
    bson_destroy(filter);
    mongoc_cursor_destroy(cursor);
    return true;
}

bool DataCenterMongoDB::ReadCalibrateParametersInDB() {
    if (init_params_.empty() && init_params_int_.empty()) {
        if (!ReadParametersInDB()) return false;
    }
    // read calibrated parameters from PARAMETERS_SPEC
    bson_t* b = BCON_NEW("SUB_MODEL", BCON_UTF8(model_cfgname_.c_str()),
                         "TASK", BCON_UTF8(task_name_.c_str()));
    std::unique_ptr<MongoCollection>
        collection(new MongoCollection(mongo_client_->GetCollection(model_name_, "PARAMETERS_SPEC")));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);
    bson_error_t err;
    if (mongoc_cursor_error(cursor, &err)) {
        LOG(INFO) << "No calibration parameters found in the collection: " << "PARAMETERS_SPEC";
        bson_destroy(b);
        mongoc_cursor_destroy(cursor);
        return true;
    }
    bson_iter_t itertor;
    const bson_t* info;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &info)) {
        string cur_paramname = "";
        if (bson_iter_init_find(&itertor, info, PARAM_FLD_NAME)) {
            cur_paramname = GetUpper(GetStringFromBsonIterator(&itertor));
        }
        if (cur_paramname.empty()) {
            continue;
        }
        // find cur_paramname in init_params_ and init_params_int_
        bool isint = false;
        if (init_params_int_.find(cur_paramname) != init_params_int_.end()) {
            isint = true;
        } else if (init_params_.find(cur_paramname) == init_params_.end()) {
            LOG(WARNING) << "The specified parameter " << cur_paramname << " is not supported!";
            continue;
        }
        bson_iter_t iter;
        FLTPT cur_impact = 0.;
        bool use_cali_values = false;
        // if item has CALI_VALUES, then use it
        if (bson_iter_init_find(&iter, info, PARAM_CALI_VALUES) && calibration_id_ >= 0) {
            // Overwrite impact according to calibration ID
            string cali_values_str = GetStringFromBsonIterator(&iter);
            vector<FLTPT> cali_values;
            SplitStringForValues(cali_values_str, ',', cali_values);
            if (calibration_id_ < CVT_INT(cali_values.size())) {
                cur_impact = cali_values[calibration_id_];
                use_cali_values = true;
            }
        }
        if (isint) {
            ParamInfo<int>* intp = init_params_int_.at(cur_paramname);
            if (bson_iter_init_find(&iter, info, PARAM_FLD_VALUE)) {
                GetNumericFromBsonIterator(&iter, intp->Value);
            }
            if (bson_iter_init_find(&iter, info, PARAM_FLD_CHANGE)) {
                intp->Change = GetStringFromBsonIterator(&iter);
            }
            if (bson_iter_init_find(&iter, info, PARAM_FLD_IMPACT)) {
                GetNumericFromBsonIterator(&iter, intp->Impact);
            }
            if (bson_iter_init_find(&iter, info, PARAM_FLD_MAX)) {
                GetNumericFromBsonIterator(&iter, intp->Maximum);
            }
            if (bson_iter_init_find(&iter, info, PARAM_FLD_MIN)) {
                GetNumericFromBsonIterator(&iter, intp->Minimum);
            }
            if (use_cali_values) {
                intp->Impact = CVT_INT(cur_impact);
            }
        } else {
            ParamInfo<FLTPT>* p = init_params_.at(cur_paramname);
            if (bson_iter_init_find(&iter, info, PARAM_FLD_VALUE)) {
                GetNumericFromBsonIterator(&iter, p->Value);
            }
            if (bson_iter_init_find(&iter, info, PARAM_FLD_CHANGE)) {
                p->Change = GetStringFromBsonIterator(&iter);
            }
            if (bson_iter_init_find(&iter, info, PARAM_FLD_IMPACT)) {
                GetNumericFromBsonIterator(&iter, p->Impact);
            }
            if (bson_iter_init_find(&iter, info, PARAM_FLD_MAX)) {
                GetNumericFromBsonIterator(&iter, p->Maximum);
            }
            if (bson_iter_init_find(&iter, info, PARAM_FLD_MIN)) {
                GetNumericFromBsonIterator(&iter, p->Minimum);
            }
            if (use_cali_values) {
                p->Impact = cur_impact;
            }
        }
    }
    return true;
}

bool DataCenterMongoDB::ReadRasterData(const string& remote_filename, FloatRaster*& flt_rst) {
    STRING_MAP opts;
    UpdateStringMap(opts, HEADER_INC_NODATA, "FALSE");
    FloatRaster* raster_data = FloatRaster::Init(spatial_gridfs_, remote_filename.c_str(),
                                                 true, mask_raster_, true,
                                                 NODATA_VALUE, opts);
    if (nullptr == raster_data) { return false; }
    // When load from MongoDB failed (i.e., file not existed), the Initialized() will return false!
    if (!raster_data->Initialized()) {
        delete raster_data;
        return false;
    }
    /// using emplace() if possible or insert() to make sure the successful insertion.
#ifdef HAS_VARIADIC_TEMPLATES
    if (!rs_map_.emplace(remote_filename, raster_data).second) {
#else
    if (!rs_map_.insert(make_pair(remote_filename, raster_data)).second) {
#endif
        delete raster_data;
        return false;
    }
    flt_rst = raster_data;
    return true;
}

bool DataCenterMongoDB::ReadRasterData(const string& remote_filename, IntRaster*& int_rst) {
    STRING_MAP opts;
    UpdateStringMap(opts, HEADER_INC_NODATA, "FALSE");
    IntRaster* raster_data = IntRaster::Init(spatial_gridfs_, remote_filename.c_str(),
                                             true, mask_raster_, true,
                                             NODATA_VALUE, opts);
    if (nullptr == raster_data) { return false; }
    // When load from MongoDB failed (i.e., file not existed), the Initialized() will return false!
    if (!raster_data->Initialized()) {
        delete raster_data;
        return false;
    }
    /// using emplace() if possible or insert() to make sure the successful insertion.
#ifdef HAS_VARIADIC_TEMPLATES
    if (!rs_int_map_.emplace(remote_filename, raster_data).second) {
#else
    if (!rs_int_map_.insert(make_pair(remote_filename, raster_data)).second) {
#endif
        delete raster_data;
        return false;
    }
    int_rst = raster_data;
    return true;
}

void DataCenterMongoDB::ReadItpWeightData(const string& remote_filename, int& num, int& stations, FLTPT**& data) {
    ItpWeightData* weight_data = new ItpWeightData(spatial_gridfs_, remote_filename);
    if (!weight_data->Initialized()) {
        delete weight_data;
        data = nullptr;
        return;
    }
    FLTPT** tmpdata = nullptr;
    weight_data->GetWeightData2D(&num, &stations, &tmpdata);
    Initialize2DArray(num, stations, data, tmpdata);
    delete weight_data;
}

void DataCenterMongoDB::Read1DArrayData(const string& remote_filename, int& num, FLTPT*& data) {
    char* databuf = nullptr;
    vint datalength;
    STRING_MAP curopts;
    if (curopts.find(HEADER_RSOUT_DATATYPE) == curopts.end()) {
        UpdateStringMap(curopts, HEADER_RSOUT_DATATYPE, FLTPT_NAME);
    }
    spatial_gridfs_->GetStreamData(remote_filename, databuf, datalength, nullptr, &curopts);
    if (nullptr == databuf) {
        data = nullptr;
        return;
    }

    if (StringMatch(FLTPT_NAME, "FLOAT")) {
        num = CVT_INT(datalength / sizeof(float));
    }
    else if (StringMatch(FLTPT_NAME, "DOUBLE")) {
        num = CVT_INT(datalength / sizeof(double));
    } else {
        databuf = nullptr;
        return;
    }

    FLTPT *tmpdata = reinterpret_cast<FLTPT*>(databuf); // deprecate C-style: (double *) databuf;
    Initialize1DArray(num, data, tmpdata);
    delete[] tmpdata;
    databuf = nullptr;
}

void DataCenterMongoDB::Read1DArrayData(const string& remote_filename, int& num, int*& data) {
    char* databuf = nullptr;
    vint datalength;
    STRING_MAP curopts;
    if (curopts.find(HEADER_RSOUT_DATATYPE) == curopts.end()) {
        UpdateStringMap(curopts, HEADER_RSOUT_DATATYPE, "INT32");
    }
    spatial_gridfs_->GetStreamData(remote_filename, databuf, datalength, nullptr, &curopts);
    if (nullptr == databuf) return;

    num = CVT_INT(datalength / sizeof(int));
    int* tmpdata = reinterpret_cast<int*>(databuf); // deprecate C-style: (int *) databuf;
    Initialize1DArray(num, data, tmpdata);
    delete[] tmpdata;
    databuf = nullptr;
}

void DataCenterMongoDB::Read2DArrayData(const string& remote_filename, int& rows, int& cols, FLTPT**& data) {
    char* databuf = nullptr;
    vint datalength;
    STRING_MAP curopts;
    if (curopts.find(HEADER_RSOUT_DATATYPE) == curopts.end()) {
        UpdateStringMap(curopts, HEADER_RSOUT_DATATYPE, FLTPT_NAME);
    }
    spatial_gridfs_->GetStreamData(remote_filename, databuf, datalength, nullptr, &curopts);
    if (nullptr == databuf) {
        data = nullptr;
        return;
    }
    FLTPT* float_values = reinterpret_cast<FLTPT*>(databuf); // deprecate C-style: (double *) databuf;
    if (!Initialize2DArray(float_values, rows, cols, data)) {
        data = nullptr;
    }
    Release1DArray(float_values);
    databuf = nullptr;
}

void DataCenterMongoDB::Read2DArrayData(const string& remote_filename, int& rows, int& cols, int**& data) {
    char* databuf = nullptr;
    vint datalength;
    STRING_MAP curopts;
    if (curopts.find(HEADER_RSOUT_DATATYPE) == curopts.end()) {
        UpdateStringMap(curopts, HEADER_RSOUT_DATATYPE, "INT32");
    }
    spatial_gridfs_->GetStreamData(remote_filename, databuf, datalength, nullptr, &curopts);
    if (nullptr == databuf) {
        data = nullptr;
        return;
    }
    int* org_values = reinterpret_cast<int*>(databuf); // deprecate C-style: (float *) databuf;
    if (!Initialize2DArray(org_values, rows, cols, data)) {
        data = nullptr;
    }
    Release1DArray(org_values);
    databuf = nullptr;
}

void DataCenterMongoDB::ReadIuhData(const string& remote_filename, int& n, FLTPT**& data) {
    char* databuf = nullptr;
    vint datalength;
    STRING_MAP curopts;
    if (curopts.find(HEADER_RSOUT_DATATYPE) == curopts.end()) {
        UpdateStringMap(curopts, HEADER_RSOUT_DATATYPE, FLTPT_NAME);
    }
    spatial_gridfs_->GetStreamData(remote_filename, databuf, datalength, nullptr, &curopts);
    if (nullptr == databuf) {
        data = nullptr;
        return;
    }
    FLTPT* float_values = reinterpret_cast<FLTPT*>(databuf); // deprecate C-style: (double *) databuf;
    // Previous code, which will cause memory leak.
    // n = CVT_INT(float_values[0]);
    // data = new FLTPT*[n];
    // int index = 1;
    // for (int i = 0; i < n; i++) {
    //     int n_sub = CVT_INT(float_values[index + 1] - float_values[index] + 3);
    //     data[i] = new FLTPT[n_sub];
    //
    //     data[i][0] = float_values[index];
    //     data[i][1] = float_values[index + 1];
    //     for (int j = 2; j < n_sub; j++) {
    //         data[i][j] = float_values[index + j];
    //     }
    //     index = index + n_sub;
    // }

    // Customize code according to Initialize2DArray. LJ, 2022-08-23
    int idx = 0;
    n = CVT_INT(float_values[idx++]);
    data = new(nothrow) FLTPT * [n];
    FLTPT* pool = nullptr;
    // Get actual data length of init_data, excluding the first element which is 'rows'
    int* cols = new int[n];
    int max_cols = -1;
    for (int i = 0; i < n; i++) {
        cols[i] = CVT_INT(float_values[idx + 1] - float_values[idx] + 3);
        idx += cols[i];
        if (cols[i] > max_cols) { max_cols = cols[i]; }
    }
    int length = idx - 1;
    // New a 1d array to store data
    Initialize1DArray(length, pool, float_values + 1);
    // Now point the row pointers to the appropriate positions in the data pool
    int pos = 0;
    for (int i = 0; i < n; ++i) {
        data[i] = pool + pos;
        pos += cols[i];
    }
    delete[] cols;

    Release1DArray(float_values);
    databuf = nullptr;
}

bool DataCenterMongoDB::SetRasterForScenario() {
    if (!use_scenario_) { return false; }
    if (nullptr == scenario_) { return false; }
    map<string, IntRaster*>& scene_rs_map = scenario_->getSceneRasterDataMap();
    if (scene_rs_map.empty()) { return false; }
    for (auto it = scene_rs_map.begin(); it != scene_rs_map.end(); ++it) {
        if (rs_int_map_.find(it->first) == rs_int_map_.end()) {
            if (!ReadRasterData(it->first, it->second)) { return false; }
        } else {
            it->second = rs_int_map_.at(it->first);
        }
    }
    return true;
}
