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
*               27-Otc-2020  Grid layering based on MFD-md algorithm
*---------------------------------------------------------------------*/

#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#include "utils_time.h"
#include "GridLayering.h"

using namespace utils_time;

int main(int argc, char** argv) {
    if (argc < 6) {
        cout << "usage: grid_layering <hostIP> <port> <output_dir> <modelName> <gridFSName> <nsubbasin>\n";
        exit(-1);
    }
    /// set default OpenMP thread number to improve compute efficiency
    SetDefaultOpenMPThread();
    /// Register GDAL drivers, REQUIRED!
    GDALAllRegister();
    char* end = nullptr;
    errno = 0;
    const char* host_name = argv[1];
    int port = strtol(argv[2], &end, 10);
    const char* output_dir = argv[3];
    const char* model_name = argv[4];
    const char* gridfs_name = argv[5];
    int n_subbasins = strtol(argv[6], &end, 10);

    /// connect to MongoDB
    MongoClient* client = MongoClient::Init(host_name, port);
    if (nullptr == client) exit(-1);
    MongoGridFs* gfs = client->GridFs(model_name, gridfs_name);

    double t1 = TimeCounting();
    int subbasin_start_id = 1;
    if (n_subbasins == 0) { subbasin_start_id = 0; }

    for (int i = subbasin_start_id; i <= n_subbasins; i++) {
        /// D8 flow model
        GridLayeringD8* grid_lyr_d8 = new GridLayeringD8(i, gfs, output_dir);
        grid_lyr_d8->Execute();
        delete grid_lyr_d8;

        /// Dinf flow model
        /// todo: I found the code may cause heap corruption because of writing out of the
        ///       allocated memory! This is caused by the very tiny flow direction partition!
        ///       In the future, this code should be carefully reviewed. By lj. 2018-8-26
        //GridLayeringDinf* grid_lyr_dinf = new GridLayeringDinf(i, gfs, output_dir);
        //grid_lyr_dinf->Execute();
        //delete grid_lyr_dinf;
    }

    delete gfs;
    client->Destroy();

    cout << "time-consuming: " << TimeCounting() - t1 << " seconds." << endl;
    return 0;
}
