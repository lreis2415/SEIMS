#if (defined _DEBUG) && (defined MSVC) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */
#include "clsRasterData.cpp"
#include "utilities.h"
#include "MongoUtil.h"

using namespace std;

int main(int argc, const char *argv[]) {
    GDALAllRegister();/// Register GDAL drivers, REQUIRED!
    SetDefaultOpenMPThread();
    std::cout << "*** Raster IO Class Demo ***\n";
    string apppath = GetAppPath();
    cout << apppath << endl;
    string ascdemfile = apppath + "../data/dem_1.asc";
    string ascdemfile2 = apppath + "../data/dem_2.asc";
    string ascdemfile3 = apppath + "../data/dem_3.asc";

    string ascmaskfile = apppath + "../data/mask1.asc";

    string ascdemout = apppath + "../data/raster1D_out.asc";
    string ascdemout3 = apppath + "../data/raster1D_out_directly.tif";
    string ascdemout4 = apppath + "../data/raster2D_out_directly.tif";
    string ascdemout2 = apppath + "../data/raster2D_out.asc";
    /******* ASCII 1D Raster Demo *********/
    cout << "--  ASCII 1D Raster Demo" << endl;
    /// 1. Constructor
    /// 1.1 Construct a void clsRasterData instance, and assign input file path or MongoDB GridFS later.
    clsRasterData<int> maskr;
    maskr.ReadASCFile(ascmaskfile);
    /// 1.2 Construct a clsRasterData instance from a full filename, with *.asc, *.tif, or others.
    clsRasterData<float, int> readr(ascdemfile, false, &maskr, false);
    /// 1.3 Construct a clsRasterData instance from data array and mask, and output directly.
    int validmaskcells = maskr.getCellNumber();
    float *vs = NULL;
    float **vs2 = NULL;
    Initialize1DArray(validmaskcells, vs, 0.f);
    Initialize2DArray(validmaskcells, 3, vs2, 0.f);
    for (int i = 0; i < validmaskcells; i++) {
        vs[i] = i;
        for (int j = 0; j < 3; j++) {
            vs2[i][j] = i * (j + 1);
        }
    }
    /// 1.4 Copy constructor
    clsRasterData<float, int> copyReadr(readr);
    /// output array to raster file and destructor the clsRasterData instance immediately
    clsRasterData<float, int>(&maskr, vs).outputToFile(ascdemout3);
    clsRasterData<float, int>(&maskr, vs2, 3).outputToFile(ascdemout4);
    Release1DArray(vs);
    Release2DArray(validmaskcells, vs2);
    /// 2. Get raster properties
    int cellnum = readr.getDataLength();
    int nrows = readr.getRows();
    int ncols = readr.getCols();
    cout << "Total cell number is: " << cellnum << ", row number is: " << nrows << ", col number is: " << ncols << endl;
    cout << "valid cell number: " << readr.getValidNumber() << endl;
    cout << "mean: " << readr.getAverage() << ", max: " << readr.getMaximum() << endl;
    cout << "min: " << readr.getMinimum() << ", std: " << readr.getSTD() << endl;
    /// get value
    cout << "value on (1, 1): " << readr.getValue(RowColCoor(1, 1)) << endl;
#if (!defined(MSVC) || _MSC_VER >= 1800)
    cout << "value on (1, 1), C++11 version: " << readr.getValue({1, 1}) << endl;
#endif /* C++11 supported in MSCV */
    cout << endl << endl;
    /// 3. Output raster to file
    readr.outputToFile(ascdemout);
    /// 4. Output raster data to MongoDB
    MongoClient client = MongoClient("127.0.0.1", 27017);
    mongoc_gridfs_t *gfs = client.getGridFS(string("test"), string("spatial"));
    readr.outputToMongoDB(string("testImport"), gfs);
    /******* ASCII 2D Raster Demo *********/
    cout << "--  ASCII 2D Raster Demo" << endl;
    /// 1. Constructor, same as the 1D raster demo, but with the vector as filenames input.
    vector<string> filenames;
    filenames.push_back(ascdemfile);
    filenames.push_back(ascdemfile2);
    filenames.push_back(ascdemfile3);
    // clsRasterData<int> maskr(ascmaskfile, true);
    clsRasterData<float> readr2D(filenames, false);
    for (int i = 1; i <= readr2D.getLayers(); i++) {
        cout << "Layer number: " << i << endl;
        cout << "  valid cell number: " << readr2D.getValidNumber(i) << endl;
        cout << "  valid cell number: " << readr2D.getValidNumber(i) << endl;
        cout << "  mean: " << readr2D.getAverage(i) << ", max: " << readr2D.getMaximum(i) << endl;
        cout << "  min: " << readr2D.getMinimum(i) << ", std: " << readr2D.getSTD(i) << endl;
    }
    /// get value
    int nlyrs = 0;
    float *cellvalues = NULL;
#if (!defined(MSVC) || _MSC_VER >= 1800)
    readr2D.getValue({5, 5}, &nlyrs, &cellvalues);
#else
    readr2D.getValue(RowColCoor(5, 5), &nlyrs, &cellvalues);
#endif /* C++11 supported in MSVC */
    if (nlyrs > 0 && cellvalues != NULL){
        cout << "there are " << nlyrs << " layers, and value on (1, 1) are: ";
        for (int i = 0; i < nlyrs; i++)
            cout << cellvalues[i] << ", ";
        cout << endl;
    }
    Release1DArray(cellvalues);
    readr2D.outputToFile(ascdemout2);
    /******* GDAL Raster Demo *********/
    cout << endl << endl;
    cout << "--  1D Raster Demo by GDAL" << endl;
    string demfile = apppath + "../data/dem_1.tif";
    string demfile2 = apppath + "../data/dem_2.tif";
    string demfile3 = apppath + "../data/dem_3.tif";

    string maskfile = apppath + "../data/mask1.tif";

    string demout = apppath + "../data/raster1D_out.tif";
    string demout2 = apppath + "../data/raster2D_out.tif";

    /// 1. Constructor
    /// 1.1 Construct a void clsRasterData instance, and assign input file path or MongoDB GridFS later.
    clsRasterData<int> gdalmaskr;
    gdalmaskr.ReadByGDAL(maskfile, true);
    /// 1.2 Construct a clsRasterData instance from a full filename, with *.asc, *.tif, or others.
    clsRasterData<float, int> gdalreadr(demfile, false, &gdalmaskr, false);
    /// 2. Get raster properties
    int gdalcellnum = gdalreadr.getDataLength();
    int gdalnrows = gdalreadr.getRows();
    int gdalncols = gdalreadr.getCols();
    cout << "Total cell number is: " << gdalcellnum << ", row number is: " << gdalnrows << ", col number is: "
         << gdalncols << endl;
    cout << "valid cell number: " << gdalreadr.getValidNumber() << endl;
    cout << "mean: " << gdalreadr.getAverage() << ", max: " << gdalreadr.getMaximum() << endl;
    cout << "min: " << gdalreadr.getMinimum() << ", std: " << gdalreadr.getSTD() << endl;
    cout << endl << endl;
    /// 3. Output raster to file
    gdalreadr.outputToFile(demout);

    cout << "--  2D Raster Demo by GDAL" << endl;
    /// 1. Constructor, same as the 1D raster demo, but with the vector as filenames input.
    vector<string> demfilenames;
    demfilenames.push_back(demfile);
    demfilenames.push_back(demfile2);
    demfilenames.push_back(demfile3);
    // clsRasterData<int> maskr(ascmaskfile, true);
    clsRasterData<float> gdalreadr2D(demfilenames);
    for (int i = 1; i <= gdalreadr2D.getLayers(); i++) {
        cout << "Layer number: " << i << endl;
        cout << "  valid cell number: " << gdalreadr2D.getValidNumber(i) << endl;
        cout << "  valid cell number: " << gdalreadr2D.getValidNumber(i) << endl;
        cout << "  mean: " << gdalreadr2D.getAverage(i) << ", max: " << gdalreadr2D.getMaximum(i) << endl;
        cout << "  min: " << gdalreadr2D.getMinimum(i) << ", std: " << gdalreadr2D.getSTD(i) << endl;
    }
    gdalreadr2D.outputToFile(demout2);
    /// Copy 2D raster
    clsRasterData<float> copied2DRaster;
    copied2DRaster.Copy(gdalreadr2D);
    copied2DRaster.setCoreName("copiedRaster");

    return 0;
}
