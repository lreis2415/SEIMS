#ifndef SEIMS_OVERLAND_ROUTING_DUMP_H
#define SEIMS_OVERLAND_ROUTING_DUMP_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

class OverlandRoutingDump : public SimulationModule {
private:

    int m_cellSize;

    //! number of subbasins
    int m_nSubbasins;
    //! subbasin IDs
    vector<int> m_subbasinIDs;
    //! All subbasins information
    clsSubbasins* m_subbasins;

    FLTPT* m_petSubbsn;
    FLTPT* m_gwSto;

public:
    OverlandRoutingDump();

    ~OverlandRoutingDump();

    virtual int Execute(void);

    virtual void Set1DData(const char *key, int nRows, FLTPT *data) OVERRIDE;

    virtual void SetValue(const char *key, FLTPT value);
    void SetSubbasins(clsSubbasins* subbsns);
    virtual void Get1DData(const char *key, int *nRows, FLTPT **data) OVERRIDE;
};
#endif /* SEIMS_OVERLAND_ROUTING_DUMP_H */
