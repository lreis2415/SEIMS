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
 *   - 3. 2022-08-22 - lj - Change float to FLTPT.
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

    void SetValue(const char* key, FLTPT value) OVERRIDE;

    void SetValue(const char* key, int value) OVERRIDE;

    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    void Set1DData(const char* key, int n, int* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, FLTPT** data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, int** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void GetValue(const char* key, FLTPT* value) OVERRIDE;

    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

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
    FLTPT m_cellWth;
    /// cell area, ha
    FLTPT m_cellArea;
    /// number of cells
    int m_nCells;
    /// soil layers
    int* m_nSoilLyrs;
    /// maximum soil layers
    int m_maxSoilLyrs;
    /// stream link
    int* m_rchID;
    /* carbon modeling method
     *   = 0 Static soil carbon (old mineralization routines)
     *   = 1 C-FARM one carbon pool model
     *   = 2 Century model
     */
    int m_cbnModel;

    /// input data

    /// factor which converts kg/kg soil to kg/ha
    FLTPT** m_cvtWt;
    /// drainage tile flow in soil profile
    FLTPT m_qtile;
    /// Phosphorus soil partitioning coefficient
    FLTPT m_phoskd;
    /// phosphorus percolation coefficient (0-1)
    FLTPT m_pperco;
    /// nitrate percolation coefficient (0-1)
    FLTPT m_nperco;
    /// Conversion factor from CBOD to COD
    FLTPT m_cod_n;
    /// Reaction coefficient from CBOD to COD
    FLTPT m_cod_k;

    /// soil loss caused by water erosion in overland
    FLTPT* m_olWtrEroSed;
    // fraction of porosity from which anions are excluded
    FLTPT* m_anionExclFr;
    // distribution of surface runoff generated
    FLTPT* m_surfRf;
    /// initial septic operational condition (active-1, failing-2, non_septic-0)
    int m_isep_opt;
    /// soil layer where drainage tile is located
    int* m_drainLyr;
    /// crack volume potential of soil
    FLTPT* m_soilCrk;
    /// distance to the downstream reach
    FLTPT* m_distToRch;
    /// amount of water held in the soil layer at saturation
    FLTPT** m_soilSat;

    /// lateral flow in soil layer
    FLTPT** m_subSurfRf;
    /// percolation from soil layer
    FLTPT** m_soilPerco;
    /// bulk density of the soil
    FLTPT** m_soilBD;
    /// depth to bottom of soil layer, sol_z in SWAT
    FLTPT** m_soilDepth;

    /// flow out indexes
    int** m_flowOutIdx;
    /// flow out fractions
    FLTPT** m_flowOutFrac;
    /**
    *	@brief Routing layers according to the flow direction
    *
    *	There are not flow relationships within each layer.
    *	The first element in each layer is the number of cells in the layer
    */
    int** m_rteLyrs;
    /// number of routing layers
    int m_nRteLyrs;
    /// amount of organic nitrogen in surface runoff
    FLTPT* m_sedorgn;
    /// average air temperature
    FLTPT* m_meanTemp;
    ///percent organic carbon in soil layer (%)
    FLTPT** m_soilCbn;
    /// soil thick of each layer (mm)
    FLTPT** m_soilThk;

    /// output data
    /// amount of nitrate transported with lateral flow, kg/ha
    FLTPT* m_latNO3;
    /// amount of nitrate percolating past bottom of soil profile, kg/ha
    FLTPT* m_percoN;
    /// amount of solute P percolating past bottom of soil profile
    FLTPT* m_percoP;
    /// amount of nitrate transported with surface runoff, kg/ha
    FLTPT* m_surfRfNO3;
    /// amount of ammonian transported with surface runoff, kg/ha
    FLTPT* m_surfRfNH4;
    /// amount of soluble phosphorus in surface runoff
    FLTPT* m_surfRfSolP;
    /// carbonaceous oxygen demand of surface runoff
    FLTPT* m_surfRfCod;
    /// chlorophyll-a concentration in water yield
    FLTPT* m_surfRfChlA;
    /// dissolved oxygen concentration in the surface runoff
    //FLTPT* m_doxq;
    /// dissolved oxygen saturation concentration
    //FLTPT* m_soxy;

    // N and P to channel
    FLTPT* m_latNO3ToCh;     ///< amount of nitrate transported with lateral flow to channel, kg
    FLTPT* m_surfRfNO3ToCh;  ///< amount of nitrate transported with surface runoff to channel, kg
    FLTPT* m_surfRfNH4ToCh;  ///< amount of ammonian transported with surface runoff to channel, kg
    FLTPT* m_surfRfSolPToCh; ///< amount of soluble phosphorus in surface runoff to channel, kg
    FLTPT* m_percoNGw;       ///< amount of nitrate percolating past bottom of soil profile sum by sub-basin, kg
    FLTPT* m_percoPGw;       ///< amount of solute P percolating past bottom of soil profile sum by sub-basin, kg
    FLTPT* m_surfRfCodToCh;  ///< amount of COD to reach in surface runoff (kg)

    /// subbasin related
    /// the total number of subbasins
    int m_nSubbsns;
    //! subbasin IDs
    vector<int> m_subbasinIDs;
    /// subbasin grid (subbasins ID)
    int* m_subbsnID;
    /// subbasins information
    clsSubbasins* m_subbasinsInfo;

    /// input & output
    /// average annual amount of phosphorus leached into second soil layer
    FLTPT m_wshdLchP;

    /// amount of nitrogen stored in the nitrate pool in soil layer
    FLTPT** m_soilNO3;
    /// amount of phosphorus stored in solution
    FLTPT** m_soilSolP;

    /* CENTURY C/N cycling model related */
    /// amount of Carbon lost with sediment, kg/ha, input from NUTRSED module
    FLTPT* m_sedLossCbn;
};
#endif /* SEIMS_MODULE_NUTRMV_H */
