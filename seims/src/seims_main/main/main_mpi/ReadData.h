#ifndef SEIMS_MPI_READDATA_H
#define SEIMS_MPI_READDATA_H

#include "clsReach.h"

#include "mongoc.h"
#include "gdal.h"
#include "ogrsf_frmts.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

/*!
 * \brief Simple struct of subbasin information for task allocation
 * \TODO Should this SubbasinStruct be integrated into Subbasin class defined in clsSubbasin of data module? by LJ
 */
struct SubbasinStruct {
    SubbasinStruct(int sid, int gidx) : id(sid), group(gidx),
                                        updown_order(-1), downup_order(-1), calculated(false),
                                        qOutlet(0.f),
                                        downStream(nullptr) {
        upStreams.clear();
    }
    int id; ///< Subbasin ID, start from 1
    int group; ///< Group index, start from 0 to (group number - 1)

    int updown_order; ///< up-down stream order
    int downup_order; ///< down-up stream order
    bool calculated; ///< whether this subbasin is already calculated

    /****** Parameters need to transferred among subbasins *******/
    float qOutlet; ///< flow out the subbasin outlet

    SubbasinStruct *downStream; ///< down stream subbasin \sa SubbasinStruct
    vector<SubbasinStruct *> upStreams; ///< up stream subbasins
};

/*!
 * \brief Read reach table from MongoDB and create reach topology for task allocation.
 */
int CreateReachTopology(MongoClient *client, string &dbname, string &group_method, int group_size,
                        map<int, SubbasinStruct *> &subbasins, set<int> &group_set);

#endif /* SEIMS_MPI_READDATA_H */
