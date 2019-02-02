/*!
 * \file Nutrient_Transformation.h
 * \brief Daily nitrogen and phosphorus mineralization and immobilization.
 *        From nminrl.f, nitvol.f, pminrl.f, and pminrl2.f of SWAT
 *
 * Changelog:
 *   - 1. 2016-04-30 - hr - Initial implementation.
 *   - 2. 2016-07-24 - lj -
 *        -# Change m_wshd_dnit etc. variables to store the statistics values of the current day.
 *        -# m_hmntl etc. variables should be DT_Raster1D rather than DT_Single since they are soil profile values in cell!
 *   - 3. 2018-05-08 - lj - Reformat, especially naming style (sync update in "text.h").
 *                          Code optimization, e.g., use multiply instead of divide.
 *
 * \author Huiran Gao, Liangjun Zhu
 */

/*!
 * \defgroup NUTR_TF
 * \ingroup Nutrient
 * \brief Daily nitrogen and phosphorus mineralization and immobilization.
 * From nminrl.f, nitvol.f, pminrl.f, and pminrl2.f of SWAT
 * \author Huiran Gao
 * \date April 2016
 */
#ifndef SEIMS_MODULE_NUTR_TF_H
#define SEIMS_MODULE_NUTR_TF_H

#include "SimulationModule.h"

/*!
 * \class Nutrient_Transformation
 * \ingroup NUTR_TF
 *
 * \brief Daily nitrogen and phosphorus mineralization and immobilization.
 *  Considering fresh organic material (plant residue) and active and stable humus material.
 *
 */
class Nutrient_Transformation: public SimulationModule {
public:
    Nutrient_Transformation();

    ~Nutrient_Transformation();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int nRows, int nCols, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void GetValue(const char* key, float* value) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void Get2DData(const char* key, int* nrows, int* ncols, float*** data) OVERRIDE;

private:
    /*!
    * \brief estimates daily nitrogen and phosphorus mineralization and immobilization.
    *        considering fresh organic material (plant residue) and active and stable humus material
    *        Execute when CSWAT = 0, rewrite from nminrl.f of SWAT
    * \return void
    */
    void MineralizationStaticCarbonMethod(int i);

    /*!
    * \brief simulates organic C, N, and P cycling in soil using C-FARM one carbon model
    *        Execute when CSWAT = 1, rewrite from carbon_new.f and ndenit.f of SWAT
    * \TODO THIS IS ON THE TODO LIST.
    * \return void
    */
    void MineralizationCfarmOneCarbonModel(int i);

    /*!
    * \brief simulates organic C, N, and P cycling in soil using CENTURY model
    *        Execute when CSWAT = 2, rewrite from carbon_zhang.f90 and ndenit.f of SWAT
    */
    void MineralizationCenturyModel(int i);

    /*!
    * \brief estimates daily mineralization (NH3 to NO3) and volatilization of NH3.
    *        rewrite from nitvol.f of SWAT
    * \return void
    */
    void Volatilization(int i);

    /*!
    * \brief Calculate P flux between the labile, active mineral and stable mineral p pools.
    *        rewrite from pminrl, and pminrl2 according to solP_model
    * \TODO In current version, the solP_model is set to 0, i.e., pminrl2 is used as default
    * \return void
    */
    void CalculatePflux(int i);

private:
    /// cell width of grid map (m)
    float m_cellWth;
    /// number of cells
    int m_nCells;
    /// fraction of each cell of the watershed
    float m_cellAreaFr;

    /// soil layers
    float* m_nSoilLyrs;
    /// maximum soil layers
    int m_maxSoilLyrs;
    /* carbon modeling method
     *   = 0 Static soil carbon (old mineralization routines)
     *   = 1 C-FARM one carbon pool model
     *   = 2 Century model
     */
    int m_cbnModel;
    /* phosphorus model selection
     * 0: dynamic coefficient method by White et al., 2009
     * 1: original method
     */
    int m_solP_model;

    /// days since P application
    float* m_phpApldDays;
    /// days since P deficit
    float* m_phpDefDays;

    /// tillage factor on SOM decomposition, used by CENTURY model
    float* m_tillSwitch;
    float* m_tillDepth;
    float* m_tillDays;
    float* m_tillFactor;
    ///input data

    /// Rate coefficient for mineralization of the humus active organic nutrients, cmn in SWAT
    float m_minrlCoef;
    ///nitrogen active pool fraction. The fraction of organic nitrogen in the active pool.
    float m_orgNFrActN;
    /// denitrification threshold: fraction of field capacity, sdnco
    float m_denitThres;
    /// Phosphorus availability index. The fraction of fertilizer P remaining in labile pool after initial rapid phase of P sorption, psp
    /// Note, In swat source code, psp is availability index, but in basin.bsn, psp is sorption index.
    ///       The variable name indicates that sorption index is more precise. ---lj.
    float m_phpSorpIdxBsn;
    float* m_phpSorpIdx;
    float** m_psp_store; // TODO, these variables should be figure out. lj
    float** m_ssp_store;
    //rate coefficient for denitrification, cdn in SWAT
    float m_denitCoef;
    ///land cover code from crop.dat
    float* m_landCover;
    ///plant residue decomposition coefficient.
    ///  The fraction of residue which will decompose in a day assuming optimal moisture,
    ///  temperature, C:N ratio, and C:P ratio
    float* m_pltRsdDecCoef;
    ///amount of residue on soil surface (kg/ha)
    float* m_rsdCovSoil;
    /// initial amount of organic matter in the soil classified as residue(kg/ha)
    float* m_rsdInitSoil;

    /// daily average temperature of soil layer(deg C)
    float* m_soilTemp;
    /// bulk density of the soil (Mg/m3)
    float** m_soilBD;
    /// Soil mass (kg/ha)
    float** m_soilMass;
    /// percent organic carbon in soil layer (%), replace organic matter (%)
    float** m_soilCbn;
    ///amount of water stored in the soil layer on current day(mm H2O)
    float** m_soilWtrSto;
    ///Water content of soil profile at field capacity(mm H2O) (FC-WP)
    float** m_soilFC;
    ///depth to bottom of soil layer
    float** m_soilDepth;
    ///Percent of clay content
    float** m_soilClay;
    /// percent of rock content
    float** m_soilRock;
    /// thick of each soil layer
    float** m_soilThk;
    ///amount of nitrogen stored in the active organic (humic) nitrogen pool(kg N/ha)
    float** m_soilActvOrgN;
    ///amount of nitrogen stored in the fresh organic (residue) pool(kg N/ha)
    float** m_soilFrshOrgN;
    ///amount of phosphorus stored in the fresh organic (residue) pool(kg P/ha)
    float** m_soilFrshOrgP;
    ///amount of phosphorus stored in the active mineral phosphorus pool
    float** m_soilActvMinP;
    ///amount of phosphorus in the soil layer stored in the stable mineral phosphorus pool
    float** m_soilStabMinP;
    /// amount of water held in the soil layer at saturation
    float** m_soilSat;
    /// porosity mm/mm
    float** m_soilPor;
    /// percent sand content of soil material
    float** m_soilSand;

    ///output data

    /************************************************************************/
    /*    CENTURY model related parameters (initialized and output)  20     */
    /************************************************************************/
    float** m_sol_WOC;   ///<
    float** m_sol_WON;   ///<
    float** m_sol_BM;    ///<
    float** m_sol_BMC;   ///<
    float** m_sol_BMN;   ///<
    float** m_sol_HP;    ///< mass of OM in passive humus
    float** m_sol_HS;    ///< mass of OM in slow humus
    float** m_sol_HSC;   ///< mass of C present in slow humus
    float** m_sol_HSN;   ///< mass of N present in slow humus
    float** m_sol_HPC;   ///< mass of C present in passive humus
    float** m_sol_HPN;   ///< mass of N present in passive humus
    float** m_sol_LM;    ///< metabolic litter SOM pool
    float** m_sol_LMC;   ///< metabolic litter C pool
    float** m_sol_LMN;   ///< metabolic litter N pool
    float** m_sol_LSC;   ///< structural litter C pool
    float** m_sol_LSN;   ///< structural litter N pool
    float** m_sol_LS;    ///< structural litter SOM pool
    float** m_sol_LSL;   ///< lignin weight in structural litter
    float** m_sol_LSLC;  ///< lignin amount in structural litter pool
    float** m_sol_LSLNC; ///< non-lignin part of the structural litter C
    float** m_sol_RNMN;  ///< non
    float** m_sol_RSPC;  ///< non
    /************************************************************************/
    ///amount of nitrogen moving from active organic to nitrate pool in soil profile on current day in cell(kg N/ha)
    float* m_hmntl;
    ///amount of phosphorus moving from the organic to labile pool in soil profile on current day in cell(kg P/ha)
    float* m_hmptl;
    ///amount of nitrogen moving from the fresh organic (residue) to the nitrate(80%) and active organic(20%) pools in soil profile on current day in cell(kg N/ha)
    float* m_rmn2tl;
    ///amount of phosphorus moving from the fresh organic (residue) to the labile(80%) and organic(20%) pools in soil profile on current day in cell(kg P/ha)
    float* m_rmptl;
    ///amount of nitrogen moving from active organic to stable organic pool in soil profile on current day in cell(kg N/ha)
    float* m_rwntl;
    ///amount of nitrogen lost from nitrate pool by denitrification in soil profile on current day in cell(kg N/ha)
    float* m_wdntl;
    ///amount of phosphorus moving from the labile mineral pool to the active mineral pool in the soil profile on the current day in cell(kg P/ha)
    float* m_rmp1tl;
    ///amount of phosphorus moving from the active mineral pool to the stable mineral pool in the soil profile on the current day in cell(kg P/ha)
    float* m_roctl;

    ///input & output data
    ///amount of nitrogen stored in the nitrate pool in soil layer(kg N/ha)
    float** m_soilNO3;
    ///amount of nitrogen stored in the stable organic N pool(kg N/ha)
    float** m_soilStabOrgN;
    ///amount of phosphorus stored in the organic P pool in soil layer(kg P/ha)
    float** m_soilHumOrgP;
    ///amount of organic matter in the soil classified as residue(kg/ha)
    float** m_soilRsd;
    ///amount of phosphorus stored in solution(kg P/ha)
    float** m_soilSolP;
    ///amount of nitrogen stored in the ammonium pool in soil layer(kg/ha)
    float** m_soilNH4;
    ///water content of soil at -1.5 MPa (wilting point)
    float** m_soilWP;
    ///nitrogen lost from nitrate pool due to denitrification in watershed(kg N/ha)
    float m_wshd_dnit;
    ///nitrogen moving from active organic to nitrate pool in watershed(kg N/ha)
    float m_wshd_hmn;
    ///phosphorus moving from organic to labile pool in watershed(kg P/ha)
    float m_wshd_hmp;
    ///nitrogen moving from fresh organic (residue) to nitrate and active organic pools in watershed(kg N/ha)
    float m_wshd_rmn;
    ///phosphorus moving from fresh organic (residue) to labile and organic pools in watershed(kg P/ha)
    float m_wshd_rmp;
    ///nitrogen moving from active organic to stable organic pool in watershed(kg N/ha)
    float m_wshd_rwn;
    ///nitrogen moving from active organic to stable organic pool in watershed(kg N/ha)
    float m_wshd_nitn;
    ///nitrogen moving from active organic to stable organic pool in watershed(kg N/ha)
    float m_wshd_voln;
    ///phosphorus moving from labile mineral to active mineral pool in watershed
    float m_wshd_pal;
    ///phosphorus moving from active mineral to stable mineral pool in watershed
    float m_wshd_pas;

    /// factor which converts kg/kg soil to kg/ha, could be used in other nutrient modules
    float** m_conv_wt;
};
#endif /* SEIMS_MODULE_NUTR_TF_H */
