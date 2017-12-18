/*!
 * \brief Fields partition incorporating spatial topology.
 * \refers Wu, Hui, A.-Xing Zhu, Jun-Zhi Liu, Yong-Bo Liu, and Jing-Chao Jiang. 2017.
                 "Best Management Practices Optimization at Watershed Scale: Incorporating
                  Spatial Topology among Fields." Water Resources Management,
                  doi: 10.1007/s11269-017-1801-8.
 * \authors Hui Wu, Liangjun Zhu
 */
#ifndef FIELD_PARTITION_HEADER
#define FIELD_PARTITION_HEADER

#include "clsRasterData.h"

enum FlowDirectionMethod {
    TauDEM = 0,
    ArcGIS = 1
};

#define IntRaster   clsRasterData<int>
#define FloatRaster clsRasterData<float>

/// Find outlet location according to flow direction, stream link, and DEM. Updated by LJ.
void findOutlet(FloatRaster *rsDEM, IntRaster *rsStreamLink, IntRaster *rsDir,
                FlowDirectionMethod flowDirMtd, int &rowIndex, int &colIndex);

/// Do field partition mission.
void DoFieldsPartition(const char *dirName, const char *LanduName, const char *maskName,
                       const char *demName, const char *streamLinkName,
                       FlowDirectionMethod flowDirMtd, int threshod);

void printUsage();
#endif
