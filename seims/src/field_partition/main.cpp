#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#include "FieldPartition.h"
#include "CellOrdering.h"

using namespace std;

int main(int argc, char **argv) {
    GDALAllRegister();
    /// Header print information
    cout << "                    Field Partition Program 1.3                        " << endl;
    cout << "                    Author: Hui Wu, Liang-Jun Zhu                      " << endl;

    int Threshod = 50;
    FlowDirectionMethod flowDirMtd = (FlowDirectionMethod) 0;
    double start = TimeCounting();
    if (argc < 11) {
        cout << "Error: To run this field partition program, please follow the COMMAND." << endl;
        printUsage();
    }
    int i = 1;
    char demfile[PATH_MAX], lufile[PATH_MAX], mfile[PATH_MAX], dirfile[PATH_MAX], strmfile[PATH_MAX];
    while (argc > i) {
        if (strcmp(argv[i], "-dem") == 0) {
            i++;
            if (argc > i) {
                strcpy(demfile, argv[i]);
                if (!FileExists(demfile)) printUsage();
                i++;
            } else { printUsage(); }
        } else if (strcmp(argv[i], "-lu") == 0) {
            i++;
            if (argc > i) {
                strcpy(lufile, argv[i]);
                if (!FileExists(lufile)) printUsage();
                i++;
            } else { printUsage(); }
        } else if (strcmp(argv[i], "-mask") == 0) {
            i++;
            if (argc > i) {
                strcpy(mfile, argv[i]);
                if (!FileExists(mfile)) printUsage();
                i++;
            } else { printUsage(); }
        } else if (strcmp(argv[i], "-flow") == 0) {
            i++;
            if (argc > i) {
                strcpy(dirfile, argv[i]);
                if (!FileExists(dirfile)) printUsage();
                i++;
            } else { printUsage(); }
        } else if (strcmp(argv[i], "-stream") == 0) {
            i++;
            if (argc > i) {
                strcpy(strmfile, argv[i]);
                if (!FileExists(strmfile)) printUsage();
                i++;
            } else { printUsage(); }
        } else if (strcmp(argv[i], "-t") == 0) {
            i++;
            if (argc > i) {
                Threshod = atoi(argv[i]);
                i++;
            } else { printUsage(); }
        } else if (strcmp(argv[i], "-arcgis") == 0) {
            i++;
            flowDirMtd = (FlowDirectionMethod) 1;
        } else { printUsage(); }
    }

    DoFieldsPartition(dirfile, lufile, mfile, demfile, strmfile, flowDirMtd, Threshod);

    double duration = TimeCounting() - start;
    cout << "Finished! Time-consuming (sec): " << duration << endl;
    return 0;
}

void printUsage() {
    cout << " Command: fieldpartition -dem <dem> -lu <landuse> -mask <mask> "
        " -stream <stream> [-t <threshold>] [-arcgis]" << endl;
    cout << "\t1. Input raster paths include dem, mask, landuse, flow_dir, "
        "stream_link, the format can be ASC or GTIFF; " << endl;
    cout << "\t2. Threshold MUST be greater than 0, the default is 50;" << endl;
    cout << "\t3. -arcgis indicate ArcGIS Flow direction coding method, the default is TauDEM." << endl;
    exit(0);
}

void findOutlet(FloatRaster *rsDEM, IntRaster *rsStreamLink, IntRaster *rsDir,
                FlowDirectionMethod flowDirMtd, int &rowIndex, int &colIndex) {
    map<int, int> dirToIndexMap;
    if (flowDirMtd) {
        /// flow direction in ArcGIS
        dirToIndexMap[1] = 0;
        dirToIndexMap[2] = 1;
        dirToIndexMap[4] = 2;
        dirToIndexMap[8] = 3;
        dirToIndexMap[16] = 4;
        dirToIndexMap[32] = 5;
        dirToIndexMap[64] = 6;
        dirToIndexMap[128] = 7;
    } else {
        /// TauDEM
        dirToIndexMap[1] = 0;
        dirToIndexMap[8] = 1;
        dirToIndexMap[7] = 2;
        dirToIndexMap[6] = 3;
        dirToIndexMap[5] = 4;
        dirToIndexMap[4] = 5;
        dirToIndexMap[3] = 6;
        dirToIndexMap[2] = 7;
    }
    if (rsDEM->getRows() <= 0 || rsDEM->getCols() <= 0) {
        cout << "Error: the input of DEM was invalid!\n";
        exit(-1);
    }
    /// updated by Liangjun Zhu, Apr. 1, 2016
    bool flag = false;
    for (int i = 0; i < rsStreamLink->getRows(); i++) {
        for (int j = 0; j < rsStreamLink->getCols(); j++) {
            if (!rsStreamLink->isNoData(i, j) && rsStreamLink->getValue(i, j) > 0) {
                colIndex = j;
                rowIndex = i;
                flag = true;
                break;
            }
        }
        if (flag) {
            break;
        }
    }
    /// first stream grid
    /// cout<<rowIndex<<","<<colIndex<<endl;
    flag = true;
    while (flag) {
        int index = dirToIndexMap[rsDir->getValue(rowIndex, colIndex)];
        int ii = rowIndex + CellOrdering::m_d1[index];
        int jj = colIndex + CellOrdering::m_d2[index];
        if (ii < rsDEM->getRows() - 1 && jj < rsDEM->getCols() - 1) {
            if (rsStreamLink->isNoData(ii, jj) || rsStreamLink->getValue(ii, jj) <= 0) {
                flag = false;
            } else {
                rowIndex = ii;
                colIndex = jj;
            }
        } else {
            flag = false;
        }
    }
    cout << "\t\tOutlet location: row is " << rowIndex << ", col is " << colIndex << endl;
}

void DoFieldsPartition(const char *dirName, const char *LanduName, const char *maskName,
                       const char *demName, const char *streamLinkName,
                       FlowDirectionMethod flowDirMtd, int threshod) {
    //output to tiff
    string dir = GetPathFromFullName(demName);
    ostringstream oss;
    oss.str("");
    //oss << dir << "field_"<<threshod<<"."<<GetLower(GetSuffix(dirName));
    oss << dir << "fields_" << threshod << ".tif";
    string rsfieldFile = oss.str();

    oss.str("");
    oss << dir << "fields_" << threshod << ".txt";
    string txtfileFile = oss.str();

    /// Write basin input and output information
    cout << "Input Files:" << endl;
    cout << "\tFlow Direction: " << dirName << endl;
    cout << "\tLanduse: " << LanduName << endl;
    cout << "\tStream Links: " << streamLinkName << endl;
    cout << "\tDEM: " << demName << endl;
    cout << "\tMask: " << maskName << endl << endl;
    cout << "Output Files:" << endl;
    cout << "\tFields Raster: " << rsfieldFile << endl;
    cout << "\tFields Flow Relationship: " << txtfileFile << endl << endl;

    cout << "Executing ..." << endl;
    cout << "\tRead input raster files..." << endl;

    IntRaster *rsMask = IntRaster::Init(maskName, false);
    //cout<<"xll: "<<rsMask->getXllCenter()<<", yll: "<<rsMask->getYllCenter()<<endl;
    IntRaster *rsDir = IntRaster::Init(dirName, false);
    IntRaster *rsLandu = IntRaster::Init(LanduName, false);
    IntRaster *rsStrLink = IntRaster::Init(streamLinkName, false);
    FloatRaster *rsDEM = FloatRaster::Init(demName, false);

    if (nullptr == rsMask || nullptr == rsDir || nullptr == rsLandu ||
        nullptr == rsStrLink || nullptr == rsDEM) {
        cout << "Read raster data failed, please check!" << endl;
        exit(-1);
    }

    int rowIndex, colIndex;
    cout << "\tFind outlet location..." << endl;
    findOutlet(rsDEM, rsStrLink, rsDir, flowDirMtd, rowIndex, colIndex);
    cout << "\tInitiate field partition class ..." << endl;
    CellOrdering cellOrdering(rsDir, rsLandu, rsMask, flowDirMtd, threshod);
    cout << "\tExecute field partition ..." << endl;
    cellOrdering.ExcuteFieldsDis(rowIndex, colIndex);
    cout << "\tWrite output fields raster and flow relationship file ..." << endl;
    cellOrdering.OutputFieldMap(rsfieldFile.c_str());
    cellOrdering.OutputFieldRelationship(txtfileFile.c_str());

    delete rsMask;
    delete rsDir;
    delete rsLandu;
    delete rsStrLink;
    delete rsDEM;
}
