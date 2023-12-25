/*!
 * \file OLR_CIUH_GAMMA.h
 * \brief 
 * \author Yujing Wang
 * \date 2023-06-15
 *
 */

#ifndef SEIMS_MODULE_OLR_CIUH_GAMMA_H
#define SEIMS_MODULE_OLR_CIUH_GAMMA_H

#include "Convolve.h"
#include "SimulationModule.h"

class OLR_CIUH_GAMMA : public SimulationModule {
private:
    FLTPT* m_gammaScale;
    FLTPT* m_gammaShape;

    ConvolveTransporterGAMMA* m_convolveTransporter;

    int m_nCells;

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

    /// cell area of the unit (m^2)
    FLTPT* m_cellArea;

    /// surface runoff
    FLTPT* m_surfaceRunoff;

    FLTPT* m_Q_SBOF;
public:
    OLR_CIUH_GAMMA();
    ~OLR_CIUH_GAMMA() OVERRIDE;
    void InitUnitHydrograph();

    void InitialOutputs() OVERRIDE;
    bool CheckInputData() OVERRIDE;
    bool CheckInputSize(const char* key, int n) OVERRIDE;
    int Execute(void) OVERRIDE;

    void SetValue(const char* key, int value) OVERRIDE;
    void Set1DData(const char* key, int nRows, FLTPT* data)  OVERRIDE;
    void Set1DData(const char* key, int n, int* data) OVERRIDE;
    void SetSubbasins(clsSubbasins* subbsns) OVERRIDE;

    void Get1DData(const char* key, int* nRows, FLTPT** data) OVERRIDE;


};
#endif /* OLR_CIUH_GAMMA */
