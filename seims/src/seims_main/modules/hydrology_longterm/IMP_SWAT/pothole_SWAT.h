/*!
 * \file pothole_SWAT.h
 * \brief Simulates depressional areas that do not drain to the stream network (pothole)
 *          and impounded areas such as rice paddies
 *
 * Changlog:
 *   - 1. 2016-09-27 - lj -
 *        -# Source code of SWAT include: pothole.f
 *        -# Add the simulation of Ammonia n transported with surface runoff
 *        -# Add m_depEvapor and m_depStorage from DEP_LENSLEY module
 *        -# Using a simple model (first-order kinetics equation) to simulate N transformation in impounded area.
 *   - 2. 2016-10-10 - lj - Update all related variables after the simulation of pothole.
 *   - 3. 2017-08-23 - lj - Solve inconsistent results when using openmp to reducing raster data according to subbasin ID.
 *   - 4. 2022-08-22 - lj - Change float to FLTPT.
 *
 * \author Liang-Jun Zhu
 */
#ifndef SEIMS_MODULE_IMP_SWAT_H
#define SEIMS_MODULE_IMP_SWAT_H

#include "SimulationModule.h"

class IMP_SWAT: public SimulationModule {
public:
    IMP_SWAT();

    ~IMP_SWAT();

    void SetValue(const char* key, FLTPT value) OVERRIDE;

    void SetValue(const char* key, int value) OVERRIDE;

    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    void Set1DData(const char* key, int n, int* data) OVERRIDE;

    void Set2DData(const char* key, int n, int col, FLTPT** data) OVERRIDE;

    void Set2DData(const char* key, int n, int col, int** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

private:
    /*!
     * \brief Simulates depressional areas that do not
     * drain to the stream network (pothole) and impounded areas such as rice paddies
     * reWrite from pothole.f of SWAT
     */
    void PotholeSimulate(int id);

    /*!
     * compute surface area assuming a cone shape, ha
     */
    void PotholeSurfaceArea(int id);

    /*!
     * release water stored in pothole
     */
    void ReleaseWater(int id);

private:
    /// conversion factor (mm/ha => m^3)
    FLTPT m_cnv;
    /// valid cells number
    int m_nCells;
    /// cell width, m
    FLTPT m_cellWidth;
    /// cell area, ha
    FLTPT m_cellArea;
    /// timestep, sec
    int m_timestep;
    /// soil layers
    int* m_nSoilLyrs;
    /// max soil layers
    int m_maxSoilLyrs;
    /// subbasin ID
    int* m_subbasin;
    /// subbasin number
    int m_nSubbasins;
    /**
    *	@brief Routing layers according to the flow direction
    *
    *	There are not flow relationships within each layer.
    *	The first element in each layer is the number of cells in the layer
    */
    int** m_rteLyrs;
    /// number of routing layers
    int m_nRteLyrs;
    /// leaf area index at which no evaporation occurs from water surface
    FLTPT m_evLAI;
    /// slope gradient (%)
    FLTPT* m_slope;
    /// saturated conductivity
    FLTPT** m_ks;
    /// saturated soil water, mm
    FLTPT** m_sol_sat;
    /// field capacity on soil profile (mm, FC-WP)
    FLTPT* m_sol_sumfc;
    /// soil thickness
    FLTPT** m_soilThick;
    /// porosity mm/mm
    FLTPT** m_sol_por;
    /// Average daily outflow to main channel from tile flow if drainage tiles are installed in the pothole, mm
    FLTPT m_potTilemm;
    /// Nitrate decay rate in impounded water body
    FLTPT m_potNo3Decay;
    /// Soluble phosphorus decay rate in impounded water body
    FLTPT m_potSolPDecay;

    /// volatilization rate constant in impounded water body, /day
    FLTPT m_kVolat;
    /// nitrification rate constant in impounded water body, /day
    FLTPT m_kNitri;
    /// hydraulic conductivity of soil surface of pothole, mm/hr
    FLTPT m_pot_k;
    /// impounding trigger
    int* m_impoundTrig;
    /// surface area of impounded area, ha
    FLTPT* m_potArea;
    /// net precipitation
    //FLTPT *m_netPrec;
    /// lai in the current day
    FLTPT* m_LAIDay;
    /// pet
    FLTPT* m_pet;
    /// evaporation from depression, mm
    FLTPT* m_depEvapor;
    /// depression storage, mm
    FLTPT* m_depStorage;
    /// surface runoff, mm
    FLTPT* m_surfaceRunoff;
    /// sediment yield transported on each cell, kg
    FLTPT* m_sedYield;
    //! sand yield
    FLTPT* m_sandYield;
    //! silt yield
    FLTPT* m_siltYield;
    //! clay yield
    FLTPT* m_clayYield;
    //! small aggregate yield
    FLTPT* m_smaggreYield;
    //! large aggregate yield
    FLTPT* m_lgaggreYield;
    /// amount of water stored in soil layers on current day, sol_st in SWAT
    FLTPT** m_soilStorage;
    /// amount of water stored in soil profile on current day, sol_sw in SWAT
    FLTPT* m_soilStorageProfile;
    /// amount of nitrate transported with surface runoff, kg/ha
    FLTPT* m_surqNo3;
    /// amount of ammonian transported with surface runoff, kg/ha
    FLTPT* m_surqNH4;
    /// amount of soluble phosphorus transported with surface runoff, kg/ha
    FLTPT* m_surqSolP;
    /// , kg/ha
    FLTPT* m_surqCOD;
    /// , kg/ha
    FLTPT* m_sedOrgN;
    ///, kg/ha
    FLTPT* m_sedOrgP;
    /// , kg/ha
    FLTPT* m_sedActiveMinP;
    /// , kg/ha
    FLTPT* m_sedStableMinP;

    /// no3 amount kg
    FLTPT* m_potNo3;
    /// nh4 amount kg
    FLTPT* m_potNH4;
    /// orgN amount kg
    FLTPT* m_potOrgN;
    /// soluble phosphorus amount, kg
    FLTPT* m_potSolP;
    /// orgP amount kg
    FLTPT* m_potOrgP;
    /// active mineral P kg
    FLTPT* m_potActMinP;
    /// stable mineral P kg
    FLTPT* m_potStaMinP;
    /// sediment amount kg
    FLTPT* m_potSed;
    /// sand
    FLTPT* m_potSand;
    /// silt
    FLTPT* m_potSilt;
    /// clay
    FLTPT* m_potClay;
    /// small aggregate
    FLTPT* m_potSag;
    /// large aggregate
    FLTPT* m_potLag;
    /// volume   mm
    FLTPT* m_potVol;
    /// maximum volume mm
    FLTPT* m_potVolMax;
    /// lowest volume mm
    FLTPT* m_potVolMin;
    /// seepage water of pothole, mm
    FLTPT* m_potSeep;
    /// evaporation, mm
    FLTPT* m_potEvap;
    ///// flow in   mm
    //FLTPT *m_potFlowIn;
    ///// flow out  mm
    //FLTPT *m_potFlowOut;
    ///// sediment entering pothole on day   kg
    //FLTPT *m_potSedIn;
    ///// sand
    //FLTPT *m_potSandIn;
    ///// silt
    //FLTPT *m_potSiltIn;
    ///// clay
    //FLTPT *m_potClayIn;
    ///// small aggregate
    //FLTPT *m_potSagIn;
    ///// large aggregate
    //FLTPT *m_potLagIn;

    /*
     * surface runoff, sediment, nutrient that into the channel
     */
    /// surface runoff to channel, m^3/s
    FLTPT* m_surfqToCh;
    /// sediment transported to channel, kg
    FLTPT* m_sedToCh;
    /// amount of nitrate transported with surface runoff
    FLTPT* m_surNO3ToCh;
    /// amount of ammonian transported with surface runoff
    FLTPT* m_surNH4ToCh;
    /// amount of soluble phosphorus in surface runoff
    FLTPT* m_surSolPToCh;
    /// cod to reach in surface runoff (kg)
    FLTPT* m_surCodToCh;
    // amount of organic nitrogen in surface runoff
    FLTPT* m_sedOrgNToCh;
    // amount of organic phosphorus in surface runoff
    FLTPT* m_sedOrgPToCh;
    // amount of active mineral phosphorus absorbed to sediment in surface runoff
    FLTPT* m_sedMinPAToCh;
    // amount of stable mineral phosphorus absorbed to sediment in surface runoff
    FLTPT* m_sedMinPSToCh;
};
#endif /* SEIMS_MODULE_IMP_SWAT_H */
