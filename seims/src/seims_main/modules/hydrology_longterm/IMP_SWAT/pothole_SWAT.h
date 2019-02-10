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

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int n, int col, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

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
    float m_cnv;
    /// valid cells number
    int m_nCells;
    /// cell width, m
    float m_cellWidth;
    /// cell area, ha
    float m_cellArea;
    /// timestep, sec
    float m_timestep;
    /// soil layers
    float* m_nSoilLyrs;
    /// max soil layers
    int m_maxSoilLyrs;
    /// subbasin ID
    float* m_subbasin;
    /// subbasin number
    int m_nSubbasins;
    /**
    *	@brief Routing layers according to the flow direction
    *
    *	There are not flow relationships within each layer.
    *	The first element in each layer is the number of cells in the layer
    */
    float** m_rteLyrs;
    /// number of routing layers
    int m_nRteLyrs;
    /// leaf area index at which no evaporation occurs from water surface
    float m_evLAI;
    /// slope gradient (%)
    float* m_slope;
    /// saturated conductivity
    float** m_ks;
    /// saturated soil water, mm
    float** m_sol_sat;
    /// field capacity on soil profile (mm, FC-WP)
    float* m_sol_sumfc;
    /// soil thickness
    float** m_soilThick;
    /// porosity mm/mm
    float** m_sol_por;
    /// Average daily outflow to main channel from tile flow if drainage tiles are installed in the pothole, mm
    float m_potTilemm;
    /// Nitrate decay rate in impounded water body
    float m_potNo3Decay;
    /// Soluble phosphorus decay rate in impounded water body
    float m_potSolPDecay;

    /// volatilization rate constant in impounded water body, /day
    float m_kVolat;
    /// nitrification rate constant in impounded water body, /day
    float m_kNitri;
    /// hydraulic conductivity of soil surface of pothole, mm/hr
    float m_pot_k;
    /// impounding trigger
    float* m_impoundTrig;
    /// surface area of impounded area, ha
    float* m_potArea;
    /// net precipitation
    //float *m_netPrec;
    /// lai in the current day
    float* m_LAIDay;
    /// pet
    float* m_pet;
    /// evaporation from depression, mm
    float* m_depEvapor;
    /// depression storage, mm
    float* m_depStorage;
    /// surface runoff, mm
    float* m_surfaceRunoff;
    /// sediment yield transported on each cell, kg
    float* m_sedYield;
    //! sand yield
    float* m_sandYield;
    //! silt yield
    float* m_siltYield;
    //! clay yield
    float* m_clayYield;
    //! small aggregate yield
    float* m_smaggreYield;
    //! large aggregate yield
    float* m_lgaggreYield;
    /// amount of water stored in soil layers on current day, sol_st in SWAT
    float** m_soilStorage;
    /// amount of water stored in soil profile on current day, sol_sw in SWAT
    float* m_soilStorageProfile;
    /// amount of nitrate transported with surface runoff, kg/ha
    float* m_surqNo3;
    /// amount of ammonian transported with surface runoff, kg/ha
    float* m_surqNH4;
    /// amount of soluble phosphorus transported with surface runoff, kg/ha
    float* m_surqSolP;
    /// , kg/ha
    float* m_surqCOD;
    /// , kg/ha
    float* m_sedOrgN;
    ///, kg/ha
    float* m_sedOrgP;
    /// , kg/ha
    float* m_sedActiveMinP;
    /// , kg/ha
    float* m_sedStableMinP;

    /// no3 amount kg
    float* m_potNo3;
    /// nh4 amount kg
    float* m_potNH4;
    /// orgN amount kg
    float* m_potOrgN;
    /// soluble phosphorus amount, kg
    float* m_potSolP;
    /// orgP amount kg
    float* m_potOrgP;
    /// active mineral P kg
    float* m_potActMinP;
    /// stable mineral P kg
    float* m_potStaMinP;
    /// sediment amount kg
    float* m_potSed;
    /// sand
    float* m_potSand;
    /// silt
    float* m_potSilt;
    /// clay
    float* m_potClay;
    /// small aggregate
    float* m_potSag;
    /// large aggregate
    float* m_potLag;
    /// volume   mm
    float* m_potVol;
    /// maximum volume mm
    float* m_potVolMax;
    /// lowest volume mm
    float* m_potVolMin;
    /// seepage water of pothole, mm
    float* m_potSeep;
    /// evaporation, mm
    float* m_potEvap;
    ///// flow in   mm
    //float *m_potFlowIn;
    ///// flow out  mm
    //float *m_potFlowOut;
    ///// sediment entering pothole on day   kg
    //float *m_potSedIn;
    ///// sand
    //float *m_potSandIn;
    ///// silt
    //float *m_potSiltIn;
    ///// clay
    //float *m_potClayIn;
    ///// small aggregate
    //float *m_potSagIn;
    ///// large aggregate
    //float *m_potLagIn;

    /*
     * surface runoff, sediment, nutrient that into the channel
     */
    /// surface runoff to channel, m^3/s
    float* m_surfqToCh;
    /// sediment transported to channel, kg
    float* m_sedToCh;
    /// amount of nitrate transported with surface runoff
    float* m_surNO3ToCh;
    /// amount of ammonian transported with surface runoff
    float* m_surNH4ToCh;
    /// amount of soluble phosphorus in surface runoff
    float* m_surSolPToCh;
    /// cod to reach in surface runoff (kg)
    float* m_surCodToCh;
    // amount of organic nitrogen in surface runoff
    float* m_sedOrgNToCh;
    // amount of organic phosphorus in surface runoff
    float* m_sedOrgPToCh;
    // amount of active mineral phosphorus absorbed to sediment in surface runoff
    float* m_sedMinPAToCh;
    // amount of stable mineral phosphorus absorbed to sediment in surface runoff
    float* m_sedMinPSToCh;
};
#endif /* SEIMS_MODULE_IMP_SWAT_H */
