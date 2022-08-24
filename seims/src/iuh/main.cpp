#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */
#include "SubbasinIUHCalculator.h"

#include "basic.h"

void MainMongoDB(const char* modelStr, const char* gridFSName, int nSubbasins, const char* host, int port, int dt) {
    // connect to mongodb
    MongoClient* client = MongoClient::Init(host, port);
    if (nullptr == client) {
        throw ModelException("IUH", "MainFunc", "Failed to connect to MongoDB!");
    }
    MongoGridFs* gfs = new MongoGridFs(client->GetGridFs(string(modelStr), string(gridFSName)));
    int subbasinStartID = nSubbasins == 0 ? 0 : 1;
    for (int i = subbasinStartID; i <= nSubbasins; i++) {
        //cout << "subbasin: " << i << endl;
        //input
        std::ostringstream oss;
        string deltaName, streamLinkName, tName, maskName, subbsnName, landcoverName;
        oss << i << "_DELTA_S";
        deltaName = oss.str();

        oss.str("");
        oss << i << "_T0_S";
        tName = oss.str();

        oss.str("");
        oss << i << "_SUBBASIN";
        subbsnName = oss.str();

        oss.str("");
        oss << i << "_LANDCOVER";
        landcoverName = oss.str();

        FloatRaster* rsMask = FloatRaster::Init(gfs, subbsnName.c_str(), true);
        if (nullptr == rsMask) {
            cout << subbsnName << " cannot be found in MongoDB! IUH will be not calculated!\n";
            continue;
        }

        STRING_MAP opts;
        UpdateStringMap(opts, HEADER_INC_NODATA, "FALSE");

        FloatRaster* rsTime = FloatRaster::Init(gfs, tName.c_str(), true,
                                                          rsMask, true, NODATA_VALUE, opts);
        FloatRaster* rsDelta = FloatRaster::Init(gfs, deltaName.c_str(), true,
                                                           rsMask, true, NODATA_VALUE, opts);
        FloatRaster* rsLandcover = FloatRaster::Init(gfs, landcoverName.c_str(), true,
                                                               rsMask, true, NODATA_VALUE, opts);
        if (nullptr == rsTime || nullptr == rsDelta || nullptr == rsLandcover) {
            cout << "Required input raster cannot be satisfied!\n";
            continue;
        }
        SubbasinIUHCalculator iuh(dt, rsMask, rsLandcover, rsTime, rsDelta, gfs);
        iuh.calCell(i);

        delete rsLandcover;
        delete rsDelta;
        delete rsTime;
        delete rsMask;
    }
    delete gfs;
    delete client;
}

int main(int argc, const char** argv) {
    if (argc < 6) {
        cout << "Usage: " <<
                "IUH <MongoDB HOST IP> <PORT> <modelName> <GridFSName> <dateInterval> <nSubbasins>\n";
        exit(-1);
    }
    try {
        const char* host = argv[1];
        int port = atoi(argv[2]);
        const char* modelName = argv[3];
        const char* gridFSName = argv[4];
        int dt = atoi(argv[5]);         //time interval in hours
        int nSubbasins = atoi(argv[6]); // the whole basin is 0

        MainMongoDB(modelName, gridFSName, nSubbasins, host, port, dt);

        cout << " IUH calculation is OK!" << endl;
    } catch (ModelException& e) {
        cout << e.ToString() << endl;
        return -1;
    }
    catch (std::exception& e) {
        cout << e.what() << endl;
        return -1;
    }
    catch (...) {
        cout << "Unknown exception occurred!" << endl;
        return -1;
    }
    return 0;
}
