/*!
 * \file NutrientMovementViaWater.h
 * \brief Simulates the loss of nitrate and phosphorus via surface runoff,
 *        lateral flow, tile flow, and percolation out of the profile.
 *        Method of SWAT.
 *
 * Changelog:
 *   - 1. 2016-05-30 - hr - Initial implementation.
 *   - 2. 2018-05-10 - lj -
 *        -# Reformat, especially naming style (sync update in "text.h").
 *        -# Code optimization, e.g., use multiply instead of divide.
 *
 * \author Huiran Gao, Liangjun Zhu
 */
#ifndef SEIMS_MODULE_NUTRMV_H
#define SEIMS_MODULE_NUTRMV_H

#include "SimulationModule.h"

/** \defgroup NutrMV
 * \ingroup Nutrient
 * \brief Simulates the loss of nitrate and phosphorus via surface runoff, lateral flow, tile flow, and percolation out of the profile.
 */

/*!
 * \class NutrientMovementViaWater
 * \ingroup NutrMV
 *
 * \brief Nutrient removed and loss in surface runoff, lateral flow, tile flow, and percolation out of the profile
 *
 */
class NutrientMovementViaWater: public SimulationModule {
public:
    NutrientMovementViaWater();

    ~NutrientMovementViaWater();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void GetValue(const char* key, float* value) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void SetSubbasins(clsSubbasins* subbasins) OVERRIDE;

private:
    /*!
    * \brief Calculate the loss of nitrate via surface runoff, lateral flow, tile flow,
    *          and percolation out of the profile.
    *        mainly rewrited from nlch.f of SWAT
    * 1. nitrate loss with surface flow
    * 2. nitrate loss with subsurface flow (routing considered)
    * 3. nitrate loss with percolation
    */
    void NitrateLoss(int i);

    /*!
    * \brief Calculates the amount of phosphorus lost from the soil
    *        profile in runoff and the movement of soluble phosphorus from the first
    *        to the second layer via percolation.
    *		 rewrite from solp.f of SWAT
    */
    void PhosphorusLoss(int i);

    /*
    * \brief compute loadings of chlorophyll-a, BOD, and dissolved oxygen to the main channel
    *        rewrite from subwq.f of SWAT
    */
    void SubbasinWaterQuality(int i);

    void SumBySubbasin();
private:
    /// cell width of grid map (m)
    float m_cellWth;
    /// cell area, ha
    float m_cellArea;
    /// number of cells
    int m_nCells;
    /// soil layers
    float* m_nSoilLyrs;
    /// maximum soil layers
    int m_maxSoilLyrs;
    /// stream link
    float* m_rchID;
    /* carbon modeling method
     *   = 0 Static soil carbon (old mineralization routines)
     *   = 1 C-FARM one carbon pool model
     *   = 2 Century model
     */
    int m_cbnModel;

    /// input data

    /// factor which converts kg/kg soil to kg/ha
    float** m_cvtWt;
    /// drainage tile flow in soil profile
    float m_qtile;
    /// Phosphorus soil partitioning coefficient
    float m_phoskd;
    /// phosphorus percolation coefficient (0-1)
    float m_pperco;
    /// nitrate percolation coefficient (0-1)
    float m_nperco;
    /// Conversion factor from CBOD to COD
    float m_cod_n;
    /// Reaction coefficient from CBOD to COD
    float m_cod_k;

    /// soil loss caused by water erosion in overland
    float* m_olWtrEroSed;
    // fraction of porosity from which anions are excluded
    float* m_anionExclFr;
    // distribution of surface runoff generated
    float* m_surfRf;
    /// initial septic operational condition (active-1, failing-2, non_septic-0)
    float m_isep_opt;
    /// soil layer where drainage tile is located
    float* m_drainLyr;
    /// crack volume potential of soil
    float* m_soilCrk;
    /// distance to the downstream reach
    float* m_distToRch;
    /// amount of water held in the soil layer at saturation
    float** m_soilSat;

    /// lateral flow in soil layer
    float** m_subSurfRf;
    /// percolation from soil layer
    float** m_soilPerco;
    /// bulk density of the soil
    float** m_soilBD;
    /// depth to bottom of soil layer, sol_z in SWAT
    float** m_soilDepth;

    /// flow out index
    float* m_flowOutIdxD8;
    /**
    *	@brief Routing layers according to the flow direction
    *
    *	There are not flow relationships within each layer.
    *	The first element in each layer is the number of cells in the layer
    */
    float** m_rteLyrs;
    /// number of routing layers
    int m_nRteLyrs;
    /// amount of organic nitrogen in surface runoff
    float* m_sedorgn;
    /// average air temperature
    float* m_meanTemp;
    ///percent organic carbon in soil layer (%)
    float** m_soilCbn;
    /// soil thick of each layer (mm)
    float** m_soilThk;

    /// output data
    /// amount of nitrate transported with lateral flow, kg/ha
    float* m_latNO3;
    /// amount of nitrate percolating past bottom of soil profile, kg/ha
    float* m_percoN;
    /// amount of solute P percolating past bottom of soil profile
    float* m_percoP;
    /// amount of nitrate transported with surface runoff, kg/ha
    float* m_surfRfNO3;
    /// amount of ammonian transported with surface runoff, kg/ha
    float* m_surfRfNH4;
    /// amount of soluble phosphorus in surface runoff
    float* m_surfRfSolP;
    /// carbonaceous oxygen demand of surface runoff
    float* m_surfRfCod;
    /// chlorophyll-a concentration in water yield
    float* m_surfRfChlA;
    /// dissolved oxygen concentration in the surface runoff
    //float* m_doxq;
    /// dissolved oxygen saturation concentration
    //float* m_soxy;

    // N and P to channel
    float* m_latNO3ToCh;     ///< amount of nitrate transported with lateral flow to channel, kg
    float* m_surfRfNO3ToCh;  ///< amount of nitrate transported with surface runoff to channel, kg
    float* m_surfRfNH4ToCh;  ///< amount of ammonian transported with surface runoff to channel, kg
    float* m_surfRfSolPToCh; ///< amount of soluble phosphorus in surface runoff to channel, kg
    float* m_percoNGw;       ///< amount of nitrate percolating past bottom of soil profile sum by sub-basin, kg
    float* m_percoPGw;       ///< amount of solute P percolating past bottom of soil profile sum by sub-basin, kg
    float* m_surfRfCodToCh;  ///< amount of COD to reach in surface runoff (kg)

    /// subbasin related
    /// the total number of subbasins
    int m_nSubbsns;
    //! subbasin IDs
    vector<int> m_subbasinIDs;
    /// subbasin grid (subbasins ID)
    float* m_subbsnID;
    /// subbasins information
    clsSubbasins* m_subbasinsInfo;

    /// input & output
    /// average annual amount of phosphorus leached into second soil layer
    float m_wshdLchP;

    /// amount of nitrogen stored in the nitrate pool in soil layer
    float** m_soilNO3;
    /// amount of phosphorus stored in solution
    float** m_soilSolP;

    /* CENTURY C/N cycling model related */
    /// amount of Carbon lost with sediment, kg/ha, input from NUTRSED module
    float* m_sedLossCbn;
};
#endif /* SEIMS_MODULE_NUTRMV_H */
