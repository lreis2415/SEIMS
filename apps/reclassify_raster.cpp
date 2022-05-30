/*!
 * \brief Reclassify raster data based on categorized data in integer types.
 *        For example, Byte, Int16, and Int32, etc.
 *        Output raster data type can be integer or float. 
 *
 * \author Liang-Jun Zhu, zlj(at)lreis.ac.cn
 * \date Feb. 2017
 * \remarks
 *     - 1. 2021-12-21 - lj - Rewrite as an stand-alone application inside CCGL
 *
 * \copyright 2017-2021. LREIS, IGSNRR, CAS
 *
 */

#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#include "reclassify_raster.h"

bool read_reclassification(const string& filename, map<vint, double>& reclass_map) {
    vector<string> strs;
    if (!LoadPlainTextFile(filename, strs)) { return false; }
    for (auto it = strs.begin(); it != strs.end(); ++it) {
        vector<double> tmpv;
        if (!SplitStringForValues(*it, ' ', tmpv)
            || tmpv.size() < 2) {
            continue;
        }
#ifdef HAS_VARIADIC_TEMPLATES
        reclass_map.emplace(CVT_VINT(tmpv[0]), tmpv[1]);
#else
        reclass_map.insert(make_pair(CVT_VINT(tmpv[0]), tmpv[1]));
#endif
    }
    return !reclass_map.empty();
}

void Usage(const string& appname, const string& error_msg /* = std::string() */) {
    if (!error_msg.empty()) {
        cout << "FAILURE: " << error_msg << endl;
    }
    string corename = GetCoreFileName(appname);
    cout << "Simple Usage:\n    " << corename << " <configFile>\n\n";
    cout << "Two complete usage modes are supported:\n";
    cout << "1. " << corename << " -infile <inputRaster> -recls <key-value> -defaultclass <defaultClass>"
            " [-outfile <outputRaster>] [-outtype <outType>] [-thread <threadsNum>]\n\n";
    cout << "2. " << corename << " -configfile <configFile> [-thread <threadsNum>]\n\n";
    cout << "\t<inputRaster> is full path of input raster.\n";
    cout << "\t<key-value> is one or more key-value pairs in format of \"k1:v1,k2:v2\"\n";
    cout << "\t<defaultClass> is default class value for reclassification.\n";
    cout << "\t<outputRaster> is full path of output raster, if not specified,"
            "<inputRaster>_recls.tif will be generated.\n";
    cout << "\t<outType> is the datatype of <outputRaster>, uint8, int8, uint16,"
            " int16, uint32, int32, float, and double are supported.\n";
    cout << "\t<threadsNum> is the number of thread used by OpenMP, which must be >= 1 (default).\n";
    cout << "\t<configFile> is a plain text file that defines all input parameters, the format is:\n";
    cout << "\t\ttypeLayer defaultClass\n";
    cout << "\t\tlookupFolder\n";
    cout << "\t\toutputFolder\n";
    cout << "\t\tnumOutputs\n";
    cout << "\t\tattribute1\t<defaultClass>\t[outType]\n";
    cout << "\t\tattribute2\t<defaultClass>\t[outType]\n";
    cout << "\t\t...\n\n";
}

InputArgs* InputArgs::Init(const int argc, const char** argv) {
    int thread_num = 1;
    string rs_path;
    vector<string> out_paths;
    vector<map<vint, double> > recls;
    vint default_class = -9999; // default class value of the categorized raster layer
    vector<double> default_values;
    vector<string> out_types;

    string config_path;
    bool simple_usage = false;
    if (argc < 2) {
        Usage(argv[0], "To run the program, "
              "use either a single configuration file and/or detail arguments as below.");
        return nullptr;
    }
    if (argc >= 2 && argv[1][0] != '-') {
        config_path = argv[1];
        simple_usage = true;
    }
    int i = 1;
    bool str2num_flag = false;
    while (!simple_usage && argc > i) {
        if (StringMatch(argv[i], "-thread")) {
            i++;
            if (argc > i) {
                thread_num = CVT_INT(IsInt(argv[i], str2num_flag));
                if (!str2num_flag) {
                    Usage(argv[0], "Error: Thread number MUST be an integer!");
                    return nullptr;
                }
                i++;
            }
            else {
                Usage(argv[0]);
                return nullptr;
            }
        }
        else if (StringMatch(argv[i], "-infile")) {
            i++;
            if (argc > i) {
                rs_path = argv[i];
                i++;
            }
            else {
                Usage(argv[0]);
                return nullptr;
            }
        }
        else if (StringMatch(argv[i], "-recls")) {
            i++;
            if (argc > i) {
                string recls_str = argv[i];
                vector<string> keyvalues = SplitString(recls_str, ',');
                if (keyvalues.empty()) {
                    Usage(argv[0], "Wrong <key-value> inputs!");
                    return nullptr;
                }
                map<vint, double> tmp_recls;
                for (auto it = keyvalues.begin(); it != keyvalues.end(); ++it) {
                    vector<double> tmpvalues;
                    if (!SplitStringForValues(*it, ':', tmpvalues) ||
                        tmpvalues.size() != 2) {
                        Usage(argv[0], "Wrong <key-value> inputs!");
                        return nullptr;
                    }
#ifdef HAS_VARIADIC_TEMPLATES
                    tmp_recls.emplace(CVT_VINT(tmpvalues[0]), tmpvalues[1]);
#else
                    tmp_recls.insert(make_pair(CVT_VINT(tmpvalues[0]), tmpvalues[1]));
#endif
                }
                recls.emplace_back(tmp_recls);
                i++;
            }
            else {
                Usage(argv[0]);
                return nullptr;
            }
        }
        else if (StringMatch(argv[i], "-defaultclass")) {
            i++;
            if (argc > i) {
                default_class = IsInt(argv[i], str2num_flag);
                if (!str2num_flag) {
                    Usage(argv[0], "Error: Default value MUST be a number!");
                    return nullptr;
                }
                i++;
            }
            else {
                Usage(argv[0]);
                return nullptr;
            }
        }
        else if (StringMatch(argv[i], "-outfile")) {
            i++;
            if (argc > i) {
                out_paths.emplace_back(argv[i]);
                i++;
            }
            else {
                Usage(argv[0]);
                return nullptr;
            }
        }
        else if (StringMatch(argv[i], "-outtype")) {
            i++;
            if (argc > i) {
                out_types.emplace_back(argv[i]);
                i++;
            }
            else {
                Usage(argv[0]);
                return nullptr;
            }
        }
        else if (StringMatch(argv[i], "-configfile")) {
            i++;
            if (argc > i) {
                config_path = argv[i];
                i++;
            }
            else {
                Usage(argv[0]);
                return nullptr;
            }
        }
        else {
            Usage(argv[0], "Error: No matching tag found!");
            return nullptr;
        }
    }
    // Make sure rs_paths, recls, out_paths, default_values, and out_types have same size
    if (!rs_path.empty()) {
        if (recls.size() != 1) { return nullptr; }
        if (recls[0].count(default_class) > 0) {
            default_values.emplace_back(recls[0].at(default_class));
        } else {
            default_values.emplace_back(NODATA_VALUE);
        }
        if (out_paths.empty()) {
            out_paths.emplace_back(AppendCoreFileName(rs_path, "recls"));
        }
        if (out_types.empty()) { out_types.emplace_back("Unknown"); }
    }
    else { // clear others
        if (!out_paths.empty()) { vector<string>().swap(out_paths); }
        if (!recls.empty()) { vector<map<vint, double> >().swap(recls); }
        if (!default_values.empty()) { vector<double>().swap(default_values); }
        if (!out_types.empty()) { vector<string>().swap(out_types); }
    }
    // Read configuration file
    if (!config_path.empty()) {
        if (!FileExists(config_path)) {
            Usage(argv[0], "Error: The configuration file MUST be existed!");
            return nullptr;
        }
        vector<string> config_strs;
        if (!LoadPlainTextFile(config_path, config_strs)) {
            Usage(argv[0], "Error: Please follow the format of configuration file!");
            return nullptr;
        }
        size_t line_count = config_strs.size();
        if (line_count < 5) {
            Usage(argv[0], "Error: Configuration file MUST have at least 5 lines!");
            return nullptr;
        }
        vector<string> cat_raster = SplitString(config_strs[0]);
        // Only one categorized raster is allowed input by argument or configuration file
        string rs_from_cfg = GetAbsolutePath(Trim(cat_raster[0]));
        if (!FileExists(rs_from_cfg)) {
            Usage(argv[0],
                  "Error: Categorized raster defined in conf. does not existed!");
            return nullptr;
        }
        if (!rs_path.empty() && !StringMatch(GetAbsolutePath(rs_path), rs_from_cfg)) {
            Usage(argv[0], "Error: Categorized raster defined in arguments and"
                  " configuration file are not consistent!");
            return nullptr;
        }
        rs_path = rs_from_cfg;

        if (cat_raster.size() >= 2) {
            default_class = IsInt(cat_raster[1], str2num_flag);
            if (!str2num_flag) {
                Usage(argv[0], "Error: default class value is"
                      " not a number: " + cat_raster[1]);
                return nullptr;
            }
        }
        string lookup_dir = config_strs[1];
        if (!DirectoryExists(lookup_dir)) {
            Usage(argv[0], "Error: Directory of lookup files is "
                  "not existed: " + lookup_dir);
            return nullptr;
        }
        string out_dir = config_strs[2];
        if (!DirectoryExists(out_dir)) { MakeDirectory(out_dir); }

        size_t inrecls_count = CVT_SIZET(IsInt(config_strs[3], str2num_flag));
        if (!str2num_flag) {
            Usage(argv[0], "Error: The 2nd line in config file MUST be an integer!");
            return nullptr;
        }
        for (size_t si = 4; si < line_count; si++) {
            vector<string> item_strs = SplitString(config_strs[si]);
            size_t item_count = item_strs.size();
            if (item_count < 1) { continue; }
            string lookupf = ConcatFullName(lookup_dir, item_strs[0], "txt");
            map<vint, double> tmprecls;
            if (!FileExists(lookupf) || !read_reclassification(lookupf, tmprecls)) {
                StatusMessage("Warning: " + lookupf + " is not existed or has waring format!");
                continue;
            }
            double tmpdefv = tmprecls.count(default_class) > 0 ? tmprecls.at(default_class) : NODATA_VALUE;
            string tmpouttype = "Unknown";
            size_t ii = 1;
            while (ii < item_count) {
                vint tmpdefcls = IsInt(item_strs[ii], str2num_flag);
                if (str2num_flag) {
                    tmpdefcls = default_class;
                    if (tmpdefcls != default_class && tmprecls.count(tmpdefcls) > 0) {
                        tmpdefv = tmprecls.at(tmpdefcls);
                    }
                } else {
                    tmpouttype = item_strs[ii];
                }
                ii++;
            }
            recls.emplace_back(tmprecls);
            default_values.emplace_back(tmpdefv);
            out_types.emplace_back(tmpouttype);
            out_paths.emplace_back(ConcatFullName(out_dir, item_strs[0], GetSuffix(rs_path)));
        }
        if (out_paths.size() != inrecls_count) {
            StatusMessage("Warning: There are " + itoa(CVT_VINT(inrecls_count - out_paths.size()))
                          + " lookup(s) have wrong formats and thus do not be concluded!");
        }
    }
    if (rs_path.empty() || recls.empty()) {
        Usage(argv[0], "Error: No qualified inputs!");
        return nullptr;
    }
    return new InputArgs(rs_path, recls, default_values, out_paths, out_types, thread_num);
}

InputArgs::InputArgs(string& in_raster, vector<map<vint, double> >& recls,
                     vector<double>& defvalues, vector<string>& out_rasters,
                     vector<string>& outtype, const int thread_num /* = 1 */) :
    thread_num(thread_num), rs_path(in_raster), recls(recls),
    out_paths(out_rasters), def_values(defvalues), out_types(outtype) {
    // Do nothing
}

bool reclassify_raster(string& infile, vector<map<vint, double> >& reclsmap,
                       vector<string>& outtypes, vector<double>& defvalues,
                       vector<string>& outfiles){
    DblRaster* category_rs = DblRaster::Init(infile, true);
    if (nullptr == category_rs) {
        cout << "Error: Initialize input categorized raster failed!" << endl;
        return false;
    }
    category_rs->BuildSubSet();
    for (auto it = reclsmap.begin(); it != reclsmap.end(); ++it) {
        size_t idx = it - reclsmap.begin();
        category_rs->SetOutDataType(outtypes[idx]);
        category_rs->OutputSubsetToFile(false, // Not original raster data
                                        true, // Output combined data
                                        outfiles[idx], *it, defvalues[idx]);
    }
    delete category_rs;
    return true;
}

int main(const int argc, const char** argv) {
    InputArgs* input_args = InputArgs::Init(argc, argv);
    if (nullptr == input_args) { return 0; }
#ifdef USE_GDAL
    GDALAllRegister();
#endif
    SetOpenMPThread(input_args->thread_num);

    reclassify_raster(input_args->rs_path, input_args->recls,
                      input_args->out_types, input_args->def_values,
                      input_args->out_paths);
    delete input_args;
    return 0;
}
