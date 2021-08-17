#include "DataCenterMongoDB.h"
#include "text.h"
#include "Logging.h"

const int MAIN_DB_TABS_REQ_NUM = 6;
const char* MAIN_DB_TABS_REQ[] = {
    DB_TAB_FILE_IN, DB_TAB_FILE_OUT, DB_TAB_SITELIST,
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
                                     ModuleFactory* factory,
                                     const int subbasin_id /* = 0 */) :
    DataCenter(input_args, factory, subbasin_id), mongodb_ip_(input_args->host.c_str()),
    mongodb_port_(input_args->port),
    clim_dbname_(""), scenario_dbname_(""),
    mongo_client_(client), main_database_(nullptr),
    spatial_gridfs_(spatial_gfs_in), spatial_gfs_out_(spatial_gfs_out) {
    //spatial_gridfs_ = new MongoGridFs(mongo_client_->GetGridFs(model_name_, DB_TAB_SPATIAL));
    //spatial_gfs_out_ = new MongoGridFs(mongo_client_->GetGridFs(model_name_, DB_TAB_OUT_SPATIAL));
    if (DataCenterMongoDB::GetFileInStringVector()) {
        input_ = SettingsInput::Init(file_in_strs_);
        if (nullptr == input_) {
            throw ModelException("DataCenterMongoDB", "Constructor", "Failed to initialize m_input!");
        }
    } else {
        throw ModelException("DataCenterMongoDB", "Constructor", "Failed to query FILE_IN!");
    }
    if (!DataCenterMongoDB::GetSubbasinNumberAndOutletID()) {
        throw ModelException("DataCenterMongoDB", "Constructor", "Query subbasin number and outlet ID failed!");
    }
    if (DataCenterMongoDB::GetFileOutVector()) {
        // The start and end time of output items should be checked and updated here! -LJ. 09/28/2020
        UpdateOutputDate(input_->getStartTime(), input_->getEndTime());
        output_ = SettingsOutput::Init(n_subbasins_, outlet_id_, subbasin_id_, origin_out_items_,
                                       scenario_id_, calibration_id_, mpi_rank_, mpi_size_);
        if (nullptr == output_) {
            throw ModelException("DataCenterMongoDB", "Constructor", "Failed to initialize m_output!");
        }
    } else {
        throw ModelException("DataCenterMongoDB", "Constructor", "Failed to query FILE_OUT!");
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
    /// 1. Check and get the main model database
    vector<string> existed_dbnames;
    mongo_client_->GetDatabaseNames(existed_dbnames);
    if (!ValueInVector(string(model_name_), existed_dbnames)) {
        LOG(ERROR) << "The main model is not existed: " << model_name_;
        return false;
    }
    main_database_ = new MongoDatabase(mongo_client_->GetDatabase(model_name_));
    /// 2. Check the existence of FILE_IN, FILE_OUT, PARAMETERS, REACHES, SITELIST, SPATIAL, etc
    vector<string> existed_main_db_tabs;
    main_database_->GetCollectionNames(existed_main_db_tabs);
    for (int i = 0; i < MAIN_DB_TABS_REQ_NUM; ++i) {
        if (!ValueInVector(string(MAIN_DB_TABS_REQ[i]), existed_main_db_tabs)) {
            LOG(ERROR) << "Table " << MAIN_DB_TABS_REQ[i] << " must be existed in " << model_name_;
            return false;
        }
    }
    /// 3. Read climate site information from Climate database
    clim_station_ = new InputStation(mongo_client_, input_->getDtHillslope(), input_->getDtChannel());
    ReadClimateSiteList();

    /// 4. Read initial parameters
    if (!ReadParametersInDB()) {
        return false;
    }
    DumpCaliParametersInDB();

    /// 5. Read Mask raster data
    std::ostringstream oss;
    oss << subbasin_id_ << "_" << Tag_Mask[0];
    string mask_filename = GetUpper(oss.str());
    mask_raster_ = FloatRaster::Init(spatial_gridfs_, mask_filename.c_str());
    assert(nullptr != mask_raster_);
#ifdef HAS_VARIADIC_TEMPLATES
    rs_map_.emplace(mask_filename, mask_raster_);
#else
    rs_map_.insert(make_pair(mask_filename, mask_raster_));
#endif

    /// 6. Constructor Subbasin data. Subbasin and slope data are required!
    oss.str("");
    oss << subbasin_id_ << "_" << VAR_SUBBSN[0];
    LoadAdjustRasterData(VAR_SUBBSN[0], GetUpper(oss.str()));

    oss.str("");
    oss << subbasin_id_ << "_" << VAR_SLOPE[0];
    LoadAdjustRasterData(VAR_SLOPE[0], GetUpper(oss.str()));

    subbasins_ = clsSubbasins::Init(rs_map_, subbasin_id_);
    assert(nullptr != subbasins_);

    /// 7. Read Reaches data, all reaches will be read for both MPI and OMP version
    reaches_ = new clsReaches(mongo_client_, model_name_, DB_TAB_REACH, lyr_method_);
    reaches_->Update(init_params_);
    /// 8. Check if Scenario will be applied, Get scenario database if necessary
    if (ValueInVector(string(DB_TAB_SCENARIO), existed_main_db_tabs) && scenario_id_ >= 0) {
        bson_t* query = bson_new();
        scenario_dbname_ = QueryDatabaseName(query, DB_TAB_SCENARIO);
        if (!scenario_dbname_.empty()) {
            use_scenario_ = true;
            //!
            //! use start & end time to initialize scenario
            scenario_ = new Scenario(mongo_client_, scenario_dbname_, subbasin_id_, scenario_id_, input_->getStartTime(), input_->getEndTime());
            if (SetRasterForScenario()) {
                scenario_->setRasterForEachBMP();
            }
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
        bson_t* b = bson_new();
        std::unique_ptr<MongoCollection>
                collection(new MongoCollection(mongo_client_->GetCollection(model_name_, DB_TAB_FILE_IN)));
        mongoc_cursor_t* cursor = collection->ExecuteQuery(b);
        bson_error_t err;
        if (mongoc_cursor_error(cursor, &err)) {
            LOG(ERROR) << "Nothing found in the collection: " << DB_TAB_FILE_IN << ".";
            return false;
        }
        bson_iter_t itertor;
        const bson_t* bson_table;
        while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bson_table)) {
            vector<string> tokens(2);
            if (bson_iter_init_find(&itertor, bson_table, Tag_ConfTag)) {
                tokens[0] = GetStringFromBsonIterator(&itertor);
            }
            if (bson_iter_init_find(&itertor, bson_table, Tag_ConfValue)) {
                tokens[1] = GetStringFromBsonIterator(&itertor);
            }
            if (StringMatch(tokens[0], Tag_Mode)) {
                model_mode_ = tokens[1];
            }
            size_t sz = file_in_strs_.size();                // get the current number of rows
            file_in_strs_.resize(sz + 1);                    // resize with one more row
            file_in_strs_[sz] = tokens[0] + "|" + tokens[1]; // keep the interface consistent
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

bool DataCenterMongoDB::GetFileOutVector() {
    if (!origin_out_items_.empty()) {
        return true;
    }
    bson_t* b = bson_new();
    std::unique_ptr<MongoCollection>
            collection(new MongoCollection(mongo_client_->GetCollection(model_name_, DB_TAB_FILE_OUT)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);
    bson_error_t err;
    if (mongoc_cursor_error(cursor, &err)) {
        LOG(ERROR) << "Nothing found in the collection: " << DB_TAB_FILE_OUT << ".";
        /// destroy
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
            tmp_output_item.outputID = GetStringFromBsonIterator(&itertor);
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
        if (tmp_output_item.use > 0) {
            origin_out_items_.emplace_back(tmp_output_item);
        }
    }
    vector<OrgOutItem>(origin_out_items_).swap(origin_out_items_);
    // m_OriginOutItems.shrink_to_fit();
    /// destroy
    bson_destroy(b);
    mongoc_cursor_destroy(cursor);
    return !origin_out_items_.empty();
}

bool DataCenterMongoDB::GetSubbasinNumberAndOutletID() {
    bson_t* b = BCON_NEW("$query", "{", PARAM_FLD_NAME, "{", "$in", "[", BCON_UTF8(VAR_OUTLETID[0]),
                         BCON_UTF8(VAR_SUBBSNID_NUM[0]),
                         "]", "}", "}");
    // printf("%s\n",bson_as_json(b, NULL));

    std::unique_ptr<MongoCollection>
            collection(new MongoCollection(mongo_client_->GetCollection(model_name_, DB_TAB_PARAMETERS)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);
    bson_error_t err;
    if (mongoc_cursor_error(cursor, &err)) {
        LOG(ERROR) << "Nothing found for subbasin number and outlet ID.";
        /// destroy
        bson_destroy(b);
        mongoc_cursor_destroy(cursor);
        return false;
    }

    bson_iter_t iter;
    const bson_t* bson_table;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bson_table)) {
        string name_tmp;
        int num_tmp = -1;
        if (bson_iter_init_find(&iter, bson_table, PARAM_FLD_NAME)) {
            name_tmp = GetStringFromBsonIterator(&iter);
        }
        if (bson_iter_init_find(&iter, bson_table, PARAM_FLD_VALUE)) {
            GetNumericFromBsonIterator(&iter, num_tmp);
        }
        if (!StringMatch(name_tmp, "") && num_tmp != -1) {
            if (StringMatch(name_tmp, VAR_OUTLETID[0])) {
                GetNumericFromBsonIterator(&iter, outlet_id_);
            } else if (StringMatch(name_tmp, VAR_SUBBSNID_NUM[0])) {
                GetNumericFromBsonIterator(&iter, n_subbasins_);
            }
        } else {
            LOG(ERROR) << "Nothing found for subbasin number and outlet ID.";
        }
    }
    bson_destroy(b);
    mongoc_cursor_destroy(cursor);
    return outlet_id_ >= 0 && n_subbasins_ >= 0;
}

void DataCenterMongoDB::ReadClimateSiteList() {
    bson_t* query = bson_new();
    // subbasin id
    BSON_APPEND_INT32(query, Tag_SubbasinId, subbasin_id_);
    // mode
    //string modelMode = m_input->getModelMode();
    BSON_APPEND_UTF8(query, Tag_Mode, input_->getModelMode().c_str());

    std::unique_ptr<MongoCollection>
            collection(new MongoCollection(mongo_client_->GetCollection(model_name_, DB_TAB_SITELIST)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(query);

    const bson_t* doc;
    while (mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, MONG_SITELIST_DB)) {
            clim_dbname_ = GetStringFromBsonIterator(&iter);
        } else {
            throw ModelException("DataCenterMongoDB", "Constructor", "The DB field does not exist in SiteList table.");
        }
        string site_list;
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE_M)) {
            site_list = GetStringFromBsonIterator(&iter);
            for (int i = 0; i < METEO_VARS_NUM; ++i) {
                clim_station_->ReadSitesData(clim_dbname_, site_list, METEO_VARS[i],
                                             input_->getStartTime(), input_->getEndTime(), input_->isStormMode());
            }
        }

        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE_P)) {
            site_list = GetStringFromBsonIterator(&iter);
            clim_station_->ReadSitesData(clim_dbname_, site_list, DataType_Precipitation,
                                         input_->getStartTime(), input_->getEndTime(), input_->isStormMode());
        }

        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE_PET)) {
            site_list = GetStringFromBsonIterator(&iter);
            clim_station_->ReadSitesData(clim_dbname_, site_list, DataType_PotentialEvapotranspiration,
                                         input_->getStartTime(), input_->getEndTime(), input_->isStormMode());
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
}

bool DataCenterMongoDB::ReadParametersInDB() {
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
        ParamInfo* p = new ParamInfo();
        bson_iter_t iter;
        if (bson_iter_init_find(&iter, info, PARAM_FLD_NAME)) {
            p->Name = GetStringFromBsonIterator(&iter);
        }
        if (bson_iter_init_find(&iter, info, PARAM_FLD_UNIT)) {
            p->Units = GetStringFromBsonIterator(&iter);
        }
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
            GetNumericFromBsonIterator(&iter, p->Minimun);
        }
        if (bson_iter_init_find(&iter, info, PARAM_CALI_VALUES) && calibration_id_ >= 0) {
            // Overwrite p->Impact according to calibration ID
            string cali_values_str = GetStringFromBsonIterator(&iter);
            vector<float> cali_values;
            SplitStringForValues(cali_values_str, ',', cali_values);
            if (calibration_id_ < CVT_INT(cali_values.size())) {
                p->Impact = cali_values[calibration_id_];
            }
        }
#ifdef HAS_VARIADIC_TEMPLATES
        if (!init_params_.emplace(GetUpper(p->Name), p).second) {
#else
        if (!init_params_.insert(make_pair(GetUpper(p->Name), p)).second) {
#endif
            LOG(ERROR) << "Load parameter: " << GetUpper(p->Name) << " failed!";
            return false;
        }
        /// Special handling code for soil water capcity parameters
        /// e.g., SOL_AWC, SOL_UL, WILTINGPOINT. By ljzhu, 2018-1-11
        if (StringMatch(p->Name, VAR_SW_CAP[0])) {
            for (int si = 0; si < SOILWATER_VARS_NUM; si++) {
                ParamInfo* tmpp = new ParamInfo(*p);
                tmpp->Name = SOILWATER_VARS[si];
#ifdef HAS_VARIADIC_TEMPLATES
                init_params_.emplace(GetUpper(tmpp->Name), tmpp);
#else
                init_params_.insert(make_pair(GetUpper(tmpp->Name), tmpp));
#endif
            }
        }
    }
    bson_destroy(filter);
    mongoc_cursor_destroy(cursor);
    return true;
}

FloatRaster* DataCenterMongoDB::ReadRasterData(const string& remote_filename) {
    FloatRaster* raster_data = FloatRaster::Init(spatial_gridfs_, remote_filename.c_str(),
                                                 true, mask_raster_, true);
    // When load from MongoDB failed (i.e., file not existed), the Initialized() will return false!
    if (!raster_data->Initialized()) {
        delete raster_data;
        return nullptr;
    }
    assert(nullptr != raster_data);
    /// using emplace() if possible or insert() to make sure the successful insertion.
#ifdef HAS_VARIADIC_TEMPLATES
    if (!rs_map_.emplace(remote_filename, raster_data).second) {
#else
    if (!rs_map_.insert(make_pair(remote_filename, raster_data)).second) {
#endif
        delete raster_data;
        return nullptr;
    }
    return raster_data;
}

void DataCenterMongoDB::ReadItpWeightData(const string& remote_filename, int& num, float*& data) {
    ItpWeightData* weight_data = new ItpWeightData(spatial_gridfs_, remote_filename);
    if (!weight_data->Initialized()) {
        delete weight_data;
        data = nullptr;
        return;
    }
    float* tmpdata = nullptr;
    weight_data->GetWeightData(&num, &tmpdata);
    Initialize1DArray(num, data, tmpdata);
    delete weight_data;
}

void DataCenterMongoDB::Read1DArrayData(const string& remote_filename, int& num, float*& data) {
    char* databuf = nullptr;
    size_t datalength;
    spatial_gridfs_->GetStreamData(remote_filename, databuf, datalength);
    if (nullptr == databuf) return;

    num = CVT_INT(datalength / 4);
    data = reinterpret_cast<float *>(databuf); // deprecate C-style: (float *) databuf;
}

void DataCenterMongoDB::Read2DArrayData(const string& remote_filename, int& rows, int& cols,
                                        float**& data) {
    char* databuf = nullptr;
    size_t datalength;
    spatial_gridfs_->GetStreamData(remote_filename, databuf, datalength);
    if (nullptr == databuf) {
        data = nullptr;
        return;
    }

    float* float_values = reinterpret_cast<float *>(databuf); // deprecate C-style: (float *) databuf;

    int n_rows = CVT_INT(float_values[0]);
    int n_cols = -1;
    rows = n_rows;
    data = new(nothrow) float*[rows];

    int index = 1;
    for (int i = 0; i < rows; i++) {
        int col = CVT_INT(float_values[index]); // real column
        if (n_cols < 0) {
            n_cols = col;
        } else if (n_cols != col) {
            n_cols = 1;
        }
        int n_sub = col + 1;
        data[i] = new(nothrow) float[n_sub];
        data[i][0] = CVT_FLT(col);
        //cout<<"index: "<<index<<",";
        for (int j = 1; j < n_sub; j++) {
            data[i][j] = float_values[index + j];
            //cout<<data[i][j]<<",";
        }
        //cout<<endl;
        index += n_sub;
    }
    cols = n_cols;
    /// release memory
    Release1DArray(float_values);
    if (nullptr != databuf) {
        databuf = nullptr;
    }
}

void DataCenterMongoDB::ReadIuhData(const string& remote_filename, int& n, float**& data) {
    char* databuf = nullptr;
    size_t datalength;
    spatial_gridfs_->GetStreamData(remote_filename, databuf, datalength);
    if (nullptr == databuf) {
        data = nullptr;
        return;
    }

    float* float_values = reinterpret_cast<float *>(databuf); // deprecate C-style: (float *) databuf;

    n = CVT_INT(float_values[0]);
    data = new float *[n];

    int index = 1;
    for (int i = 0; i < n; i++) {
        int n_sub = CVT_INT(float_values[index + 1] - float_values[index] + 3);
        data[i] = new float[n_sub];

        data[i][0] = float_values[index];
        data[i][1] = float_values[index + 1];
        for (int j = 2; j < n_sub; j++) {
            data[i][j] = float_values[index + j];
        }
        index = index + n_sub;
    }
    /// release memory
    Release1DArray(float_values);

    if (nullptr != databuf) {
        databuf = nullptr;
    }
}

bool DataCenterMongoDB::SetRasterForScenario() {
    if (!use_scenario_) return false;
    if (nullptr == scenario_) return false;
    map<string, FloatRaster *>& scene_rs_map = scenario_->getSceneRasterDataMap();
    if (scene_rs_map.empty()) return false;
    for (auto it = scene_rs_map.begin(); it != scene_rs_map.end(); ++it) {
        if (rs_map_.find(it->first) == rs_map_.end()) {
            it->second = ReadRasterData(it->first);
        } else {
            it->second = rs_map_.at(it->first);
        }
    }
    return true;
}
