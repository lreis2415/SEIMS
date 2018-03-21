// TODO: Make this program more common to use! lj
#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#include "CombineRaster.h"

int main() {
    int nSubbasins = 5;
    string folder = "D:\\test\\model_dianbu2_30m_demo\\OUTPUT0";
    string sVar = "SOLST_AVE";
    GDALAllRegister();
    //CombineRasterResults(folder, sVar, "tif", nSubbasins);
    // test mongo
    MongoClient *client = MongoClient::Init("127.0.0.1", 27017);
    MongoGridFS *gfs = new MongoGridFS(client->getGridFS("model_dianbu2_30m_demo", "OUTPUT"));
    CombineRasterResultsMongo(gfs, sVar, nSubbasins, folder);
    // clean up
    delete gfs;
    delete client;
    return 0;
}