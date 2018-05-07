/*!
 * \brief Grid layering based on D8 model
 * \author originally by Junzhi Liu
 * \author Liangjun Zhu
 * \date 12-28-2017
 */
#ifndef GRID_LAYERING_D8_H
#define GRID_LAYERING_D8_H
#include "GridLayering.h"

class GridLayeringD8: public GridLayering {
public:
    GridLayeringD8(int id, MongoGridFs* gfs, const char* out_dir);

    virtual ~GridLayeringD8() {
    };
    bool LoadData() OVERRIDE;
    bool OutputFlowOut() OVERRIDE;
};

#endif /* GRID_LAYERING_D8_H */
