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
*               29-Dec-2018  Refactor to make code more clearly
*---------------------------------------------------------------------*/

#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#include "GridLayeringD8.h"
#include "GridLayeringDinf.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc < 6) {
        cout << "usage: grid_layering <hostIP> <port> <output_dir> <modelName> <gridFSName> <nsubbasin>\n";
        exit(-1);
    }
    /// set default OpenMP thread number to improve compute efficiency
    SetDefaultOpenMPThread();
    /// Register GDAL drivers, REQUIRED!
    GDALAllRegister();

    const char *hostName = argv[1];
    int port = atoi(argv[2]);
    const char *outputDir = argv[3];
    const char *modelName = argv[4];
    const char *gridFSName = argv[5];
    int nSubbasins = atoi(argv[6]);

    /// connect to MongoDB
    MongoClient *client = MongoClient::Init(hostName, port);
    if (nullptr == client) exit(-1);
    MongoGridFS *gfs = client->GridFS(modelName, gridFSName);

    double t1 = TimeCounting();
    int subbasinStartID = 1;
    if (nSubbasins == 0) { subbasinStartID = 0; }

    for (int i = subbasinStartID; i <= nSubbasins; i++) {
        /// D8 flow model
        GridLayeringD8 *gridLyrD8 = new GridLayeringD8(i, gfs, outputDir);
        gridLyrD8->Execute();
        delete gridLyrD8;

        /// Dinf flow model
        GridLayeringDinf *gridLyrDinf = new GridLayeringDinf(i, gfs, outputDir);
        gridLyrDinf->Execute();
        delete gridLyrDinf;
    }

    delete gfs;
    delete client;

    cout << "time-consuming: " << TimeCounting() - t1 << " seconds." << endl;
    return 0;
}
