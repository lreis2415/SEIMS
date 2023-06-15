/*!
 * \file OverlandRoutingDump.h
 * \brief 
 * \author Yujing Wang
 * \date 2023-06-15
 *
 */

#ifndef SEIMS_OVERLAND_ROUTING_DUMP_H
#define SEIMS_OVERLAND_ROUTING_DUMP_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

class OverlandRoutingDump : public SimulationModule {
private:
    bool m_isInitialized;

    int m_nCells;
    /// time step (sec)
    int m_timeStep;

    //! number of subbasins
    int m_nSubbasins;
    //! subbasin IDs
    vector<int> m_subbasinIDs;
    //! All subbasins information
    clsSubbasins* m_subbasins;
    /// current subbasin ID, 0 for the entire watershed
    int m_inputSubbasinId;
    /// subbasin grid (subbasins ID)
    int* m_cellsMappingToSubbasinId;
    
    /// cell width of the grid (m)
    FLTPT m_cellWidth;
    /// cell area, BE CAUTION, the unit is m^2, NOT ha!!!
    FLTPT m_cellArea;

    /// surface runoff
    FLTPT* m_surfaceRunoff;

    FLTPT* m_Q_SBOF;
    FLTPT* m_Q_SB_ZEROS;

public:
    OverlandRoutingDump();
    ~OverlandRoutingDump();
    void InitialOutputs() OVERRIDE;
    bool CheckInputData() OVERRIDE;

    int Execute(void);

    void SetValue(const char *key, int value) OVERRIDE;
    void SetValue(const char *key, FLTPT value) OVERRIDE;
    void Set1DData(const char *key, int nRows, FLTPT *data)  OVERRIDE;
    void OverlandRoutingDump::Set1DData(const char* key, int n, int* data) OVERRIDE;
    void SetSubbasins(clsSubbasins* subbsns) OVERRIDE;

    void Get1DData(const char *key, int *nRows, FLTPT **data) OVERRIDE;
};
#endif /* SEIMS_OVERLAND_ROUTING_DUMP_H */
