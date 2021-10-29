/*----------------------------------------------------------------------
*	Purpose:  Grid layering functions
*
*	Created:	Junzhi Liu
*	Date:		28-March-2013
*
*	Revision:   Liangjun Zhu
*   Date:       21-July-2016
*               9-February-2017
*               4-July-2017  Check if success after import layering to MongoDB
*               29-Dec-2017  Refactor to make code more clearly
*                5-Mar-2018  Use CCGL, and reformat code style
*               31-Mar-2021  Rewrite most core parts and now support MFD-md algorithm
*               18-May-2021  Force each stream grid flow into one downstream grid
*               27-Aug-2021  Add new layering method named _EVEN to balance computation amount
*---------------------------------------------------------------------*/

#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#include "utils_time.h"
#include "GridLayering.h"

using namespace utils_time;

void Usage(const string& error_msg = "") {
    if (!error_msg.empty()) {
        cout << "FAILURE: " << error_msg << endl << endl;
    }
    cout << " Usage: grid_layering -alg <flow_direction_algorithm> -outdir <output_dir> "
            "[-mask <mask_raster>] [-stream <stream_shp>]"
            "[-file <input_dir> <corenames> <nsubbasin>] "
            "[-mongo <ip> <port> <database_name> <gridfs_name> <nsubbasin>] " << endl << endl;
    cout << "Available flow direction algorithms include d8, dinf, and mfdmd." << endl;
    cout << "Ex.1. grid_layering -alg d8 -outdir d:/tmp -file d:/test/fd_rasters flow_dir.tif 0" << endl;
    cout << "Ex.2. grid_layering -alg dinf -outdir d:/tmp -mask d:/tmp/mask.tif -file "
            "d:/test/fd_rasters flow_dir_dinf.tif,flow_dir_angle_dinf.tif" << endl;
    cout << "Ex.3. grid_layering -alg dinf -outdir d:/tmp -mongo 127.0.0.1 27017 "
            "model_dianbu2_30m_demo SPATIAL 15" << endl;
    cout << "Ex.4. grid_layering -alg mfdmd -outdir d:/tmp -mask d:/test/mask.tif "
            "-stream d:/test/stream.shp -file d:/test/fd_rasters mfdmd.tif,mfdfraction.tif 0"
            << endl << endl;
    exit(1);
}

flowDirTypes MatchFlowDirAlg(const char* algstr) {
    if (StringMatch(algstr, "d8")) return FD_D8;
    if (StringMatch(algstr, "dinf")) return FD_Dinf;
    if (StringMatch(algstr, "mfdmd")) return FD_MFDmd;
    return FD_D8;
}

int main(int argc, char** argv) {
    /// set default OpenMP thread number to improve compute efficiency
    SetDefaultOpenMPThread();
    /// Register GDAL drivers, REQUIRED!
    GDALAllRegister();
    /// Define input arguments
    flowDirTypes fdtype = FD_D8;
    const char* out_dir = nullptr;
    bool from_file = false;
    string mask_file;
    string stream_file;
    string input_dir;
    string core_names;

    bool from_mongo = false;
    const char* mongodb_ip = nullptr;
    vuint16_t port = 27017;
    string db_name;
    string gridfs_name;

    int n_subbasins = -1;

    /// Parse input arguments
    if (argc < 9) {
        // At least needs 8 arguments
        Usage("Too few arguments to run this program.");
        exit(-1);
    }
    int i = 1;
    while (argc > i) {
        if (StringMatch(argv[i], "-alg")) {
            i++;
            if (argc > i) {
                fdtype = MatchFlowDirAlg(argv[i]);
                i++;
            } else {
                Usage("No argument followed '-alg'!");
            }
        } else if (StringMatch(argv[i], "-outdir")) {
            i++;
            if (argc > i) {
                out_dir = argv[i];
                i++;
            } else {
                Usage("No argument followed '-outdir'!");
            }
        } else if (StringMatch(argv[i], "-mask")) {
            i++;
            if (argc > i) {
                mask_file = argv[i];
                i++;
            } else {
                Usage("No argument followed '-mask'!");
            }
        } else if (StringMatch(argv[i], "-stream")) {
            i++;
            if (argc > i) {
                stream_file = argv[i];
                i++;
            } else {
                Usage("No argument followed '-stream'!");
            }
        } else if (StringMatch(argv[i], "-file")) {
            if (argc < i + 3) {
                Usage("Three arguments are required followed '-file'!");
            }
            from_file = true;
            input_dir = argv[i + 1];
            core_names = argv[i + 2];
            n_subbasins = CVT_INT(ToInt(argv[i + 3]));
            i += 4;
        } else if (StringMatch(argv[i], "-mongo")) {
            if (argc < i + 5) {
                // at least 5 arguments
                Usage("Five arguments are required followed '-mongo'!");
            }
            from_mongo = true;
            mongodb_ip = argv[i + 1];
            if (!IsIpAddress(mongodb_ip)) {
                Usage("The input MongoDB Host is not a valid IP address!");
            }
            port = static_cast<vuint16_t>(ToInt(argv[i + 2]));
            db_name = argv[i + 3];
            gridfs_name = argv[i + 4];
            n_subbasins = CVT_INT(ToInt(argv[i + 5]));
            i += 6;
        } else {
            cout << argv[i] << endl;
            Usage("The above input argument not matched!");
        }
    }
    if (n_subbasins < 0) {
        Usage("Subbasin count must be greater than 0!");
    }

    /// Execute
    double t1 = TimeCounting();
    int subbasin_start_id = 1;
    if (n_subbasins == 0) {
        subbasin_start_id = 0;
    }
    if (from_file) {
        if (fdtype == FD_D8) {
            string in_file = input_dir + SEP + core_names;
            if (mask_file.empty()) mask_file = in_file;
            GridLayeringD8* grid_lyr_d8 = new GridLayeringD8(n_subbasins, in_file.c_str(),
                                                             mask_file.c_str(), out_dir);
            grid_lyr_d8->Execute();
            delete grid_lyr_d8;
        } else if (fdtype == FD_Dinf) {
            vector<string> in_files = SplitString(core_names, ',');
            if (in_files.size() != 2) Usage("Two input files are required for Dinf model!");
            string cfdir_file = input_dir + SEP + in_files[0];
            string ffrac_file = input_dir + SEP + in_files[1];
            if (mask_file.empty()) mask_file = cfdir_file;
            GridLayeringDinf* grid_lyr_dinf = new GridLayeringDinf(n_subbasins, cfdir_file.c_str(),
                                                                   ffrac_file.c_str(), mask_file.c_str(),
                                                                   stream_file.c_str(), out_dir);
            grid_lyr_dinf->Execute();
            delete grid_lyr_dinf;
        } else if (fdtype == FD_MFDmd) {
            vector<string> in_files = SplitString(core_names, ',');
            if (in_files.size() != 2) Usage("Two input files are required for MFD-md model!");
            string cfdir_file = input_dir + SEP + in_files[0];
            string ffrac_file = input_dir + SEP + in_files[1];
            if (mask_file.empty()) mask_file = cfdir_file;
            GridLayeringMFDmd* grid_lyr_mfdmd = new GridLayeringMFDmd(n_subbasins, cfdir_file.c_str(),
                                                                      ffrac_file.c_str(), mask_file.c_str(),
                                                                      stream_file.c_str(), out_dir);
            grid_lyr_mfdmd->Execute();
            delete grid_lyr_mfdmd;
        } else Usage("Unsupported flow direction algorithm!");
    }
#ifdef USE_MONGODB
    else if (from_mongo) {
        /// connect to MongoDB
        MongoClient* client = MongoClient::Init(mongodb_ip, port);
        if (nullptr == client) Usage("Unable to connect to MongoDB!");
        MongoGridFs* gfs = client->GridFs(db_name, gridfs_name);

        for (int idx = subbasin_start_id; idx <= n_subbasins; idx++) {
            if (fdtype == FD_D8) {
                GridLayeringD8* grid_lyr_d8 = new GridLayeringD8(idx, gfs, out_dir);
                grid_lyr_d8->Execute();
                delete grid_lyr_d8;
            } else if (fdtype == FD_Dinf) {
                GridLayeringDinf* grid_lyr_dinf = new GridLayeringDinf(idx, gfs, stream_file.c_str(), out_dir);
                grid_lyr_dinf->Execute();
                delete grid_lyr_dinf;
            } else if (fdtype == FD_MFDmd) {
                GridLayeringMFDmd* grid_lyr_mfdmd = new GridLayeringMFDmd(idx, gfs, stream_file.c_str(), out_dir);
                grid_lyr_mfdmd->Execute();
                delete grid_lyr_mfdmd;
            } else Usage("Unsupported flow direction algorithm!");
        }
        // clean up
        delete gfs;
        client->Destroy();
    }
#else
    Usage("The '-mongo' option require mongo-c-driver and MongoDB!");
#endif
    cout << "time-consuming: " << TimeCounting() - t1 << " seconds." << endl;
    return 0;
}
