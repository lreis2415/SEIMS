#include "seims.h"
#include "PlantGrowthCommon.h"

#include "managementOperation_SWAT.h"

using namespace std;

MGTOpt_SWAT::MGTOpt_SWAT(void) : m_nCells(-1), m_nSub(-1), m_soilLayers(-1),
                                 m_cellWidth(NODATA_VALUE), m_cellArea(NODATA_VALUE),
    /// add parameters from MongoDB
                                 m_subBsnID(NULL), m_landUse(NULL), m_landCover(NULL), m_mgtFields(NULL), m_tBase(NULL),
    /// Soil related parameters from MongoDB
                                 m_nSoilLayers(NULL), m_soilDepth(NULL), m_soilThick(NULL), m_soilZMX(NULL),
                                 m_soilBD(NULL), m_soilSumFC(NULL), m_soilN(NULL), m_soilCarbon(NULL),
                                 m_soilRock(NULL), m_soilClay(NULL), m_soilSand(NULL), m_soilSilt(NULL),
    /// Soil related parameters
                                 m_soilActiveOrgN(NULL), m_soilFreshOrgN(NULL), m_soilFreshOrgP(NULL),
                                 m_soilNH4(NULL), m_soilNO3(NULL), m_soilStableOrgN(NULL),
                                 m_soilOrgP(NULL), m_soilSolP(NULL),
    /// Plant operation related parameters
                                 m_landuseLookup(NULL), m_landuseNum(-1), m_CN2(NULL), m_igro(NULL),
                                 m_landCoverCls(NULL),
                                 m_HarvestIdxTarg(NULL), m_BiomassTarg(NULL), m_curYearMat(NULL),
                                 m_wtrStrsYF(NULL), m_LAIDay(NULL), m_phuBase(NULL), m_phuAcc(NULL), m_phuPlant(NULL),
                                 m_dormFlag(NULL), m_havstIdx(NULL), m_havstIdxAdj(NULL),
                                 m_LAIMaxFr(NULL), m_oLAI(NULL), m_frPlantN(NULL), m_plantN(NULL), m_plantP(NULL),
                                 m_pltET(NULL), m_pltPET(NULL), m_frRoot(NULL), m_biomass(NULL),
    /// Harvest and Kill, harvest, harvgrain operation
                                 m_soilRsd(NULL), m_frStrsWa(NULL), m_cropLookup(NULL), m_cropNum(-1),
                                 m_lastSoilRootDepth(NULL),
                                 m_grainc_d(NULL), m_stoverc_d(NULL), m_rsdc_d(NULL),
    /// Fertilizer operation
                                 m_fertilizerLookup(NULL), m_fertilizerNum(-1), m_CbnModel(0),
                                 m_soilManureC(NULL), m_soilManureN(NULL), m_soilManureP(NULL),
    /// Irrigation
                                 m_irrFlag(NULL), m_appliedWater(NULL), m_irrSurfQWater(NULL), m_deepWaterDepth(NULL),
                                 m_shallowWaterDepth(NULL),
                                 m_impoundArea(NULL), m_deepIrrWater(NULL), m_shallowIrrWater(NULL),
    /// auto irrigation operation
                                 m_wtrStrsID(NULL), m_autoWtrStres(NULL), m_autoIrrSource(NULL), m_autoIrrNo(NULL),
                                 m_autoIrrEfficiency(NULL),
                                 m_autoIrrWtrDepth(NULL), m_autoSurfRunRatio(NULL),
    /// bacteria related
    //m_bactSwf(NODATA),	m_bactLessPersistPlt(NULL), m_bactLessPersistSol(NULL), m_bactLessPersistParticle(NULL),
    //m_bactPersistPlt(NULL), m_bactPersistSol(NULL), m_bactPersistParticle(NULL),
    /// Tillage operation
                                 m_tillageLookup(NULL), m_soilActiveMinP(NULL), m_soilStableMinP(NULL),
    /// tillage factor on SOM decomposition, used by CENTURY model
                                 m_tillage_switch(NULL), m_tillage_depth(NULL), m_tillage_days(NULL),
                                 m_tillage_factor(NULL),
    /// auto fertilizer operation
                                 m_fertilizerID(NULL), m_NStressCode(NULL), m_autoNStress(NULL),
                                 m_autoMaxAppliedN(NULL), m_autoAnnMaxAppliedMinN(NULL),
                                 m_targNYld(NULL), m_autoFertEfficiency(NULL), m_autoFertSurface(NULL),
    /// Grazing operation
                                 m_nGrazingDays(NULL), m_grzFlag(NULL),
    /// Release or impound operation
                                 m_impoundTriger(NULL), m_potVol(NULL), m_potVolMax(NULL), m_potVolLow(NULL),
                                 m_sol_fc(NULL), m_sol_sat(NULL), m_soilStorage(NULL), m_soilStorageProfile(NULL),
                                 m_potNo3(NULL), m_potNH4(NULL), m_potSolP(NULL),
    /// CENTURY C/N cycling related variables
                                 m_sol_HSN(NULL), m_sol_LM(NULL), m_sol_LMC(NULL), m_sol_LMN(NULL), m_sol_LSC(NULL),
                                 m_sol_LSN(NULL), m_sol_LS(NULL), m_sol_LSL(NULL), m_sol_LSLC(NULL), m_sol_LSLNC(NULL),
                                 m_sol_BMN(NULL), m_sol_HPN(NULL),
    /// Temporary parameters
                                 m_doneOpSequence(NULL),
                                 m_initialized(false) {
}

MGTOpt_SWAT::~MGTOpt_SWAT(void) {
    /// release map containers
    if (!m_mgtFactory.empty()) {
        for (map<int, BMPPlantMgtFactory *>::iterator it = m_mgtFactory.begin(); it != m_mgtFactory.end();) {
            if (it->second != NULL) {
                delete it->second;
                it->second = NULL;
            }
            m_mgtFactory.erase(it++);
        }
        m_mgtFactory.clear();
    }
    if (!m_landuseLookupMap.empty()) {
        for (map<int, float *>::iterator it = m_landuseLookupMap.begin(); it != m_landuseLookupMap.end();) {
            if (it->second != NULL) {
                delete[] it->second;
                it->second = NULL;
            }
            it->second = NULL;
            m_landuseLookupMap.erase(it++);
        }
        m_landuseLookupMap.clear();
    }
    if (!m_cropLookupMap.empty()) {
        for (map<int, float *>::iterator it = m_cropLookupMap.begin(); it != m_cropLookupMap.end();) {
            if (it->second != NULL) {
                delete[] it->second;
                it->second = NULL;
            }
            it->second = NULL;
            m_cropLookupMap.erase(it++);
        }
        m_cropLookupMap.clear();
    }
    if (!m_fertilizerLookupMap.empty()) {
        for (map<int, float *>::iterator it = m_fertilizerLookupMap.begin(); it != m_fertilizerLookupMap.end();) {
            if (it->second != NULL) {
                delete[] it->second;
                it->second = NULL;
            }
            it->second = NULL;
            m_fertilizerLookupMap.erase(it++);
        }
        m_fertilizerLookupMap.clear();
    }
    if (!m_tillageLookupMap.empty()) {
        for (map<int, float *>::iterator it = m_tillageLookupMap.begin(); it != m_tillageLookupMap.end();) {
            if (it->second != NULL) {
                delete[] it->second;
                it->second = NULL;
            }
            it->second = NULL;
            m_tillageLookupMap.erase(it++);
        }
        m_tillageLookupMap.clear();
    }
    /// release output parameters
    /// plant operation
    if (m_HarvestIdxTarg != NULL) Release1DArray(m_HarvestIdxTarg);
    if (m_BiomassTarg != NULL) Release1DArray(m_BiomassTarg);
    /// auto irrigation operation
    if (m_irrFlag != NULL) Release1DArray(m_irrFlag);
    if (m_appliedWater != NULL) Release1DArray(m_appliedWater);
    if (m_irrSurfQWater != NULL) Release1DArray(m_irrSurfQWater);
    if (m_wtrStrsID != NULL) Release1DArray(m_wtrStrsID);
    if (m_autoWtrStres != NULL) Release1DArray(m_autoWtrStres);
    if (m_autoIrrSource != NULL) Release1DArray(m_autoIrrSource);
    if (m_autoIrrNo != NULL) Release1DArray(m_autoIrrNo);
    if (m_autoIrrEfficiency != NULL) Release1DArray(m_autoIrrEfficiency);
    if (m_autoIrrWtrDepth != NULL) Release1DArray(m_autoIrrWtrDepth);
    if (m_autoSurfRunRatio != NULL) Release1DArray(m_autoSurfRunRatio);
    /// fertilizer / auto fertilizer operation
    if (m_fertilizerID != NULL) Release1DArray(m_fertilizerID);
    if (m_NStressCode != NULL) Release1DArray(m_NStressCode);
    if (m_autoNStress != NULL) Release1DArray(m_autoNStress);
    if (m_autoMaxAppliedN != NULL) Release1DArray(m_autoMaxAppliedN);
    if (m_autoAnnMaxAppliedMinN != NULL) Release1DArray(m_autoAnnMaxAppliedMinN);
    if (m_targNYld != NULL) Release1DArray(m_targNYld);
    if (m_autoFertEfficiency != NULL) Release1DArray(m_autoFertEfficiency);
    if (m_autoFertSurface != NULL) Release1DArray(m_autoFertSurface);
    /// Grazing operation
    if (m_nGrazingDays != NULL) Release1DArray(m_nGrazingDays);
    if (m_grzFlag != NULL) Release1DArray(m_grzFlag);
    /// Impound/Release operation
    if (m_impoundTriger != NULL) Release1DArray(m_impoundTriger);
    if (m_potVolMax != NULL) Release1DArray(m_potVolMax);
    if (m_potVolLow != NULL) Release1DArray(m_potVolLow);
}

void MGTOpt_SWAT::SetValue(const char *key, float data) {
    string sk(key);
    if (StringMatch(sk, VAR_OMP_THREADNUM)) { SetOpenMPThread((int) data); }
    else if (StringMatch(sk, VAR_CSWAT)) { m_CbnModel = (int) data; }
    else if (StringMatch(sk, Tag_CellWidth)) { m_cellWidth = data; }
    else {
        throw ModelException(MID_PLTMGT_SWAT, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

bool MGTOpt_SWAT::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputSize", "Input data for " + string(key) +
            " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException(MID_PLTMGT_SWAT, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input raster data should have same size.");
            return false;
        }
    }
    return true;
}

void MGTOpt_SWAT::Set1DData(const char *key, int n, float *data) {
    string sk(key);
    if (StringMatch(sk, VAR_SBGS))// TODO
    {
        m_deepWaterDepth = data;
        m_shallowWaterDepth = data;
        return;
    }
    CheckInputSize(key, n);
    if (StringMatch(sk, VAR_SUBBSN)) { m_subBsnID = data; }
    else if (StringMatch(sk, VAR_MGT_FIELD)) { m_mgtFields = data; }
    else if (StringMatch(sk, VAR_LANDUSE)) { m_landUse = data; }
    else if (StringMatch(sk, VAR_LANDCOVER)) { m_landCover = data; }
    else if (StringMatch(sk, VAR_IDC)) {
        m_landCoverCls = data;
        /// Soil related parameters from MongoDB
    } else if (StringMatch(sk, VAR_SOILLAYERS)) { m_nSoilLayers = data; }
    else if (StringMatch(sk, VAR_SOL_ZMX)) { m_soilZMX = data; }
    else if (StringMatch(sk, VAR_SOL_SUMAWC)) { m_soilSumFC = data; }
    else if (StringMatch(sk, VAR_T_BASE)) {
        m_tBase = data;
        ///  Plant operation related parameters
    } else if (StringMatch(sk, VAR_CN2)) { m_CN2 = data; }
    else if (StringMatch(sk, VAR_HVSTI)) { m_havstIdx = data; }
    else if (StringMatch(sk, VAR_WSYF)) { m_wtrStrsYF = data; }
    else if (StringMatch(sk, VAR_PHUPLT)) { m_phuPlant = data; }
    else if (StringMatch(sk, VAR_PHUBASE)) { m_phuBase = data; }
    else if (StringMatch(sk, VAR_IGRO)) { m_igro = data; }
    else if (StringMatch(sk, VAR_FR_PHU_ACC)) { m_phuAcc = data; }
    else if (StringMatch(sk, VAR_TREEYRS)) { m_curYearMat = data; }
    else if (StringMatch(sk, VAR_HVSTI_ADJ)) { m_havstIdxAdj = data; }
    else if (StringMatch(sk, VAR_LAIDAY)) { m_LAIDay = data; }
    else if (StringMatch(sk, VAR_DORMI)) { m_dormFlag = data; }
    else if (StringMatch(sk, VAR_LAIMAXFR)) { m_LAIMaxFr = data; }
    else if (StringMatch(sk, VAR_OLAI)) { m_oLAI = data; }
    else if (StringMatch(sk, VAR_PLANT_N)) { m_plantN = data; }
    else if (StringMatch(sk, VAR_PLANT_P)) { m_plantP = data; }
    else if (StringMatch(sk, VAR_FR_PLANT_N)) { m_frPlantN = data; }
    else if (StringMatch(sk, VAR_FR_PLANT_P)) { m_frPlantP = data; }
    else if (StringMatch(sk, VAR_PLTET_TOT)) { m_pltET = data; }
    else if (StringMatch(sk, VAR_PLTPET_TOT)) { m_pltPET = data; }
    else if (StringMatch(sk, VAR_FR_ROOT)) { m_frRoot = data; }
    else if (StringMatch(sk, VAR_BIOMASS)) {
        m_biomass = data;
        //// Harvest and Kill operation
    } else if (StringMatch(sk, VAR_LAST_SOILRD)) {
        m_lastSoilRootDepth = data;
        /// Irrigation operation
    } else if (StringMatch(sk, VAR_FR_STRSWTR)) {
        m_frStrsWa = data;
        //else if (StringMatch(sk, VAR_DEEPST)) m_deepWaterDepth = data;
        //else if (StringMatch(sk, VAR_SHALLST)) m_shallowWaterDepth = data;
        /// impound/release
    } else if (StringMatch(sk, VAR_POT_VOL)) { m_potVol = data; }
    else if (StringMatch(sk, VAR_POT_SA)) { m_impoundArea = data; }
    else if (StringMatch(sk, VAR_POT_NO3)) { m_potNo3 = data; }
    else if (StringMatch(sk, VAR_POT_NH4)) { m_potNH4 = data; }
    else if (StringMatch(sk, VAR_POT_SOLP)) { m_potSolP = data; }
    else if (StringMatch(sk, VAR_SOL_SW)) { m_soilStorageProfile = data; }
    else {
        throw ModelException(MID_PLTMGT_SWAT, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

bool MGTOpt_SWAT::CheckInputSize2D(const char *key, int n, int col) {
    CheckInputSize(key, n);
    if (col <= 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputSize2D", "Input data for " + string(key) +
            " is invalid. The layer number could not be less than zero.");
    }
    if (m_soilLayers != col) {
        if (m_soilLayers <= 0) {
            m_soilLayers = col;
        } else {
            throw ModelException(MID_PLTMGT_SWAT, "CheckInputSize2D", "Input data for " + string(key) +
                " is invalid. All the layers of input 2D raster data should have same size of " +
                ValueToString(m_soilLayers) + " instead of " +
                ValueToString(col) + ".");
            return false;
        }
    }
    return true;
}

void MGTOpt_SWAT::Set2DData(const char *key, int n, int col, float **data) {
    string sk(key);
    /// lookup tables
    if (StringMatch(sk, VAR_LANDUSE_LOOKUP)) {
        m_landuseLookup = data;
        m_landuseNum = n;
        initializeLanduseLookup();
        if (col != (int) LANDUSE_PARAM_COUNT) {
            throw ModelException(MID_PLTMGT_SWAT, "ReadLanduseLookup", "The field number " + ValueToString(col) +
                "is not coincident with LANDUSE_PARAM_COUNT: " +
                ValueToString((int) LANDUSE_PARAM_COUNT));
        }
        return;
    } else if (StringMatch(sk, VAR_CROP_LOOKUP)) {
        m_cropLookup = data;
        m_cropNum = n;
        initializeCropLookup();
        if (col != (int) CROP_PARAM_COUNT) {
            throw ModelException(MID_PLTMGT_SWAT, "ReadCropLookup", "The field number " + ValueToString(col) +
                "is not coincident with CROP_PARAM_COUNT: " +
                ValueToString((int) CROP_PARAM_COUNT));
        }
        return;
    } else if (StringMatch(sk, VAR_FERTILIZER_LOOKUP)) {
        m_fertilizerLookup = data;
        m_fertilizerNum = n;
        initializeFertilizerLookup();
        if (col != (int) FERTILIZER_PARAM_COUNT) {
            throw ModelException(MID_PLTMGT_SWAT, "ReadFertilizerLookup", "The field number " + ValueToString(col) +
                "is not coincident with FERTILIZER_PARAM_COUNT: " +
                ValueToString((int) FERTILIZER_PARAM_COUNT));
        }
        return;
    } else if (StringMatch(sk, VAR_TILLAGE_LOOKUP)) {
        m_tillageLookup = data;
        m_tillageNum = n;
        initializeTillageLookup();
        if (col != (int) TILLAGE_PARAM_COUNT) {
            throw ModelException(MID_PLTMGT_SWAT, "ReadTillageLookup", "The field number " + ValueToString(col) +
                "is not coincident with TILLAGE_PARAM_COUNT: " +
                ValueToString((int) TILLAGE_PARAM_COUNT));
        }
        return;
    }
    /// 2D raster data
    CheckInputSize2D(key, n, col);
    /// Soil related parameters from MongoDB
    if (StringMatch(sk, VAR_SOILDEPTH)) { m_soilDepth = data; }
    else if (StringMatch(sk, VAR_SOILTHICK)) { m_soilThick = data; }
    else if (StringMatch(sk, VAR_SOL_BD)) { m_soilBD = data; }
    else if (StringMatch(sk, VAR_SOL_CBN)) { m_soilCarbon = data; }
    else if (StringMatch(sk, VAR_SOL_N)) { m_soilN = data; }
    else if (StringMatch(sk, VAR_CLAY)) { m_soilClay = data; }
    else if (StringMatch(sk, VAR_SILT)) { m_soilSilt = data; }
    else if (StringMatch(sk, VAR_SAND)) { m_soilSand = data; }
    else if (StringMatch(sk, VAR_ROCK)) {
        m_soilRock = data;
        /// Soil related parameters --  inputs from other modules
    } else if (StringMatch(sk, VAR_SOL_SORGN)) { m_soilStableOrgN = data; }
    else if (StringMatch(sk, VAR_SOL_HORGP)) { m_soilOrgP = data; }
    else if (StringMatch(sk, VAR_SOL_SOLP)) { m_soilSolP = data; }
    else if (StringMatch(sk, VAR_SOL_NH4)) { m_soilNH4 = data; }
    else if (StringMatch(sk, VAR_SOL_NO3)) { m_soilNO3 = data; }
    else if (StringMatch(sk, VAR_SOL_AORGN)) { m_soilActiveOrgN = data; }
    else if (StringMatch(sk, VAR_SOL_FORGN)) { m_soilFreshOrgN = data; }
    else if (StringMatch(sk, VAR_SOL_FORGP)) { m_soilFreshOrgP = data; }
    else if (StringMatch(sk, VAR_SOL_ACTP)) { m_soilActiveMinP = data; }
    else if (StringMatch(sk, VAR_SOL_STAP)) { m_soilStableMinP = data; }
    else if (StringMatch(sk, VAR_SOL_RSD)) { m_soilRsd = data; }
    else if (StringMatch(sk, VAR_SOL_AWC)) { m_sol_fc = data; }
    else if (StringMatch(sk, VAR_SOL_UL)) { m_sol_sat = data; }
    else if (StringMatch(sk, VAR_SOL_ST)) {
        m_soilStorage = data;
        /// inputs for CENTURY C/N cycling model in stated and necessary
    } else if (StringMatch(sk, VAR_SOL_HSN)) { m_sol_HSN = data; }
    else if (StringMatch(sk, VAR_SOL_LM)) { m_sol_LM = data; }
    else if (StringMatch(sk, VAR_SOL_LMC)) { m_sol_LMC = data; }
    else if (StringMatch(sk, VAR_SOL_LMN)) { m_sol_LMN = data; }
    else if (StringMatch(sk, VAR_SOL_LSC)) { m_sol_LSC = data; }
    else if (StringMatch(sk, VAR_SOL_LSN)) { m_sol_LSN = data; }
    else if (StringMatch(sk, VAR_SOL_LS)) { m_sol_LS = data; }
    else if (StringMatch(sk, VAR_SOL_LSL)) { m_sol_LSL = data; }
    else if (StringMatch(sk, VAR_SOL_LSLC)) { m_sol_LSLC = data; }
    else if (StringMatch(sk, VAR_SOL_LSLNC)) {
        m_sol_LSLNC = data;
        //else if (StringMatch(sk, VAR_SOL_WON)) m_sol_WON = data;
        //else if (StringMatch(sk, VAR_SOL_BM)) m_sol_BM = data;
        //else if (StringMatch(sk, VAR_SOL_BMC)) m_sol_BMC = data;
    } else if (StringMatch(sk, VAR_SOL_BMN)) {
        m_sol_BMN = data;
        //else if (StringMatch(sk, VAR_SOL_HP)) m_sol_HP = data;
        //else if (StringMatch(sk, VAR_SOL_HS)) m_sol_HS = data;
        //else if (StringMatch(sk, VAR_SOL_HSC)) m_sol_HSC = data;
        //else if (StringMatch(sk, VAR_SOL_HPC)) m_sol_HPC = data;
    } else if (StringMatch(sk, VAR_SOL_HPN)) {
        m_sol_HPN = data;
        //else if (StringMatch(sk, VAR_SOL_RNMN)) m_sol_RNMN = data;
        //else if (StringMatch(sk, VAR_SOL_RSPC)) m_sol_RSPC = data;
    } else {
        throw ModelException(MID_PLTMGT_SWAT, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void MGTOpt_SWAT::SetScenario(Scenario *sce) {
    if (NULL == sce) {
        throw ModelException(MID_PLTMGT_SWAT, "SetScenario", "The Scenario data can not to be NULL.");
    }
    map<int, BMPFactory *> tmpBMPFactories = sce->GetBMPFactories();
    if (!m_mgtFactory.empty()) {
        m_mgtFactory.clear();
    }
    for (map<int, BMPFactory *>::iterator it = tmpBMPFactories.begin(); it != tmpBMPFactories.end(); it++) {
        /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
        if (it->first / 100000 == BMP_TYPE_PLANT_MGT) {
            /// calculate unique index for the key of m_mgtFactory, using Landuse_ID * 100 + subScenario
            int uniqueIdx = ((BMPPlantMgtFactory *) it->second)->GetLUCCID() * 100 + it->second->GetSubScenarioId();
            m_mgtFactory[uniqueIdx] = (BMPPlantMgtFactory *) it->second;
        }
    }
}

void MGTOpt_SWAT::SetSubbasins(clsSubbasins *subbasins) {
    if (NULL == subbasins) {
        throw ModelException(MID_PLTMGT_SWAT, "SetSubbasins", "The Subbasins data can not to be NULL.");
    } else {
        m_nSub = subbasins->GetSubbasinNumber();
        if (!m_nCellsSubbsn.empty() || !m_nAreaSubbsn.empty()) return;
        vector<int> subIDs = subbasins->GetSubbasinIDs();
        for (vector<int>::iterator it = subIDs.begin(); it != subIDs.end(); it++) {
            Subbasin *tmpSubbsn = subbasins->GetSubbasinByID(*it);
            m_nCellsSubbsn[*it] = tmpSubbsn->getCellCount();
            m_nAreaSubbsn[*it] = tmpSubbsn->getArea();
        }
    }
}

bool MGTOpt_SWAT::CheckInputData(void) {
    // DT_Single
    if (m_date <= 0) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "You have not set the time.");
    if (m_nCells <= 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    }
    if (m_cellWidth <= 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData",
                             "The cell width of the input data can not be less than zero.");
    }
    if (m_soilLayers <= 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData",
                             "The layer number of the input 2D raster data can not be less than zero.");
    }
    if (m_CbnModel < 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Carbon modeling method must be 0, 1, or 2");
    } else if (m_CbnModel == 2) {
        /// Check for the CENTURY required initialized variables
        if (m_sol_HSN == NULL) {
            ModelException(MID_PLTMGT_SWAT, "CheckInputData", "m_sol_HSN must not be NULL.");
        }
        if (m_sol_LM == NULL) {
            ModelException(MID_PLTMGT_SWAT, "CheckInputData", "m_sol_LM must not be NULL.");
        }
        if (m_sol_LMC == NULL) {
            ModelException(MID_PLTMGT_SWAT, "CheckInputData", "m_sol_LMC must not be NULL.");
        }
        if (m_sol_LMN == NULL) {
            ModelException(MID_PLTMGT_SWAT, "CheckInputData", "m_sol_LMN must not be NULL.");
        }
        if (m_sol_LSC == NULL) {
            ModelException(MID_PLTMGT_SWAT, "CheckInputData", "m_sol_LSC must not be NULL.");
        }
        if (m_sol_LSN == NULL) {
            ModelException(MID_PLTMGT_SWAT, "CheckInputData", "m_sol_LSN must not be NULL.");
        }
        if (m_sol_LS == NULL) {
            ModelException(MID_PLTMGT_SWAT, "CheckInputData", "m_sol_LS must not be NULL.");
        }
        if (m_sol_LSL == NULL) {
            ModelException(MID_PLTMGT_SWAT, "CheckInputData", "m_sol_LSL must not be NULL.");
        }
        if (m_sol_LSLC == NULL) {
            ModelException(MID_PLTMGT_SWAT, "CheckInputData", "m_sol_LSLC must not be NULL.");
        }
        if (m_sol_LSLNC == NULL) {
            ModelException(MID_PLTMGT_SWAT, "CheckInputData", "m_sol_LSLNC must not be NULL.");
        }
    }
    /// DT_Raster
    if (m_subBsnID == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "SubBasin ID must not be NULL");
    if (m_landUse == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Landuse must not be NULL");
    if (m_landCover == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Landcover must not be NULL");
    if (m_mgtFields == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Management fields must not be NULL");
    }
    if (m_tBase == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData",
                             "Base or minimum temperature for plant growth must not be NULL");
    }
    if (m_nSoilLayers == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil layers number must not be NULL");
    }
    if (m_soilZMX == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Maximum soil root depth must not be NULL");
    }
    if (m_soilSumFC == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData",
                             "Summary amount water in field capacity must not be NULL");
    }
    if (m_CN2 == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "CN2 value must not be NULL");
    if (m_igro == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Plant growth code must not be NULL");
    if (m_landCoverCls == NULL) {
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The land cover/plant classification can not be NULL.");
    }
    if (m_curYearMat == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Current growth year must not be NULL");
    }
    if (m_wtrStrsYF == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Value of harvest index must not be NULL");
    }
    if (m_LAIDay == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "LAI in current day must not be NULL");
    }
    if (m_phuBase == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Base heat units fraction must not be NULL");
    }
    if (m_phuAcc == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Accumulated heat units fraction must not be NULL");
    }
    if (m_phuPlant == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData",
                             "Heat units needed by plant to maturity must not be NULL");
    }
    if (m_dormFlag == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Dormancy flag must not be NULL");
    if (m_havstIdx == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Harvest index must not be NULL");
    if (m_havstIdxAdj == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Adjusted harvest index must not be NULL");
    }
    if (m_LAIMaxFr == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "LAI maximum fraction must not be NULL");
    }
    if (m_oLAI == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "oLAI must not be NULL");
    if (m_frPlantN == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Fraction of biomass in nitrogen must not be NULL");
    }
    if (m_frPlantP == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Fraction of biomass in phosphorus must not be NULL");
    }
    if (m_pltET == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData",
                             "Actual ET simulated during life of plant must not be NULL");
    }
    if (m_pltPET == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData",
                             "Potential ET simulated during life of plant must not be NULL");
    }
    if (m_frRoot == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Fraction of total biomass in roots must not be NULL");
    }
    if (m_biomass == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Biomass must not be NULL");
    if (m_lastSoilRootDepth == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Last root depth in soil must not be NULL");
    }
    if (m_deepWaterDepth == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Depth of water in deep aquifer must not be NULL");
    }
    if (m_shallowWaterDepth == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Depth of water in shallow aquifer must not be NULL");
    }
    /// DT_Raster2D
    if (m_soilDepth == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil depth must not be NULL");
    if (m_soilThick == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil thickness must not be NULL");
    if (m_soilBD == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil bulk density must not be NULL");
    if (m_soilN == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil nitrogen must not be NULL");
    if (m_soilCarbon == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil carbon content must not be NULL");
    }
    if (m_soilRock == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil rock content must not be NULL");
    }
    if (m_soilClay == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil clay content must not be NULL");
    }
    if (m_soilSand == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil sand content must not be NULL");
    }
    if (m_soilSilt == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil silt content must not be NULL");
    }
    if (m_soilActiveOrgN == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil active organic N must not be NULL");
    }
    if (m_soilFreshOrgN == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil fresh organic N must not be NULL");
    }
    if (m_soilFreshOrgP == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil fresh organic P must not be NULL");
    }
    if (m_soilNH4 == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil NH4 must not be NULL");
    if (m_soilNO3 == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil NO3 must not be NULL");
    if (m_soilStableOrgN == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil stable organic N must not be NULL");
    }
    if (m_soilOrgP == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil organic P must not be NULL");
    if (m_soilSolP == NULL) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil soluble P must not be NULL");
    if (m_soilActiveMinP == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil active mineral P must not be NULL");
    }
    if (m_soilStableMinP == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Soil stable mineral P must not be NULL");
    }
    if (m_soilRsd == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData",
                             "OM classified as residue in soil layers must not be NULL");
    }
    return true;
}

bool MGTOpt_SWAT::GetOperationCode(int i, int &factoryID, vector<int> &nOps) {
    if (m_mgtFactory.empty()) return false;
    int curLanduseID = int(m_landUse[i]);
    int curMgtField = int(m_mgtFields[i]);
    factoryID = -1;
    /// 1. Is there any plant management operations are suitable to current cell.
    for (map<int, BMPPlantMgtFactory *>::iterator it = m_mgtFactory.begin(); it != m_mgtFactory.end(); it++) {
        /// Key is unique plant management Index, which is calculated by Landuse_ID * 100 + subScenario;
        if (curLanduseID == (it->first) / 100) {
            /// 2. If current cell located in the locations of this BMPPlantMgtFactory
            ///    locations is empty means this operation may be applied to all cells
            if (it->second->GetLocations().empty() || ValueInVector(curMgtField, it->second->GetLocations())) {
                factoryID = it->first;
                break;
            }
        }
    }
    if (factoryID < 0) return false;
    /// 3. Figure out if any management operation should be applied, i.e., find sequence IDs (nOps)
    vector<int> tmpOpSeqences = m_mgtFactory[factoryID]->GetOperationSequence();
    map <int, PlantManagementOperation *> tmpOperations = m_mgtFactory[factoryID]->GetOperations();
    // get the next should be done sequence number
    int curSeq = m_doneOpSequence[i];
    int nextSeq = -1;
    if (curSeq == -1 || (unsigned) curSeq == tmpOpSeqences.size() - 1) {
        nextSeq = 0;
    } else {
        nextSeq = curSeq + 1;
    }
    int opCode = tmpOpSeqences[nextSeq];
    // figure out the nextSeq is satisfied or not.
    if (tmpOperations.find(opCode) != tmpOperations.end()) {
        PlantManagementOperation *tmpOperation = tmpOperations.at(opCode);
        /// *seqIter is calculated by: seqNo. * 1000 + operationCode
        bool dateDepent = false, huscDepent = false;
        /// If operation applied date (month and day) are defined
        if (tmpOperation->GetMonth() != 0 && tmpOperation->GetDay() != 0) {
            struct tm dateInfo;
            LocalTime(m_date, &dateInfo);
            if (dateInfo.tm_mon == tmpOperation->GetMonth() &&
                dateInfo.tm_mday == tmpOperation->GetDay()) {
                    dateDepent = true;
            }
        }
        /// If husc is defined
        if (tmpOperation->GetHUFraction() >= 0.f) {
            float aphu; /// fraction of total heat units accumulated
            if (FloatEqual(m_dormFlag[i], 1.f)) {
                aphu = NODATA_VALUE;
            } else {
                if (tmpOperation->UseBaseHUSC() && FloatEqual(m_igro[i], 0.f)) // use base hu
                {
                    aphu = m_phuBase[i];
                    if (aphu >= tmpOperation->GetHUFraction()) {
                        huscDepent = true;
                    }
                } else { // use accumulated plant hu
                    aphu = m_phuAcc[i];
                    if (aphu >= tmpOperation->GetHUFraction()) {
                        huscDepent = true;
                    }
                }
            }
        }
        /// The operation will be applied either date or HUSC are satisfied,
        /// and also in case of repeated run
        if (dateDepent || huscDepent) {
            nOps.push_back(opCode);
            m_doneOpSequence[i] = nextSeq; /// update value
        }
    }
    if (nOps.empty()) return false;
    return true;
}

void MGTOpt_SWAT::initializeLanduseLookup() {
    /// Check input data
    if (m_landuseLookup == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Landuse lookup array must not be NULL");
    }
    if (m_landuseNum <= 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Landuse number must be greater than 0");
    }
    if (!m_landuseLookupMap.empty()) {
        return;
    } else {
        for (int i = 0; i < m_landuseNum; i++) {
            m_landuseLookupMap[(int) m_landuseLookup[i][1]] = m_landuseLookup[i];
        }
    }
}

void MGTOpt_SWAT::initializeCropLookup() {
    /// Check input data
    if (m_cropLookup == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Crop lookup array must not be NULL");
    }
    if (m_cropNum <= 0) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Crop number must be greater than 0");

    if (!m_cropLookupMap.empty()) {
        return;
    } else {
        for (int i = 0; i < m_cropNum; i++) {
            m_cropLookupMap[(int) m_cropLookup[i][1]] = m_cropLookup[i];
        }
    }
}

void MGTOpt_SWAT::initializeFertilizerLookup() {
    /// Check input data
    if (m_fertilizerLookup == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Fertilizer lookup array must not be NULL");
    }
    if (m_fertilizerNum <= 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Fertilizer number must be greater than 0");
    }

    if (!m_fertilizerLookupMap.empty()) {
        return;
    } else {
        for (int i = 0; i < m_fertilizerNum; i++) {
            m_fertilizerLookupMap[(int) m_fertilizerLookup[i][1]] = m_fertilizerLookup[i];
        }
    }
}

void MGTOpt_SWAT::initializeTillageLookup() {
    /// Check input data
    if (m_tillageLookup == NULL) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Tillage lookup array must not be NULL");
    }
    if (m_tillageNum <= 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Tillage number must be greater than 0");
    }
    if (!m_tillageLookupMap.empty()) {
        return;
    } else {
        for (int i = 0; i < m_tillageNum; i++) {
            m_tillageLookupMap[(int) m_tillageLookup[i][1]] = m_tillageLookup[i];
        }
    }
}

void MGTOpt_SWAT::ExecutePlantOperation(int i, int &factoryID, int nOp) {
    PlantOperation *curOperation = (PlantOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    /// initialize parameters
    m_igro[i] = 1.f;
    m_HarvestIdxTarg[i] = curOperation->HITarg();
    m_BiomassTarg[i] = curOperation->BIOTarg(); /// kg/ha
    m_curYearMat[i] = curOperation->CurYearMaturity();
    int newPlantID = curOperation->PlantID();
    m_landCover[i] = newPlantID;
    m_phuPlant[i] = curOperation->HeatUnits();
    m_dormFlag[i] = 0.f;
    m_phuAcc[i] = 0.f;
    m_plantN[i] = 0.f;
    m_plantP[i] = 0.f;
    m_pltET[i] = 0.f;
    m_pltPET[i] = 0.f;
    m_LAIMaxFr[i] = 0.f;
    m_havstIdxAdj[i] = 0.f;
    m_oLAI[i] = 0.f;
    m_frRoot[i] = 0.f;
    /// update crop-related parameters in order to calculate phuAcc. by LJ
    if (m_cropLookupMap.find(newPlantID) == m_cropLookupMap.end()) {
        throw ModelException(MID_PLTMGT_SWAT, "ExecutePlantOperation",
                             "The new plant ID: " + ValueToString(newPlantID) +
                                 " is not prepared in cropLookup table!");
    }
    // update IDC
    m_landCoverCls[i] = m_cropLookupMap.at(newPlantID)[CROP_PARAM_IDX_IDC];
    m_tBase[i] = m_cropLookupMap.at(newPlantID)[CROP_PARAM_IDX_T_BASE];
    /// initialize transplant variables
    if (curOperation->LAIInit() > 0.f) {
        m_LAIDay[i] = curOperation->LAIInit();
        m_biomass[i] = curOperation->BIOInit();
    }
    /// compare maximum rooting depth in soil to maximum rooting depth of plant
    m_soilZMX[i] = m_soilDepth[i][(int) m_nSoilLayers[i] - 1];
    /// if the land cover does existed, throw an exception.
    if (m_landuseLookupMap.find(int(m_landCover[i])) == m_landuseLookupMap.end()) {
        throw ModelException(MID_PLTMGT_SWAT, "ExecutePlantOperation",
                             "Land use ID: " + ValueToString(int(m_landCover[i])) +
                                 " does not existed in Landuse lookup table, please check and retry!");
    }
    float pltRootDepth = m_landuseLookupMap[(int) m_landCover[i]][LANDUSE_PARAM_ROOT_DEPTH_IDX];
    m_soilZMX[i] = min(m_soilZMX[i], pltRootDepth);
    //if (i == 5878)
    //	cout<<"new plant: "<<newPlantID<<", IDC: "<<m_landCoverCls[5878]<<", tbase: "<<m_tBase[5878]<<", solZMX: "<<m_soilZMX[5878]<<endl;
    /// reset curve number if necessary
    if (curOperation->CNOP() > 0.f)   /// curno.f
    {
        float cnn = curOperation->CNOP();
        m_CN2[i] = cnn;
    }
}

void MGTOpt_SWAT::ExecuteIrrigationOperation(int i, int &factoryID, int nOp) {
    IrrigationOperation *curOperation = (IrrigationOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    /// initialize parameters
    /// irrigation source
    int m_irrSource;
    /// irrigation source location code
    int m_irrNo;
    /// irrigation apply depth (mm)
    float m_irrApplyDepth;
    /// float* m_irrSalt; /// currently not used
    /// irrigation efficiency
    float m_irrEfficiency;
    m_irrSource = curOperation->IRRSource();
    m_irrNo = (curOperation->IRRNo() <= 0) ? (int) m_subBsnID[i] : curOperation->IRRNo();
    m_irrApplyDepth = curOperation->IRRApplyDepth();
    m_irrEfficiency = curOperation->IRREfficiency();
    m_irrFlag[i] = 1;
    int tmpSubbsnID = (int) m_subBsnID[i];
    if (m_irrSource > IRR_SRC_RES) /// irrigation from reach and reservoir are irr_rch.f and irr_res.f, respectively
    {
        /// call irrsub.f
        /// Performs the irrigation operation when the source is the shallow or deep aquifer or a source outside the watershed
        float vmma = 0.f; /// amount of water in source, mm
        float vmm = 0.f;  ///maximum amount of water to be applied, mm
        float cnv = 0.f;    /// conversion factor (mm/ha => m^3)
        float vmxi = 0.f;   /// amount of water specified in irrigation operation, mm
        float vol = 0.f;      /// volume of water to be applied in irrigation, m^3
        float vmms = 0.f; /// amount of water in shallow aquifer, m^3
        float vmmd = 0.f; /// amount of water in deep aquifer, m^3
        /// Whether m_irrNo is valid
        if (m_nCellsSubbsn.find(m_irrNo) == m_nCellsSubbsn.end()) {
            m_irrNo = int(m_subBsnID[i]);
        }

        cnv = m_nAreaSubbsn[m_irrNo] * 10.f; /// area of current subbasin
        switch (m_irrSource) {
            /// in SEIMS, we hypothesis that shallow aquifer and deep aquifer is consistent within subbasin.
            case IRR_SRC_SHALLOW:
                if (m_shallowWaterDepth[tmpSubbsnID] < UTIL_ZERO) {
                    m_shallowWaterDepth[tmpSubbsnID] = 0.f;
                }
                vmma += m_shallowWaterDepth[tmpSubbsnID] * cnv * m_irrEfficiency;
                vmms = vmma;
                vmma /= m_nCellsSubbsn[m_irrNo];
                vmm = min(m_soilSumFC[i], vmma);
                break;
            case IRR_SRC_DEEP:vmma += m_deepWaterDepth[tmpSubbsnID] * cnv * m_irrEfficiency;
                vmmd = vmma;
                vmma /= m_nCellsSubbsn[m_irrNo];
                vmm = min(m_soilSumFC[i], vmma);
                break;
            case IRR_SRC_OUTWTSD: /// unlimited source
                vmm = m_soilSumFC[i];
                break;
        }
        /// if water available from source, proceed with irrigation
        if (vmm > 0.f) {
            cnv = m_cellArea * 10.f;
            vmxi = (m_irrApplyDepth < UTIL_ZERO) ? m_soilSumFC[i] : m_irrApplyDepth;
            if (vmm > vmxi) vmm = vmxi;
            vol = vmm * cnv;
            float pot_fr = 0.f;
            if (FloatEqual(m_impoundTriger[i], 0.f) && m_potVol != NULL) {
                /// m_impoundTriger equals to 0 means pot_fr is 1.
                /// and m_impoundArea is set to m_cellArea.
                pot_fr = 1.f;
                if (m_impoundArea != NULL) {
                    m_potVol[i] += vol / (10.f * m_impoundArea[i]);
                } else {
                    m_potVol[i] += vol / (10.f * m_cellArea);
                }
                m_appliedWater[i] = vmm;  ///added rice irrigation 11/10/11
            } else {
                pot_fr = 0.f;
                /// Call irrigate(i, vmm) /// irrigate.f
                m_appliedWater[i] = vmm * (1.f - curOperation->IRRSQfrac());
                m_irrSurfQWater[i] = vmm * curOperation->IRRSQfrac();
            }
            /// subtract irrigation from shallow or deep aquifer
            if (pot_fr > UTIL_ZERO) {
                vol = m_appliedWater[i] * cnv * m_irrEfficiency;
            }
            switch (m_irrSource) {
                case IRR_SRC_SHALLOW:cnv = m_nAreaSubbsn[m_irrNo] * 10.f;
                    vmma = 0.f;
                    if (vmms > -0.01f) {
                        vmma = vol * m_shallowWaterDepth[tmpSubbsnID] * cnv / vmms;
                    }
                    vmma /= cnv;
                    m_shallowWaterDepth[tmpSubbsnID] -= vmma;
                    if (m_shallowWaterDepth[tmpSubbsnID] < 0.f) {
                        vmma += m_shallowWaterDepth[tmpSubbsnID];
                        m_shallowWaterDepth[tmpSubbsnID] = 0.f;
                    }
                    m_shallowIrrWater[i] += vmma;
                    break;
                case IRR_SRC_DEEP:cnv = m_nAreaSubbsn[m_irrNo] * 10.f;
                    vmma = 0.f;
                    if (vmmd > 0.01f) {
                        vmma = vol * (m_deepWaterDepth[tmpSubbsnID] * cnv / vmmd);
                    }
                    vmma /= cnv;
                    m_deepWaterDepth[tmpSubbsnID] -= vmma;
                    if (m_deepWaterDepth[tmpSubbsnID] < 0.f) {
                        vmma += m_deepWaterDepth[tmpSubbsnID];
                        m_deepWaterDepth[tmpSubbsnID] = 0.f;
                    }
                    m_deepIrrWater[i] += vmma;
                    break;
            }
        }
    }
}

void MGTOpt_SWAT::ExecuteFertilizerOperation(int i, int &factoryID, int nOp) {
    /* Briefly change log
	 * 1. Translate from fert.f, remains CSWAT = 1 and 2 to be done!!! by LJ
	 * 2. CSWAT = 1 and 2, were implemented on 2016-9-29, by LJ.
	 * 3. Consider paddy rice field according to Chowdary et al., 2004, 2016-10-9, by LJ.
	 */
    //initializeFertilizerLookup();
    FertilizerOperation *curOperation = (FertilizerOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    /// fertilizer type, ifrt
    int fertilizerID = curOperation->FertilizerID();
    /// kg/ha         |amount of fertilizer applied to HRU
    float fertilizerKgHa = curOperation->FertilizerKg_per_ha();
    /* fraction of fertilizer which is applied to the top 10 mm of soil 
	 * the remaining fraction: 
     *  - is dissolved in the impounded water, if current landcover is paddy rice
	 *  - is applied to first soil layer defined by user, otherwise
	 */
    float fertilizerSurfFrac = curOperation->FertilizerSurfaceFrac();
    /// if the fertilizerID is not existed in lookup table, then throw an exception
    if (m_fertilizerLookupMap.find(fertilizerID) == m_fertilizerLookupMap.end()) {
        throw ModelException(MID_PLTMGT_SWAT, "ExecuteFertilizerOperation", "Fertilizer ID " +
            ValueToString(fertilizerID) +
            " is not existed in Fertilizer Database!");
    }
    /**fertilizer paramters derived from lookup table**/
    //!!    fminn(:)      |kg minN/kg frt|fraction of fertilizer that is mineral N (NO3 + NH4)
    float fertMinN = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_FMINN_IDX];
    //!!    fminp(:)      |kg minP/kg frt|fraction of fertilizer that is mineral P
    float fertMinP = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_FMINP_IDX];
    //!!    forgn(:)      |kg orgN/kg frt|fraction of fertilizer that is organic N
    float fertOrgN = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_FORGN_IDX];
    //!!    forgp(:)      |kg orgP/kg frt|fraction of fertilizer that is organic P
    float fertOrgP = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_FORGP_IDX];
    //!!    fnh4n(:)      |kg NH4-N/kgminN|fraction of mineral N in fertilizer that is NH4-N
    float fertNH4N = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_FNH4N_IDX];
    //!!    bactpdb(:)    |# cfu/g   frt |concentration of persistent bacteria in fertilizer
    //float bactPDB = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_BACTPDB_IDX];
    //!!    bactlpdb(:)   |# cfu/g   frt |concentration of less persistent bacteria in fertilizer
    //float bactLPDB = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_BATTLPDB_IDX];
    //!!    bactkddb(:)   |none          |fraction of bacteria in solution (the remaining fraction is sorbed to soil particles)
    //float bactKDDB = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_BACKTKDDB_IDX];
    // commercial fertilizer (0) or manure (1)
    int fertype = (int) m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_MANURE_IDX];
    /**summary output**/
    //!!    fertn         |kg N/ha       |total amount of nitrogen applied to soil in cell on day
    //float fertN = 0.f;
    //!!    fertp         |kg P/ha       |total amount of phosphorus applied to soil in cell on day
    //float fertP = 0.f;
    //float fertSolP = 0.f;
    /// cfertn       |kg N/ha       |total amount of nitrogen applied to soil during continuous fertilizer operation in cell on day
    //float cFertN = 0.f;
    /// cfertp       |kg P/ha       |total amount of phosphorus applied to soil during continuous fertilizer operation in cell on day
    //float cFertP = 0.f;
    /// weighting factor used to partition the organic N & P content of the fertilizer
    /// between the fresh organic and the active organic pools
    float rtof = 0.5f;
    float xx; /// fraction of fertilizer applied to layer
    float gc = 0.f; //, gc1 = 0.f;
    /// if current landcover is paddy rice, then apply the commercial fertilizer to the top surface and pothole.
    int lyrs = 2;
    if (m_potVol != NULL) {
        if (FloatEqual((int) m_landCover[i], CROP_PADDYRICE) && fertype == 0 && m_potVol[i] > 0.f) {
            lyrs = 1;
            xx = 1.f - fertilizerSurfFrac;
            m_potNo3[i] += xx * fertilizerKgHa * (1.f - fertNH4N) * fertMinN * m_cellArea; /// kg/ha * ha ==> kg
            m_potNH4[i] += xx * fertilizerKgHa * fertNH4N * fertMinN * m_cellArea;
            m_potSolP[i] += xx * fertilizerKgHa * fertMinP * m_cellArea;
            // if (i == 46364) cout<<"fert pot no3: "<<m_potNo3[46364]<<", nh4: "<<m_potNH4[46364]<<endl;
        }
    }
    for (int l = 0; l < lyrs; l++) /// top surface and first layer
    {
        if (l == 0) xx = fertilizerSurfFrac;
        if (l == 1) xx = 1.f - fertilizerSurfFrac;
        m_soilNO3[i][l] += xx * fertilizerKgHa * (1.f - fertNH4N) * fertMinN;
        if (m_CbnModel == 0) /// Static model
        {
            m_soilFreshOrgN[i][l] += rtof * xx * fertilizerKgHa * fertOrgN;
            m_soilActiveOrgN[i][l] += (1.f - rtof) * xx * fertilizerKgHa * fertOrgN;
            m_soilFreshOrgP[i][l] += rtof * xx * fertilizerKgHa * fertOrgP;
            m_soilOrgP[i][l] += (1.f - rtof) * xx * fertilizerKgHa * fertOrgP;
        } else if (m_CbnModel == 1) /// C-FARM one carbon pool model
        {
            m_soilManureC[i][l] += xx * fertilizerKgHa * fertOrgN * 10.f; /// assume C:N = 10:1
            m_soilManureN[i][l] += xx * fertilizerKgHa * fertOrgN;
            m_soilManureP[i][l] += xx * fertilizerKgHa * fertOrgP;
        } else if (m_CbnModel == 2) /// CENTURY model for C/N cycling
        {
            float X1 = 0.f, X8 = 0.f, X10 = 0.f, XXX = 0.f, YY = 0.f;
            float ZZ = 0.f, XZ = 0.f, YZ = 0.f, RLN = 0.f;
            /// the fraction of organic carbon in fertilizer, for most fertilizers orgc_f is set to 0.
            float orgc_f = 0.f;
            m_soilFreshOrgP[i][l] += rtof * xx * fertilizerKgHa * fertOrgP;
            m_soilOrgP[i][l] += (1.f - rtof) * xx * fertilizerKgHa * fertOrgP;
            /// allocate organic fertilizer to slow (SWAT active) N pool, i.e., m_soilActiveOrgN
            m_sol_HSN[i][l] += (1.f - rtof) * xx * fertilizerKgHa * fertOrgN;
            m_soilActiveOrgN[i][l] = m_sol_HSN[i][l];
            /// X1 is fertilizer applied to layer (kg/ha)
            X1 = xx * fertilizerKgHa;
            // X8 is organic carbon applied (kg C/ha)
            X8 = X1 * orgc_f;
            /// RLN is calculated as a function of C:N ration in fertilizer
            RLN = 0.175f * (orgc_f) / (fertMinN + fertOrgN + 1.e-5f);
            /// X10 is the fraction of carbon in fertilizer that is allocated to metabolic litter C pool
            X10 = 0.85f - 0.018f * RLN;
            if (X10 < 0.01f) { X10 = 0.01f; }
            else if (X10 > 0.7f) X10 = 0.7f;

            /// XXX is the amount of organic carbon allocated to metabolic litter C pool
            XXX = X8 * X10;
            m_sol_LMC[i][l] += XXX;
            /// YY is the amount of fertilizer (including C and N) allocated into metabolic litter SOM pool
            YY = X1 * X10;
            m_sol_LM[i][l] += YY;
            /// ZZ is amount of organic N allocated to metabolic litter N pool
            ZZ = X1 * rtof * fertOrgN * X10;
            m_sol_LMN[i][l] += ZZ;

            /// remaining organic N is allocated to structural litter N pool
            m_sol_LSN[i][l] += X1 * fertOrgN - ZZ;
            /// XZ is the amount of organic carbon allocated to structural litter C pool
            XZ = X1 * orgc_f - XXX;
            m_sol_LSC[i][l] += XZ;
            /// assuming lignin C fraction of organic carbon to be 0.175;
            float lignin_C_frac = 0.175f;
            /// updating lignin amount in structural litter pool
            m_sol_LSLC[i][l] += XZ * lignin_C_frac;
            /// non-lignin part of the structural litter C is also updated;
            m_sol_LSLNC[i][l] += XZ * (1.f - lignin_C_frac);
            /// YZ is the amount of fertilizer (including C and N) allocated into structural litter SOM pool
            YZ = X1 - YY;
            m_sol_LS[i][l] += YZ;
            /// assuming lignin fraction of the organic fertilizer allocated into structure litter SOM pool to be 0.175
            float lingnin_SOM_frac = 0.175f;
            /// update lignin weight in structural litter.
            m_sol_LSL[i][l] += YZ * lingnin_SOM_frac;
            m_soilFreshOrgN[i][l] = m_sol_LMN[i][l] + m_sol_LSN[i][l];
        }
        m_soilNH4[i][l] += xx * fertilizerKgHa * fertNH4N * fertMinN;
        m_soilSolP[i][l] += xx * fertilizerKgHa * fertMinP;
    }
    /// add bacteria - #cfu/g * t(manure)/ha * 1.e6g/t * ha/10,000m^2 = 100.
    /// calculate ground cover
    gc = (1.99532f - Erfc(1.333f * m_LAIDay[i] - 2.f)) / 2.1f;
    if (gc < 0.f) gc = 0.f;
    //gc1 = 1.f - gc;
    /// bact_swf    |none          |fraction of manure containing active colony forming units (cfu)
    //frt_t = m_bactSwf * fertilizerKg / 1000.;
    //m_bactPersistPlt[i] += gc * bactPDB * frt_t * 100.;
    //m_bactLessPersistPlt[i] += gc * bactLPDB * frt_t * 100.;

    //m_bactPersistSol[i] += gc1 * bactPDB * frt_t * 100.;
    //m_bactPersistSol[i] *= bactKDDB;

    //m_bactPersistParticle[i] += gc1 * bactPDB * frt_t * 100.;
    //m_bactPersistParticle[i] *= (1. - bactKDDB);

    //m_bactLessPersistSol[i] += gc1 * bactLPDB *frt_t * 100.;
    //m_bactLessPersistSol[i] *= bactKDDB;

    //m_bactLessPersistParticle[i] += gc1 * bactLPDB * frt_t * 100.;
    //m_bactLessPersistParticle[i] *= (1. - bactKDDB);

    /// summary calculations, currently not used for output. TODO in the future.
    //float fertNO3 = fertilizerKgHa * fertMinN * (1.f - fertNH4N);
    //float fertNH4 = fertilizerKgHa * (fertMinN * fertNH4N);
    //fertOrgN = fertilizerKgHa * fertOrgN;
    //fertOrgP = fertilizerKgHa * fertOrgP;
    //fertSolP = fertilizerKgHa * fertSolP;
    //fertN += (fertilizerKgHa + cFertN) * (fertMinN + fertOrgN); /// should be array, but cureently not useful
    //fertP += (fertilizerKgHa + cFertP) * (fertMinP + fertOrgP);
}

void MGTOpt_SWAT::ExecutePesticideOperation(int i, int &factoryID, int nOp) {
    /// TODO
}

void MGTOpt_SWAT::ExecuteHarvestKillOperation(int i, int &factoryID, int nOp) {
    //// TODO: Yield is not set as outputs yet. by LJ
    /// harvkillop.f
    HarvestKillOperation *curOperation = (HarvestKillOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    /// initialize parameters
    float cnop = curOperation->CNOP();
    float wur = 0.f, hiad1 = 0.f;
    if (m_cropLookupMap.find(int(m_landCover[i])) == m_cropLookupMap.end()) {
        throw ModelException(MID_PLTMGT_SWAT, "ExecuteHarvestKillOperation",
                             "The landcover ID " + ValueToString(m_landCover[i])
                                 + " is not existed in crop lookup table!");
    }
    /// Get some parameters of current crop / landcover
    float hvsti = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_HVSTI];
    float wsyf = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_WSYF];
    int idc = (int) m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_IDC];
    float bio_leaf = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_BIO_LEAF];
    float cnyld = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_CNYLD];
    float cpyld = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_CPYLD];

    /// calculate modifier for autofertilization target nitrogen content
    // TODO
    //tnyld(j) = 0.
    //tnyld(j) = (1. - rwt(j)) * bio_ms(j) * pltfr_n(j) * auto_eff(j)

    if (m_HarvestIdxTarg[i] > 0.f) {
        hiad1 = m_HarvestIdxTarg[i];
    } else {
        if (m_pltPET[i] < 10.f) {
            wur = 100.f;
        } else {
            wur = 100.f * m_pltET[i] / m_pltPET[i];
        }
        hiad1 = (m_havstIdxAdj[i] - wsyf) * (wur / (wur + exp(6.13f - 0.0883f * wur))) + wsyf;
        if (hiad1 > hvsti) hiad1 = hvsti;
    }
    /// check if yield is from above or below ground
    float yield = 0.f, resnew = 0.f, rtresnew = 0.f;

    /// stover fraction during harvest and kill operation
    float hi_ovr = curOperation->HarvestIndexOverride();
    float xx = curOperation->StoverFracRemoved();
    if (xx < UTIL_ZERO) {
        xx = hi_ovr;
    }
    if (hi_ovr > UTIL_ZERO) {
        yield = m_biomass[i] * hi_ovr;
        resnew = m_biomass[i] - yield;
    } else {
        if (idc == CROP_IDC_TREES) {
            yield = m_biomass[i] * (1.f - bio_leaf);
            resnew = m_biomass[i] - yield;
        } else {
            if (hvsti > 1.001f) {
                yield = m_biomass[i] * (1.f - 1.f / (1.f + hiad1));
                resnew = m_biomass[i] / (1.f + hiad1);
                resnew *= (1.f - xx);
            } else {
                yield = (1.f - m_frRoot[i]) * m_biomass[i] * hiad1;
                resnew = (1.f - m_frRoot[i]) * (1.f - hiad1) * m_biomass[i];
                /// remove stover during harvestKill operation
                resnew *= (1.f - xx);
                rtresnew = m_frRoot[i] * m_biomass[i];
            }
        }
    }
    if (yield < 0.f) yield = 0.f;
    if (resnew < 0.f) resnew = 0.f;
    if (rtresnew < 0.f) rtresnew = 0.f;

    if (m_CbnModel == 2) {
        m_grainc_d[i] += yield * 0.42f;
        m_stoverc_d[i] += (m_biomass[i] - yield - rtresnew) * 0.42f * xx;
        m_rsdc_d[i] += resnew * 0.42f;
        m_rsdc_d[i] += rtresnew * 0.42f;
    }
    /// calculate nutrient removed with yield
    float yieldn = 0.f, yieldp = 0.f;
    yieldn = yield * cnyld;
    yieldp = yield * cpyld;
    yieldn = min(yieldn, 0.80f * m_plantN[i]);
    yieldp = min(yieldp, 0.80f * m_plantP[i]);

    /// call rootfr.f to distributes dead root mass through the soil profile
    /// i.e., derive fraction of roots in each layer
    float *rtfr = new float[(int) m_nSoilLayers[i]];
    rootFraction(i, rtfr);

    /// fraction of N, P in residue (ff1) or roots (ff2)
    float ff1 = (1.f - hiad1) / (1.f - hiad1 + m_frRoot[i]);
    float ff2 = 1.f - ff1;
    /// update residue, N, P on soil surface
    m_soilRsd[i][0] += resnew;
    m_soilFreshOrgN[i][0] += ff1 * (m_plantN[i] - yieldn);
    m_soilFreshOrgP[i][0] += ff1 * (m_plantP[i] - yieldp);
    m_soilRsd[i][0] = max(m_soilRsd[i][0], 0.f);
    m_soilFreshOrgN[i][0] = max(m_soilFreshOrgN[i][0], 0.f);
    m_soilFreshOrgP[i][0] = max(m_soilFreshOrgP[i][0], 0.f);

    /// define variables of CENTURY model
    float BLG1 = 0.f, BLG2 = 0.f, BLG3 = 0.f, CLG = 0.f;
    float sf = 0.f, sol_min_n = 0.f, resnew_n = 0.f, resnew_ne = 0.f;
    float LMF = 0.f, LSF = 0.f;
    float RLN = 0.f, RLR = 0.f;
    /// insert new biomass of CENTURY model
    if (m_CbnModel == 2) {
        BLG1 = 0.01f / 0.10f;
        BLG2 = 0.99f;
        BLG3 = 0.10f;
        float XX = log(0.5f / BLG1 - 0.5f);
        BLG2 = (XX - log(1.f / BLG2 - 1.f)) / (1.f - 0.5f);
        BLG1 = XX + 0.5f * BLG2;
        CLG = BLG3 * m_phuAcc[i] / (m_phuAcc[i] + exp(BLG1 - BLG2 * m_phuAcc[i]));
        sf = 0.05f;
        sol_min_n = m_soilNO3[i][0] + m_soilNH4[i][0];
        resnew = resnew;
        resnew_n = ff1 * (m_plantN[i] - yieldn);
        resnew_ne = resnew_n + sf * sol_min_n;

        RLN = resnew * CLG / (resnew_n + 1.e-5f);
        RLR = min(0.8f, resnew * CLG / (resnew + 1.e-5f));
        LMF = 0.85f - 0.018f * RLN;
        if (LMF < 0.01f) { LMF = 0.01f; }
        else if (LMF > 0.7f) LMF = 0.7f;
        LSF = 1.f - LMF;
        m_sol_LM[i][0] += LMF * resnew;
        m_sol_LS[i][0] += LSF * resnew;

        m_sol_LSL[i][0] += RLR * resnew;
        m_sol_LSC[i][0] += 0.42f * LSF * resnew;

        m_sol_LSLC[i][0] += RLR * 0.42f * resnew;
        m_sol_LSLNC[i][0] = m_sol_LSC[i][0] - m_sol_LSLC[i][0];

        if (resnew_n > (0.42f * LSF * resnew / 150.f)) {
            m_sol_LSN[i][0] += 0.42f * LSF * resnew / 150.f;
            m_sol_LMN[i][0] += resnew_n - (0.42f * LSF * resnew / 150.f) + 1.e-25f;
        } else {
            m_sol_LSN[i][0] += resnew_n;
            m_sol_LMN[i][0] += 1.e-25f;
        }
        m_sol_LMC[i][0] += 0.42f * LMF * resnew;
        /// update no3 and nh4 in soil
        m_soilNO3[i][0] *= (1.f - sf);
        m_soilNH4[i][0] *= (1.f - sf);
    }
    /// end insert new biomass of CENTURY model

    /// allocate dead roots, N, P to soil layers
    for (int l = 0; l < m_nSoilLayers[i]; l++) {
        m_soilRsd[i][l] += rtfr[l] * rtresnew;
        m_soilFreshOrgN[i][l] += rtfr[l] * ff2 * (m_plantN[i] - yieldn);
        m_soilFreshOrgP[i][l] += rtfr[l] * ff2 * (m_plantP[i] - yieldp);

        /// insert new biomass of CENTURY model
        if (m_CbnModel == 2) {
            if (l == 1) { sf = 0.05f; }
            else { sf = 0.1f; }

            sol_min_n = m_soilNO3[i][l] + m_soilNH4[i][l]; // kg/ha
            resnew = rtfr[l] * rtresnew;
            resnew_n = rtfr[l] * ff2 * (m_plantN[i] - yieldn);
            resnew_ne = resnew_n + sf * sol_min_n;

            RLN = resnew * CLG / (resnew_n + 1.e-5f);
            RLR = min(0.8f, resnew * CLG / 1000.f / (resnew / 1000.f + 1.e-5f));
            LMF = 0.85f - 0.018f * RLN;
            if (LMF < 0.01f) { LMF = 0.01f; }
            else if (LMF > 0.7f) LMF = 0.7f;

            LSF = 1.f - LMF;
            m_sol_LM[i][l] += LMF * resnew;
            m_sol_LS[i][l] += LSF * resnew;

            /// here a simplified assumption of 0.5 LSL
            //LSLF = 0.f;
            //LSLF = CLG;

            m_sol_LSL[i][l] += RLR * LSF * resnew;
            m_sol_LSC[i][l] += 0.42f * LSF * resnew;

            m_sol_LSLC[i][l] += RLR * 0.42f * LSF * resnew;
            m_sol_LSLNC[i][l] = m_sol_LSC[i][l] - m_sol_LSLC[i][l];

            if (resnew_ne > (0.42f * LSF * resnew / 150.f)) {
                m_sol_LSN[i][l] += 0.42f * LSF * resnew / 150.f;
                m_sol_LMN[i][l] += resnew_ne - (0.42f * LSF * resnew / 150.f) + 1.e-25f;
            } else {
                m_sol_LSN[i][l] += resnew_ne;
                m_sol_LMN[i][l] += 1.e-25f;
            }
            m_sol_LMC[i][l] += 0.42f * LMF * resnew;
            /// update no3 and nh4 in soil
            m_soilNO3[i][l] *= (1.f - sf);
            m_soilNH4[i][l] *= (1.f - sf);
        }
        /// end insert new biomass of CENTURY model
    }
    if (cnop > 0.f) {
        m_CN2[i] = cnop;
    } /// TODO: Is necessary to isolate the curno.f function for SUR_CN?
    /// reset variables
    m_igro[i] = 0.f;
    m_dormFlag[i] = 0.f;
    m_biomass[i] = 0.f;
    m_frRoot[i] = 0.f;
    m_plantN[i] = 0.f;
    m_plantP[i] = 0.f;
    m_frStrsWa[i] = 1.f;
    m_LAIDay[i] = 0.f;
    m_havstIdxAdj[i] = 0.f;
    Release1DArray(rtfr);
    m_phuAcc[i] = 0.f;
    m_phuPlant[i] = 0.f;
}

void MGTOpt_SWAT::rootFraction(int i, float *&root_fr) {
    float cum_rd = 0.f, cum_d = 0.f, cum_rf = 0.f, x1 = 0.f, x2 = 0.f;
    if (m_lastSoilRootDepth[i] < UTIL_ZERO) {
        root_fr[0] = 1.f;
        return;
    }
    /// Normalized Root Density = 1.15*exp[-11.7*NRD] + 0.022, where NRD = normalized rooting depth
    /// Parameters of Normalized Root Density Function from Dwyer et al 19xx
    float a = 1.15f, b = 11.7f, c = 0.022f,
        d = 0.12029f; /// Integral of Normalized Root Distribution Function  from 0 to 1 (normalized depth) = 0.12029
    int k;/// used as layer identifier
    for (int l = 0; l < (int) m_nSoilLayers[i]; l++) {
        cum_d += m_soilThick[i][l];
        if (cum_d >= m_lastSoilRootDepth[i]) cum_rd = m_lastSoilRootDepth[i];
        if (cum_d < m_lastSoilRootDepth[i]) cum_rd = cum_d;
        x1 = (cum_rd - m_soilThick[i][l]) / m_lastSoilRootDepth[i];
        x2 = cum_rd / m_lastSoilRootDepth[i];
        float xx1 = -b * x1;
        if (xx1 > 20.f) xx1 = 20.f;
        float xx2 = -b * x2;
        if (xx2 > 20.f) xx2 = 20.f;
        root_fr[l] = (a / b * (exp(xx1) - exp(xx2)) + c * (x2 - x1)) / d;
        float xx = cum_rf;
        cum_rf += root_fr[l];
        if (cum_rf > 1.f) {
            root_fr[l] = 1.f - xx;
            cum_rf = 1.f;
        }
        k = l;
        if (cum_rd >= m_lastSoilRootDepth[i]) {
            break;
        }
    }
    /// ensures that cumulative fractional root distribution = 1
    for (int l = 0; l < (int) m_nSoilLayers[i]; l++) {
        root_fr[l] /= cum_rf;
        if (l == k) { /// exits loop on the same layer as the previous loop
            break;
        }
    }
}

void MGTOpt_SWAT::ExecuteTillageOperation(int i, int &factoryID, int nOp) {
    /// newtillmix.f
    /// Mix residue and nutrients during tillage and biological mixing
    TillageOperation *curOperation = (TillageOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    /// initialize parameters
    int tillID = curOperation->TillageID();
    float cnop = curOperation->CNOP();
    if (m_tillageLookupMap.find(tillID) == m_tillageLookupMap.end()) {
        throw ModelException(MID_PLTMGT_SWAT, "ExecuteTillageOperation", "The tillage ID " + ValueToString(tillID)
            + " is not existed in tillage lookup table!");
    }
    float deptil = m_tillageLookupMap[tillID][TILLAGE_PARAM_DEPTIL_IDX];
    float effmix = m_tillageLookupMap[tillID][TILLAGE_PARAM_EFFMIX_IDX];
    float bmix = 0.f;
    float emix = 0.f, dtil = 0.f, XX = 0.f, WW1 = 0.f, WW2 = 0.f;
    float WW3 = 0.f, WW4 = 0.f, maxmix = 0.f;
    float *soilMass = new float[(int) m_nSoilLayers[i]];
    float *soilMixedMass = new float[(int) m_nSoilLayers[i]];  /// mass of soil mixed for the layer
    float *soilNotMixedMass = new float[(int) m_nSoilLayers[i]];  /// mass of soil not mixed for the layer

    if (bmix > UTIL_ZERO) {
        /// biological mixing, TODO, in SWAT, this occurs at the end of year process.
        emix = bmix;
        dtil = min(m_soilDepth[i][(int) m_nSoilLayers[i] - 1], 50.f);
    } else {
        /// tillage operation
        emix = effmix;
        dtil = deptil;
    }
    if (tillID >= 1) {
        /// TODO, this is associated with erosion modules. Drainmod tile equations
        ///Updated dynamic depressional storage D.Moriasi 4/8/2014
        //cumei(jj)   = 0.
        //cumeira(jj) = 0.
        //cumrt(jj)   = 0.
        //cumrai(jj)  = 0.
        //ranrns_hru(jj) = ranrns(idtill)
    }
    if (m_CbnModel == 2) /// DSSAT tillage
    {
        m_tillage_days[i] = 0;
        m_tillage_depth[i] = dtil;
        m_tillage_switch[i] = 1;
    }
    ///  smix(:)     |varies        |amount of substance in soil profile that is being redistributed between mixed layers
    int npmx = 0; /// number of different pesticides, TODO in next version
    //float *smix = new float[22 + npmx + 12];
    //for (int k = 0; k < 22 + npmx + 12; k++)
    //    smix[k] = 0.f;
    float *smix(NULL);
    Initialize1DArray(22 + npmx + 12, smix, 0.f);
    /// incorporate bacteria, no mixing, and lost from transport
    if (dtil > 10.f) {
        //m_bactPersistSol[i] *= (1. - emix);
        //m_bactPersistParticle[i] *= (1. - emix);
        //m_bactLessPersistSol[i] *= (1. - emix);
        //m_bactLessPersistParticle[i] *= (1. - emix);
    }
    /// calculate max mixing to preserve target surface residue
    /// Assume residue in all other layers is negligible to simplify calculation and remove depth dependency
    /// TODO, m_minResidue is defined in OPS files in SWAT as residue management operation, 
    ///       which should be implemented later as BMP operation. By LJ
    float m_minResidue = 10.f; /// currently, set m_minResidue to 10 as default.
    if (m_minResidue > 1.f && bmix < 0.001f) {
        maxmix = 1.f - m_minResidue / m_soilRsd[i][0];
        if (maxmix < 0.05f) maxmix = 0.05f;
        if (emix > maxmix) emix = maxmix;
    }
    for (int l = 0; l < (int) m_nSoilLayers[i]; l++) {
        soilMass[l] = 10000.f * m_soilThick[i][l] * m_soilBD[i][l] * (1 - m_soilRock[i][l] / 100.f);
        soilMixedMass[l] = 0.f;
        soilNotMixedMass[l] = 0.f;
    }
    if (dtil > 0.f) {
        if (dtil < 10.f) dtil = 11.;
        for (int l = 0; l < (int) m_nSoilLayers[i]; l++) {
            if (m_soilDepth[i][l] <= dtil) {
                soilMixedMass[l] = emix * soilMass[l];
                soilNotMixedMass[l] = soilMass[l] - soilMixedMass[l];
            } else if (m_soilDepth[i][l] > dtil && m_soilDepth[i][l - 1] < dtil) {
                soilMixedMass[l] = emix * soilMass[l] * (dtil - m_soilDepth[i][l - 1]) / m_soilThick[i][l];
                soilNotMixedMass[l] = soilMass[l] - soilMixedMass[l];
            } else {
                soilMixedMass[l] = 0.f;
                soilNotMixedMass[l] = soilMass[l];
            }
            /// calculate the mass or concentration of each mixed element
            /// 1. mass based mixing
            WW1 = soilMixedMass[l] / (soilMixedMass[l] + soilNotMixedMass[l]);
            smix[0] += m_soilNO3[i][l] * WW1;
            smix[1] += m_soilStableOrgN[i][l] * WW1;
            smix[2] += m_soilNH4[i][l] * WW1;
            smix[3] += m_soilSolP[i][l] * WW1;
            smix[4] += m_soilOrgP[i][l] * WW1;
            smix[5] += m_soilActiveOrgN[i][l] * WW1;
            smix[6] += m_soilActiveMinP[i][l] * WW1;
            smix[7] += m_soilFreshOrgN[i][l] * WW1;
            smix[8] += m_soilFreshOrgP[i][l] * WW1;
            smix[9] += m_soilStableMinP[i][l] * WW1;
            smix[10] += m_soilRsd[i][l] * WW1;
            if (m_CbnModel == 1) /// C-FARM one carbon pool model
            {
                smix[11] += m_soilManureC[i][l] * WW1;
                smix[12] += m_soilManureN[i][l] * WW1;
                smix[13] += m_soilManureP[i][l] * WW1;
            }

            /// 2. concentration based mixing
            WW2 = XX + soilMixedMass[l];
            smix[14] = (XX * smix[14] + m_soilCarbon[i][l] * soilMixedMass[l]) / WW2;
            smix[15] = (XX * smix[15] + m_soilN[i][l] * soilMixedMass[l]) / WW2;
            smix[16] = (XX * smix[16] + m_soilClay[i][l] * soilMixedMass[l]) / WW2;
            smix[17] = (XX * smix[17] + m_soilSilt[i][l] * soilMixedMass[l]) / WW2;
            smix[18] = (XX * smix[18] + m_soilSand[i][l] * soilMixedMass[l]) / WW2;
            /// 3. mass based distribution
            for (int k = 0; k < npmx; k++) {
                /// TODO
                /// smix[19+k] += sol_pst(k,jj,l) * WW1
            }
            /// 4. For CENTURY model
            if (m_CbnModel == 2) {
                smix[19 + npmx + 1] += m_sol_LSC[i][l] * WW1;
                smix[19 + npmx + 2] += m_sol_LSLC[i][l] * WW1;
                smix[19 + npmx + 3] += m_sol_LSLNC[i][l] * WW1;
                smix[19 + npmx + 4] += m_sol_LMC[i][l] * WW1;
                smix[19 + npmx + 5] += m_sol_LM[i][l] * WW1;
                smix[19 + npmx + 6] += m_sol_LSL[i][l] * WW1;
                smix[19 + npmx + 7] += m_sol_LS[i][l] * WW1;

                smix[19 + npmx + 8] += m_sol_LSN[i][l] * WW1;
                smix[19 + npmx + 9] += m_sol_LMN[i][l] * WW1;
                smix[19 + npmx + 10] += m_sol_BMN[i][l] * WW1;
                smix[19 + npmx + 11] += m_sol_HSN[i][l] * WW1;
                smix[19 + npmx + 12] += m_sol_HPN[i][l] * WW1;
            }
            XX += soilMixedMass[l];
        }
        for (int l = 0; l < (int) m_nSoilLayers[i]; l++) {
            /// reconstitute each soil layer
            WW3 = soilNotMixedMass[l] / soilMass[l];
            WW4 = soilMixedMass[l] / XX;
            m_soilNO3[i][l] = m_soilNO3[i][l] * WW3 + smix[0] * WW4;
            m_soilStableOrgN[i][l] = m_soilStableOrgN[i][l] * WW3 + smix[1] * WW4;
            m_soilNH4[i][l] = m_soilNH4[i][l] * WW3 + smix[2] * WW4;
            m_soilSolP[i][l] = m_soilSolP[i][l] * WW3 + smix[3] * WW4;
            m_soilOrgP[i][l] = m_soilOrgP[i][l] * WW3 + smix[4] * WW4;
            m_soilActiveOrgN[i][l] = m_soilActiveOrgN[i][l] * WW3 + smix[5] * WW4;
            m_soilActiveMinP[i][l] = m_soilActiveMinP[i][l] * WW3 + smix[6] * WW4;
            m_soilFreshOrgN[i][l] = m_soilFreshOrgN[i][l] * WW3 + smix[7] * WW4;
            m_soilFreshOrgP[i][l] = m_soilFreshOrgP[i][l] * WW3 + smix[8] * WW4;
            m_soilStableMinP[i][l] = m_soilStableMinP[i][l] * WW3 + smix[9] * WW4;
            m_soilRsd[i][l] = m_soilRsd[i][l] * WW3 + smix[10] * WW4;
            if (m_soilRsd[i][l] < 1.e-10f) m_soilRsd[i][l] = 1.e-10f;
            if (m_CbnModel == 1) {
                m_soilManureC[i][l] = m_soilManureC[i][l] * WW3 + smix[11] * WW4;
                m_soilManureN[i][l] = m_soilManureN[i][l] * WW3 + smix[12] * WW4;
                m_soilManureP[i][l] = m_soilManureP[i][l] * WW3 + smix[13] * WW4;
            }
            m_soilCarbon[i][l] = (m_soilCarbon[i][l] * soilNotMixedMass[l] + smix[14] * soilMixedMass[l]) / soilMass[l];
            m_soilN[i][l] = (m_soilN[i][l] * soilNotMixedMass[l] + smix[15] * soilMixedMass[l]) / soilMass[l];
            m_soilClay[i][l] = (m_soilClay[i][l] * soilNotMixedMass[l] + smix[16] * soilMixedMass[l]) / soilMass[l];
            m_soilSilt[i][l] = (m_soilSilt[i][l] * soilNotMixedMass[l] + smix[17] * soilMixedMass[l]) / soilMass[l];
            m_soilSand[i][l] = (m_soilSand[i][l] * soilNotMixedMass[l] + smix[18] * soilMixedMass[l]) / soilMass[l];

            for (int k = 0; k < npmx; k++) {
                /// TODO
                /// sol_pst(k,jj,l) = sol_pst(k,jj,l) * WW3 + smix(20+k) * WW4
            }
            if (m_CbnModel == 2) {
                m_sol_LSC[i][l] = m_sol_LSC[i][l] * WW3 + smix[19 + npmx + 1] * WW4;
                m_sol_LSLC[i][l] = m_sol_LSLC[i][l] * WW3 + smix[19 + npmx + 2] * WW4;
                m_sol_LSLNC[i][l] = m_sol_LSLNC[i][l] * WW3 + smix[19 + npmx + 3] * WW4;
                m_sol_LMC[i][l] = m_sol_LMC[i][l] * WW3 + smix[19 + npmx + 4] * WW4;
                m_sol_LM[i][l] = m_sol_LM[i][l] * WW3 + smix[19 + npmx + 5] * WW4;
                m_sol_LSL[i][l] = m_sol_LSL[i][l] * WW3 + smix[19 + npmx + 6] * WW4;
                m_sol_LS[i][l] = m_sol_LS[i][l] * WW3 + smix[19 + npmx + 7] * WW4;
                m_sol_LSN[i][l] = m_sol_LSN[i][l] * WW3 + smix[19 + npmx + 8] * WW4;
                m_sol_LMN[i][l] = m_sol_LMN[i][l] * WW3 + smix[19 + npmx + 9] * WW4;
                m_sol_BMN[i][l] = m_sol_BMN[i][l] * WW3 + smix[19 + npmx + 10] * WW4;
                m_sol_HSN[i][l] = m_sol_HSN[i][l] * WW3 + smix[19 + npmx + 11] * WW4;
                m_sol_HPN[i][l] = m_sol_HPN[i][l] * WW3 + smix[19 + npmx + 12] * WW4;
            }
            if (m_CbnModel == 1) {
                /// TODO
                /// call tillfactor(jj,bmix,emix,dtil,sol_thick)
            }
        }
    }
    if (cnop > 1.e-4f) m_CN2[i] = cnop;
    if (soilMass != NULL) Release1DArray(soilMass);
    if (soilMixedMass != NULL) Release1DArray(soilMixedMass);
    if (soilNotMixedMass != NULL) Release1DArray(soilNotMixedMass);
}

void MGTOpt_SWAT::ExecuteHarvestOnlyOperation(int i, int &factoryID, int nOp) {
    /// TODO to be implemented!
    /// harvestop.f
    //  HarvestOnlyOperation *curOperation = (HarvestOnlyOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    //  /// initialize parameters
    //  float hi_bms = curOperation->HarvestIndexBiomass();
    //  float hi_rsd = curOperation->HarvestIndexResidue();
    //  float harveff = curOperation->HarvestEfficiency();
    //  if (m_cropLookupMap.find(int(m_landCover[i])) == m_cropLookupMap.end())
    //throw ModelException(MID_PLTMGT_SWAT, "ExecuteHarvestOnlyOperation", "The landcover ID " + ValueToString(m_landCover[i])
    //+ " is not existed in crop lookup table!");
    //  /// Get some parameters of current crop / landcover
    //  float hvsti = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_HVSTI];
    //  float wsyf = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_WSYF];
    //  int idc = (int) m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_IDC];
    //  float bio_leaf = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_BIO_LEAF];
    //  float cnyld = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_CNYLD];
    //  float cpyld = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_CPYLD];
    //  float alai_min = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_ALAI_MIN];

    //  float yieldTbr = 0.f, yieldNtbr = 0.f, yieldPtbr = 0.f;
    //  float clipTbr = 0.f, clipNtbr = 0.f, clipPtbr = 0.f;
    //  float ssb = m_biomass[i];
    //  float ssabg = m_biomass[i] * (1.f - m_frRoot[i]);
    //  float ssr = ssb * m_frRoot[i];
    //  float ssn = m_plantN[i];
    //  float ssp = m_plantP[i];
    //  /// calculate modifier for auto fertilization target nitrogen content
    //  m_targNYld[i] = (1.f - m_frRoot[i] * m_biomass[i] * m_frPlantN[i] * m_autoFertEfficiency[i]);
    //  /// compute grain yield
    //  float hiad1 = 0.f; /// hiad1       |none           |actual harvest index (adj for water/growth)
    //  float wur = 0.f; /// wur         |none           |water deficiency factor
    //  if (m_pltPET[i] < 10.f)
    //      wur = 100.f;
    //  else
    //      wur = 100.f * m_pltET[i] / m_pltPET[i];
    //  hiad1 = (m_havstIdxAdj[i] - wsyf) * (wur / (wur + exp(6.13f - 0.0883f * wur))) + wsyf;
    //  if (hiad1 > hvsti) hiad1 = hvsti;
    //  /// check if yield is from above or below ground
    //  if (hvsti > 1.001f)
    //  {
    //      /// determine clippings (biomass left behind) and update yield
    //      yieldTbr = m_biomass[i] * (1.f - 1.f / (1.f + hiad1)) * harveff;
    //      clipTbr = m_biomass[i] * (1.f - 1.f / (1.f + hiad1)) * (1.f - harveff);
    //      m_biomass[i] -= (yieldTbr + clipTbr);
    //      /// calculate nutrients removed with yield
    //      yieldNtbr = min(yieldTbr * cnyld, 0.80f * m_plantN[i]);
    //      yieldPtbr = min(yieldTbr * cpyld, 0.80f * m_plantP[i]);
    //      /// calculate nutrients removed with clippings
    //      clipNtbr = min(clipTbr * m_frPlantN[i], m_plantN[i] - yieldNtbr);
    //      clipPtbr = min(clipTbr * m_frPlantP[i], m_plantP[i] - yieldPtbr);
    //      m_plantN[i] -= (yieldNtbr + clipNtbr);
    //      m_plantP[i] -= (yieldPtbr + clipPtbr);
    //  }
    //  float yieldBms = 0.f, yieldNbms = 0.f, yieldPbms = 0.f;
    //  float clipBms = 0.f, clipNbms = 0.f, clipPbms = 0.f;
    //  float yieldGrn = 0.f, yieldNgrn = 0.f, yieldPgrn = 0.f;
    //  float clipGrn = 0.f, clipNgrn = 0.f, clipPgrn = 0.f;

    //  if (hi_bms > 0.f)
    //  {
    //      /// compute biomass yield
    //      yieldBms = hi_bms * (1.f - m_frRoot[i]) * m_biomass[i] * harveff;
    //      clipBms = hi_bms * (1.f - m_frRoot[i]) * m_biomass[i] * (1.f - harveff);
    //      m_biomass[i] -= (yieldBms + clipBms);
    //      /// compute nutrients removed with yield
    //      yieldNbms = min(yieldBms * cnyld, 0.80f * m_plantN[i]);
    //      yieldPbms = min(yieldBms * cpyld, 0.80f * m_plantP[i]);
    //      /// calculate nutrients removed with clippings
    //      clipNbms = min(clipBms * m_frPlantN[i], m_plantN[i] - yieldNbms);
    //      clipPbms = min(clipBms * m_frPlantP[i], m_plantP[i] - yieldPbms);
    //      m_plantN[i] -= (yieldNbms + clipNbms);
    //      m_plantP[i] -= (yieldPbms + clipPbms);
    //  } else
    //  {
    //      /// compute grain yields
    //      yieldGrn = (1.f - m_frRoot[i]) * m_biomass[i] * hiad1 * harveff;
    //      /// determine clippings (biomass left behind) and update yield
    //      clipGrn = (1.f - m_frRoot[i]) * m_biomass[i] * hiad1 * (1.f - harveff);
    //      m_biomass[i] -= (yieldGrn + clipGrn);
    //      /// calculate nutrients removed with yield
    //      yieldNgrn = min(yieldGrn * cnyld, 0.80f * m_plantN[i]);
    //      yieldPgrn = min(yieldGrn * cpyld, 0.80f * m_plantP[i]);
    //      /// calculate nutrients removed with clippings
    //      clipNgrn = min(clipGrn * m_frPlantN[i], m_plantN[i] - yieldNgrn);
    //      clipPgrn = min(clipGrn * m_frPlantP[i], m_plantP[i] - yieldPgrn);
    //      m_plantN[i] -= (yieldNgrn + clipNgrn);
    //      m_plantP[i] -= (yieldPgrn + clipPgrn);
    //  }
    //  /// add clippings to residue and organic N and P
    //  m_soilRsd[i][0] += (clipGrn + clipBms + clipTbr);
    //  m_soilFreshOrgN[i][0] += (clipNgrn + clipNbms + clipNtbr);
    //  m_soilFreshOrgP[i][0] += (clipPgrn + clipPbms + clipPtbr);
    //  /// compute residue yield
    //  float yieldRsd = 0.f, yieldNrsd = 0.f, yieldPrsd = 0.f;
    //  if (hi_rsd > 0.)
    //  {
    //      yieldRsd = hi_rsd * m_soilRsd[i][0];
    //      yieldNrsd = hi_rsd * m_soilFreshOrgN[i][0];
    //      yieldPrsd = hi_rsd * m_soilFreshOrgP[i][0];
    //      m_soilRsd[i][0] -= yieldRsd;
    //      m_soilFreshOrgN[i][0] -= yieldNrsd;
    //      m_soilFreshOrgP[i][0] -= yieldPrsd;
    //  }
    //  float yield = 0.f, yieldN = 0.f, yieldP = 0.f;
    //  yield = yieldGrn + yieldBms + yieldTbr + yieldRsd;
    //  yieldN = yieldNgrn + yieldNbms + yieldNtbr + yieldNrsd;
    //  yieldP = yieldPgrn + yieldPbms + yieldPtbr + yieldPrsd;
    //  float clip = 0.f, clipN = 0.f, clipP = 0.f;
    //  clip = clipGrn + clipBms + clipTbr;
    //  clipN = clipNgrn + clipNbms + clipNtbr;
    //  clipP = clipPgrn + clipPbms + clipNtbr;

    //  /// Calculation for dead roots allocations, resetting phenology, updating other pools
    //  float ff3 = 0.f;
    //  if (ssabg > UTIL_ZERO)
    //      ff3 = (yield + clip) / ssabg;
    //  else
    //      ff3 = 1.f;
    //  if (ff3 > 1.f) ff3 = 1.f;
    //  /// reset leaf area index and fraction of growing season
    //  if (ssb > 0.001f)
    //  {
    //      m_LAIDay[i] *= (1.f - ff3);
    //      if (m_LAIDay[i] < alai_min)
    //          m_LAIDay[i] = alai_min;
    //      m_phuAcc[i] *= (1.f - ff3);
    //      m_frRoot[i] = 0.4f - 0.2f * m_phuAcc[i];
    //  } else
    //  {
    //      m_biomass[i] = 0.f;
    //      m_LAIDay[i] = 0.f;
    //      m_phuAcc[i] = 0.f;
    //  }
    //  /// allocate roots, N, and P to soil pools
    //  for (int l = 0; l < (int) m_nSoilLayers[i]; l++)
    //  {
    //      /// TODO, check it out in the near future.
    //  }
}

void MGTOpt_SWAT::ExecuteKillOperation(int i, int &factoryID, int nOp) {
    /// killop.f
    // KillOperation *curOperation = (KillOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    float resnew = 0.f, rtresnew = 0.f;
    resnew = m_biomass[i] * (1.f - m_frRoot[i]);
    rtresnew = m_biomass[i] * m_frRoot[i];
    /// call rootfr.f to distributes dead root mass through the soil profile
    /// i.e., derive fraction of roots in each layer
    float *rtfr = new float[(int) m_nSoilLayers[i]];
    rootFraction(i, rtfr);
    /// update residue, N, P on soil surface
    m_soilRsd[i][0] += resnew;
    m_soilFreshOrgN[i][0] += m_plantN[i] * (1.f - m_frRoot[i]);
    m_soilFreshOrgP[i][0] += m_plantP[i] * (1.f - m_frRoot[i]);
    m_soilRsd[i][0] = max(m_soilRsd[i][0], 0.f);
    m_soilFreshOrgN[i][0] = max(m_soilFreshOrgN[i][0], 0.f);
    m_soilFreshOrgP[i][0] = max(m_soilFreshOrgP[i][0], 0.f);

    /// allocate dead roots, N and P to soil layers
    for (int l = 0; l < (int) m_nSoilLayers[i]; l++) {
        m_soilRsd[i][l] += rtfr[l] * rtresnew;
        m_soilFreshOrgN[i][l] += rtfr[l] * m_plantN[i] * m_frRoot[i];
        m_soilFreshOrgP[i][l] += rtfr[l] * m_plantP[i] * m_frRoot[i];
    }
    /// reset variables
    m_igro[i] = 0.f;
    m_dormFlag[i] = 0.f;
    m_biomass[i] = 0.f;
    m_frRoot[i] = 0.f;
    m_plantN[i] = 0.f;
    m_plantP[i] = 0.f;
    m_frStrsWa[i] = 1.f;
    m_LAIDay[i] = 0.f;
    m_havstIdxAdj[i] = 0.f;
    Release1DArray(rtfr);
    m_phuAcc[i] = 0.f;
}

void MGTOpt_SWAT::ExecuteGrazingOperation(int i, int &factoryID, int nOp) {
    /// TODO
    /// graze.f, simulate biomass lost to grazing
    //GrazingOperation *curOperation = (GrazingOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    //int manureID = curOperation->ManureID();
    //int grzDays = curOperation->GrazingDays();
    //float bioEat = curOperation->BiomassConsumed();
    //float bioTrmp = curOperation->BiomassTrampled();
    //float manueKg = curOperation->ManureDeposited();
}

void MGTOpt_SWAT::ExecuteAutoIrrigationOperation(int i, int &factoryID, int nOp) {
    AutoIrrigationOperation *curOperation = (AutoIrrigationOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    m_autoIrrSource[i] = curOperation->AutoIrrSrcCode();
    m_autoIrrNo[i] = (curOperation->AutoIrrSrcLocs() <= 0) ? (int) m_subBsnID[i] : curOperation->AutoIrrSrcLocs();
    m_wtrStrsID[i] = curOperation->WaterStrsIdent();
    m_autoWtrStres[i] = curOperation->AutoWtrStrsThrsd();
    m_autoIrrEfficiency[i] = curOperation->IrrigationEfficiency();
    m_autoIrrWtrDepth[i] = curOperation->IrrigationWaterApplied();
    m_autoSurfRunRatio[i] = curOperation->SurfaceRunoffRatio();
    m_irrFlag[i] = 1;
    /// call autoirr.f
    /// TODO, this will be implemented as an isolated module in the near future.
}

void MGTOpt_SWAT::ExecuteAutoFertilizerOperation(int i, int &factoryID, int nOp) {
    AutoFertilizerOperation *curOperation = (AutoFertilizerOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    m_fertilizerID[i] = curOperation->FertilizerID();
    m_NStressCode[i] = curOperation->NitrogenMethod();
    m_autoNStress[i] = curOperation->NitrogenStrsFactor();
    m_autoMaxAppliedN[i] = curOperation->MaxMineralN();
    m_autoAnnMaxAppliedMinN[i] = curOperation->MaxMineralNYearly();
    m_autoFertEfficiency[i] = curOperation->FertEfficiency();
    m_autoFertSurface[i] = curOperation->SurfaceFracApplied();
    if (m_cropLookupMap.find(int(m_landCover[i])) == m_cropLookupMap.end()) {
        return;
    }
    float cnyld = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_CNYLD];
    float bio_e = m_cropLookupMap[(int) m_landCover[i]][CROP_PARAM_IDX_BIO_E];
    /// calculate modifier for auto fertilization target nitrogen content'
    if (m_targNYld[i] < UTIL_ZERO) {
        m_targNYld[i] = 150.f * cnyld * bio_e;
    }
    /// call anfert.f
    /// TODO, this will be implemented as an isolated module in the near future.
}

void MGTOpt_SWAT::ExecuteReleaseImpoundOperation(int i, int &factoryID, int nOp) {
    /// No more executable code here.
    ReleaseImpoundOperation *curOperation = (ReleaseImpoundOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    m_impoundTriger[i] = curOperation->ImpoundTriger();
    /// pothole.f and potholehr.f for sub-daily timestep simulation, TODO
    /// IF IMP_SWAT module is not configured, then this operation will be ignored. By LJ
    if (m_potVol == NULL) {
        return;
    }
    /// 1. pothole module has been added by LJ, 2016-9-6, IMP_SWAT
    /// paddy rice module should be added!
    m_potVolMax[i] = curOperation->MaxDepth();
    m_potVolLow[i] = curOperation->LowDepth();
    if (FloatEqual(m_impoundTriger[i], 0.f)) {
        /// Currently, add pothole volume (mm) to the max depth directly (in case of infiltration).
        /// TODO, autoirrigation operations should be triggered. BY lj
        m_potVol[i] = curOperation->MaxDepth();
        /// force the soil water storage to field capacity
        for (int ly = 0; ly < (int) m_nSoilLayers[i]; ly++) {
            // float dep2cap = m_sol_sat[i][ly] - m_soilStorage[i][ly];
            float dep2cap = m_sol_fc[i][ly] - m_soilStorage[i][ly];
            if (dep2cap > 0.f) {
                dep2cap = min(dep2cap, m_potVol[i]);
                m_soilStorage[i][ly] += dep2cap;
                m_potVol[i] -= dep2cap;
            }
        }
        if (m_potVol[i] < curOperation->UpDepth()) {
            m_potVol[i] = curOperation->UpDepth();
        } /// force to reach the up depth.
        /// recompute total soil water storage
        m_soilStorageProfile[i] = 0.f;
        for (int ly = 0; ly < (int) m_nSoilLayers[i]; ly++)
            m_soilStorageProfile[i] += m_soilStorage[i][ly];
    } else {
        m_potVolMax[i] = 0.f;
        m_potVolLow[i] = 0.f;
    }
}

void MGTOpt_SWAT::ExecuteContinuousFertilizerOperation(int i, int &factoryID, int nOp) {
    // TODO
    // ContinuousFertilizerOperation *curOperation = (ContinuousFertilizerOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
}

void MGTOpt_SWAT::ExecuteContinuousPesticideOperation(int i, int &factoryID, int nOp) {
    /// TODO
    // ContinuousPesticideOperation *curOperation = (ContinuousPesticideOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
}

void MGTOpt_SWAT::ExecuteBurningOperation(int i, int &factoryID, int nOp) {
    // BurningOperation *curOperation = (BurningOperation *) m_mgtFactory[factoryID]->GetOperations()[nOp];
    /// TODO
}

void MGTOpt_SWAT::ScheduledManagement(int cellIdx, int &factoryID, int nOp) {
    /// nOp is seqNo. * 1000 + operationCode
    int mgtCode = nOp % 1000;
    switch (mgtCode) {
        case BMP_PLTOP_Plant:ExecutePlantOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Irrigation:ExecuteIrrigationOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Fertilizer:ExecuteFertilizerOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Pesticide:ExecutePesticideOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_HarvestKill:ExecuteHarvestKillOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Tillage:ExecuteTillageOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Harvest:ExecuteHarvestOnlyOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Kill:ExecuteKillOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Grazing:ExecuteGrazingOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_AutoIrrigation:ExecuteAutoIrrigationOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_AutoFertilizer:ExecuteAutoFertilizerOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_ReleaseImpound:ExecuteReleaseImpoundOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_ContinuousFertilizer:ExecuteContinuousFertilizerOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_ContinuousPesticide:ExecuteContinuousPesticideOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Burning:ExecuteBurningOperation(cellIdx, factoryID, nOp);
            break;
    }
}

int MGTOpt_SWAT::Execute() {
    CheckInputData();  /// essential input data, other inputs for specific management operation will be check separately.
    initialOutputs(); /// all possible outputs will be initialized to avoid NULL pointer problems.
    /// initialize arrays at the beginning of the current day, derived from sim_initday.f of SWAT
    //cout<<"PLTMGT_SWAT, pre: "<<m_soilSolP[46364][0];
    //int cellid = 918;
    //cout<<"PLTMGT_SWAT, cell id: "<<cellid<<", sol_no3[0]: "<<m_soilNO3[cellid][0]<<endl;
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_grainc_d != NULL) m_grainc_d[i] = 0.f;
        if (m_stoverc_d != NULL) m_stoverc_d[i] = 0.f;
        if (m_rsdc_d != NULL) m_rsdc_d[i] = 0.f;
    }
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        int curFactoryID = -1;
        vector<int> curOps;
        /* Output HUSC to txt files for debugging purpose */
        //if (i == 8144){
        //	ofstream fs;
        //	utils util;
        //	string filename = "e:\\husc.txt";
        //	fs.open(filename.c_str(), ios::out|ios::app);
        //	if (fs.is_open())
        //	{
        //		fs << util.ConvertToString(&this->m_date) <<", IGRO: "<<m_igro[i]<<", phuBase: "<<m_phuBase[i]<<", phuAcc: "<<m_phuAcc[i]<<", phuPlt: "<<m_phuPlant[i]<< endl;
        //		fs.close();
        //	}
        //}
        if (GetOperationCode(i, curFactoryID, curOps)) {
            for (vector<int>::iterator it = curOps.begin(); it != curOps.end(); it++) {
                //cout<<curFactoryID<<","<<*it<<endl;
                ScheduledManagement(i, curFactoryID, *it);
                // output for debug, by LJ.
                //if (i == 8144){
                //	ofstream fs;
                //	utils util;
                //	//string filename = "D:\\pltMgt.txt";
                //	string filename = "pltMgt.txt";
                //	fs.open(filename.c_str(), ios::out|ios::app);
                //	if (fs.is_open())
                //	{
                //		fs << util.ConvertToString(&this->m_date) <<", IGRO: "<<m_igro[i]<<", phuBase: "<<m_phuBase[i]<<", phuAcc: "<<m_phuAcc[i]<<", phuPlt: "<<m_phuPlant[i]<<", optCode: "<<*it<< endl;
                //		fs.close();
                //	}
                //}
            }
        }
    }
    //cout<<"PLTMGT_SWAT, cell id: "<<cellid<<", sol_no3[0]: "<<m_soilNO3[cellid][0]<<endl;
    //cout<<", new: "<<m_soilSolP[46364][0]<<endl;
    return 0;
}

void MGTOpt_SWAT::Get1DData(const char *key, int *n, float **data) {
    initialOutputs();
    string sk(key);
    *n = m_nCells;
    /// plant operation
    if (StringMatch(sk, VAR_HITARG)) { *data = m_HarvestIdxTarg; }
    else if (StringMatch(sk, VAR_BIOTARG)) {
        *data = m_BiomassTarg;
        /// auto irrigation operation
    } else if (StringMatch(sk, VAR_IRR_FLAG)) { *data = m_irrFlag; }
    else if (StringMatch(sk, VAR_IRR_WTR)) { *data = m_appliedWater; }
    else if (StringMatch(sk, VAR_IRR_SURFQ)) { *data = m_irrSurfQWater; }
    else if (StringMatch(sk, VAR_AWTR_STRS_ID)) { *data = m_wtrStrsID; }
    else if (StringMatch(sk, VAR_AWTR_STRS_TRIG)) { *data = m_autoWtrStres; }
    else if (StringMatch(sk, VAR_AIRR_SOURCE)) { *data = m_autoIrrSource; }
    else if (StringMatch(sk, VAR_AIRR_LOCATION)) { *data = m_autoIrrNo; }
    else if (StringMatch(sk, VAR_AIRR_EFF)) { *data = m_autoIrrEfficiency; }
    else if (StringMatch(sk, VAR_AIRRWTR_DEPTH)) { *data = m_autoIrrWtrDepth; }
    else if (StringMatch(sk, VAR_AIRRSURF_RATIO)) {
        *data = m_autoSurfRunRatio;
        /// fertilizer / auto fertilizer operation
    } else if (StringMatch(sk, VAR_AFERT_ID)) { *data = m_fertilizerID; }
    else if (StringMatch(sk, VAR_AFERT_NSTRSID)) { *data = m_NStressCode; }
    else if (StringMatch(sk, VAR_AFERT_NSTRS)) { *data = m_autoNStress; }
    else if (StringMatch(sk, VAR_AFERT_MAXN)) { *data = m_autoMaxAppliedN; }
    else if (StringMatch(sk, VAR_AFERT_AMAXN)) { *data = m_autoAnnMaxAppliedMinN; }
    else if (StringMatch(sk, VAR_AFERT_NYLDT)) { *data = m_targNYld; }
    else if (StringMatch(sk, VAR_AFERT_FRTEFF)) { *data = m_autoFertEfficiency; }
    else if (StringMatch(sk, VAR_AFERT_FRTSURF)) {
        *data = m_autoFertSurface;
        /// Grazing operation
    } else if (StringMatch(sk, VAR_GRZ_DAYS)) { *data = m_nGrazingDays; }
    else if (StringMatch(sk, VAR_GRZ_FLAG)) {
        *data = m_grzFlag;
        /// Impound/Release operation
    } else if (StringMatch(sk, VAR_IMPOUND_TRIG)) { *data = m_impoundTriger; }
    else if (StringMatch(sk, VAR_POT_VOLMAXMM)) { *data = m_potVolMax; }
    else if (StringMatch(sk, VAR_POT_VOLLOWMM)) {
        *data = m_potVolLow;
        /// tillage operation of CENTURY model
    } else if (StringMatch(sk, VAR_TILLAGE_DAYS)) { *data = m_tillage_days; }
    else if (StringMatch(sk, VAR_TILLAGE_DEPTH)) { *data = m_tillage_depth; }
    else if (StringMatch(sk, VAR_TILLAGE_FACTOR)) { *data = m_tillage_factor; }
    else if (StringMatch(sk, VAR_TILLAGE_SWITCH)) { *data = m_tillage_switch; }
    else {
        throw ModelException(MID_PLTMGT_SWAT, "Get1DData", "Parameter " + sk + " is not existed!");
    }
}

void MGTOpt_SWAT::Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
    initialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_soilLayers;
    /// fertilizer operation
    if (StringMatch(sk, VAR_SOL_MC)) { *data = m_soilManureC; }
    else if (StringMatch(sk, VAR_SOL_MN)) { *data = m_soilManureN; }
    else if (StringMatch(sk, VAR_SOL_MP)) { *data = m_soilManureP; }
    else {
        throw ModelException(MID_PLTMGT_SWAT, "Get1DData", "Parameter " + sk + " is not existed!");
    }
}

void MGTOpt_SWAT::initialOutputs() {
    if (m_cellArea < 0.f) m_cellArea = m_cellWidth * m_cellWidth / 10000.f; // unit: ha
    /// figure out all the management codes, and initialize the corresponding variables, aimed to save memory. By LJ
    if (m_initialized) return;
    vector<int> definedMgtCodes;
    for (map<int, BMPPlantMgtFactory *>::iterator it = m_mgtFactory.begin(); it != m_mgtFactory.end(); it++) {
        int factoryID = it->first;
        vector<int> tmpOpSeqences = m_mgtFactory[factoryID]->GetOperationSequence();
        for (vector<int>::iterator seqIter = tmpOpSeqences.begin(); seqIter != tmpOpSeqences.end(); seqIter++) {
            /// *seqIter is calculated by: seqNo. * 1000 + operationCode
            int curMgtCode = *seqIter % 1000;
            if (find(definedMgtCodes.begin(), definedMgtCodes.end(), curMgtCode) == definedMgtCodes.end()) {
                definedMgtCodes.push_back(curMgtCode);
            }
        }
    }
    /// plant operation
    if (find(definedMgtCodes.begin(), definedMgtCodes.end(), BMP_PLTOP_Plant) != definedMgtCodes.end()) {
        if (m_HarvestIdxTarg == NULL) Initialize1DArray(m_nCells, m_HarvestIdxTarg, 0.f);
        if (m_BiomassTarg == NULL) Initialize1DArray(m_nCells, m_BiomassTarg, 0.f);
    }
    /// irrigation / auto irrigation operations
    if (find(definedMgtCodes.begin(), definedMgtCodes.end(), BMP_PLTOP_Irrigation) != definedMgtCodes.end() ||
        find(definedMgtCodes.begin(), definedMgtCodes.end(), BMP_PLTOP_AutoIrrigation) != definedMgtCodes.end()) {
        if (m_appliedWater == NULL) Initialize1DArray(m_nCells, m_appliedWater, 0.f);
        if (m_irrSurfQWater == NULL) Initialize1DArray(m_nCells, m_irrSurfQWater, 0.f);
        if (m_irrFlag == NULL) Initialize1DArray(m_nCells, m_irrFlag, 0.f);
        if (m_autoIrrSource == NULL) Initialize1DArray(m_nCells, m_autoIrrSource, (float) IRR_SRC_OUTWTSD);
        if (m_autoIrrNo == NULL) Initialize1DArray(m_nCells, m_autoIrrNo, -1.f);
        if (m_wtrStrsID == NULL) Initialize1DArray(m_nCells, m_wtrStrsID, 1.f); /// By default, plant water demand
        if (m_autoWtrStres == NULL) Initialize1DArray(m_nCells, m_autoWtrStres, 0.f);
        if (m_autoIrrEfficiency == NULL) Initialize1DArray(m_nCells, m_autoIrrEfficiency, 0.f);
        if (m_autoIrrWtrDepth == NULL) Initialize1DArray(m_nCells, m_autoIrrWtrDepth, 0.f);
        if (m_autoSurfRunRatio == NULL) Initialize1DArray(m_nCells, m_autoSurfRunRatio, 0.f);
    }
    /// fertilizer / auto fertilizer operations
    if (find(definedMgtCodes.begin(), definedMgtCodes.end(), BMP_PLTOP_Fertilizer) != definedMgtCodes.end() ||
        find(definedMgtCodes.begin(), definedMgtCodes.end(), BMP_PLTOP_AutoFertilizer) != definedMgtCodes.end()) {
        if (m_fertilizerID == NULL) Initialize1DArray(m_nCells, m_fertilizerID, -1.f);
        if (m_NStressCode == NULL) Initialize1DArray(m_nCells, m_NStressCode, 0.f);
        if (m_autoNStress == NULL) Initialize1DArray(m_nCells, m_autoNStress, 0.f);
        if (m_autoMaxAppliedN == NULL) Initialize1DArray(m_nCells, m_autoMaxAppliedN, 0.f);
        if (m_targNYld == NULL) Initialize1DArray(m_nCells, m_targNYld, 0.f);
        if (m_autoAnnMaxAppliedMinN == NULL) Initialize1DArray(m_nCells, m_autoAnnMaxAppliedMinN, 0.f);
        if (m_autoFertEfficiency == NULL) Initialize1DArray(m_nCells, m_autoFertEfficiency, 0.f);
        if (m_autoFertSurface == NULL) Initialize1DArray(m_nCells, m_autoFertSurface, 0.f);

        if (m_CbnModel == 1) {
            if (m_soilManureC == NULL) Initialize2DArray(m_nCells, m_soilLayers, m_soilManureC, 0.f);
            if (m_soilManureN == NULL) Initialize2DArray(m_nCells, m_soilLayers, m_soilManureN, 0.f);
            if (m_soilManureP == NULL) Initialize2DArray(m_nCells, m_soilLayers, m_soilManureP, 0.f);
        }
    }
    /// impound/release operation
    if (find(definedMgtCodes.begin(), definedMgtCodes.end(), BMP_PLTOP_ReleaseImpound) != definedMgtCodes.end()) {
        if (m_impoundTriger == NULL) Initialize1DArray(m_nCells, m_impoundTriger, 1.f);
        if (m_potVolMax == NULL) Initialize1DArray(m_nCells, m_potVolMax, 0.f);
        if (m_potVolLow == NULL) Initialize1DArray(m_nCells, m_potVolLow, 0.f);
    }
    /// tillage
    if (find(definedMgtCodes.begin(), definedMgtCodes.end(), BMP_PLTOP_Tillage) != definedMgtCodes.end()) {
        if (m_CbnModel == 2) {
            if (m_tillage_days == NULL) Initialize1DArray(m_nCells, m_tillage_days, 0.f);
            if (m_tillage_switch == NULL) Initialize1DArray(m_nCells, m_tillage_switch, 0.f);
            if (m_tillage_depth == NULL) Initialize1DArray(m_nCells, m_tillage_depth, 0.f);
            if (m_tillage_factor == NULL) Initialize1DArray(m_nCells, m_tillage_factor, 0.f);
        }
    }
    /// harvestkill
    if (find(definedMgtCodes.begin(), definedMgtCodes.end(), BMP_PLTOP_HarvestKill) != definedMgtCodes.end()) {
        if (m_CbnModel == 2) {
            if (m_grainc_d == NULL) Initialize1DArray(m_nCells, m_grainc_d, 0.f);
            if (m_stoverc_d == NULL) Initialize1DArray(m_nCells, m_stoverc_d, 0.f);
            if (m_rsdc_d == NULL) Initialize1DArray(m_nCells, m_rsdc_d, 0.f);
        }
    }
    if (m_doneOpSequence == NULL) Initialize1DArray(m_nCells, m_doneOpSequence, -1);
    m_initialized = true;
}

float MGTOpt_SWAT::Erfc(float xx) {
    float c1 = .19684f, c2 = .115194f;
    float c3 = .00034f, c4 = .019527f;
    float x = 0.f, erf = 0.f, erfc = 0.f;
    x = abs(sqrt(2.f) * xx);
    erf = 1.f - pow(float(1.f + c1 * x + c2 * x * x + c3 * pow(x, 3.f) + c4 * pow(x, 4.f)), -4.f);
    if (xx < 0.f) erf = -erf;
    erfc = 1.f - erf;
    return erfc;
}
