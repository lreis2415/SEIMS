#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#include "CombineRaster.h"

void Usage(const string& error_msg = "") {
    cout << " Usage: combine_raster -var <variable name> -n <subbasin count> "
            "[-file <dirpath> <suffix>] "
            "[-mongo <ip> <port> <database name> <gridfs name> [dirpath]] "
            "[-sce <scenario id>] [-cali <calibration id>]" << endl << endl;
    cout << "Example.1. combine_raster -var lai -n 5 -file d:/test/split_rasters tif" << endl;
    cout << "Example.2. combine_raster -var lai -n 5 -mongo 127.0.0.1 27017 "
            "model_dianbu2_30m_demo OUTPUT d:/test/combined_raster -sce 12 -cali -1" << endl << endl;
    if (!error_msg.empty()) {
        cout << "FAILURE: " << error_msg << endl;
    }
    exit(1);
}

int main(int argc, char** argv) {
#ifdef USE_GDAL
    GDALAllRegister();
#endif
    int n_subbasins = -1;
    string s_var;
    bool from_file = false;
    string folder;
    string suffix;
    bool from_mongo = false;
    string mongodb_ip;
    int port = -1;
    string db_name;
    string gridfs_name;
    int scenario_id = 0;
    int calibration_id = -1;
    /// Parse input arguments.
    if (argc < 7) {
        Usage("Too few arguments to run this program.");
    }
    int i = 1;
    while (argc > i) {
        if (StringMatch(argv[i], "-var")) {
            i++;
            if (argc > i) {
                s_var = argv[i];
                i++;
            } else { Usage("No argument followed '-var'!"); }
        } else if (StringMatch(argv[i], "-n")) {
            i++;
            if (argc > i) {
                n_subbasins = ToInt(argv[i]);
                i++;
            } else { Usage("No argument followed '-n'!"); }
        } else if (StringMatch(argv[i], "-file")) {
            if (argc < i + 2) {
                Usage("Two arguments are required followed '-file'!");
            }
            from_file = true;
            folder = argv[i + 1];
            suffix = argv[i + 2];
            i += 3;
        } else if (StringMatch(argv[i], "-mongo")) {
            if (argc < i + 4) { // at least 4 arguments
                Usage("Four arguments are required followed '-mongo'!");
            }
            from_mongo = true;
            mongodb_ip = argv[i + 1];
            if (!IsIpAddress(mongodb_ip.c_str())) {
                Usage("The input MongoDB Host is not a valid IP address!");
            }
            port = ToInt(argv[i + 2]);
            db_name = argv[i + 3];
            gridfs_name = argv[i + 4];
            i += 5;
            if (argc >= i && argv[i][0] != '-') {
                // folder is specified
                folder = argv[i];
                i++;
            }
        } else if (StringMatch(argv[i], "-sce")) {
            i++;
            if (argc > i) {
                scenario_id = ToInt(argv[i]);
                i++;
            }
        } else if (StringMatch(argv[i], "-cali")) {
            i++;
            if (argc > i) {
                calibration_id = ToInt(argv[i]);
                i++;
            }
        }
    }
    if (n_subbasins < 0) {
        Usage("Subbasin count must be greater than 0!");
    }
    if (from_file) {
        CombineRasterResults(folder, s_var, suffix, n_subbasins);
    }
    if (from_mongo) {
        MongoClient* client = MongoClient::Init(mongodb_ip.c_str(), port);
        if (nullptr == client) {
            Usage("Unable to connect to MongoDB!");
        }
        MongoGridFs* gfs = new MongoGridFs(client->GetGridFs(db_name, gridfs_name));
        CombineRasterResultsMongo(gfs, s_var, n_subbasins, folder, scenario_id, calibration_id);
        // clean up
        delete gfs;
        client->Destroy();
    }
    return 0;
}
