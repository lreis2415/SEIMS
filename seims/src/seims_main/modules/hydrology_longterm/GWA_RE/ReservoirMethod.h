/**
*	@version	1.0
*	@author    Wu Hui
*	@date	24-January-2011
*
*	@brief	Reservoir Method to calculate groundwater balance and baseflow
*
*	Revision: Zhiqiang Yu
*   Date:	  2011-2-11
*
*	Revision: Zhiqiang Yu
*	Date:	  2011-2-18
*	Description:
*	1.  Add judgment to calculation of EG (Revap). The average percolation of
*		one subbasin is first calculated. If the percolation is less than 0.01,
*		EG is set to 0 directly. (in function setInputs of class subbasin)
*	2.	Add member variable m_isRevapChanged to class subbasin. This variable
*		is the flag whether the Revap is changed by current time step. This flag
*		can avoid repeating setting values when converting subbasin average Revap
*		to cell Revap.(in function Execute of class ReservoirMethod)
*
*	Revision:	Zhiqiang Yu
*	Date:		2011-3-14
*	Description:
*	1.	Add codes to process the groundwater which comes from bank storage in
*		channel routing module. The water volume of this part of groundwater is
*		added to the groundwater storage. The input variable "T_GWNEW" is used
*		for this purpose. One additional parameter is added to function setInputs
*		of class subbasin. See equation 8 in memo "Channel water balance" for detailed
*		reference.
*
*	Revision:	Liang-Jun Zhu
*	Date:		2016-7-27
*	Description:
*	1.	Move subbasin class to base/data module for sharing with other modules
*/
#ifndef SEIMS_MODULE_GWA_RE_H
#define SEIMS_MODULE_GWA_RE_H

#include "SimulationModule.h"
#include "clsSubbasin.h"

/** \defgroup GWA_RE
 * \ingroup Hydrology_longterm
 * \brief Reservoir Method to calculate groundwater balance and baseflow of longterm model
 *
 */

/*!
 * \class ReservoirMethod
 * \ingroup GWA_RE
 * \brief Reservoir Method to calculate groundwater balance and baseflow of longterm model
 *
 */
class ReservoirMethod: public SimulationModule {
public:
    ReservoirMethod();

    ~ReservoirMethod();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    void SetSubbasins(clsSubbasins*) OVERRIDE;

    int Execute() OVERRIDE;

    void GetValue(const char* key, float* value) OVERRIDE;

    void Get1DData(const char* key, int* nRows, float** data) OVERRIDE;

    void Get2DData(const char* key, int* nRows, int* nCols, float*** data) OVERRIDE;

private:

    /**
    *	@brief check the input data. Make sure all the input data is available.
    *
    *	@return bool The validity of the input data.
    */
    bool CheckInputData();

    /**
    *	@brief check the input size. Make sure all the input data have same dimension.
    *
    *	@param key: The key of the input data
    *	@param n: The input data dimension
    *	@return bool The validity of the dimension
    */
    bool CheckInputSize(const char* key, int n);

    /*
     * \brief initial outputs as default values
     */
    void InitialOutputs();

    /*
     * \brief Set groundwater related subbasin parameters
     * \sa Subbasin
     * \sa clsSubbasins
     */
    void SetSubbasinInfos();

private:
    //inputs

    //! time step, second
    int m_dt;
    //! Valid cells number
    int m_nCells;
    //! cell size of the grid (m)
    float m_cellWth;
    //! maximum soil layers number
    int m_nMaxSoilLayers;
    //! soil layers number of each cell
    float* m_nSoilLyrs;
    //! soil thickness of each layer
    float** m_soilThk;

    //float m_upSoilDepth;

    //! groundwater Revap coefficient
    float m_dp_co;
    //! baseflow recession coefficient
    float m_Kg;
    //! baseflow recession exponent
    float m_Base_ex;
    //! the amount of water percolated from the soil water reservoir and input to the groundwater reservoir from the percolation module(mm)
    float** m_soilPerco;
    //! evaporation from interception storage (mm) from the interception module
    float* m_IntcpET;
    //! evaporation from the depression storage (mm) from the depression module
    float* m_deprStoET;
    //! evaporation from the soil water storage (mm) from the soil ET module
    float* m_soilET;
    //! actual amount of transpiration (mm H2O)
    float* m_actPltET;
    //! PET(mm) from the PET modules
    float* m_pet;
    //! initial ground water storage (or at time t-1)
    float m_GW0;
    //! maximum ground water storage
    float m_GWMAX;
    //!
    float* m_petSubbasin;
    //!
    float* m_gwStore;

    /// slope (percent, or drop/distance, or tan) of each cell
    float* m_slope;

    //! soil storage
    float** m_soilWtrSto;
    //! soil depth of each layer, the maximum soil depth is used here, i.e., m_soilDepth[i][(int)m_soilLayers[i]]
    float** m_soilDepth;
    //! ground water from bank storage, passed from channel routing module
    float* m_VgroundwaterFromBankStorage;

    //output
    //!
    float* m_T_Perco;
    //!
    float* m_T_PerDep;
    //!
    float* m_T_RG;
    //!
    float* m_T_QG;
    //!
    float* m_D_Revap;
    //!
    float* m_T_Revap;
    //! groundwater water balance statistics
    float** m_T_GWWB;

    ////! subbasin grid
    //   float *m_subbasin;
    //! subbasin number
    int m_nSubbsns;
    //! current subbasin ID, 0 for the entire watershed
    int m_inputSubbsnID;
    //! subbasin IDs
    vector<int> m_subbasinIDs;
    ////! selected count of output subbasin
    //   int m_subbasinSelectedCount;
    ////! subbasin selected to output
    //   float *m_subbasinSelected;
    bool m_firstRun;
    //! All subbasins information,\sa clsSubbasins, \sa Subbasin
    clsSubbasins* m_subbasinsInfo;
    ////! vector of all Subbasin instances
    // vector<Subbasin *> m_subbasinList;

};
#endif /* SEIMS_MODULE_GWA_RE_H */
