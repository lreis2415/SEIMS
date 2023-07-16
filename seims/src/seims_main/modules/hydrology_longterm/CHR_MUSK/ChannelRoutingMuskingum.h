/*!
 * \file ChannelRoutingMuskingum.h
 * \brief 
 * \author Yujing Wang
 * \date 2023-06-15
 *
 */

#ifndef SEIMS_CHANNEL_ROUTING_MUSKINGUM_H
#define SEIMS_CHANNEL_ROUTING_MUSKINGUM_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

class ChannelRoutingMuskingum : public SimulationModule {
private:
    bool m_isInitialized;

    int m_dt;  ///< time step (sec)
    int m_nCells;
    int m_nReaches;  ///< reach number (= subbasin number)
    
    int m_inputSubbasinId;  ///< current subbasin ID, 0 for the entire watershed
    int m_outletID; ///< outlet ID, also can be derived by m_reachLayers.rbegin()->second[0];
    int* m_reachDownStream; ///< downstream id (The value is -1 if there if no downstream reach)
    
    /*!
     * Index of upstream Ids (The value is -1 if there if no upstream reach)
     * m_reachUpStream.size() = N+1
     * m_reachUpStream[1] = [2, 3] means Reach 2 and Reach 3 flow into Reach 1.
     */
    vector<vector<int> > m_reachUpStream;
    
    /*!
     * reach layers
     * key: computing order, \sa LayeringMethod
     * value: reach ID
     */
    map<int, vector<int> > m_routeLayers;


    FLTPT K;
    FLTPT X;

    FLTPT* m_Q_SBOF;
    FLTPT* m_Q_in;  ///< reach inflow (m^3/s)
    FLTPT* m_Q_inLast;  ///< reach inflow of last timestep (m^3/s)
    FLTPT* m_Q_out;  ///< reach outflow (m^3/s)
    FLTPT* m_Q_outLast;  ///< reach outflow of last timestep (m^3/s)
    

public:
    ChannelRoutingMuskingum();
    ~ChannelRoutingMuskingum();
    TimeStepType GetTimeStepType() OVERRIDE { return TIMESTEP_CHANNEL; }

    void InitialOutputs() OVERRIDE;
    bool CheckInputData() OVERRIDE;


    void SetValue(const char *key, int value) OVERRIDE;
    void SetValue(const char* key, FLTPT value) OVERRIDE;
    void SetValueByIndex(const char* key, const int index, const FLTPT value) OVERRIDE;
    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;
    void SetReaches(clsReaches* reaches);

    void GetValue(const char* key, FLTPT* value) OVERRIDE;
    void Get1DData(const char *key, int *nRows, FLTPT **data) OVERRIDE;


    int Execute(void);
    void ChannelFlow(const int i);

};
#endif /* SEIMS_OVERLAND_ROUTING_MUSKINGUM_H */
