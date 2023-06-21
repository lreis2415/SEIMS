/*!
 * \brief Calculate groundwater using reservoir method
 * \author Junzhi Liu
 * \date Oct. 2011
 * \revesed LiangJun Zhu
 * \note Change the module name from GWATER_RESERVOIR to GW_RSVR
 */
#ifndef SEIMS_MODULE_GW_RSVR_H
#define SEIMS_MODULE_GW_RSVR_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

/** \defgroup GW_RSVR
 * \ingroup Hydrology
 * \brief Calculate groundwater using reservoir method
 *
 */
/*!
 * \class GWaterReservoir
 * \ingroup GW_RSVR
 *
 * \brief Calculate groundwater using reservoir method
 *
 */
class GWaterReservoir : public SimulationModule {
public:
    //! Constructor
    GWaterReservoir(void);

    //! Destructor
    ~GWaterReservoir(void);

    void Set1DData(const char *key, int n, FLTPT *data) OVERRIDE;

    void SetValue(const char *key, FLTPT value) OVERRIDE;

    void Get1DData(const char *key, int *n, FLTPT **data) OVERRIDE;

    void SetReaches(clsReaches *reaches) OVERRIDE;

    int Execute(void) OVERRIDE;

    //virtual TimeStepType GetTimeStepType()
    //{
    //	return TIMESTEP_CHANNEL;
    //};

private:
    /**
    *	@brief check the input data. Make sure all the input data is available.
    *
    *	@return bool The validity of the input data.
    */
    bool CheckInputData(void);

    /**
    *	@brief check the input size. Make sure all the input data have same dimension.
    *
    *	@param key: The key of the input data
    *	@param n: The input data dimension
    *	@return bool The validity of the dimension
    */
    bool CheckInputSize(const char *, int n);

    void InitOutputs(void);

private:

    /// time step(seconds)
    FLTPT m_dt;

    /// count of valid cells
    int m_nCells;

    /// count of reaches
    int m_nReaches;

    /// cell area of the unit (m^2)
    FLTPT* m_cellArea;

    /// subbasin ID, 0 for entire watershed
    int m_subbasinID;

    /// subbasin grid
    FLTPT *m_subbasin;

    /// percolation from soil layer (mm)
    FLTPT *m_recharge;
    /// the amount of water recharged to the deep groundwater reservoir
    FLTPT m_deepCoefficient;

    /// baseflow recession coefficient
    FLTPT m_recessionCoefficient;

    /// baseflow recession exponent
    FLTPT m_recessionExponent;

    /// ground water storage (mm)
    FLTPT *m_storage;
    /// Maximum groundwater storage
    FLTPT m_storageMax;

    /// outflow at basin outlet
    FLTPT m_qOutlet;

    FLTPT *m_qg;

    /// temporary variables
    FLTPT *m_percSubbasin;
    int *m_nCellsSubbasin;
    FLTPT m_initStorage;
};
#endif /* SEIMS_MODULE_GW_RSVR_H */
