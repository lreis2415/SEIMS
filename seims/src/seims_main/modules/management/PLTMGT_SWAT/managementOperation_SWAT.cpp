#include "managementOperation_SWAT.h"

#include "text.h"
#include "PlantGrowthCommon.h"

MGTOpt_SWAT::MGTOpt_SWAT() :
    m_subSceneID(-1), m_nCells(-1), m_cellWth(NODATA_VALUE), m_cellArea(NODATA_VALUE),
    m_nSubbsns(-1), m_subbsnID(nullptr),
    /// add parameters from MongoDB
    m_landUse(nullptr), m_landCover(nullptr), m_mgtFields(nullptr),
    m_nSoilLyrs(nullptr), m_maxSoilLyrs(-1),
    /// Soil related parameters from MongoDB
    m_soilDepth(nullptr), m_soilThick(nullptr), m_soilMaxRootD(nullptr), m_soilBD(nullptr),
    m_soilSumFC(nullptr), m_soilN(nullptr), m_soilCbn(nullptr), m_soilRock(nullptr),
    m_soilClay(nullptr), m_soilSand(nullptr), m_soilSilt(nullptr), m_soilActvOrgN(nullptr),
    /// Soil related parameters
    m_soilFrshOrgN(nullptr), m_soilFrshOrgP(nullptr), m_soilNH4(nullptr),
    m_soilNO3(nullptr), m_soilStabOrgN(nullptr), m_soilHumOrgP(nullptr),
    m_soilSolP(nullptr), m_pgTempBase(nullptr),
    /// Plant operation related parameters
    m_doneOpSequence(nullptr), m_landuseLookup(nullptr), m_landuseNum(-1), m_cn2(nullptr),
    m_igro(nullptr),
    m_landCoverCls(nullptr), m_HvstIdxTrgt(nullptr), m_biomTrgt(nullptr),
    m_curYrMat(nullptr), m_wtrStrsHvst(nullptr), m_lai(nullptr), m_phuBase(nullptr),
    m_phuAccum(nullptr),
    m_phuPlt(nullptr), m_dormFlag(nullptr), m_hvstIdx(nullptr),
    m_hvstIdxAdj(nullptr), m_laiMaxFr(nullptr), m_oLai(nullptr),
    m_frPltN(nullptr), m_frPltP(nullptr), m_pltN(nullptr), m_pltP(nullptr),
    m_totActPltET(nullptr), m_totPltPET(nullptr), m_frRoot(nullptr),
    /// Harvest and Kill, harvest, harvgrain operation
    m_biomass(nullptr), m_soilRsd(nullptr), m_frStrsWtr(nullptr), m_cropLookup(nullptr),
    m_cropNum(-1),
    m_fertLookup(nullptr), m_fertNum(-1), m_cbnModel(0),
    /// Fertilizer operation
    m_soilManC(nullptr), m_soilManN(nullptr), m_soilManP(nullptr),
    m_soilHSN(nullptr), m_soilLM(nullptr), m_soilLMC(nullptr),
    /// Irrigation
    m_soilLMN(nullptr), m_soilLSC(nullptr), m_soilLSN(nullptr), m_soilLS(nullptr),
    m_soilLSL(nullptr),
    m_soilLSLC(nullptr), m_soilLSLNC(nullptr), m_tillSwitch(nullptr),
    /// auto irrigation operation
    m_tillDepth(nullptr), m_tillDays(nullptr), m_tillFactor(nullptr),
    m_soilBMN(nullptr),
    m_soilHPN(nullptr),
    m_irrFlag(nullptr), m_irrWtrAmt(nullptr),
    /// bacteria related
    //m_bactSwf(NODATA),	m_bactLessPersistPlt(nullptr), m_bactLessPersistSol(nullptr), m_bactLessPersistParticle(nullptr),
    //m_bactPersistPlt(nullptr), m_bactPersistSol(nullptr), m_bactPersistParticle(nullptr),
    /// Tillage operation
    m_irrWtr2SurfqAmt(nullptr), m_deepWaterDepth(nullptr), m_shallowWaterDepth(nullptr),
    /// tillage factor on SOM decomposition, used by CENTURY model
    m_potArea(nullptr), m_deepIrrWater(nullptr), m_shallowIrrWater(nullptr),
    m_wtrStrsID(nullptr),
    /// auto fertilizer operation
    m_autoWtrStrsTrig(nullptr), m_autoIrrSrc(nullptr), m_autoIrrLocNo(nullptr),
    m_autoIrrEff(nullptr), m_autoIrrWtrD(nullptr),
    m_autoIrrWtr2SurfqR(nullptr), m_stoSoilRootD(nullptr), m_grainc_d(nullptr),
    /// Grazing operation
    m_rsdc_d(nullptr), m_stoverc_d(nullptr),
    /// Release or impound operation
    m_tillageLookup(nullptr), m_tillageNum(-1), m_soilActvMinP(nullptr),
    m_soilStabMinP(nullptr), m_fertID(nullptr),
    m_NStrsMeth(nullptr), m_autoNStrsTrig(nullptr), m_autoFertMaxApldN(nullptr),
    m_autoFertMaxAnnApldMinN(nullptr),
    m_autoFertNtrgtMod(nullptr), m_autoFertEff(nullptr), m_autoFertSurfFr(nullptr),
    /// CENTURY C/N cycling related variables
    m_nGrazDays(nullptr), m_grazFlag(nullptr), m_impndTrig(nullptr), m_potVol(nullptr),
    m_potVolMax(nullptr),
    m_potVolLow(nullptr), m_potNo3(nullptr), m_potNH4(nullptr), m_potSolP(nullptr),
    m_soilFC(nullptr),
    m_soilSat(nullptr), m_soilWtrSto(nullptr),
    /// Temporary parameters
    m_soilWtrStoPrfl(nullptr), m_initialized(false),
    tmp_rtfr(nullptr), tmp_soilMass(nullptr), tmp_soilMixedMass(nullptr),
    tmp_soilNotMixedMass(nullptr), tmp_smix(nullptr) {
}

MGTOpt_SWAT::~MGTOpt_SWAT() {
    /// release map containers
    if (!m_mgtFactory.empty()) {
        for (auto it = m_mgtFactory.begin(); it != m_mgtFactory.end();) {
            if (it->second != nullptr) {
                delete it->second;
                it->second = nullptr;
            }
            m_mgtFactory.erase(it++);
        }
        m_mgtFactory.clear();
    }
    if (!m_landuseLookupMap.empty()) {
        for (auto it = m_landuseLookupMap.begin(); it != m_landuseLookupMap.end();) {
            if (it->second != nullptr) {
                delete[] it->second;
                it->second = nullptr;
            }
            it->second = nullptr;
            m_landuseLookupMap.erase(it++);
        }
        m_landuseLookupMap.clear();
    }
    if (!m_cropLookupMap.empty()) {
        for (auto it = m_cropLookupMap.begin(); it != m_cropLookupMap.end();) {
            if (it->second != nullptr) {
                delete[] it->second;
                it->second = nullptr;
            }
            it->second = nullptr;
            m_cropLookupMap.erase(it++);
        }
        m_cropLookupMap.clear();
    }
    if (!m_fertilizerLookupMap.empty()) {
        for (auto it = m_fertilizerLookupMap.begin(); it != m_fertilizerLookupMap.end();) {
            if (it->second != nullptr) {
                delete[] it->second;
                it->second = nullptr;
            }
            it->second = nullptr;
            m_fertilizerLookupMap.erase(it++);
        }
        m_fertilizerLookupMap.clear();
    }
    if (!m_tillageLookupMap.empty()) {
        for (auto it = m_tillageLookupMap.begin(); it != m_tillageLookupMap.end();) {
            if (it->second != nullptr) {
                delete[] it->second;
                it->second = nullptr;
            }
            it->second = nullptr;
            m_tillageLookupMap.erase(it++);
        }
        m_tillageLookupMap.clear();
    }
    /// release output parameters
    /// plant operation
    if (m_HvstIdxTrgt != nullptr) Release1DArray(m_HvstIdxTrgt);
    if (m_biomTrgt != nullptr) Release1DArray(m_biomTrgt);
    /// auto irrigation operation
    if (m_irrFlag != nullptr) Release1DArray(m_irrFlag);
    if (m_irrWtrAmt != nullptr) Release1DArray(m_irrWtrAmt);
    if (m_irrWtr2SurfqAmt != nullptr) Release1DArray(m_irrWtr2SurfqAmt);
    if (m_wtrStrsID != nullptr) Release1DArray(m_wtrStrsID);
    if (m_autoWtrStrsTrig != nullptr) Release1DArray(m_autoWtrStrsTrig);
    if (m_autoIrrSrc != nullptr) Release1DArray(m_autoIrrSrc);
    if (m_autoIrrLocNo != nullptr) Release1DArray(m_autoIrrLocNo);
    if (m_autoIrrEff != nullptr) Release1DArray(m_autoIrrEff);
    if (m_autoIrrWtrD != nullptr) Release1DArray(m_autoIrrWtrD);
    if (m_autoIrrWtr2SurfqR != nullptr) Release1DArray(m_autoIrrWtr2SurfqR);
    /// fertilizer / auto fertilizer operation
    if (m_fertID != nullptr) Release1DArray(m_fertID);
    if (m_NStrsMeth != nullptr) Release1DArray(m_NStrsMeth);
    if (m_autoNStrsTrig != nullptr) Release1DArray(m_autoNStrsTrig);
    if (m_autoFertMaxApldN != nullptr) Release1DArray(m_autoFertMaxApldN);
    if (m_autoFertMaxAnnApldMinN != nullptr) Release1DArray(m_autoFertMaxAnnApldMinN);
    if (m_autoFertNtrgtMod != nullptr) Release1DArray(m_autoFertNtrgtMod);
    if (m_autoFertEff != nullptr) Release1DArray(m_autoFertEff);
    if (m_autoFertSurfFr != nullptr) Release1DArray(m_autoFertSurfFr);
    /// Grazing operation
    if (m_nGrazDays != nullptr) Release1DArray(m_nGrazDays);
    if (m_grazFlag != nullptr) Release1DArray(m_grazFlag);
    /// Impound/Release operation
    if (m_impndTrig != nullptr) Release1DArray(m_impndTrig);
    if (m_potVolMax != nullptr) Release1DArray(m_potVolMax);
    if (m_potVolLow != nullptr) Release1DArray(m_potVolLow);
    /// temporary variables
    if (nullptr != tmp_rtfr) Release2DArray(m_nCells, tmp_rtfr);
    if (nullptr != tmp_soilMass) Release2DArray(m_nCells, tmp_soilMass);
    if (nullptr != tmp_soilMixedMass) Release2DArray(m_nCells, tmp_soilMixedMass);
    if (nullptr != tmp_soilNotMixedMass) Release2DArray(m_nCells, tmp_soilNotMixedMass);
    if (nullptr != tmp_smix) Release2DArray(m_nCells, tmp_smix);
}

void MGTOpt_SWAT::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, VAR_CSWAT)) {
        m_cbnModel = CVT_INT(value);
    } else if (StringMatch(sk, Tag_CellWidth)) {
        m_cellWth = value;
    } else if (StringMatch(sk, VAR_SUBBSNID_NUM)) {
        m_nSubbsns = CVT_INT(value);
    } else {
        throw ModelException(MID_PLTMGT_SWAT, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void MGTOpt_SWAT::Set1DData(const char* key, const int n, float* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SBGS)) {
        // TODO, current version, the shalow and deep water depths are regarded the same.
        m_deepWaterDepth = data;
        m_shallowWaterDepth = data;
        return;
    }
    CheckInputSize(MID_PLTMGT_SWAT, key, n, m_nCells);
    if (StringMatch(sk, VAR_SUBBSN)) {
        m_subbsnID = data;
    } else if (StringMatch(sk, VAR_LANDUSE)) {
        m_landUse = data;
    } else if (StringMatch(sk, VAR_LANDCOVER)) {
        m_landCover = data;
    } else if (StringMatch(sk, VAR_IDC)) {
        m_landCoverCls = data;
    }
        /// Soil related parameters from MongoDB
    else if (StringMatch(sk, VAR_SOILLAYERS)) {
        m_nSoilLyrs = data;
    } else if (StringMatch(sk, VAR_SOL_ZMX)) {
        m_soilMaxRootD = data;
    } else if (StringMatch(sk, VAR_SOL_SUMAWC)) {
        m_soilSumFC = data;
    } else if (StringMatch(sk, VAR_T_BASE)) {
        m_pgTempBase = data;
    }
        ///  Plant operation related parameters
    else if (StringMatch(sk, VAR_CN2)) {
        m_cn2 = data;
    } else if (StringMatch(sk, VAR_HVSTI)) {
        m_hvstIdx = data;
    } else if (StringMatch(sk, VAR_WSYF)) {
        m_wtrStrsHvst = data;
    } else if (StringMatch(sk, VAR_PHUPLT)) {
        m_phuPlt = data;
    } else if (StringMatch(sk, VAR_PHUBASE)) {
        m_phuBase = data;
    } else if (StringMatch(sk, VAR_IGRO)) {
        m_igro = data;
    } else if (StringMatch(sk, VAR_FR_PHU_ACC)) {
        m_phuAccum = data;
    } else if (StringMatch(sk, VAR_TREEYRS)) {
        m_curYrMat = data;
    } else if (StringMatch(sk, VAR_HVSTI_ADJ)) {
        m_hvstIdxAdj = data;
    } else if (StringMatch(sk, VAR_LAIDAY)) {
        m_lai = data;
    } else if (StringMatch(sk, VAR_DORMI)) {
        m_dormFlag = data;
    } else if (StringMatch(sk, VAR_LAIMAXFR)) {
        m_laiMaxFr = data;
    } else if (StringMatch(sk, VAR_OLAI)) {
        m_oLai = data;
    } else if (StringMatch(sk, VAR_PLANT_N)) {
        m_pltN = data;
    } else if (StringMatch(sk, VAR_PLANT_P)) {
        m_pltP = data;
    } else if (StringMatch(sk, VAR_FR_PLANT_N)) {
        m_frPltN = data;
    } else if (StringMatch(sk, VAR_FR_PLANT_P)) {
        m_frPltP = data;
    } else if (StringMatch(sk, VAR_PLTET_TOT)) {
        m_totActPltET = data;
    } else if (StringMatch(sk, VAR_PLTPET_TOT)) {
        m_totPltPET = data;
    } else if (StringMatch(sk, VAR_FR_ROOT)) {
        m_frRoot = data;
    } else if (StringMatch(sk, VAR_BIOMASS)) {
        m_biomass = data;
    }
        //// Harvest and Kill operation
    else if (StringMatch(sk, VAR_LAST_SOILRD)) {
        m_stoSoilRootD = data;
    }
        /// Irrigation operation
    else if (StringMatch(sk, VAR_FR_STRSWTR)) {
        m_frStrsWtr = data;
    }
        /// impound/release
    else if (StringMatch(sk, VAR_POT_VOL)) {
        m_potVol = data;
    } else if (StringMatch(sk, VAR_POT_SA)) {
        m_potArea = data;
    } else if (StringMatch(sk, VAR_POT_NO3)) {
        m_potNo3 = data;
    } else if (StringMatch(sk, VAR_POT_NH4)) {
        m_potNH4 = data;
    } else if (StringMatch(sk, VAR_POT_SOLP)) {
        m_potSolP = data;
    } else if (StringMatch(sk, VAR_SOL_SW)) {
        m_soilWtrStoPrfl = data;
    } else {
        throw ModelException(MID_PLTMGT_SWAT, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void MGTOpt_SWAT::Set2DData(const char* key, const int n, const int col, float** data) {
    string sk(key);
    /// lookup tables
    if (StringMatch(sk, VAR_LANDUSE_LOOKUP)) {
        m_landuseLookup = data;
        m_landuseNum = n;
        InitializeLanduseLookup();
        if (col != LANDUSE_PARAM_COUNT) {
            throw ModelException(MID_PLTMGT_SWAT, "ReadLanduseLookup", "The field number " + ValueToString(col) +
                                 "is not coincident with LANDUSE_PARAM_COUNT: " +
                                 ValueToString(LANDUSE_PARAM_COUNT));
        }
        return;
    }
    if (StringMatch(sk, VAR_CROP_LOOKUP)) {
        m_cropLookup = data;
        m_cropNum = n;
        InitializeCropLookup();
        if (col != CROP_PARAM_COUNT) {
            throw ModelException(MID_PLTMGT_SWAT, "ReadCropLookup", "The field number " + ValueToString(col) +
                                 "is not coincident with CROP_PARAM_COUNT: " +
                                 ValueToString(CROP_PARAM_COUNT));
        }
        return;
    }
    if (StringMatch(sk, VAR_FERTILIZER_LOOKUP)) {
        m_fertLookup = data;
        m_fertNum = n;
        InitializeFertilizerLookup();
        if (col != FERTILIZER_PARAM_COUNT) {
            throw ModelException(MID_PLTMGT_SWAT, "ReadFertilizerLookup", "The field number " + ValueToString(col) +
                                 "is not coincident with FERTILIZER_PARAM_COUNT: " +
                                 ValueToString(FERTILIZER_PARAM_COUNT));
        }
        return;
    }
    if (StringMatch(sk, VAR_TILLAGE_LOOKUP)) {
        m_tillageLookup = data;
        m_tillageNum = n;
        InitializeTillageLookup();
        if (col != TILLAGE_PARAM_COUNT) {
            throw ModelException(MID_PLTMGT_SWAT, "ReadTillageLookup", "The field number " + ValueToString(col) +
                                 "is not coincident with TILLAGE_PARAM_COUNT: " +
                                 ValueToString(TILLAGE_PARAM_COUNT));
        }
        return;
    }
    /// 2D raster data
    CheckInputSize2D(MID_PLTMGT_SWAT, key, n, col, m_nCells, m_maxSoilLyrs);
    /// Soil related parameters from MongoDB
    if (StringMatch(sk, VAR_SOILDEPTH)) {
        m_soilDepth = data;
    } else if (StringMatch(sk, VAR_SOILTHICK)) {
        m_soilThick = data;
    } else if (StringMatch(sk, VAR_SOL_BD)) {
        m_soilBD = data;
    } else if (StringMatch(sk, VAR_SOL_CBN)) {
        m_soilCbn = data;
    } else if (StringMatch(sk, VAR_SOL_N)) {
        m_soilN = data;
    } else if (StringMatch(sk, VAR_CLAY)) {
        m_soilClay = data;
    } else if (StringMatch(sk, VAR_SILT)) {
        m_soilSilt = data;
    } else if (StringMatch(sk, VAR_SAND)) {
        m_soilSand = data;
    } else if (StringMatch(sk, VAR_ROCK)) {
        m_soilRock = data;
    }
        /// Soil related parameters --  inputs from other modules
    else if (StringMatch(sk, VAR_SOL_SORGN)) {
        m_soilStabOrgN = data;
    } else if (StringMatch(sk, VAR_SOL_HORGP)) {
        m_soilHumOrgP = data;
    } else if (StringMatch(sk, VAR_SOL_SOLP)) {
        m_soilSolP = data;
    } else if (StringMatch(sk, VAR_SOL_NH4)) {
        m_soilNH4 = data;
    } else if (StringMatch(sk, VAR_SOL_NO3)) {
        m_soilNO3 = data;
    } else if (StringMatch(sk, VAR_SOL_AORGN)) {
        m_soilActvOrgN = data;
    } else if (StringMatch(sk, VAR_SOL_FORGN)) {
        m_soilFrshOrgN = data;
    } else if (StringMatch(sk, VAR_SOL_FORGP)) {
        m_soilFrshOrgP = data;
    } else if (StringMatch(sk, VAR_SOL_ACTP)) {
        m_soilActvMinP = data;
    } else if (StringMatch(sk, VAR_SOL_STAP)) {
        m_soilStabMinP = data;
    } else if (StringMatch(sk, VAR_SOL_RSD)) {
        m_soilRsd = data;
    } else if (StringMatch(sk, VAR_SOL_AWC)) {
        m_soilFC = data;
    } else if (StringMatch(sk, VAR_SOL_UL)) {
        m_soilSat = data;
    } else if (StringMatch(sk, VAR_SOL_ST)) {
        m_soilWtrSto = data;
    }
        /// inputs for CENTURY C/N cycling model in stated and necessary
    else if (StringMatch(sk, VAR_SOL_HSN)) {
        m_soilHSN = data;
    } else if (StringMatch(sk, VAR_SOL_LM)) {
        m_soilLM = data;
    } else if (StringMatch(sk, VAR_SOL_LMC)) {
        m_soilLMC = data;
    } else if (StringMatch(sk, VAR_SOL_LMN)) {
        m_soilLMN = data;
    } else if (StringMatch(sk, VAR_SOL_LSC)) {
        m_soilLSC = data;
    } else if (StringMatch(sk, VAR_SOL_LSN)) {
        m_soilLSN = data;
    } else if (StringMatch(sk, VAR_SOL_LS)) {
        m_soilLS = data;
    } else if (StringMatch(sk, VAR_SOL_LSL)) {
        m_soilLSL = data;
    } else if (StringMatch(sk, VAR_SOL_LSLC)) {
        m_soilLSLC = data;
    } else if (StringMatch(sk, VAR_SOL_LSLNC)) {
        m_soilLSLNC = data;
    }
        //else if (StringMatch(sk, VAR_SOL_WON)) m_sol_WON = data;
        //else if (StringMatch(sk, VAR_SOL_BM)) m_sol_BM = data;
        //else if (StringMatch(sk, VAR_SOL_BMC)) m_sol_BMC = data;
    else if (StringMatch(sk, VAR_SOL_BMN)) {
        m_soilBMN = data;
    }
        //else if (StringMatch(sk, VAR_SOL_HP)) m_sol_HP = data;
        //else if (StringMatch(sk, VAR_SOL_HS)) m_sol_HS = data;
        //else if (StringMatch(sk, VAR_SOL_HSC)) m_sol_HSC = data;
        //else if (StringMatch(sk, VAR_SOL_HPC)) m_sol_HPC = data;
    else if (StringMatch(sk, VAR_SOL_HPN)) {
        m_soilHPN = data;
    }
        //else if (StringMatch(sk, VAR_SOL_RNMN)) m_sol_RNMN = data;
        //else if (StringMatch(sk, VAR_SOL_RSPC)) m_sol_RSPC = data;
    else {
        throw ModelException(MID_PLTMGT_SWAT, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void MGTOpt_SWAT::SetScenario(Scenario* sce) {
    if (nullptr == sce) {
        throw ModelException(MID_PLTMGT_SWAT, "SetScenario", "The Scenario data can not to be nullptr.");
    }
    map<int, BMPFactory *>& tmpBMPFactories = sce->GetBMPFactories();
    if (!m_mgtFactory.empty()) {
        m_mgtFactory.clear();
    }
    if (!m_landuseMgtOp.empty()) {
        m_landuseMgtOp.clear();
    }
    for (auto it = tmpBMPFactories.begin(); it != tmpBMPFactories.end(); ++it) {
        /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
        if (it->first / 100000 == BMP_TYPE_PLANT_MGT) {
            /// calculate unique index for the key of m_mgtFactory, using Landuse_ID * 100 + subScenario
            //BMPPlantMgtFactory* tmpPltFactory = static_cast<BMPPlantMgtFactory *>(it->second);
            BMPPlantMgtFactory* tmpPltFactory = dynamic_cast<BMPPlantMgtFactory *>(it->second);
            m_landuseMgtOp.emplace_back(tmpPltFactory->GetLUCCID());
            if (m_subSceneID < 0) m_subSceneID = it->second->GetSubScenarioId();
            int uniqueIdx = tmpPltFactory->GetLUCCID() * 100 + m_subSceneID;
            m_mgtFactory[uniqueIdx] = tmpPltFactory;
            /// Set plant management spatial units
            if (nullptr == m_mgtFields) {
                m_mgtFields = tmpPltFactory->GetRasterData();
            }
        }
    }
}

void MGTOpt_SWAT::SetSubbasins(clsSubbasins* subbasins) {
    if (nullptr == subbasins) {
        throw ModelException(MID_PLTMGT_SWAT, "SetSubbasins", "The Subbasins data can not to be nullptr.");
    }
    // m_nSub = subbasins->GetSubbasinNumber(); // Set in SetValue()
    if (!m_nCellsSubbsn.empty() || !m_nAreaSubbsn.empty()) return;
    vector<int>& subIDs = subbasins->GetSubbasinIDs();
    for (auto it = subIDs.begin(); it != subIDs.end(); ++it) {
        Subbasin* tmpSubbsn = subbasins->GetSubbasinByID(*it);
#ifdef HAS_VARIADIC_TEMPLATES
        m_nCellsSubbsn.emplace(*it, tmpSubbsn->GetCellCount());
        m_nAreaSubbsn.emplace(*it, tmpSubbsn->GetArea());
#else
        m_nCellsSubbsn.insert(make_pair(*it, tmpSubbsn->GetCellCount()));
        m_nAreaSubbsn.insert(make_pair(*it, tmpSubbsn->GetArea()));
#endif
    }
}

bool MGTOpt_SWAT::CheckInputData() {
    // DT_Single
    CHECK_POSITIVE(MID_PLTMGT_SWAT, m_nCells);
    CHECK_POSITIVE(MID_PLTMGT_SWAT, m_cellWth);
    CHECK_POSITIVE(MID_PLTMGT_SWAT, m_maxSoilLyrs);
    CHECK_NONNEGATIVE(MID_PLTMGT_SWAT, m_cbnModel);
    if (m_cbnModel == 2) {
        /// Check for the CENTURY required initialized variables
        CHECK_POINTER(MID_PLTMGT_SWAT, m_soilHSN);
        CHECK_POINTER(MID_PLTMGT_SWAT, m_soilLM);
        CHECK_POINTER(MID_PLTMGT_SWAT, m_soilLMC);
        CHECK_POINTER(MID_PLTMGT_SWAT, m_soilLMN);
        CHECK_POINTER(MID_PLTMGT_SWAT, m_soilLSC);
        CHECK_POINTER(MID_PLTMGT_SWAT, m_soilLSN);
        CHECK_POINTER(MID_PLTMGT_SWAT, m_soilLS);
        CHECK_POINTER(MID_PLTMGT_SWAT, m_soilLSL);
        CHECK_POINTER(MID_PLTMGT_SWAT, m_soilLSLC);
        CHECK_POINTER(MID_PLTMGT_SWAT, m_soilLSLNC);
    }
    /// DT_Raster
    CHECK_POINTER(MID_PLTMGT_SWAT, m_subbsnID);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_landUse);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_landCover);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_mgtFields);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_pgTempBase);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_nSoilLyrs);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilMaxRootD);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilSumFC);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_cn2);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_igro);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_landCoverCls);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_curYrMat);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_wtrStrsHvst);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_lai);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_phuBase);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_phuAccum);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_phuPlt);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_dormFlag);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_hvstIdx);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_hvstIdxAdj);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_laiMaxFr);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_oLai);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_frPltN);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_frPltP);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_totActPltET);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_totPltPET);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_frRoot);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_biomass);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_stoSoilRootD);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_deepWaterDepth);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_shallowWaterDepth);
    /// DT_Raster2D
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilDepth);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilThick);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilBD);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilN);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilCbn);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilClay);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilSand);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilSilt);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilRock);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilActvOrgN);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilFrshOrgN);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilFrshOrgP);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilNO3);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilStabOrgN);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilHumOrgP);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilSolP);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilActvMinP);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilStabMinP);
    CHECK_POINTER(MID_PLTMGT_SWAT, m_soilRsd);
    return true;
}

bool MGTOpt_SWAT::GetOperationCode(const int i, const int factoryID, vector<int>& nOps) {
    /// Figure out if any management operation should be applied, i.e., find sequence IDs (nOps)
    vector<int>& tmpOpSeqences = m_mgtFactory[factoryID]->GetOperationSequence();
    map<int, PltMgtOp *>& tmpOperations = m_mgtFactory[factoryID]->GetOperations();
    // get the next should be done sequence number
    int nextSeq = -1;
    if (m_doneOpSequence[i] == -1 || m_doneOpSequence[i] == CVT_INT(tmpOpSeqences.size()) - 1) {
        nextSeq = 0;
    } else {
        nextSeq = m_doneOpSequence[i] + 1;
    }
    int opCode = tmpOpSeqences[nextSeq];
    // figure out the nextSeq is satisfied or not.
    if (tmpOperations.find(opCode) == tmpOperations.end()) {
        return false;
    }
    PltMgtOp* tmpOperation = tmpOperations.at(opCode);
    bool dateDepent = false;
    bool huscDepent = false;
    /// If operation applied date (month and day) are defined
    if (m_month == tmpOperation->GetMonth() && m_day == tmpOperation->GetDay()) {
        dateDepent = true;
    }
    /// If husc is defined
    if (tmpOperation->GetHUFraction() >= 0.f) {
        float aphu = NODATA_VALUE; /// fraction of total heat units accumulated
        if (!FloatEqual(m_dormFlag[i], 1.f)) {
            if (tmpOperation->UseBaseHUSC() && FloatEqual(m_igro[i], 0.f)) {
                // use base hu
                aphu = m_phuBase[i];
                if (aphu >= tmpOperation->GetHUFraction()) {
                    huscDepent = true;
                }
            } else {
                // use accumulated plant hu
                aphu = m_phuAccum[i];
                if (aphu >= tmpOperation->GetHUFraction()) {
                    huscDepent = true;
                }
            }
        }
    }
    /// The operation will be applied either date or HUSC are satisfied,
    /// and also in case of repeated run
    if (dateDepent || huscDepent) {
        nOps.emplace_back(opCode);
        m_doneOpSequence[i] = nextSeq; /// update value
    }
    return !nOps.empty();
}

void MGTOpt_SWAT::InitializeLanduseLookup() {
    /// Check input data
    if (m_landuseLookup == nullptr) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Landuse lookup array must not be nullptr");
    }
    if (m_landuseNum <= 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Landuse number must be greater than 0");
    }
    if (!m_landuseLookupMap.empty()) {
        return;
    }
    for (int i = 0; i < m_landuseNum; i++) {
#ifdef HAS_VARIADIC_TEMPLATES
        m_landuseLookupMap.emplace(CVT_INT(m_landuseLookup[i][1]), m_landuseLookup[i]);
#else
        m_landuseLookupMap.insert(make_pair(CVT_INT(m_landuseLookup[i][1]),  m_landuseLookup[i]));
#endif
    }
}

void MGTOpt_SWAT::InitializeCropLookup() {
    /// Check input data
    if (m_cropLookup == nullptr) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Crop lookup array must not be nullptr");
    }
    if (m_cropNum <= 0) throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Crop number must be greater than 0");

    if (!m_cropLookupMap.empty()) {
        return;
    }
    for (int i = 0; i < m_cropNum; i++) {
#ifdef HAS_VARIADIC_TEMPLATES
        m_cropLookupMap.emplace(CVT_INT(m_cropLookup[i][1]), m_cropLookup[i]);
#else
        m_cropLookupMap.insert(make_pair(CVT_INT(m_cropLookup[i][1]), m_cropLookup[i]));
#endif
    }
}

void MGTOpt_SWAT::InitializeFertilizerLookup() {
    /// Check input data
    if (m_fertLookup == nullptr) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Fertilizer lookup array must not be nullptr");
    }
    if (m_fertNum <= 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Fertilizer number must be greater than 0");
    }
    if (!m_fertilizerLookupMap.empty()) {
        return;
    }
    for (int i = 0; i < m_fertNum; i++) {
#ifdef HAS_VARIADIC_TEMPLATES
        m_fertilizerLookupMap.emplace(CVT_INT(m_fertLookup[i][1]), m_fertLookup[i]);
#else
        m_fertilizerLookupMap.insert(make_pair(CVT_INT(m_fertLookup[i][1]), m_fertLookup[i]));
#endif
    }
}

void MGTOpt_SWAT::InitializeTillageLookup() {
    /// Check input data
    if (m_tillageLookup == nullptr) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Tillage lookup array must not be nullptr");
    }
    if (m_tillageNum <= 0) {
        throw ModelException(MID_PLTMGT_SWAT, "CheckInputData", "Tillage number must be greater than 0");
    }
    if (!m_tillageLookupMap.empty()) {
        return;
    }
    for (int i = 0; i < m_tillageNum; i++) {
#ifdef HAS_VARIADIC_TEMPLATES
        m_tillageLookupMap.emplace(CVT_INT(m_tillageLookup[i][1]), m_tillageLookup[i]);
#else
        m_tillageLookupMap.insert(make_pair(CVT_INT(m_tillageLookup[i][1]), m_tillageLookup[i]));
#endif
    }
}

void MGTOpt_SWAT::ExecutePlantOperation(const int i, const int factoryID, const int nOp) {
    PltOp* curOperation = static_cast<PltOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    /// initialize parameters
    m_igro[i] = 1.f;
    m_HvstIdxTrgt[i] = curOperation->HITarg();
    m_biomTrgt[i] = curOperation->BIOTarg(); /// kg/ha
    m_curYrMat[i] = curOperation->CurYearMaturity();
    int newPlantID = curOperation->PlantID();
    m_landCover[i] = CVT_FLT(newPlantID);
    m_phuPlt[i] = curOperation->HeatUnits();
    m_dormFlag[i] = 0.f;
    m_phuAccum[i] = 0.f;
    m_pltN[i] = 0.f;
    m_pltP[i] = 0.f;
    m_totActPltET[i] = 0.f;
    m_totPltPET[i] = 0.f;
    m_laiMaxFr[i] = 0.f;
    m_hvstIdxAdj[i] = 0.f;
    m_oLai[i] = 0.f;
    m_frRoot[i] = 0.f;
    /// update crop-related parameters in order to calculate phuAcc. by LJ
    if (m_cropLookupMap.find(newPlantID) == m_cropLookupMap.end()) {
        throw ModelException(MID_PLTMGT_SWAT, "ExecutePlantOperation",
                             "The new plant ID: " + ValueToString(newPlantID) +
                             " is not prepared in cropLookup table!");
    }
    // update IDC
    m_landCoverCls[i] = m_cropLookupMap.at(newPlantID)[CROP_PARAM_IDX_IDC];
    m_pgTempBase[i] = m_cropLookupMap.at(newPlantID)[CROP_PARAM_IDX_T_BASE];
    /// initialize transplant variables
    if (curOperation->LAIInit() > 0.f) {
        m_lai[i] = curOperation->LAIInit();
        m_biomass[i] = curOperation->BIOInit();
    }
    /// compare maximum rooting depth in soil to maximum rooting depth of plant
    m_soilMaxRootD[i] = m_soilDepth[i][CVT_INT(m_nSoilLyrs[i] - 1)];
    /// if the land cover does existed, throw an exception.
    if (m_landuseLookupMap.find(CVT_INT(m_landCover[i])) == m_landuseLookupMap.end()) {
        throw ModelException(MID_PLTMGT_SWAT, "ExecutePlantOperation",
                             "Land use ID: " + ValueToString(CVT_INT(m_landCover[i])) +
                             " does not existed in Landuse lookup table, please check and retry!");
    }
    float pltRootDepth = m_landuseLookupMap[CVT_INT(m_landCover[i])][LANDUSE_PARAM_ROOT_DEPTH_IDX];
    m_soilMaxRootD[i] = Min(m_soilMaxRootD[i], pltRootDepth);
    /// reset curve number if necessary
    if (curOperation->CNOP() > 0.f) {
        /// curno.f
        float cnn = curOperation->CNOP();
        m_cn2[i] = cnn;
    }
}

void MGTOpt_SWAT::ExecuteIrrigationOperation(const int i, const int factoryID, const int nOp) {
    IrrOp* curOperation = static_cast<IrrOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    /// initialize parameters
    /// irrigation source
    int m_irrSource = curOperation->IRRSource();
    /// irrigation source location code
    int m_irrNo = curOperation->IRRNo() <= 0 ? CVT_INT(m_subbsnID[i]) : curOperation->IRRNo();
    /// irrigation apply depth (mm)
    float m_irrApplyDepth = curOperation->IRRApplyDepth();
    /// float* m_irrSalt; /// currently not used
    /// irrigation efficiency
    float m_irrEfficiency = curOperation->IRREfficiency();

    m_irrFlag[i] = 1;
    int tmpSubbsnID = CVT_INT(m_subbsnID[i]);
    if (m_irrSource > IRR_SRC_RES) {
        /// irrigation from reach and reservoir are irr_rch.f and irr_res.f, respectively
               /// call irrsub.f
               /// Performs the irrigation operation when the source is the shallow or deep aquifer or a source outside the watershed
        float vmma = 0.f; /// amount of water in source, mm
        float vmm = 0.f;  ///maximum amount of water to be applied, mm
        float cnv = 0.f;  /// conversion factor (mm/ha => m^3)
        float vmxi = 0.f; /// amount of water specified in irrigation operation, mm
        float vol = 0.f;  /// volume of water to be applied in irrigation, m^3
        float vmms = 0.f; /// amount of water in shallow aquifer, m^3
        float vmmd = 0.f; /// amount of water in deep aquifer, m^3
        /// Whether m_irrNo is valid
        if (m_nCellsSubbsn.find(m_irrNo) == m_nCellsSubbsn.end()) {
            m_irrNo = CVT_INT(m_subbsnID[i]);
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
                vmm = Min(m_soilSumFC[i], vmma);
                break;
            case IRR_SRC_DEEP: vmma += m_deepWaterDepth[tmpSubbsnID] * cnv * m_irrEfficiency;
                vmmd = vmma;
                vmma /= m_nCellsSubbsn[m_irrNo];
                vmm = Min(m_soilSumFC[i], vmma);
                break;
            case IRR_SRC_OUTWTSD: /// unlimited source
                vmm = m_soilSumFC[i];
                break;
            default: break;
        }
        /// if water available from source, proceed with irrigation
        if (vmm > 0.f) {
            cnv = m_cellArea * 10.f;
            vmxi = m_irrApplyDepth < UTIL_ZERO ? m_soilSumFC[i] : m_irrApplyDepth;
            if (vmm > vmxi) vmm = vmxi;
            vol = vmm * cnv;
            float pot_fr = 0.f;
            if (FloatEqual(m_impndTrig[i], 0.f) && m_potVol != nullptr) {
                /// m_impoundTrig equals to 0 means pot_fr is 1.
                /// and m_impoundArea is set to m_cellArea.
                pot_fr = 1.f;
                if (m_potArea != nullptr) {
                    m_potVol[i] += vol / (10.f * m_potArea[i]);
                } else {
                    m_potVol[i] += vol / (10.f * m_cellArea);
                }
                m_irrWtrAmt[i] = vmm; ///added rice irrigation 11/10/11
            } else {
                pot_fr = 0.f;
                /// Call irrigate(i, vmm) /// irrigate.f
                m_irrWtrAmt[i] = vmm * (1.f - curOperation->IRRSQfrac());
                m_irrWtr2SurfqAmt[i] = vmm * curOperation->IRRSQfrac();
            }
            /// subtract irrigation from shallow or deep aquifer
            if (pot_fr > UTIL_ZERO) {
                vol = m_irrWtrAmt[i] * cnv * m_irrEfficiency;
            }
            switch (m_irrSource) {
                case IRR_SRC_SHALLOW: cnv = m_nAreaSubbsn[m_irrNo] * 10.f;
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
                case IRR_SRC_DEEP: cnv = m_nAreaSubbsn[m_irrNo] * 10.f;
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
                default: break;
            }
        }
    }
}

void MGTOpt_SWAT::ExecuteFertilizerOperation(const int i, const int factoryID, const int nOp) {
    /* Briefly change log
	 * 1. Translate from fert.f, remains CSWAT = 1 and 2 to be done!!! by LJ
	 * 2. CSWAT = 1 and 2, were implemented on 2016-9-29, by LJ.
	 * 3. Consider paddy rice field according to Chowdary et al., 2004, 2016-10-9, by LJ.
	 */
    //initializeFertilizerLookup();
    FertOp* curOperation = static_cast<FertOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
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
    int fertype = CVT_INT(m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_MANURE_IDX]);
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
    float xx = 0.f; /// fraction of fertilizer applied to layer
    float gc = 0.f; //, gc1 = 0.f;
    /// if current landcover is paddy rice, then apply the commercial fertilizer to the top surface and pothole.
    int lyrs = 2;
    if (m_potVol != nullptr) {
        if (FloatEqual(CVT_INT(m_landCover[i]), CROP_PADDYRICE) && fertype == 0 && m_potVol[i] > 0.f) {
            lyrs = 1;
            xx = 1.f - fertilizerSurfFrac;
            m_potNo3[i] += xx * fertilizerKgHa * (1.f - fertNH4N) * fertMinN * m_cellArea; /// kg/ha * ha ==> kg
            m_potNH4[i] += xx * fertilizerKgHa * fertNH4N * fertMinN * m_cellArea;
            m_potSolP[i] += xx * fertilizerKgHa * fertMinP * m_cellArea;
        }
    }
    for (int l = 0; l < lyrs; l++) {
        /// top surface and first layer
        if (l == 0) xx = fertilizerSurfFrac;
        if (l == 1) xx = 1.f - fertilizerSurfFrac;
        m_soilNO3[i][l] += xx * fertilizerKgHa * (1.f - fertNH4N) * fertMinN;
        if (m_cbnModel == 0) {
            /// Static model
            m_soilFrshOrgN[i][l] += rtof * xx * fertilizerKgHa * fertOrgN;
            m_soilActvOrgN[i][l] += (1.f - rtof) * xx * fertilizerKgHa * fertOrgN;
            m_soilFrshOrgP[i][l] += rtof * xx * fertilizerKgHa * fertOrgP;
            m_soilHumOrgP[i][l] += (1.f - rtof) * xx * fertilizerKgHa * fertOrgP;
        } else if (m_cbnModel == 1) {
            /// C-FARM one carbon pool model
            m_soilManC[i][l] += xx * fertilizerKgHa * fertOrgN * 10.f; /// assume C:N = 10:1
            m_soilManN[i][l] += xx * fertilizerKgHa * fertOrgN;
            m_soilManP[i][l] += xx * fertilizerKgHa * fertOrgP;
        } else if (m_cbnModel == 2) {
            /// CENTURY model for C/N cycling
            float X1 = 0.f, X8 = 0.f, X10 = 0.f, XXX = 0.f, YY = 0.f;
            float ZZ = 0.f, XZ = 0.f, YZ = 0.f, RLN = 0.f;
            /// the fraction of organic carbon in fertilizer, for most fertilizers orgc_f is set to 0.
            float orgc_f = 0.f;
            m_soilFrshOrgP[i][l] += rtof * xx * fertilizerKgHa * fertOrgP;
            m_soilHumOrgP[i][l] += (1.f - rtof) * xx * fertilizerKgHa * fertOrgP;
            /// allocate organic fertilizer to slow (SWAT active) N pool, i.e., m_soilActiveOrgN
            m_soilHSN[i][l] += (1.f - rtof) * xx * fertilizerKgHa * fertOrgN;
            m_soilActvOrgN[i][l] = m_soilHSN[i][l];
            /// X1 is fertilizer applied to layer (kg/ha)
            X1 = xx * fertilizerKgHa;
            // X8 is organic carbon applied (kg C/ha)
            X8 = X1 * orgc_f;
            /// RLN is calculated as a function of C:N ration in fertilizer
            RLN = 0.175f * orgc_f / (fertMinN + fertOrgN + 1.e-5f);
            /// X10 is the fraction of carbon in fertilizer that is allocated to metabolic litter C pool
            X10 = 0.85f - 0.018f * RLN;
            if (X10 < 0.01f) { X10 = 0.01f; } else if (X10 > 0.7f) X10 = 0.7f;

            /// XXX is the amount of organic carbon allocated to metabolic litter C pool
            XXX = X8 * X10;
            m_soilLMC[i][l] += XXX;
            /// YY is the amount of fertilizer (including C and N) allocated into metabolic litter SOM pool
            YY = X1 * X10;
            m_soilLM[i][l] += YY;
            /// ZZ is amount of organic N allocated to metabolic litter N pool
            ZZ = X1 * rtof * fertOrgN * X10;
            m_soilLMN[i][l] += ZZ;

            /// remaining organic N is allocated to structural litter N pool
            m_soilLSN[i][l] += X1 * fertOrgN - ZZ;
            /// XZ is the amount of organic carbon allocated to structural litter C pool
            XZ = X1 * orgc_f - XXX;
            m_soilLSC[i][l] += XZ;
            /// assuming lignin C fraction of organic carbon to be 0.175;
            float lignin_C_frac = 0.175f;
            /// updating lignin amount in structural litter pool
            m_soilLSLC[i][l] += XZ * lignin_C_frac;
            /// non-lignin part of the structural litter C is also updated;
            m_soilLSLNC[i][l] += XZ * (1.f - lignin_C_frac);
            /// YZ is the amount of fertilizer (including C and N) allocated into structural litter SOM pool
            YZ = X1 - YY;
            m_soilLS[i][l] += YZ;
            /// assuming lignin fraction of the organic fertilizer allocated into structure litter SOM pool to be 0.175
            float lingnin_SOM_frac = 0.175f;
            /// update lignin weight in structural litter.
            m_soilLSL[i][l] += YZ * lingnin_SOM_frac;
            m_soilFrshOrgN[i][l] = m_soilLMN[i][l] + m_soilLSN[i][l];
        }
        m_soilNH4[i][l] += xx * fertilizerKgHa * fertNH4N * fertMinN;
        m_soilSolP[i][l] += xx * fertilizerKgHa * fertMinP;
    }
    /// add bacteria - #cfu/g * t(manure)/ha * 1.e6g/t * ha/10,000m^2 = 100.
    /// calculate ground cover
    gc = (1.99532f - Erfc(1.333f * m_lai[i] - 2.f)) / 2.1f;
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

void MGTOpt_SWAT::ExecutePesticideOperation(const int i, const int factoryID, const int nOp) {
    /// TODO
    PestOp* curOperation = static_cast<PestOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
}

void MGTOpt_SWAT::ExecuteHarvestKillOperation(const int i, const int factoryID, const int nOp) {
    //// TODO: Yield is not set as outputs yet. by LJ
    /// harvkillop.f
    HvstKillOp* curOperation = static_cast<HvstKillOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    /// initialize parameters
    float cnop = curOperation->CNOP();
    float wur = 0.f, hiad1 = 0.f;
    if (m_cropLookupMap.find(CVT_INT(m_landCover[i])) == m_cropLookupMap.end()) {
        throw ModelException(MID_PLTMGT_SWAT, "ExecuteHarvestKillOperation",
                             "The landcover ID " + ValueToString(m_landCover[i])
                             + " is not existed in crop lookup table!");
    }
    /// Get some parameters of current crop / landcover
    float hvsti = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_HVSTI];
    float wsyf = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_WSYF];
    int idc = CVT_INT(m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_IDC]);
    float bio_leaf = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_BIO_LEAF];
    float cnyld = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_CNYLD];
    float cpyld = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_CPYLD];

    /// calculate modifier for autofertilization target nitrogen content
    // TODO
    //tnyld(j) = 0.
    //tnyld(j) = (1. - rwt(j)) * bio_ms(j) * pltfr_n(j) * auto_eff(j)

    if (m_HvstIdxTrgt[i] > 0.f) {
        hiad1 = m_HvstIdxTrgt[i];
    } else {
        if (m_totPltPET[i] < 10.f) {
            wur = 100.f;
        } else {
            wur = 100.f * m_totActPltET[i] / m_totPltPET[i];
        }
        hiad1 = (m_hvstIdxAdj[i] - wsyf) * (wur / (wur + exp(6.13f - 0.0883f * wur))) + wsyf;
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
                resnew *= 1.f - xx;
            } else {
                yield = (1.f - m_frRoot[i]) * m_biomass[i] * hiad1;
                resnew = (1.f - m_frRoot[i]) * (1.f - hiad1) * m_biomass[i];
                /// remove stover during harvestKill operation
                resnew *= 1.f - xx;
                rtresnew = m_frRoot[i] * m_biomass[i];
            }
        }
    }
    if (yield < 0.f) yield = 0.f;
    if (resnew < 0.f) resnew = 0.f;
    if (rtresnew < 0.f) rtresnew = 0.f;

    if (m_cbnModel == 2) {
        m_grainc_d[i] += yield * 0.42f;
        m_stoverc_d[i] += (m_biomass[i] - yield - rtresnew) * 0.42f * xx;
        m_rsdc_d[i] += resnew * 0.42f;
        m_rsdc_d[i] += rtresnew * 0.42f;
    }
    /// calculate nutrient removed with yield
    float yieldn = 0.f, yieldp = 0.f;
    yieldn = yield * cnyld;
    yieldp = yield * cpyld;
    yieldn = Min(yieldn, 0.80f * m_pltN[i]);
    yieldp = Min(yieldp, 0.80f * m_pltP[i]);

    /// call rootfr.f to distributes dead root mass through the soil profile
    /// i.e., derive fraction of roots in each layer
    for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) tmp_rtfr[i][j] = 0.f;
    RootFraction(i, tmp_rtfr[i]);

    /// fraction of N, P in residue (ff1) or roots (ff2)
    float ff1 = (1.f - hiad1) / (1.f - hiad1 + m_frRoot[i]);
    float ff2 = 1.f - ff1;
    /// update residue, N, P on soil surface
    m_soilRsd[i][0] += resnew;
    m_soilFrshOrgN[i][0] += ff1 * (m_pltN[i] - yieldn);
    m_soilFrshOrgP[i][0] += ff1 * (m_pltP[i] - yieldp);
    m_soilRsd[i][0] = Max(m_soilRsd[i][0], 0.f);
    m_soilFrshOrgN[i][0] = Max(m_soilFrshOrgN[i][0], 0.f);
    m_soilFrshOrgP[i][0] = Max(m_soilFrshOrgP[i][0], 0.f);

    /// define variables of CENTURY model
    float BLG1 = 0.f, BLG2 = 0.f, BLG3 = 0.f, CLG = 0.f;
    float sf = 0.f, sol_min_n = 0.f, resnew_n = 0.f, resnew_ne = 0.f;
    float LMF = 0.f, LSF = 0.f;
    float RLN = 0.f, RLR = 0.f;
    /// insert new biomass of CENTURY model
    if (m_cbnModel == 2) {
        BLG1 = 0.01f / 0.10f;
        BLG2 = 0.99f;
        BLG3 = 0.10f;
        float XX = log(0.5f / BLG1 - 0.5f);
        BLG2 = (XX - log(1.f / BLG2 - 1.f)) / (1.f - 0.5f);
        BLG1 = XX + 0.5f * BLG2;
        CLG = BLG3 * m_phuAccum[i] / (m_phuAccum[i] + exp(BLG1 - BLG2 * m_phuAccum[i]));
        sf = 0.05f;
        sol_min_n = m_soilNO3[i][0] + m_soilNH4[i][0];
        // resnew = resnew; // why do this?
        resnew_n = ff1 * (m_pltN[i] - yieldn);
        resnew_ne = resnew_n + sf * sol_min_n;

        RLN = resnew * CLG / (resnew_n + 1.e-5f);
        RLR = Min(0.8f, resnew * CLG / (resnew + 1.e-5f));
        LMF = 0.85f - 0.018f * RLN;
        if (LMF < 0.01f) { LMF = 0.01f; } else if (LMF > 0.7f) LMF = 0.7f;
        LSF = 1.f - LMF;
        m_soilLM[i][0] += LMF * resnew;
        m_soilLS[i][0] += LSF * resnew;

        m_soilLSL[i][0] += RLR * resnew;
        m_soilLSC[i][0] += 0.42f * LSF * resnew;

        m_soilLSLC[i][0] += RLR * 0.42f * resnew;
        m_soilLSLNC[i][0] = m_soilLSC[i][0] - m_soilLSLC[i][0];

        if (resnew_n > 0.42f * LSF * resnew / 150.f) {
            m_soilLSN[i][0] += 0.42f * LSF * resnew / 150.f;
            m_soilLMN[i][0] += resnew_n - 0.42f * LSF * resnew / 150.f + 1.e-25f;
        } else {
            m_soilLSN[i][0] += resnew_n;
            m_soilLMN[i][0] += 1.e-25f;
        }
        m_soilLMC[i][0] += 0.42f * LMF * resnew;
        /// update no3 and nh4 in soil
        m_soilNO3[i][0] *= 1.f - sf;
        m_soilNH4[i][0] *= 1.f - sf;
    }
    /// end insert new biomass of CENTURY model

    /// allocate dead roots, N, P to soil layers
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        m_soilRsd[i][l] += tmp_rtfr[i][l] * rtresnew;
        m_soilFrshOrgN[i][l] += tmp_rtfr[i][l] * ff2 * (m_pltN[i] - yieldn);
        m_soilFrshOrgP[i][l] += tmp_rtfr[i][l] * ff2 * (m_pltP[i] - yieldp);

        /// insert new biomass of CENTURY model
        if (m_cbnModel == 2) {
            if (l == 1) { sf = 0.05f; } else { sf = 0.1f; }

            sol_min_n = m_soilNO3[i][l] + m_soilNH4[i][l]; // kg/ha
            resnew = tmp_rtfr[i][l] * rtresnew;
            resnew_n = tmp_rtfr[i][l] * ff2 * (m_pltN[i] - yieldn);
            resnew_ne = resnew_n + sf * sol_min_n;

            RLN = resnew * CLG / (resnew_n + 1.e-5f);
            RLR = Min(0.8f, resnew * CLG / 1000.f / (resnew / 1000.f + 1.e-5f));
            LMF = 0.85f - 0.018f * RLN;
            if (LMF < 0.01f) { LMF = 0.01f; } else if (LMF > 0.7f) LMF = 0.7f;

            LSF = 1.f - LMF;
            m_soilLM[i][l] += LMF * resnew;
            m_soilLS[i][l] += LSF * resnew;

            /// here a simplified assumption of 0.5 LSL
            //LSLF = 0.f;
            //LSLF = CLG;

            m_soilLSL[i][l] += RLR * LSF * resnew;
            m_soilLSC[i][l] += 0.42f * LSF * resnew;

            m_soilLSLC[i][l] += RLR * 0.42f * LSF * resnew;
            m_soilLSLNC[i][l] = m_soilLSC[i][l] - m_soilLSLC[i][l];

            if (resnew_ne > 0.42f * LSF * resnew / 150.f) {
                m_soilLSN[i][l] += 0.42f * LSF * resnew / 150.f;
                m_soilLMN[i][l] += resnew_ne - 0.42f * LSF * resnew / 150.f + 1.e-25f;
            } else {
                m_soilLSN[i][l] += resnew_ne;
                m_soilLMN[i][l] += 1.e-25f;
            }
            m_soilLMC[i][l] += 0.42f * LMF * resnew;
            /// update no3 and nh4 in soil
            m_soilNO3[i][l] *= 1.f - sf;
            m_soilNH4[i][l] *= 1.f - sf;
        }
        /// end insert new biomass of CENTURY model
    }
    if (cnop > 0.f) {
        m_cn2[i] = cnop;
    } /// TODO: Is necessary to isolate the curno.f function for SUR_CN?
    /// reset variables
    m_igro[i] = 0.f;
    m_dormFlag[i] = 0.f;
    m_biomass[i] = 0.f;
    m_frRoot[i] = 0.f;
    m_pltN[i] = 0.f;
    m_pltP[i] = 0.f;
    m_frStrsWtr[i] = 1.f;
    m_lai[i] = 0.f;
    m_hvstIdxAdj[i] = 0.f;
    m_phuAccum[i] = 0.f;
    m_phuPlt[i] = 0.f;
}

void MGTOpt_SWAT::RootFraction(const int i, float*& root_fr) {
    float cum_rd = 0.f, cum_d = 0.f, cum_rf = 0.f, x1 = 0.f, x2 = 0.f;
    if (m_stoSoilRootD[i] < UTIL_ZERO) {
        root_fr[0] = 1.f;
        return;
    }
    /// Normalized Root Density = 1.15*exp[-11.7*NRD] + 0.022, where NRD = normalized rooting depth
    /// Parameters of Normalized Root Density Function from Dwyer et al 19xx
    float a = 1.15f;
    float b = 11.7f;
    float c = 0.022f;
    float d = 0.12029f; /// Integral of Normalized Root Distribution Function  from 0 to 1 (normalized depth) = 0.12029
    int k = 0;          /// used as layer identifier
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        cum_d += m_soilThick[i][l];
        if (cum_d >= m_stoSoilRootD[i]) cum_rd = m_stoSoilRootD[i];
        if (cum_d < m_stoSoilRootD[i]) cum_rd = cum_d;
        x1 = (cum_rd - m_soilThick[i][l]) / m_stoSoilRootD[i];
        x2 = cum_rd / m_stoSoilRootD[i];
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
        if (cum_rd >= m_stoSoilRootD[i]) {
            break;
        }
    }
    /// ensures that cumulative fractional root distribution = 1
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        root_fr[l] /= cum_rf;
        if (l == k) {
            /// exits loop on the same layer as the previous loop
            break;
        }
    }
}

void MGTOpt_SWAT::ExecuteTillageOperation(const int i, const int factoryID, const int nOp) {
    /// newtillmix.f
    /// Mix residue and nutrients during tillage and biological mixing
    TillOp* curOperation = static_cast<TillOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
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

    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        tmp_soilMass[i][l] = 0.f;
        tmp_soilMixedMass[i][l] = 0.f;
        tmp_soilNotMixedMass[i][l] = 0.f;
    }

    if (bmix > UTIL_ZERO) {
        /// biological mixing, TODO, in SWAT, this occurs at the end of year process.
        emix = bmix;
        dtil = Min(m_soilDepth[i][CVT_INT(m_nSoilLyrs[i]) - 1], 50.f);
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
    if (m_cbnModel == 2) {
        /// DSSAT tillage
        m_tillDays[i] = 0;
        m_tillDepth[i] = dtil;
        m_tillSwitch[i] = 1;
    }
    ///  smix(:)     |varies        |amount of substance in soil profile that is being redistributed between mixed layers
    int npmx = 0; /// number of different pesticides, TODO in next version
    for (int ii = 0; ii < 22 + npmx + 12; ii++) tmp_smix[i][ii] = 0.f;

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
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        tmp_soilMass[i][l] = 10000.f * m_soilThick[i][l] * m_soilBD[i][l] * (1 - m_soilRock[i][l] / 100.f);
        tmp_soilMixedMass[i][l] = 0.f;
        tmp_soilNotMixedMass[i][l] = 0.f;
    }
    if (dtil > 0.f) {
        if (dtil < 10.f) dtil = 11.;
        for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
            if (m_soilDepth[i][l] <= dtil) {
                tmp_soilMixedMass[i][l] = emix * tmp_soilMass[i][l];
                tmp_soilNotMixedMass[i][l] = tmp_soilMass[i][l] - tmp_soilMixedMass[i][l];
            } else if (m_soilDepth[i][l] > dtil && m_soilDepth[i][l - 1] < dtil) {
                tmp_soilMixedMass[i][l] = emix * tmp_soilMass[i][l] *
                        (dtil - m_soilDepth[i][l - 1]) / m_soilThick[i][l];
                tmp_soilNotMixedMass[i][l] = tmp_soilMass[i][l] - tmp_soilMixedMass[i][l];
            } else {
                tmp_soilMixedMass[i][l] = 0.f;
                tmp_soilNotMixedMass[i][l] = tmp_soilMass[i][l];
            }
            /// calculate the mass or concentration of each mixed element
            /// 1. mass based mixing
            WW1 = tmp_soilMixedMass[i][l] / (tmp_soilMixedMass[i][l] + tmp_soilNotMixedMass[i][l]);
            tmp_smix[i][0] += m_soilNO3[i][l] * WW1;
            tmp_smix[i][1] += m_soilStabOrgN[i][l] * WW1;
            tmp_smix[i][2] += m_soilNH4[i][l] * WW1;
            tmp_smix[i][3] += m_soilSolP[i][l] * WW1;
            tmp_smix[i][4] += m_soilHumOrgP[i][l] * WW1;
            tmp_smix[i][5] += m_soilActvOrgN[i][l] * WW1;
            tmp_smix[i][6] += m_soilActvMinP[i][l] * WW1;
            tmp_smix[i][7] += m_soilFrshOrgN[i][l] * WW1;
            tmp_smix[i][8] += m_soilFrshOrgP[i][l] * WW1;
            tmp_smix[i][9] += m_soilStabMinP[i][l] * WW1;
            tmp_smix[i][10] += m_soilRsd[i][l] * WW1;
            if (m_cbnModel == 1) {
                /// C-FARM one carbon pool model
                tmp_smix[i][11] += m_soilManC[i][l] * WW1;
                tmp_smix[i][12] += m_soilManN[i][l] * WW1;
                tmp_smix[i][13] += m_soilManP[i][l] * WW1;
            }

            /// 2. concentration based mixing
            WW2 = XX + tmp_soilMixedMass[i][l];
            tmp_smix[i][14] = (XX * tmp_smix[i][14] + m_soilCbn[i][l] * tmp_soilMixedMass[i][l]) / WW2;
            tmp_smix[i][15] = (XX * tmp_smix[i][15] + m_soilN[i][l] * tmp_soilMixedMass[i][l]) / WW2;
            tmp_smix[i][16] = (XX * tmp_smix[i][16] + m_soilClay[i][l] * tmp_soilMixedMass[i][l]) / WW2;
            tmp_smix[i][17] = (XX * tmp_smix[i][17] + m_soilSilt[i][l] * tmp_soilMixedMass[i][l]) / WW2;
            tmp_smix[i][18] = (XX * tmp_smix[i][18] + m_soilSand[i][l] * tmp_soilMixedMass[i][l]) / WW2;
            /// 3. mass based distribution
            for (int k = 0; k < npmx; k++) {
                /// TODO
                /// smix[19+k] += sol_pst(k,jj,l) * WW1
            }
            /// 4. For CENTURY model
            if (m_cbnModel == 2) {
                tmp_smix[i][19 + npmx + 1] += m_soilLSC[i][l] * WW1;
                tmp_smix[i][19 + npmx + 2] += m_soilLSLC[i][l] * WW1;
                tmp_smix[i][19 + npmx + 3] += m_soilLSLNC[i][l] * WW1;
                tmp_smix[i][19 + npmx + 4] += m_soilLMC[i][l] * WW1;
                tmp_smix[i][19 + npmx + 5] += m_soilLM[i][l] * WW1;
                tmp_smix[i][19 + npmx + 6] += m_soilLSL[i][l] * WW1;
                tmp_smix[i][19 + npmx + 7] += m_soilLS[i][l] * WW1;

                tmp_smix[i][19 + npmx + 8] += m_soilLSN[i][l] * WW1;
                tmp_smix[i][19 + npmx + 9] += m_soilLMN[i][l] * WW1;
                tmp_smix[i][19 + npmx + 10] += m_soilBMN[i][l] * WW1;
                tmp_smix[i][19 + npmx + 11] += m_soilHSN[i][l] * WW1;
                tmp_smix[i][19 + npmx + 12] += m_soilHPN[i][l] * WW1;
            }
            XX += tmp_soilMixedMass[i][l];
        }
        for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
            /// reconstitute each soil layer
            WW3 = tmp_soilNotMixedMass[i][l] / tmp_soilMass[i][l];
            WW4 = tmp_soilMixedMass[i][l] / XX;
            m_soilNO3[i][l] = m_soilNO3[i][l] * WW3 + tmp_smix[i][0] * WW4;
            m_soilStabOrgN[i][l] = m_soilStabOrgN[i][l] * WW3 + tmp_smix[i][1] * WW4;
            m_soilNH4[i][l] = m_soilNH4[i][l] * WW3 + tmp_smix[i][2] * WW4;
            m_soilSolP[i][l] = m_soilSolP[i][l] * WW3 + tmp_smix[i][3] * WW4;
            m_soilHumOrgP[i][l] = m_soilHumOrgP[i][l] * WW3 + tmp_smix[i][4] * WW4;
            m_soilActvOrgN[i][l] = m_soilActvOrgN[i][l] * WW3 + tmp_smix[i][5] * WW4;
            m_soilActvMinP[i][l] = m_soilActvMinP[i][l] * WW3 + tmp_smix[i][6] * WW4;
            m_soilFrshOrgN[i][l] = m_soilFrshOrgN[i][l] * WW3 + tmp_smix[i][7] * WW4;
            m_soilFrshOrgP[i][l] = m_soilFrshOrgP[i][l] * WW3 + tmp_smix[i][8] * WW4;
            m_soilStabMinP[i][l] = m_soilStabMinP[i][l] * WW3 + tmp_smix[i][9] * WW4;
            m_soilRsd[i][l] = m_soilRsd[i][l] * WW3 + tmp_smix[i][10] * WW4;
            if (m_soilRsd[i][l] < 1.e-10f) m_soilRsd[i][l] = 1.e-10f;
            if (m_cbnModel == 1) {
                m_soilManC[i][l] = m_soilManC[i][l] * WW3 + tmp_smix[i][11] * WW4;
                m_soilManN[i][l] = m_soilManN[i][l] * WW3 + tmp_smix[i][12] * WW4;
                m_soilManP[i][l] = m_soilManP[i][l] * WW3 + tmp_smix[i][13] * WW4;
            }
            m_soilCbn[i][l] = (m_soilCbn[i][l] * tmp_soilNotMixedMass[i][l] + tmp_smix[i][14] * tmp_soilMixedMass[i][l])
                    /
                    tmp_soilMass[i][l];
            m_soilN[i][l] = (m_soilN[i][l] * tmp_soilNotMixedMass[i][l] + tmp_smix[i][15] * tmp_soilMixedMass[i][l]) /
                    tmp_soilMass[i][l];
            m_soilClay[i][l] = (m_soilClay[i][l] * tmp_soilNotMixedMass[i][l] + tmp_smix[i][16] * tmp_soilMixedMass[i][l
                    ]) /
                    tmp_soilMass[i][l];
            m_soilSilt[i][l] = (m_soilSilt[i][l] * tmp_soilNotMixedMass[i][l] + tmp_smix[i][17] * tmp_soilMixedMass[i][l
                    ]) /
                    tmp_soilMass[i][l];
            m_soilSand[i][l] = (m_soilSand[i][l] * tmp_soilNotMixedMass[i][l] + tmp_smix[i][18] * tmp_soilMixedMass[i][l
                    ]) /
                    tmp_soilMass[i][l];

            for (int k = 0; k < npmx; k++) {
                /// TODO
                /// sol_pst(k,jj,l) = sol_pst(k,jj,l) * WW3 + smix(20+k) * WW4
            }
            if (m_cbnModel == 2) {
                m_soilLSC[i][l] = m_soilLSC[i][l] * WW3 + tmp_smix[i][19 + npmx + 1] * WW4;
                m_soilLSLC[i][l] = m_soilLSLC[i][l] * WW3 + tmp_smix[i][19 + npmx + 2] * WW4;
                m_soilLSLNC[i][l] = m_soilLSLNC[i][l] * WW3 + tmp_smix[i][19 + npmx + 3] * WW4;
                m_soilLMC[i][l] = m_soilLMC[i][l] * WW3 + tmp_smix[i][19 + npmx + 4] * WW4;
                m_soilLM[i][l] = m_soilLM[i][l] * WW3 + tmp_smix[i][19 + npmx + 5] * WW4;
                m_soilLSL[i][l] = m_soilLSL[i][l] * WW3 + tmp_smix[i][19 + npmx + 6] * WW4;
                m_soilLS[i][l] = m_soilLS[i][l] * WW3 + tmp_smix[i][19 + npmx + 7] * WW4;
                m_soilLSN[i][l] = m_soilLSN[i][l] * WW3 + tmp_smix[i][19 + npmx + 8] * WW4;
                m_soilLMN[i][l] = m_soilLMN[i][l] * WW3 + tmp_smix[i][19 + npmx + 9] * WW4;
                m_soilBMN[i][l] = m_soilBMN[i][l] * WW3 + tmp_smix[i][19 + npmx + 10] * WW4;
                m_soilHSN[i][l] = m_soilHSN[i][l] * WW3 + tmp_smix[i][19 + npmx + 11] * WW4;
                m_soilHPN[i][l] = m_soilHPN[i][l] * WW3 + tmp_smix[i][19 + npmx + 12] * WW4;
            }
            if (m_cbnModel == 1) {
                /// TODO
                /// call tillfactor(jj,bmix,emix,dtil,sol_thick)
            }
        }
    }
    if (cnop > 1.e-4f) m_cn2[i] = cnop;
}

void MGTOpt_SWAT::ExecuteHarvestOnlyOperation(const int i, const int factoryID, const int nOp) {
    /// TODO to be implemented!
    /// harvestop.f
    HvstOnlyOp* curOperation = static_cast<HvstOnlyOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    //  /// initialize parameters
    //  float hi_bms = curOperation->HarvestIndexBiomass();
    //  float hi_rsd = curOperation->HarvestIndexResidue();
    //  float harveff = curOperation->HarvestEfficiency();
    //  if (m_cropLookupMap.find(CVT_INT(m_landCover[i])) == m_cropLookupMap.end())
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
    //  m_targNYld[i] = (1.f - m_frRoot[i] * m_biomass[i] * m_frPltN[i] * m_autoFertEfficiency[i]);
    //  /// compute grain yield
    //  float hiad1 = 0.f; /// hiad1       |none           |actual harvest index (adj for water/growth)
    //  float wur = 0.f; /// wur         |none           |water deficiency factor
    //  if (m_totPltPET[i] < 10.f)
    //      wur = 100.f;
    //  else
    //      wur = 100.f * m_totActPltET[i] / m_totPltPET[i];
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
    //      yieldNtbr = Min(yieldTbr * cnyld, 0.80f * m_plantN[i]);
    //      yieldPtbr = Min(yieldTbr * cpyld, 0.80f * m_plantP[i]);
    //      /// calculate nutrients removed with clippings
    //      clipNtbr = Min(clipTbr * m_frPltN[i], m_plantN[i] - yieldNtbr);
    //      clipPtbr = Min(clipTbr * m_frPltP[i], m_plantP[i] - yieldPtbr);
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
    //      yieldNbms = Min(yieldBms * cnyld, 0.80f * m_plantN[i]);
    //      yieldPbms = Min(yieldBms * cpyld, 0.80f * m_plantP[i]);
    //      /// calculate nutrients removed with clippings
    //      clipNbms = Min(clipBms * m_frPltN[i], m_plantN[i] - yieldNbms);
    //      clipPbms = Min(clipBms * m_frPltP[i], m_plantP[i] - yieldPbms);
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
    //      yieldNgrn = Min(yieldGrn * cnyld, 0.80f * m_plantN[i]);
    //      yieldPgrn = Min(yieldGrn * cpyld, 0.80f * m_plantP[i]);
    //      /// calculate nutrients removed with clippings
    //      clipNgrn = Min(clipGrn * m_frPltN[i], m_plantN[i] - yieldNgrn);
    //      clipPgrn = Min(clipGrn * m_frPltP[i], m_plantP[i] - yieldPgrn);
    //      m_plantN[i] -= (yieldNgrn + clipNgrn);
    //      m_plantP[i] -= (yieldPgrn + clipPgrn);
    //  }
    //  /// add clippings to residue and organic N and P
    //  m_soilRsd[i][0] += (clipGrn + clipBms + clipTbr);
    //  m_soilFrshOrgN[i][0] += (clipNgrn + clipNbms + clipNtbr);
    //  m_soilFrshOrgP[i][0] += (clipPgrn + clipPbms + clipPtbr);
    //  /// compute residue yield
    //  float yieldRsd = 0.f, yieldNrsd = 0.f, yieldPrsd = 0.f;
    //  if (hi_rsd > 0.)
    //  {
    //      yieldRsd = hi_rsd * m_soilRsd[i][0];
    //      yieldNrsd = hi_rsd * m_soilFrshOrgN[i][0];
    //      yieldPrsd = hi_rsd * m_soilFrshOrgP[i][0];
    //      m_soilRsd[i][0] -= yieldRsd;
    //      m_soilFrshOrgN[i][0] -= yieldNrsd;
    //      m_soilFrshOrgP[i][0] -= yieldPrsd;
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

void MGTOpt_SWAT::ExecuteKillOperation(const int i, const int factoryID, const int nOp) {
    /// killop.f
    float resnew = 0.f, rtresnew = 0.f;
    resnew = m_biomass[i] * (1.f - m_frRoot[i]);
    rtresnew = m_biomass[i] * m_frRoot[i];
    /// call rootfr.f to distributes dead root mass through the soil profile
    /// i.e., derive fraction of roots in each layer
    for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) tmp_rtfr[i][j] = 0.f;
    RootFraction(i, tmp_rtfr[i]);
    /// update residue, N, P on soil surface
    m_soilRsd[i][0] += resnew;
    m_soilFrshOrgN[i][0] += m_pltN[i] * (1.f - m_frRoot[i]);
    m_soilFrshOrgP[i][0] += m_pltP[i] * (1.f - m_frRoot[i]);
    m_soilRsd[i][0] = Max(m_soilRsd[i][0], 0.f);
    m_soilFrshOrgN[i][0] = Max(m_soilFrshOrgN[i][0], 0.f);
    m_soilFrshOrgP[i][0] = Max(m_soilFrshOrgP[i][0], 0.f);

    /// allocate dead roots, N and P to soil layers
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        m_soilRsd[i][l] += tmp_rtfr[i][l] * rtresnew;
        m_soilFrshOrgN[i][l] += tmp_rtfr[i][l] * m_pltN[i] * m_frRoot[i];
        m_soilFrshOrgP[i][l] += tmp_rtfr[i][l] * m_pltP[i] * m_frRoot[i];
    }
    /// reset variables
    m_igro[i] = 0.f;
    m_dormFlag[i] = 0.f;
    m_biomass[i] = 0.f;
    m_frRoot[i] = 0.f;
    m_pltN[i] = 0.f;
    m_pltP[i] = 0.f;
    m_frStrsWtr[i] = 1.f;
    m_lai[i] = 0.f;
    m_hvstIdxAdj[i] = 0.f;
    m_phuAccum[i] = 0.f;
}

void MGTOpt_SWAT::ExecuteGrazingOperation(const int i, const int factoryID, const int nOp) {
    /// TODO, graze.f, simulate biomass lost to grazing
    GrazOp* curOperation = static_cast<GrazOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    //int manureID = curOperation->ManureID();
    //int grzDays = curOperation->GrazingDays();
    //float bioEat = curOperation->BiomassConsumed();
    //float bioTrmp = curOperation->BiomassTrampled();
    //float manueKg = curOperation->ManureDeposited();
}

void MGTOpt_SWAT::ExecuteAutoIrrigationOperation(const int i, const int factoryID, const int nOp) {
    AutoIrrOp* curOperation = static_cast<AutoIrrOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    m_autoIrrSrc[i] = CVT_FLT(curOperation->AutoIrrSrcCode());
    m_autoIrrLocNo[i] = curOperation->AutoIrrSrcLocs() <= 0
                            ? CVT_FLT(m_subbsnID[i])
                            : CVT_FLT(curOperation->AutoIrrSrcLocs());
    m_wtrStrsID[i] = CVT_FLT(curOperation->WaterStrsIdent());
    m_autoWtrStrsTrig[i] = curOperation->AutoWtrStrsThrsd();
    m_autoIrrEff[i] = curOperation->IrrigationEfficiency();
    m_autoIrrWtrD[i] = curOperation->IrrigationWaterApplied();
    m_autoIrrWtr2SurfqR[i] = curOperation->SurfaceRunoffRatio();
    m_irrFlag[i] = 1;
    /// call autoirr.f
    /// TODO, this will be implemented as an isolated module in the near future.
}

void MGTOpt_SWAT::ExecuteAutoFertilizerOperation(const int i, const int factoryID, const int nOp) {
    AutoFertOp* curOperation = static_cast<AutoFertOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    m_fertID[i] = CVT_FLT(curOperation->FertilizerID());
    m_NStrsMeth[i] = CVT_FLT(curOperation->NitrogenMethod());
    m_autoNStrsTrig[i] = curOperation->NitrogenStrsFactor();
    m_autoFertMaxApldN[i] = curOperation->MaxMineralN();
    m_autoFertMaxAnnApldMinN[i] = curOperation->MaxMineralNYearly();
    m_autoFertEff[i] = curOperation->FertEfficiency();
    m_autoFertSurfFr[i] = curOperation->SurfaceFracApplied();
    if (m_cropLookupMap.find(CVT_INT(m_landCover[i])) == m_cropLookupMap.end()) {
        return;
    }
    float cnyld = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_CNYLD];
    float bio_e = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_BIO_E];
    /// calculate modifier for auto fertilization target nitrogen content'
    if (m_autoFertNtrgtMod[i] < UTIL_ZERO) {
        m_autoFertNtrgtMod[i] = 150.f * cnyld * bio_e;
    }
    /// call anfert.f
    /// TODO, this will be implemented as an isolated module in the near future.
}

void MGTOpt_SWAT::ExecuteReleaseImpoundOperation(const int i, const int factoryID, const int nOp) {
    /// No more executable code here.
    RelImpndOp* curOperation = static_cast<RelImpndOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    m_impndTrig[i] = CVT_FLT(curOperation->ImpoundTriger());
    /// pothole.f and potholehr.f for sub-daily timestep simulation, TODO
    /// IF IMP_SWAT module is not configured, then this operation will be ignored. By LJ
    if (m_potVol == nullptr) {
        return;
    }
    /// 1. pothole module has been added by LJ, 2016-9-6, IMP_SWAT
    /// paddy rice module should be added!
    m_potVolMax[i] = curOperation->MaxPondDepth();
    m_potVolLow[i] = curOperation->MinFitDepth();
    if (FloatEqual(m_impndTrig[i], 0.f)) {
        /// Currently, add pothole volume (mm) to the max depth directly (in case of infiltration).
        /// TODO, autoirrigation operations should be triggered. BY lj
        m_potVol[i] = curOperation->MaxPondDepth();
        /// force the soil water storage to field capacity
        for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
            // float dep2cap = m_sol_sat[i][ly] - m_soilStorage[i][ly];
            float dep2cap = m_soilFC[i][ly] - m_soilWtrSto[i][ly];
            if (dep2cap > 0.f) {
                dep2cap = Min(dep2cap, m_potVol[i]);
                m_soilWtrSto[i][ly] += dep2cap;
                m_potVol[i] -= dep2cap;
            }
        }
        if (m_potVol[i] < curOperation->MaxFitDepth()) {
            m_potVol[i] = curOperation->MaxFitDepth();
        } /// force to reach the up depth.
        /// recompute total soil water storage
        m_soilWtrStoPrfl[i] = 0.f;
        for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
            m_soilWtrStoPrfl[i] += m_soilWtrSto[i][ly];
        }
    } else {
        m_potVolMax[i] = 0.f;
        m_potVolLow[i] = 0.f;
    }
}

void MGTOpt_SWAT::ExecuteContinuousFertilizerOperation(const int i, const int factoryID, const int nOp) {
    // TODO
    ContFertOp* curOperation = static_cast<ContFertOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
}

void MGTOpt_SWAT::ExecuteContinuousPesticideOperation(const int i, const int factoryID, const int nOp) {
    /// TODO
    ContPestOp* curOperation = static_cast<ContPestOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
}

void MGTOpt_SWAT::ExecuteBurningOperation(const int i, const int factoryID, const int nOp) {
    BurnOp* curOperation = static_cast<BurnOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    /// TODO
}

void MGTOpt_SWAT::ScheduledManagement(const int cellIdx, const int factoryID, const int nOp) {
    /// nOp is seqNo. * 1000 + operationCode
    int mgtCode = nOp % 1000;
    switch (mgtCode) {
        case BMP_PLTOP_Plant: ExecutePlantOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Irrigation: ExecuteIrrigationOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Fertilizer: ExecuteFertilizerOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Pesticide: ExecutePesticideOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_HarvestKill: ExecuteHarvestKillOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Tillage: ExecuteTillageOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Harvest: ExecuteHarvestOnlyOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Kill: ExecuteKillOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Grazing: ExecuteGrazingOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_AutoIrrigation: ExecuteAutoIrrigationOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_AutoFertilizer: ExecuteAutoFertilizerOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_ReleaseImpound: ExecuteReleaseImpoundOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_ContinuousFertilizer: ExecuteContinuousFertilizerOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_ContinuousPesticide: ExecuteContinuousPesticideOperation(cellIdx, factoryID, nOp);
            break;
        case BMP_PLTOP_Burning: ExecuteBurningOperation(cellIdx, factoryID, nOp);
            break;
        default: break;
    }
}

int MGTOpt_SWAT::Execute() {
    CheckInputData(); /// essential input data, other inputs for specific management operation will be check separately.
    InitialOutputs(); /// all possible outputs will be initialized to avoid nullptr pointer problems.
    /// initialize arrays at the beginning of the current day, derived from sim_initday.f of SWAT
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_grainc_d != nullptr) m_grainc_d[i] = 0.f;
        if (m_stoverc_d != nullptr) m_stoverc_d[i] = 0.f;
        if (m_rsdc_d != nullptr) m_rsdc_d[i] = 0.f;
    }

    if (m_mgtFactory.empty()) return 0; /// Nothing to do

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        int curLanduseID = CVT_INT(m_landUse[i]);
        int curMgtField = CVT_INT(m_mgtFields[i]);
        /// 1. Is there any plant management operations are suitable to current cell.
        if (!ValueInVector(curLanduseID, m_landuseMgtOp)) continue;

        /// 2. If current cell located in the locations of this BMPPlantMgtFactory
        ///    locations is empty means this operation may be applied to all cells
        int curFactoryID = curLanduseID * 100 + m_subSceneID;
        if (!(m_mgtFactory[curFactoryID]->GetLocations().empty() || // which means all fields will be executed
            ValueInVector(curMgtField, m_mgtFactory[curFactoryID]->GetLocations()))) {
            continue;
        }
        /// 3. Check if there are suitable operations, and execute them.
        vector<int> curOps;
        if (GetOperationCode(i, curFactoryID, curOps)) {
            for (auto it = curOps.begin(); it != curOps.end(); ++it) {
                ScheduledManagement(i, curFactoryID, *it);
            }
        }
    }
    return 0;
}

void MGTOpt_SWAT::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    /// plant operation
    if (StringMatch(sk, VAR_HITARG)) {
        *data = m_HvstIdxTrgt;
    } else if (StringMatch(sk, VAR_BIOTARG)) {
        *data = m_biomTrgt;
        /// auto irrigation operation
    } else if (StringMatch(sk, VAR_IRR_FLAG)) {
        *data = m_irrFlag;
    } else if (StringMatch(sk, VAR_IRR_WTR)) {
        *data = m_irrWtrAmt;
    } else if (StringMatch(sk, VAR_IRR_SURFQ)) {
        *data = m_irrWtr2SurfqAmt;
    } else if (StringMatch(sk, VAR_AWTR_STRS_ID)) {
        *data = m_wtrStrsID;
    } else if (StringMatch(sk, VAR_AWTR_STRS_TRIG)) {
        *data = m_autoWtrStrsTrig;
    } else if (StringMatch(sk, VAR_AIRR_SOURCE)) {
        *data = m_autoIrrSrc;
    } else if (StringMatch(sk, VAR_AIRR_LOCATION)) {
        *data = m_autoIrrLocNo;
    } else if (StringMatch(sk, VAR_AIRR_EFF)) {
        *data = m_autoIrrEff;
    } else if (StringMatch(sk, VAR_AIRRWTR_DEPTH)) {
        *data = m_autoIrrWtrD;
    } else if (StringMatch(sk, VAR_AIRRSURF_RATIO)) {
        *data = m_autoIrrWtr2SurfqR;
        /// fertilizer / auto fertilizer operation
    } else if (StringMatch(sk, VAR_AFERT_ID)) {
        *data = m_fertID;
    } else if (StringMatch(sk, VAR_AFERT_NSTRSID)) {
        *data = m_NStrsMeth;
    } else if (StringMatch(sk, VAR_AFERT_NSTRS)) {
        *data = m_autoNStrsTrig;
    } else if (StringMatch(sk, VAR_AFERT_MAXN)) {
        *data = m_autoFertMaxApldN;
    } else if (StringMatch(sk, VAR_AFERT_AMAXN)) {
        *data = m_autoFertMaxAnnApldMinN;
    } else if (StringMatch(sk, VAR_AFERT_NYLDT)) {
        *data = m_autoFertNtrgtMod;
    } else if (StringMatch(sk, VAR_AFERT_FRTEFF)) {
        *data = m_autoFertEff;
    } else if (StringMatch(sk, VAR_AFERT_FRTSURF)) {
        *data = m_autoFertSurfFr;
        /// Grazing operation
    } else if (StringMatch(sk, VAR_GRZ_DAYS)) {
        *data = m_nGrazDays;
    } else if (StringMatch(sk, VAR_GRZ_FLAG)) {
        *data = m_grazFlag;
        /// Impound/Release operation
    } else if (StringMatch(sk, VAR_IMPOUND_TRIG)) {
        *data = m_impndTrig;
    } else if (StringMatch(sk, VAR_POT_VOLMAXMM)) {
        *data = m_potVolMax;
    } else if (StringMatch(sk, VAR_POT_VOLLOWMM)) {
        *data = m_potVolLow;
        /// tillage operation of CENTURY model
    } else if (StringMatch(sk, VAR_TILLAGE_DAYS)) {
        *data = m_tillDays;
    } else if (StringMatch(sk, VAR_TILLAGE_DEPTH)) {
        *data = m_tillDepth;
    } else if (StringMatch(sk, VAR_TILLAGE_FACTOR)) {
        *data = m_tillFactor;
    } else if (StringMatch(sk, VAR_TILLAGE_SWITCH)) {
        *data = m_tillSwitch;
    } else {
        throw ModelException(MID_PLTMGT_SWAT, "Get1DData", "Parameter " + sk + " is not existed!");
    }
}

void MGTOpt_SWAT::Get2DData(const char* key, int* nRows, int* nCols, float*** data) {
    InitialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_maxSoilLyrs;
    /// fertilizer operation
    if (StringMatch(sk, VAR_SOL_MC)) {
        *data = m_soilManC;
    } else if (StringMatch(sk, VAR_SOL_MN)) {
        *data = m_soilManN;
    } else if (StringMatch(sk, VAR_SOL_MP)) {
        *data = m_soilManP;
    } else {
        throw ModelException(MID_PLTMGT_SWAT, "Get2DData", "Parameter " + sk + " is not existed!");
    }
}

void MGTOpt_SWAT::InitialOutputs() {
    if (m_initialized) return;
    CHECK_POSITIVE(MID_PLTMGT_SWAT, m_nCells);
    if (m_cellArea < 0.f) m_cellArea = m_cellWth * m_cellWth * 0.0001f; // unit: ha
    /// figure out all the management codes, and initialize the corresponding variables, aimed to save memory. By LJ
    vector<int> defined_mgt_codes;
    for (auto it = m_mgtFactory.begin(); it != m_mgtFactory.end(); ++it) {
        int factory_id = it->first;
        vector<int>& tmp_op_seqences = m_mgtFactory[factory_id]->GetOperationSequence();
        for (auto seq_iter = tmp_op_seqences.begin(); seq_iter != tmp_op_seqences.end(); ++seq_iter) {
            /// *seq_iter is calculated by: seqNo. * 1000 + operationCode
            int cur_mgt_code = *seq_iter % 1000;
            if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), cur_mgt_code) == defined_mgt_codes.end()) {
                defined_mgt_codes.emplace_back(cur_mgt_code);
            }
        }
    }
    /// plant operation
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_Plant) != defined_mgt_codes.end()) {
        if (m_HvstIdxTrgt == nullptr) Initialize1DArray(m_nCells, m_HvstIdxTrgt, 0.f);
        if (m_biomTrgt == nullptr) Initialize1DArray(m_nCells, m_biomTrgt, 0.f);
    }
    /// irrigation / auto irrigation operations
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_Irrigation) != defined_mgt_codes.end() ||
        find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_AutoIrrigation) != defined_mgt_codes.end()) {
        if (m_irrWtrAmt == nullptr) Initialize1DArray(m_nCells, m_irrWtrAmt, 0.f);
        if (m_irrWtr2SurfqAmt == nullptr) Initialize1DArray(m_nCells, m_irrWtr2SurfqAmt, 0.f);
        if (m_irrFlag == nullptr) Initialize1DArray(m_nCells, m_irrFlag, 0.f);
        if (m_autoIrrSrc == nullptr) Initialize1DArray(m_nCells, m_autoIrrSrc, IRR_SRC_OUTWTSD);
        if (m_autoIrrLocNo == nullptr) Initialize1DArray(m_nCells, m_autoIrrLocNo, -1.f);
        if (m_wtrStrsID == nullptr) Initialize1DArray(m_nCells, m_wtrStrsID, 1.f); /// By default, plant water demand
        if (m_autoWtrStrsTrig == nullptr) Initialize1DArray(m_nCells, m_autoWtrStrsTrig, 0.f);
        if (m_autoIrrEff == nullptr) Initialize1DArray(m_nCells, m_autoIrrEff, 0.f);
        if (m_autoIrrWtrD == nullptr) Initialize1DArray(m_nCells, m_autoIrrWtrD, 0.f);
        if (m_autoIrrWtr2SurfqR == nullptr) Initialize1DArray(m_nCells, m_autoIrrWtr2SurfqR, 0.f);
    }
    /// fertilizer / auto fertilizer operations
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_Fertilizer) != defined_mgt_codes.end() ||
        find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_AutoFertilizer) != defined_mgt_codes.end()) {
        if (m_fertID == nullptr) Initialize1DArray(m_nCells, m_fertID, -1.f);
        if (m_NStrsMeth == nullptr) Initialize1DArray(m_nCells, m_NStrsMeth, 0.f);
        if (m_autoNStrsTrig == nullptr) Initialize1DArray(m_nCells, m_autoNStrsTrig, 0.f);
        if (m_autoFertMaxApldN == nullptr) Initialize1DArray(m_nCells, m_autoFertMaxApldN, 0.f);
        if (m_autoFertNtrgtMod == nullptr) Initialize1DArray(m_nCells, m_autoFertNtrgtMod, 0.f);
        if (m_autoFertMaxAnnApldMinN == nullptr) Initialize1DArray(m_nCells, m_autoFertMaxAnnApldMinN, 0.f);
        if (m_autoFertEff == nullptr) Initialize1DArray(m_nCells, m_autoFertEff, 0.f);
        if (m_autoFertSurfFr == nullptr) Initialize1DArray(m_nCells, m_autoFertSurfFr, 0.f);

        if (m_cbnModel == 1) {
            if (m_soilManC == nullptr) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilManC, 0.f);
            if (m_soilManN == nullptr) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilManN, 0.f);
            if (m_soilManP == nullptr) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilManP, 0.f);
        }
    }
    /// impound/release operation
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_ReleaseImpound) != defined_mgt_codes.end()) {
        if (m_impndTrig == nullptr) Initialize1DArray(m_nCells, m_impndTrig, 1.f);
        if (m_potVolMax == nullptr) Initialize1DArray(m_nCells, m_potVolMax, 0.f);
        if (m_potVolLow == nullptr) Initialize1DArray(m_nCells, m_potVolLow, 0.f);
    }
    /// tillage
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_Tillage) != defined_mgt_codes.end()) {
        if (m_cbnModel == 2) {
            if (m_tillDays == nullptr) Initialize1DArray(m_nCells, m_tillDays, 0.f);
            if (m_tillSwitch == nullptr) Initialize1DArray(m_nCells, m_tillSwitch, 0.f);
            if (m_tillDepth == nullptr) Initialize1DArray(m_nCells, m_tillDepth, 0.f);
            if (m_tillFactor == nullptr) Initialize1DArray(m_nCells, m_tillFactor, 0.f);
        }
    }
    /// harvestkill
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_HarvestKill) != defined_mgt_codes.end()) {
        if (m_cbnModel == 2) {
            if (m_grainc_d == nullptr) Initialize1DArray(m_nCells, m_grainc_d, 0.f);
            if (m_stoverc_d == nullptr) Initialize1DArray(m_nCells, m_stoverc_d, 0.f);
            if (m_rsdc_d == nullptr) Initialize1DArray(m_nCells, m_rsdc_d, 0.f);
        }
    }
    /// temporary variables
    if (m_doneOpSequence == nullptr) Initialize1DArray(m_nCells, m_doneOpSequence, -1);
    if (nullptr == tmp_rtfr) Initialize2DArray(m_nCells, m_maxSoilLyrs, tmp_rtfr, 0.f);
    if (nullptr == tmp_soilMass) Initialize2DArray(m_nCells, m_maxSoilLyrs, tmp_soilMass, 0.f);
    if (nullptr == tmp_soilMixedMass) Initialize2DArray(m_nCells, m_maxSoilLyrs, tmp_soilMixedMass, 0.f);
    if (nullptr == tmp_soilNotMixedMass) Initialize2DArray(m_nCells, m_maxSoilLyrs, tmp_soilNotMixedMass, 0.f);
    if (nullptr == tmp_smix) Initialize2DArray(m_nCells, 22 + 12, tmp_smix, 0.f);
    m_initialized = true;
}

float MGTOpt_SWAT::Erfc(const float xx) {
    float c1 = .19684f, c2 = .115194f;
    float c3 = .00034f, c4 = .019527f;
    float x = 0.f, erf = 0.f, erfc = 0.f;
    x = Abs(sqrt(2.f) * xx);
    erf = 1.f - pow(CVT_FLT(1.f + c1 * x + c2 * x * x + c3 * pow(x, 3.f) + c4 * pow(x, 4.f)), -4.f);
    if (xx < 0.f) erf = -erf;
    erfc = 1.f - erf;
    return erfc;
}
