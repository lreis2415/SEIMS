/*!
 * \brief IO of raster data or dataset based on mask layer.
 *
 * \author Liang-Jun Zhu, zlj(at)lreis.ac.cn
 * \remarks
 *     - 1. 2021-11-25 - lj - Rewrite as an stand-alone application inside CCGL
 *     - 2. 2022-04-29 - lj - Support multiple subsets, support IO of file and MongoDB
 *
 * \copyright 2017-2022. LREIS, IGSNRR, CAS
 *
 */

#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#include "mask_rasterio.h"

IOMODE StringToIOMode(const string& str) {
    string mode_str = GetUpper(str);
    if (mode_str == "DEC" || mode_str == "DECOMPOSE") { return DEC; }
    if (mode_str == "COM" || mode_str == "COMBINE") { return COM; }
    if (mode_str == "MASK") { return MASK; }
    return UNKNOWNMODE;
}

DATAFMT StringToDataFormat(const string& str) {
    string fmt_str = GetUpper(str);
    if (fmt_str == "SFILE" || fmt_str == "FILE") { return SFILE; }
    if (fmt_str == "GFS" || fmt_str == "GRIDFS") { return GFS; }
    return UNKNOWNFMT;
}

void Usage(const string& appname, const string& error_msg /* = std::string() */) {
    if (!error_msg.empty()) {
        cout << "FAILURE: " << error_msg << endl;
    }
    string corename = GetCoreFileName(appname);
    cout << "Simple Usage:\n    " << corename << " <configFile>\n\n";
    cout << "Two complete usage modes are supported:\n";
    cout << "1. " << corename <<
            " -in [<inFmt>] [<inFile>[;<inFile2>;...]] [<inGFSName>]"
            " [-mask [<maskFmt>] [<maskFile>] [<maskGFSName>]]"
            " [-mode <IOMode>]"
            " [-out [<outFmt>] [<outFile>] [<outGFSName>]"
            " [-outdatatype <outDataType>] [-default <defaultValue>]"
            " [-include_nodata <includeNoData>]"
            " [-mongo <host> <port> <DB> <GFS>]"
            " [-thread <threadsNum>]\n\n";
    cout << "2. " << corename << " -configfile <configFile> [-thread <threadsNum>]\n\n";

    cout << "\t<xxFmt> is data format for <in>, <out>, and <mask>, can be FILE or GFS.\n";
    cout << "\t<xxFile> and <xxFile2>... are full paths of raster, ASCII and GeoTIFF are recommended.\n";
    cout << "\t<xxGFSName> is the GridFS file name in MongoDB.\n";
    cout << "\t<defaultValue> is default value for nodata locations that covered by mask.\n";
    cout << "\t<IOMode> is the operation type, can be MASK (default), DEC(Decompose), or COM(Combine).\n";
    cout << "\tIf -out is not specified, <in>_masked.tif will be generated.\n";
    cout << "\t<outDataType> is the data type of <out>, uint8, int8, uint16,"
            " int16, uint32, int32, float, and double are supported.\n";
    cout << "\t<includeNoData> is used when output raster data into MongoDB, can be 1 or 0.\n";
    cout << "\t<threadsNum> is the number of thread used by OpenMP, which must be >= 1 (default).\n";
    cout << "\t-mongo specify the MongoDB configuration, including host, port, DB, and GFS.\n";
    cout << "\t<configFile> is a plain text file that defines all input parameters, the format is:\n";
    cout << "\t\t[-mode <IOMode>]";
    cout << "\t\t[-mongo <host> <port> <DB> <GFS>]\n";
    cout << "\t\t-mask <maskFmt> [<maskFile>] [<maskGFSName>]";
    cout << "\t\t[-in <inFmt>]\n";
    cout << "\t\t[-out <outFmt>]\n";
    cout << "\t\t[-outdatatype <outDataType>]\n";
    cout << "\t\t[-include_nodata <includeNoData>]\n";
    cout << "\t\t\"<in1>;<in2>;... <out> [<defaultValue>][<outDataType>]\"\n";
    cout << "\t\t...\n\n";
}


bool parse_fmt_paths(const string& tag, vector<string>& strs, DATAFMT& fmt, vector<string>& paths) {
    if (strs.empty()) { return false; }
    size_t tmpcount = strs.size();
    int path_idx = tmpcount > 1 ? 1 : 0;
    DATAFMT tmpfmt = StringToDataFormat(strs[0]);
    vector<string> file_paths = SplitString(strs[path_idx], ';');
    if (!StringMatch(tag, "OUT") && FilesExist(file_paths)) {
        // 1: {"/path/to/single_file.tif"},
        // 3: {"file", "/path/to/single_file.tif"}
        // 5: {"/path/to/file.tif;/path/to/file2.tif"}
        // 6: {"file", "/path/to/file.tif;/path/to/file2.tif"}
        // other: {"any_words", "path/to/file.tif"}
        fmt = SFILE;
    } else {
        if (tmpcount == 1) {
            if (tmpfmt == GFS || tmpfmt == SFILE) {
                // 7: {"file"} or {"gfs"}
                fmt = tmpfmt;
                return true;
            }
            // 2: {"single_gfsname"}
            fmt = GFS; // we can only guess this is a GridFS filename!
        }
        else {
            if (tmpfmt == GFS || tmpfmt == SFILE) { // 4: {"gfs", "single_gfsname"} or for "OUT" tag!
                fmt = tmpfmt;
            }
            else {
                cout << "Error: Illegal format for " << tag << "\n";
                return false;
            }
        }
    }
    for (auto it = file_paths.begin(); it != file_paths.end(); ++it) {
        paths.emplace_back(*it);
    }
    return true;
}

/*!
 * \return
 *   0. Succeed
 *   1. Format error of input arguments
 *   2. File specified but not existed, including configuration file and input data files
 *   3. File content loaded failed or wrong format
 *
 */
int main(const int argc, const char** argv) {
    //InputArgs* input_args = InputArgs::Init(argc, argv);
    //if (nullptr == input_args) { return EXIT_FAILURE; }
    IOMODE mode = MASK;
    int thread_num = 1;
    bool inc_nodata = true;

    bool use_mongo = false;
#ifdef USE_MONGODB
    string mongo_host;
    vint16_t mongo_port;
    string dbname;
    string gfsname;
    MongoClient* client = nullptr;
    MongoGridFs* gfs = nullptr;
#endif

    DATAFMT mask_fmt = UNKNOWNFMT;
    string mask_path;
    DATAFMT global_infmt = UNKNOWNFMT;
    vector<DATAFMT> in_fmts;
    vector<vector<string> > in_paths;
    DATAFMT global_outfmt = UNKNOWNFMT;
    vector<DATAFMT> out_fmts;
    vector<string> out_paths;
    double global_defaultv = NODATA_VALUE;
    vector<double> default_values;
    RasterDataType global_outtype = RDT_Unknown;
    vector<RasterDataType> out_types;

    if (argc < 2) {
        Usage(argv[0], "To run the program, "
              "use either a single configuration file and/or detail arguments as below.");
        return 1;
    }
    string config_path;
    vector<int> arg_sep;
    for (int idx = 1; idx < argc; idx++) {
        if (argv[idx] == nullptr || argv[idx][0] != '-') { continue; }
        if (StringMatch(argv[idx], "-configfile") && argc > idx && argv[idx + 1][0] != '-') {
            config_path = argv[idx + 1];
        }
        arg_sep.push_back(idx);
    }
    arg_sep.push_back(argc);

    map<string, vector<string> > key_args;
    if (arg_sep.empty()) {
        if (config_path.empty()) {
            if (argc >= 2 && argv[1][0] != '-') {
                config_path = argv[1];
            }
            else {
                Usage(argv[0], "Illegal inputs!");
                return 1;
            }
        }
    }
    else {
        for (auto arg_idx = arg_sep.begin(); arg_idx != arg_sep.end() - 1; ++arg_idx) {
            vector<string> tmpargs(argv + *arg_idx + 1, argv + *(arg_idx + 1));
            if (tmpargs.empty()) { continue; }
            string key(argv[*arg_idx]);
            if (StringMatch(key, "-configfile")) { continue; }
            key_args.insert(std::make_pair(GetUpper(key.substr(1, string::npos)), tmpargs));
        }
    }
    vector<string> config_strs;
    vector<vector<string> > io_strs;
    if (!config_path.empty()) {
        if (!FileExists(config_path)) {
            Usage(argv[0], "Configuration file specified but not existed!");
            return 2;
        }
        // Read configuration file
        if (!LoadPlainTextFile(config_path, config_strs)) {
            Usage(argv[0]);
            return 3;
        }
    }
    // Concatenate arguments from command line and configuration file
    for (auto it = config_strs.begin(); it != config_strs.end(); ++it) {
        vector<string> tmpconfig = SplitString(*it, '\t'); // Only support \t!!!
        if (tmpconfig.empty() || tmpconfig.size() < 2) { continue; }
        if (tmpconfig[0][0] == '-') {
            string tmpkey = GetUpper(tmpconfig[0].substr(1, string::npos));
            if (key_args.find(tmpkey) != key_args.end()) { continue; }
            key_args.insert(std::make_pair(tmpkey, vector<string>(tmpconfig.begin() + 1,
                                                                  tmpconfig.end())));
        }
        else {
            io_strs.emplace_back(tmpconfig);
        }
    }
    // Parse mixed arguments
    char* strend = nullptr;
    bool str2num_flag = false;
    for (auto itkv = key_args.begin(); itkv != key_args.end(); ++itkv) {
        if (itkv->first == "MODE") {
            mode = StringToIOMode(itkv->second.at(0));
            if (mode == UNKNOWNMODE) { return 1; }
        }
        else if (itkv->first == "IN" || itkv->first == "INPUT") {
            vector<string> tmppaths;
            if (!parse_fmt_paths("IN", itkv->second, global_infmt, tmppaths)) {
                return 2;
            }
            if (!tmppaths.empty()) {
                in_paths.emplace_back(tmppaths);
                in_fmts.emplace_back(global_infmt);
            }
        }
        else if (itkv->first == "MASK") {
            vector<string> tmppaths;
            if (!parse_fmt_paths("MASK", itkv->second, mask_fmt, tmppaths)
                && tmppaths.empty()) {
                return 2;
            }
            mask_path = tmppaths[0];
        }
        else if (itkv->first == "OUT" || itkv->first == "OUTPUT") {
            vector<string> tmppaths;
            if (parse_fmt_paths("OUT", itkv->second, global_outfmt, tmppaths)) {
                if (!tmppaths.empty()) {
                    out_paths.emplace_back(tmppaths[0]);
                    out_fmts.emplace_back(global_outfmt);
                }
            }
        }
        else if (itkv->first == "OUTDATATYPE") {
            global_outtype = StringToRasterDataType(itkv->second.at(0));
        }
        else if (itkv->first == "DEFAULT" || itkv->first == "DEFAULTVALUE") {
            global_defaultv = IsDouble(itkv->second.at(0), str2num_flag);
            if (!str2num_flag) { global_defaultv = NODATA_VALUE; }
        }
        else if (itkv->first == "MONGO" || itkv->first == "MONGODB") {
#ifdef USE_MONGODB
            if (itkv->second.size() < 4) {
                cout << "Warning: -mongo tag MUST have 4 arguments!\n";
                return 1;
            }
            mongo_host = itkv->second.at(0);
            mongo_port = strtol(itkv->second.at(1).c_str(), &strend, 10);
            dbname = itkv->second.at(2);
            gfsname = itkv->second.at(3);
            client = MongoClient::Init(mongo_host.c_str(), mongo_port);
            if (nullptr == client) {
                cout << "Warning: Illegal arguments for MongoDB!\n";
            }
            else {
                gfs = client->GridFs(dbname, gfsname);
                if (nullptr == gfs) {
                    cout << "Warning: Get or create GridFS failed!\n";
                    use_mongo = false;
                }
                else { use_mongo = true; }
            }
#else
            cout << "Warning: MongoDB is not supported\n";
#endif
        }
        else if (itkv->first == "THREAD") {
            thread_num = CVT_INT(IsInt(itkv->second.at(0), str2num_flag));
            if (!str2num_flag) {
                cout << "Warning: Illegal thread number, use thread = 1 instead!\n";
                thread_num = 1;
            }
        }
        else if (itkv->first == "INCLUDE_NODATA") {
            inc_nodata = IsInt(itkv->second.at(0), str2num_flag) > 0;
            if (!str2num_flag) {
                cout << "Warning: Illegal flag for INCLUDE_NODATA, use 1 instead!\n";
                inc_nodata = true;
            }
        }
        else {
            cout << "Warning: Unknown Tag that will be ignored: " << itkv->first << "\n";
        }
    }
    // Make sure in_paths, out_paths, default_values, and out_types have same size
    if (!use_mongo) {
        if (global_outfmt == GFS) { global_outfmt = SFILE; }
    }
    if (global_outfmt == UNKNOWNFMT) {
        global_outfmt = global_infmt;
    }
    if (!in_paths.empty()) {
        if (in_paths.size() != 1) { return 1; }
        if (out_paths.empty()) {
            out_paths.emplace_back(AppendCoreFileName(in_paths[0][0], "masked"));
        }
        if (default_values.empty()) { default_values.emplace_back(global_defaultv); }
        if (out_types.empty()) { out_types.emplace_back(global_outtype); }
        if (out_fmts.empty()) { out_fmts.emplace_back(global_outfmt); }
        else { out_fmts[0] = global_outfmt; }
    }
    else { // clear others
        if (!out_paths.empty()) { vector<string>().swap(out_paths); }
        if (!default_values.empty()) { vector<double>().swap(default_values); }
        if (!out_types.empty()) { vector<RasterDataType>().swap(out_types); }
    }
    // Parse input and output settings in configuration file
    for (auto ioit = io_strs.begin(); ioit != io_strs.end(); ++ioit) {
        size_t item_count = (*ioit).size();
        if (item_count < 1) { continue; }
        int inidx = 0;
        int outidx = -1;
        int dvidx = -1;
        int outtypeidx = -1;
        if (item_count == 2) {
            outidx = 1;
            dvidx = 1;
            outtypeidx = 1;
        }
        else if (item_count == 3) {
            outidx = 1;
            dvidx = 2;
            outtypeidx = 2;
        }
        else if (item_count == 4) {
            outidx = 1;
            dvidx = 2;
            outtypeidx = 3;
        }
        vector<string> tmpin = SplitString((*ioit).at(inidx), ';');
        string tmpout = outidx > 0 ? (*ioit).at(outidx) : "";
        double tmpdefv = NODATA_VALUE;
        if (outidx > 0) {
            tmpdefv = IsDouble((*ioit).at(dvidx), str2num_flag);
            if (!str2num_flag) { tmpdefv = NODATA_VALUE; }
        }
        RasterDataType tmpouttype = static_cast<RasterDataType>(RDT_Unknown);
        if (outtypeidx > 0) {
            tmpouttype = StringToRasterDataType((*ioit).at(outtypeidx));
            if (tmpouttype == static_cast<RasterDataType>(RDT_Unknown)) {
                tmpouttype = global_outtype;
            }
        }
        if ((global_infmt == SFILE && FilesExist(tmpin) || global_infmt == GFS)) {
            in_paths.emplace_back(tmpin);
            in_fmts.emplace_back(SFILE);
            out_paths.emplace_back(tmpout);
            out_fmts.emplace_back(global_outfmt);
            default_values.emplace_back(tmpdefv);
            out_types.emplace_back(tmpouttype);
        }
    }

#ifdef USE_GDAL
    GDALAllRegister();
#endif

    SetOpenMPThread(thread_num);

    // Load mask layer
    IntRaster* mask_layer = nullptr;
    if (!mask_path.empty()) {
        if (mask_fmt == SFILE && FileExists(mask_path)) {
            mask_layer = IntRaster::Init(mask_path);
            if (nullptr == mask_layer) { return 3; }
        } else if (mask_fmt == GFS && use_mongo) {
#ifdef USE_MONGODB
            mask_layer = IntRaster::Init(gfs, mask_path.c_str());
            if (nullptr == mask_layer) { return 3; }
#endif
        } else {
            // No mask layer
        }
    }

    // Load input raster
    STRING_MAP opts; // Additional options, e.g., output data type
    bool output_subset = false;
    bool combine_subset = false;
    if (mask_layer != nullptr) {
        if (mode == DEC || mode == COM) {
            mask_layer->BuildSubSet();
            output_subset = mode == DEC;
            combine_subset = mode == COM;
        }
    }

    for (auto in_it = in_paths.begin(); in_it != in_paths.end(); ++in_it) {
        size_t in_idx = in_it - in_paths.begin();
        if (out_types.at(in_idx) == static_cast<RasterDataType>(RDT_Unknown)) {
            UpdateStringMap(opts, HEADER_RSOUT_DATATYPE, "DOUBLE");
        } else {
            UpdateStringMap(opts, HEADER_RSOUT_DATATYPE, RasterDataTypeToString(out_types.at(in_idx)));
        }
        if (mode == MASK || mode == DEC) { // paths in *in_it are regarded as multiple layers
            DblIntRaster* rs = nullptr;
            if (in_fmts.at(in_idx) == SFILE && FilesExist(*in_it)) {
                rs = DblIntRaster::Init(*in_it,
                                        false,            // No need to calculate valid positions
                                        mask_layer, true, // Use entire extent of Mask
                                        default_values.at(in_idx),
                                        opts);
            }
            else if (in_fmts.at(in_idx) == GFS && use_mongo) {
#ifdef USE_MONGODB
                rs = DblIntRaster::Init(gfs, (*in_it).at(0).c_str(),
                                        false,
                                        mask_layer, true,
                                        default_values.at(in_idx), opts);
#endif
            }
            else {
                continue;
            }
            if (nullptr == rs) { continue; }
            rs->CalculateStatistics();
            if (out_fmts.at(in_idx) == SFILE) {
                rs->OutputToFile(out_paths.at(in_idx), output_subset);
            } else if (out_fmts.at(in_idx) == GFS && use_mongo) {
#ifdef USE_MONGODB
                rs->OutputToMongoDB(gfs, out_paths.at(in_idx), opts, inc_nodata, output_subset);
#endif
            } else {
                // Nothing to do
            }
            delete rs;
        } else if (mode == COM) {
            map<int, SubsetPositions*>& subset = mask_layer->GetSubset();
            for (auto it = subset.begin(); it != subset.end(); ++it) {
                it->second->usable = false;
            }
            for (auto inf_it = (*in_it).begin(); inf_it != (*in_it).end(); ++inf_it) {
                string tmpcname = GetCoreFileName(*inf_it);
                vector<string> cname_strs = SplitString(tmpcname, '_');
                if (cname_strs.empty()) { continue; }
                int subid = CVT_INT(IsInt(cname_strs[0], str2num_flag));
                if (!str2num_flag) { continue; }
                if (subset.find(subid) == subset.end()) { continue; }

                DblRaster* tmpsubrs = nullptr;
                if (in_fmts.at(in_idx) == SFILE && FileExists(*inf_it)) {
                    tmpsubrs = DblRaster::Init(*inf_it, !inc_nodata);
                } else if (in_fmts.at(in_idx) == GFS && use_mongo) {
#ifdef USE_MONGODB
                    tmpsubrs = DblRaster::Init(gfs, (*inf_it).c_str(), !inc_nodata);
#endif
                } else {
                    continue;
                }
                if (nullptr == tmpsubrs) { continue; }
                int tmpsublen;
                double* tmpsubdata = nullptr;
                tmpsubrs->GetRasterData(&tmpsublen, &tmpsubdata);
                subset.at(subid)->SetData(tmpsublen, tmpsubdata);

                delete tmpsubrs;
            }

            if (out_fmts.at(in_idx) == SFILE) {
                mask_layer->OutputToFile(out_paths.at(in_idx), combine_subset);
            }
            else if (out_fmts.at(in_idx) == GFS && use_mongo) {
#ifdef USE_MONGODB
                mask_layer->OutputToMongoDB(gfs, out_paths.at(in_idx), opts, inc_nodata, combine_subset);
#endif
            }
            else {
                // Nothing to do
            }
        } else {
            // Nothing to do
        }
    }

    delete mask_layer;
#ifdef USE_MONGODB
    if (use_mongo) {
        delete gfs;
        client->Destroy();
        delete client;
    }
#endif
    return 0;
}
