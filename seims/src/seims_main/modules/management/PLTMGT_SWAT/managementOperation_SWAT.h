/*!
 * \brief All management operation in SWAT, e.g., plantop, killop, harvestop, etc.
 * \author Liang-Jun Zhu
 * \date June 2016
 *           1. Source code of SWAT include: readmgt.f, operatn.f, sched_mgt.f, plantop.f, harvkillop.f, harvestop.f, killop.f, newtillmix.f, etc.
 *           2. Preliminary implemented version, not include grazing, auto fertilizer, etc. See detail please find the TODOs.
 * \date 2016-9-29
 * \description: 1. Add the CENTURY model related code, mainly include fert.f, newtillmix.f, and harvestop.f
 *               2. Update fertilizer operation for paddy rice, i.e., ExecuteFertilizerOperation()
 */
#ifndef SEIMS_MODULE_PLTMGT_SWAT_H
#define SEIMS_MODULE_PLTMGT_SWAT_H

#include "SimulationModule.h"
#include "Scenario.h"
#include "ClimateParams.h"

using namespace std;
using namespace MainBMP;
/** \defgroup PLTMGT_SWAT
 * \ingroup Management
 * \brief All management operation in SWAT, e.g., plantop, killop, harvestop, etc.
 */
/*!
 * \class MGTOpt_SWAT
 * \ingroup PLTMGT_SWAT
 * \brief All management operation in SWAT, e.g., plantop, killop, harvestop, etc.
 */
class MGTOpt_SWAT : public SimulationModule {
private:
    /*
    * Plant management factory derived from BMPs Scenario
    * Key is  uniqueBMPID, which is calculated by Landuse_ID * 100 + subScenario;
    * Value is a series of plant management operations
    */
    map<int, BMPPlantMgtFactory *> m_mgtFactory;
    /// valid cells number
    int m_nCells;
    /// cell width (m)
    float m_cellWidth;
    /// cell area (ha)
    float m_cellArea;
    /// the total number of subbasins
    int m_nSub;
    /// valid cell numbers in each subbasin
    map<int, int> m_nCellsSubbsn;
    /// subbasin area, key is subbasinID, value is area (ha)
    map<int, float> m_nAreaSubbsn;

    /**Parameters from MongoDB**/

    /// subbasin ID of each cell
    float *m_subBsnID;
    /// land use
    float *m_landUse;
    /// land cover
    float *m_landCover;
    /// management unit, e.g., fields
    float *m_mgtFields;

    /// soil layers
    float *m_nSoilLayers;
    /// maximum soil layers
    int m_soilLayers;
    /// depth to bottom of soil layer, sol_z, mm
    float **m_soilDepth;
    /// soil thick
    float **m_soilThick;
    /// maximum root depth of soil
    float *m_soilZMX;
    /// bulk density of top soil layer in cell, sol_bd, Mg/m^3, i.e., g/cm3        |
    float **m_soilBD;
    /// sol_sumfc(:)   |mm H2O        |amount of water held in the soil profile at field capacity (FC-WP)
    float *m_soilSumFC;
    /// sol_n, initialized by sol_cbn / 11.0 according to readsol.f in SWAT
    float **m_soilN;
    /// soil carbon, sol_cbn
    float **m_soilCarbon;
    /// soil rock (%)
    float **m_soilRock;
    /// soil clay (%)
    float **m_soilClay;
    /// soil sand (%)
    float **m_soilSand;
    /// soil silt (%)
    float **m_soilSilt;
    /**soil properties, both as input and ouput**/

    ///    sol_aorgn(:,:)|kg N/ha       |amount of nitrogen stored in the active organic (humic) nitrogen pool
    float **m_soilActiveOrgN;
    ///    sol_fon(:,:)  |kg N/ha       |amount of nitrogen stored in the fresh organic (residue) pool
    float **m_soilFreshOrgN;
    ///    sol_fop(:,:)  |kg P/ha       |amount of phosphorus stored in the fresh organic (residue) pool
    float **m_soilFreshOrgP;
    ///    sol_nh4(:,:)  |kg N/ha       |amount of nitrogen stored in the ammonium pool in soil layer
    float **m_soilNH4;
    ///    sol_no3(:,:)  |kg N/ha       |amount of nitrogen stored in the nitrate pool in soil layer
    float **m_soilNO3;
    /// sol_orgn(:,:) |kg N/ha       |amount of nitrogen stored in the stable organic N pool
    float **m_soilStableOrgN;
    ///    sol_orgp(:,:) |kg P/ha       |amount of phosphorus stored in the organic P pool
    float **m_soilOrgP;
    ///    sol_solp(:,:) |kg P/ha       |amount of inorganic phosphorus stored in solution
    float **m_soilSolP;

    /// minimum temperature for plant growth
    float *m_tBase;
    /** Temporary parameters**/

    /// Sequence number of management operations done in the previous time step run
    int *m_doneOpSequence;

    /** plant operation related parameters **/

    /// landuse lookup table
    float **m_landuseLookup;
    /// landuse number
    int m_landuseNum;
    /// map from m_landuseLookup
    map<int, float *> m_landuseLookupMap;
    /// CN2 values
    float *m_CN2;
    /// plant growth code, 0 or 1
    float *m_igro;
    /// land cover/crop  classification:1-7, i.e., IDC
    float *m_landCoverCls;
    /// Harvest index target, defined in plant operation and used in harvest/kill operation
    float *m_HarvestIdxTarg;
    /// Biomass target
    float *m_BiomassTarg;
    /// current year in rotation to maturity
    float *m_curYearMat;
    /// wsyf(:)     |(kg/ha)/(kg/ha)|Value of harvest index between 0 and HVSTI
    /// which represents the lowest value expected due to water stress
    float *m_wtrStrsYF;
    /// the leaf area indices for day i
    float *m_LAIDay;
    /// phu base
    float *m_phuBase;
    /// phu accumulated
    float *m_phuAcc;
    /// total number of heat units to bring plant to maturity
    float *m_phuPlant;
    /// dormancy flat, idorm in SWAT, 1 is growing and 0 is dormancy
    float *m_dormFlag;
    /// hvsti(:)    |(kg/ha)/(kg/ha)|harvest index: crop yield/aboveground biomass
    float *m_havstIdx;
    /// hvstiadj(:) |(kg/ha)/(kg/ha)|optimal harvest index for current time during growing season
    float *m_havstIdxAdj;
    /// DO NOT KNOW NOW, initialized as 0.
    float *m_LAIMaxFr;
    /// DO NOT KNOW NOW
    float *m_oLAI;
    /// fraction of plant biomass that is nitrogen, pltfr_n in SWAT
    float *m_frPlantN;
    /// fraction of plant biomass that is phosphorous, pltfr_p in SWAT
    float *m_frPlantP;
    /// amount of nitrogen in plant biomass (kg/ha), plantn in SWAT
    float *m_plantN;
    /// amount of phosphorus in plant biomass (kg/ha), plantp in SWAT
    float *m_plantP;
    /// actual ET simulated during life of plant, plt_et in SWAT
    float *m_pltET;
    /// potential ET simulated during life of plant, plt_pet in SWAT
    float *m_pltPET;
    /// fraction of total plant biomass that is in roots, rwt in SWAT
    float *m_frRoot;
    /// land cover/crop biomass (dry weight), bio_ms in SWAT
    float *m_biomass;

    /** HarvestKill operation related **/
    ///// bio_hv(:,:,:)|kg/ha          |harvested biomass (dry weight)
    //float** m_harvBiomass;
    ///// bio_yrms(:) |metric tons/ha |annual biomass (dry weight) in the cell
    //float* m_annualBiomass;
    ///// yldkg(:,:,:)|kg/ha          |yield (dry weight) by crop type in the HRU
    ////float** m_cropYld;
    ///// yldanu(:)   |metric tons/ha |annual yield (dry weight) in the HRU
    //float* m_annualYld;

    /// amount of organic matter in the soil layer classified as residue,sol_rsd(:,:)|kg/ha
    float **m_soilRsd;
    /// fraction of potential plant growth achieved where the reduction is caused by water stress, strsw in SWAT
    float *m_frStrsWa;

    /** Crop related parameters **/

    /// crop lookup table
    float **m_cropLookup;
    /// crop number
    int m_cropNum;
    /// map for m_cropLookup
    map<int, float *> m_cropLookupMap;

    /** Fertilizer related parameters **/

    /// fertilizer lookup table
    float **m_fertilizerLookup;
    /// fertilizer number
    int m_fertilizerNum;
    /// map for m_fertilizerLookup
    map<int, float *> m_fertilizerLookupMap;
    /* carbon modeling method
     *   = 0 Static soil carbon (old mineralization routines)
     *   = 1 C-FARM one carbon pool model
     *   = 2 Century model
	 */
    int m_CbnModel;
    /**** 1 - C-FARM model ****/
    /// manure organic carbon in soil, kg/ha
    float **m_soilManureC;
    /// manure organic nitrogen in soil, kg/ha
    float **m_soilManureN;
    /// manure organic phosphorus in soil, kg/ha
    float **m_soilManureP;
    /**** 2 - CENTURY model ****/
    float **m_sol_HSN; /// slow Nitrogen pool in soil, equals to soil active organic n pool in SWAT
    float **m_sol_LM; /// metabolic litter SOM pool
    float **m_sol_LMC; /// metabolic litter C pool
    float **m_sol_LMN; /// metabolic litter N pool
    float **m_sol_LSC; /// structural litter C pool
    float **m_sol_LSN; /// structural litter N pool
    float **m_sol_LS; /// structural litter SOM pool
    float **m_sol_LSL; /// lignin weight in structural litter
    float **m_sol_LSLC; /// lignin amount in structural litter pool
    float **m_sol_LSLNC; /// non-lignin part of the structural litter C

    /// tillage factor on SOM decomposition, used by CENTURY model
    float *m_tillage_switch;
    float *m_tillage_depth;
    float *m_tillage_days;
    float *m_tillage_factor;
    float **m_sol_BMN; ///
    float **m_sol_HPN; ///

    /** Irrigation operation related **/

    /// irrigation flag
    float *m_irrFlag;
    /// aird(:)        |mm H2O        |amount of water applied to cell on current day
    float *m_appliedWater;
    /// qird(:)        |mm H2O        |amount of water from irrigation to become surface runoff
    float *m_irrSurfQWater;
    /// Currently, deep and shallow aquifer are not distinguished in SEIMS.
    /// So, m_deepWaterDepth and m_shallowWaterDepth are all set to m_SBGS
    ///  deepst(:)      |mm H2O        |depth of water in deep aquifer
    float *m_deepWaterDepth;
    ///shallst | mm H2O        |depth of water in shallow aquifer
    float *m_shallowWaterDepth;
    /// potsa(:)       |ha            |surface area of impounded water body
    float *m_impoundArea;
    /// deepirr(:)  |mm H2O        |amount of water removed from deep aquifer for irrigation
    float *m_deepIrrWater;
    /// shallirr(:) |mm H2O        |amount of water removed from shallow aquifer for irrigation
    float *m_shallowIrrWater;
    /** auto irrigation operation related**/

    /// Water stress identifier, 1 plant water demand, 2 soil water content
    float *m_wtrStrsID;
    /// Water stress threshold that triggers irrigation, if m_wtrStresID is 1, the value usually 0.90 ~ 0.95
    float *m_autoWtrStres;
    /// irrigation source
    float *m_autoIrrSource;
    /// irrigation source location code
    float *m_autoIrrNo;
    /// auto irrigation efficiency, 0 ~ 100, IRR_EFF
    float *m_autoIrrEfficiency;
    /// amount of irrigation water applied each time auto irrigation is triggered (mm), 0 ~ 100, IRR_MX
    float *m_autoIrrWtrDepth;
    /// surface runoff ratio (0-1) (0.1 is 10% surface runoff), IRR_ASQ
    float *m_autoSurfRunRatio;
    ///**Bacteria related**/
    //////// TODO, bacteria modeling will be implemented in the future. (bacteria.f)
    ///// fraction of manure containing active colony forming units (cfu)
    //float m_bactSwf;
    /////  bactlp_plt(:) |# cfu/m^2     |less persistent bacteria on foliage
    //float* m_bactLessPersistPlt;
    /////  bactlpq(:)    |# cfu/m^2     |less persistent bacteria in soil solution
    //float* m_bactLessPersistSol;
    /////  bactlps(:)    |# cfu/m^2     |less persistent bacteria attached to soil particles
    //float* m_bactLessPersistParticle;
    /////  bactp_plt(:)  |# cfu/m^2     |persistent bacteria on foliage
    //float* m_bactPersistPlt;
    /////  bactpq(:)     |# cfu/m^2     |persistent bacteria in soil solution
    //float* m_bactPersistSol;
    /////  bactps(:)     |# cfu/m^2     |persistent bacteria attached to soil particles
    //float* m_bactPersistParticle;

    /** HarvestKill, Harvest, and Kill operation related  **/

    /// stsol_rd(:) |mm            |storing last soil root depth for use in harvestkillop/killop /// defined in swu.f
    float *m_lastSoilRootDepth;
    /**** Daily carbon change by different means (entire soil profile for each cell) ****/
    /**** For 2-CENTURY C/N cycling model, these variables will be initialized as 0  ****/
    /**** at the beginning of the current day ****/
    /**** 1 harvest, 2 harvestkill, 3 harvgrain op ****/
    float *m_grainc_d; /// 1,2,3
    float *m_rsdc_d; /// 1, 2
    float *m_stoverc_d; /// 2

    float *m_sedc_d;
    float *m_surfqc_d;
    float *m_latc_d;
    float *m_percc_d;
    float *m_foc_d;
    float *m_NPPC_d;
    float *m_soc_d;
    float *m_rspc_d;
    float *m_emitc_d; // include biomass_c eaten by grazing, burnt


    /** tillage operation related **/

    /// tillage lookup table
    float **m_tillageLookup;
    /// tillage number
    int m_tillageNum;
    /// map for m_tillageLookup
    map<int, float *> m_tillageLookupMap;
    /// sol_actp(:,:) |kg P/ha       |amount of phosphorus stored in the active mineral phosphorus pool
    float **m_soilActiveMinP;
    /// sol_stap(:,:) |kg P/ha       |amount of phosphorus in the soil layer stored in the stable mineral phosphorus pool
    float **m_soilStableMinP;
    ///// min_res(:)	|kg/ha		   |Min residue allowed due to implementation of residue management in the OPS file.
    //float* m_minResidue;

    /**auto fertilizer operation**/

    /// fertilizer ID from fertilizer database
    float *m_fertilizerID;
    /* Code for approach used to determine amount of nitrogen to Cell
            0 Nitrogen target approach
            1 annual max approach */
    float *m_NStressCode;
    /// Nitrogen stress factor of cover/plant that triggers fertilization, usually set 0.90 to 0.95
    float *m_autoNStress;
    /// Maximum amount of mineral N allowed in any one application (kg N/ha), auto_napp
    float *m_autoMaxAppliedN;
    /// Maximum amount of mineral N allowed to be applied in any one year (kg N/ha), auto_nyr
    float *m_autoAnnMaxAppliedMinN;
    /// modifier for auto fertilization target nitrogen content, tnylda
    float *m_targNYld;
    /// auto_eff(:) |none           |fertilizer application efficiency calculated as the amount of N applied divided by the amount of N removed at harvest
    float *m_autoFertEfficiency;
    /// Fraction of fertilizer applied to top 10mm of soil, the default is 0.2
    float *m_autoFertSurface;
    /** Grazing operation **/

    /// ndeat(:)    |days          |number of days cell has been grazed
    float *m_nGrazingDays;
    /*  igrz(:)     |none          |grazing flag for cell:
                                                |0 cell currently not grazed
                                                |1 cell currently grazed */
    float *m_grzFlag;
    /** Release or Impound Operation **/

    /* |release/impound action code:
           |0 begin impounding water
           |1 release impounded water
	 */
    float *m_impoundTriger;
    /// volume of water stored in the depression/impounded area, mm
    float *m_potVol;
    /// maximum volume of water stored in the depression/impounded area, mm
    float *m_potVolMax;
    /// low depth ...., mm
    float *m_potVolLow;
    /// no3 amount kg
    float *m_potNo3;
    /// nh4 amount kg
    float *m_potNH4;
    /// soluble phosphorus amount, kg
    float *m_potSolP;
    /// field capacity (FC-WP), mm
    float **m_sol_fc;
    /// amount of water held in the soil layer at saturation (sat - wp water), mm
    float **m_sol_sat;
    /// soil water storage (mm)
    float **m_soilStorage;
    /// soil water storage in soil profile (mm)
    float *m_soilStorageProfile;
    /// flag to identify the initialization
    bool m_initialized;

    /** Temporary variables **/
    float *tmp_rtfr;  ///< fraction of roots in each soil layer
    float *tmp_soilMass;  ///<
    float *tmp_soilMixedMass;  ///< mass of soil mixed for the layer
    float *tmp_soilNotMixedMass;  ///<
    float *tmp_smix;
public:
    //! Constructor
    MGTOpt_SWAT();

    //! Destructor
    ~MGTOpt_SWAT();

    int Execute();

    void SetValue(const char *key, float data);

    void Set1DData(const char *key, int n, float *data);

    void Get1DData(const char *key, int *n, float **data);

    void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

    void Set2DData(const char *key, int n, int col, float **data);

    void SetScenario(Scenario *sce);

    void SetSubbasins(clsSubbasins *subbasins);

private:
    /*!
     * \brief Get operation parameters according to operation sequence number
     * \param[in] cellIdx current cell index
     * \param[out] factoryID Index of Plant BMPs factory
     * \param[out] nOps Operation sequence number, and there might be several operation occurred on one day
     */
    bool GetOperationCode(int cellIdx, int &factoryID, vector<int> &nOps);

    /*!
     * \brief Manager all operations on schedule
     * \param[in] cellIdx Index of valid cell
     * \param[in] factoryID Index of Plant BMPs factory
     * \param[in] nOp Operation sequence
     */
    void ScheduledManagement(int cellIdx, int &factoryID, int nOp);

    void ExecutePlantOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteIrrigationOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteFertilizerOperation(int cellIdx, int &factoryID, int nOp);

    void ExecutePesticideOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteHarvestKillOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteTillageOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteHarvestOnlyOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteKillOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteGrazingOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteAutoIrrigationOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteAutoFertilizerOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteReleaseImpoundOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteContinuousFertilizerOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteContinuousPesticideOperation(int cellIdx, int &factoryID, int nOp);

    void ExecuteBurningOperation(int cellIdx, int &factoryID, int nOp);

    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData();

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);

    /*!
     * \brief check the input size of 2D data. Make sure all the input data have same dimension.
     *
     *
     * \param[in] key The key of the input data
     * \param[in] n The first dimension input data 
     * \param[in] col The second dimension of input data
     * \return bool The validity of the dimension
     */
    bool CheckInputSize2D(const char *key, int n, int col);

    /// initialize all possible outputs
    void initialOutputs();

    /// Handle lookup tables ///

    /// landuse lookup table
    void initializeLanduseLookup();

    /// crop lookup table
    void initializeCropLookup();

    /// fertilizer lookup table
    void initializeFertilizerLookup();

    /// tillage lookup table
    void initializeTillageLookup();

    /// the complementary error function
    float Erfc(float xx);

    /// distributes dead root mass through the soil profile
    void rootFraction(int i, float *&root_fr);
};

#endif /* SEIMS_MODULE_PLTMGT_SWAT_H */