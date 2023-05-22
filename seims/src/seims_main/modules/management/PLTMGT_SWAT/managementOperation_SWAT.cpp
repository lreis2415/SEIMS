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
        for (auto it = m_mgtFactory.begin(); it != m_mgtFactory.end(); ++it) {
            if (it->second != nullptr) {
                delete it->second;
                it->second = nullptr;
            }
        }
        m_mgtFactory.clear();
    }
    if (!m_landuseLookupMap.empty()) {
        for (auto it = m_landuseLookupMap.begin(); it != m_landuseLookupMap.end(); ++it) {
            if (it->second != nullptr) {
                delete[] it->second;
                it->second = nullptr;
            }
            it->second = nullptr;
        }
        m_landuseLookupMap.clear();
    }
    if (!m_cropLookupMap.empty()) {
        for (auto it = m_cropLookupMap.begin(); it != m_cropLookupMap.end(); ++it) {
            if (it->second != nullptr) {
                delete[] it->second;
                it->second = nullptr;
            }
            it->second = nullptr;
        }
        m_cropLookupMap.clear();
    }
    if (!m_fertilizerLookupMap.empty()) {
        for (auto it = m_fertilizerLookupMap.begin(); it != m_fertilizerLookupMap.end(); ++it) {
            if (it->second != nullptr) {
                delete[] it->second;
                it->second = nullptr;
            }
            it->second = nullptr;
        }
        m_fertilizerLookupMap.clear();
    }
    if (!m_tillageLookupMap.empty()) {
        for (auto it = m_tillageLookupMap.begin(); it != m_tillageLookupMap.end(); ++it) {
            if (it->second != nullptr) {
                delete[] it->second;
                it->second = nullptr;
            }
            it->second = nullptr;
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
    if (nullptr != tmp_rtfr) Release2DArray(tmp_rtfr);
    if (nullptr != tmp_soilMass) Release2DArray(tmp_soilMass);
    if (nullptr != tmp_soilMixedMass) Release2DArray(tmp_soilMixedMass);
    if (nullptr != tmp_soilNotMixedMass) Release2DArray(tmp_soilNotMixedMass);
    if (nullptr != tmp_smix) Release2DArray(tmp_smix);
}

void MGTOpt_SWAT::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, Tag_CellWidth[0])) {
        m_cellWth = value;
    } else {
        throw ModelException(M_PLTMGT_SWAT[0], "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void MGTOpt_SWAT::SetValue(const char* key, const int value) {
    string sk(key);
    if (StringMatch(sk, VAR_CSWAT[0])) {
        m_cbnModel = value;
    } else if (StringMatch(sk, VAR_SUBBSNID_NUM[0])) {
        m_nSubbsns = CVT_INT(value);
    } else {
        throw ModelException(M_PLTMGT_SWAT[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void MGTOpt_SWAT::Set1DData(const char* key, const int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SBGS[0])) {
        // TODO, current version, the shallow and deep water depths are regarded the same.
        m_deepWaterDepth = data;
        m_shallowWaterDepth = data;
        return;
    }
    CheckInputSize(M_PLTMGT_SWAT[0], key, n, m_nCells);
    /// Soil related parameters from MongoDB
    if (StringMatch(sk, VAR_SOL_ZMX[0])) {
        m_soilMaxRootD = data;
    } else if (StringMatch(sk, VAR_SOL_SUMAWC[0])) {
        m_soilSumFC = data;
    } else if (StringMatch(sk, VAR_T_BASE[0])) {
        m_pgTempBase = data;
    }
    ///  Plant operation related parameters
    else if (StringMatch(sk, VAR_CN2[0])) {
        m_cn2 = data;
    } else if (StringMatch(sk, VAR_HVSTI[0])) {
        m_hvstIdx = data;
    } else if (StringMatch(sk, VAR_WSYF[0])) {
        m_wtrStrsHvst = data;
    } else if (StringMatch(sk, VAR_PHUPLT[0])) {
        m_phuPlt = data;
    } else if (StringMatch(sk, VAR_PHUBASE[0])) {
        m_phuBase = data;
    } else if (StringMatch(sk, VAR_FR_PHU_ACC[0])) {
        m_phuAccum = data;
    } else if (StringMatch(sk, VAR_TREEYRS[0])) {
        m_curYrMat = data;
    } else if (StringMatch(sk, VAR_HVSTI_ADJ[0])) {
        m_hvstIdxAdj = data;
    } else if (StringMatch(sk, VAR_LAIDAY[0])) {
        m_lai = data;
    } else if (StringMatch(sk, VAR_LAIMAXFR[0])) {
        m_laiMaxFr = data;
    } else if (StringMatch(sk, VAR_OLAI[0])) {
        m_oLai = data;
    } else if (StringMatch(sk, VAR_PLANT_N[0])) {
        m_pltN = data;
    } else if (StringMatch(sk, VAR_PLANT_P[0])) {
        m_pltP = data;
    } else if (StringMatch(sk, VAR_FR_PLANT_N[0])) {
        m_frPltN = data;
    } else if (StringMatch(sk, VAR_FR_PLANT_P[0])) {
        m_frPltP = data;
    } else if (StringMatch(sk, VAR_PLTET_TOT[0])) {
        m_totActPltET = data;
    } else if (StringMatch(sk, VAR_PLTPET_TOT[0])) {
        m_totPltPET = data;
    } else if (StringMatch(sk, VAR_FR_ROOT[0])) {
        m_frRoot = data;
    } else if (StringMatch(sk, VAR_BIOMASS[0])) {
        m_biomass = data;
    }
        //// Harvest and Kill operation
    else if (StringMatch(sk, VAR_LAST_SOILRD[0])) {
        m_stoSoilRootD = data;
    }
        /// Irrigation operation
    else if (StringMatch(sk, VAR_FR_STRSWTR[0])) {
        m_frStrsWtr = data;
    }
        /// impound/release
    else if (StringMatch(sk, VAR_POT_VOL[0])) {
        m_potVol = data;
    } else if (StringMatch(sk, VAR_POT_SA[0])) {
        m_potArea = data;
    } else if (StringMatch(sk, VAR_POT_NO3[0])) {
        m_potNo3 = data;
    } else if (StringMatch(sk, VAR_POT_NH4[0])) {
        m_potNH4 = data;
    } else if (StringMatch(sk, VAR_POT_SOLP[0])) {
        m_potSolP = data;
    } else if (StringMatch(sk, VAR_SOL_SW[0])) {
        m_soilWtrStoPrfl = data;
    } else {
        throw ModelException(M_PLTMGT_SWAT[0], "Set1DData", "Parameter " + sk + " does not exist.");
    }
}


void MGTOpt_SWAT::Set1DData(const char* key, const int n, int* data) {
    string sk(key);
    CheckInputSize(M_PLTMGT_SWAT[0], key, n, m_nCells);
    if (StringMatch(sk, VAR_SUBBSN[0])) {
        m_subbsnID = data;
    } else if (StringMatch(sk, VAR_LANDUSE[0])) {
        m_landUse = data;
    } else if (StringMatch(sk, VAR_LANDCOVER[0])) {
        m_landCover = data;
    } else if (StringMatch(sk, VAR_IDC[0])) {
        m_landCoverCls = data;
    }
    /// Soil related parameters from MongoDB
    else if (StringMatch(sk, VAR_SOILLAYERS[0])) {
        m_nSoilLyrs = data;
    }
    ///  Plant operation related parameters
    else if (StringMatch(sk, VAR_IGRO[0])) {
        m_igro = data;
    }
    else if (StringMatch(sk, VAR_DORMI[0])) {
        m_dormFlag = data;
    }
    else {
        throw ModelException(M_PLTMGT_SWAT[0], "Set1DData",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void MGTOpt_SWAT::Set2DData(const char* key, const int n, const int col, FLTPT** data) {
    string sk(key);
    /// lookup tables
    if (StringMatch(sk, VAR_LANDUSE_LOOKUP[0])) {
        m_landuseLookup = data;
        m_landuseNum = n;
        InitializeLanduseLookup();
        if (col != LANDUSE_PARAM_COUNT) {
            throw ModelException(M_PLTMGT_SWAT[0], "ReadLanduseLookup",
                                 "The field number " + ValueToString(col) +
                                 "is not coincident with LANDUSE_PARAM_COUNT: " +
                                 ValueToString(LANDUSE_PARAM_COUNT));
        }
        return;
    }
    if (StringMatch(sk, VAR_CROP_LOOKUP[0])) {
        m_cropLookup = data;
        m_cropNum = n;
        InitializeCropLookup();
        if (col != CROP_PARAM_COUNT) {
            throw ModelException(M_PLTMGT_SWAT[0], "ReadCropLookup",
                                 "The field number " + ValueToString(col) +
                                 "is not coincident with CROP_PARAM_COUNT: " +
                                 ValueToString(CROP_PARAM_COUNT));
        }
        return;
    }
    if (StringMatch(sk, VAR_FERTILIZER_LOOKUP[0])) {
        m_fertLookup = data;
        m_fertNum = n;
        InitializeFertilizerLookup();
        if (col != FERTILIZER_PARAM_COUNT) {
            throw ModelException(M_PLTMGT_SWAT[0], "ReadFertilizerLookup",
                                 "The field number " + ValueToString(col) +
                                 "is not coincident with FERTILIZER_PARAM_COUNT: " +
                                 ValueToString(FERTILIZER_PARAM_COUNT));
        }
        return;
    }
    if (StringMatch(sk, VAR_TILLAGE_LOOKUP[0])) {
        m_tillageLookup = data;
        m_tillageNum = n;
        InitializeTillageLookup();
        if (col != TILLAGE_PARAM_COUNT) {
            throw ModelException(M_PLTMGT_SWAT[0], "ReadTillageLookup",
                                 "The field number " + ValueToString(col) +
                                 "is not coincident with TILLAGE_PARAM_COUNT: " +
                                 ValueToString(TILLAGE_PARAM_COUNT));
        }
        return;
    }
    /// 2D raster data
    CheckInputSize2D(M_PLTMGT_SWAT[0], key, n, col, m_nCells, m_maxSoilLyrs);
    /// Soil related parameters from MongoDB
    if (StringMatch(sk, VAR_SOILDEPTH[0])) {
        m_soilDepth = data;
    } else if (StringMatch(sk, VAR_SOILTHICK[0])) {
        m_soilThick = data;
    } else if (StringMatch(sk, VAR_SOL_BD[0])) {
        m_soilBD = data;
    } else if (StringMatch(sk, VAR_SOL_CBN[0])) {
        m_soilCbn = data;
    } else if (StringMatch(sk, VAR_SOL_N[0])) {
        m_soilN = data;
    } else if (StringMatch(sk, VAR_CLAY[0])) {
        m_soilClay = data;
    } else if (StringMatch(sk, VAR_SILT[0])) {
        m_soilSilt = data;
    } else if (StringMatch(sk, VAR_SAND[0])) {
        m_soilSand = data;
    } else if (StringMatch(sk, VAR_ROCK[0])) {
        m_soilRock = data;
    }
        /// Soil related parameters --  inputs from other modules
    else if (StringMatch(sk, VAR_SOL_SORGN[0])) {
        m_soilStabOrgN = data;
    } else if (StringMatch(sk, VAR_SOL_HORGP[0])) {
        m_soilHumOrgP = data;
    } else if (StringMatch(sk, VAR_SOL_SOLP[0])) {
        m_soilSolP = data;
    } else if (StringMatch(sk, VAR_SOL_NH4[0])) {
        m_soilNH4 = data;
    } else if (StringMatch(sk, VAR_SOL_NO3[0])) {
        m_soilNO3 = data;
    } else if (StringMatch(sk, VAR_SOL_AORGN[0])) {
        m_soilActvOrgN = data;
    } else if (StringMatch(sk, VAR_SOL_FORGN[0])) {
        m_soilFrshOrgN = data;
    } else if (StringMatch(sk, VAR_SOL_FORGP[0])) {
        m_soilFrshOrgP = data;
    } else if (StringMatch(sk, VAR_SOL_ACTP[0])) {
        m_soilActvMinP = data;
    } else if (StringMatch(sk, VAR_SOL_STAP[0])) {
        m_soilStabMinP = data;
    } else if (StringMatch(sk, VAR_SOL_RSD[0])) {
        m_soilRsd = data;
    } else if (StringMatch(sk, VAR_SOL_AWC[0])) {
        m_soilFC = data;
    } else if (StringMatch(sk, VAR_SOL_UL[0])) {
        m_soilSat = data;
    } else if (StringMatch(sk, VAR_SOL_ST[0])) {
        m_soilWtrSto = data;
    }
        /// inputs for CENTURY C/N cycling model in stated and necessary
    else if (StringMatch(sk, VAR_SOL_HSN[0])) {
        m_soilHSN = data;
    } else if (StringMatch(sk, VAR_SOL_LM[0])) {
        m_soilLM = data;
    } else if (StringMatch(sk, VAR_SOL_LMC[0])) {
        m_soilLMC = data;
    } else if (StringMatch(sk, VAR_SOL_LMN[0])) {
        m_soilLMN = data;
    } else if (StringMatch(sk, VAR_SOL_LSC[0])) {
        m_soilLSC = data;
    } else if (StringMatch(sk, VAR_SOL_LSN[0])) {
        m_soilLSN = data;
    } else if (StringMatch(sk, VAR_SOL_LS[0])) {
        m_soilLS = data;
    } else if (StringMatch(sk, VAR_SOL_LSL[0])) {
        m_soilLSL = data;
    } else if (StringMatch(sk, VAR_SOL_LSLC[0])) {
        m_soilLSLC = data;
    } else if (StringMatch(sk, VAR_SOL_LSLNC[0])) {
        m_soilLSLNC = data;
    }
        //else if (StringMatch(sk, VAR_SOL_WON[0])) m_sol_WON = data;
        //else if (StringMatch(sk, VAR_SOL_BM[0])) m_sol_BM = data;
        //else if (StringMatch(sk, VAR_SOL_BMC[0])) m_sol_BMC = data;
    else if (StringMatch(sk, VAR_SOL_BMN[0])) {
        m_soilBMN = data;
    }
        //else if (StringMatch(sk, VAR_SOL_HP[0])) m_sol_HP = data;
        //else if (StringMatch(sk, VAR_SOL_HS[0])) m_sol_HS = data;
        //else if (StringMatch(sk, VAR_SOL_HSC[0])) m_sol_HSC = data;
        //else if (StringMatch(sk, VAR_SOL_HPC[0])) m_sol_HPC = data;
    else if (StringMatch(sk, VAR_SOL_HPN[0])) {
        m_soilHPN = data;
    }
        //else if (StringMatch(sk, VAR_SOL_RNMN[0])) m_sol_RNMN = data;
        //else if (StringMatch(sk, VAR_SOL_RSPC[0])) m_sol_RSPC = data;
    else {
        throw ModelException(M_PLTMGT_SWAT[0], "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void MGTOpt_SWAT::SetScenario(Scenario* sce) {
    if (nullptr == sce) {
        throw ModelException(M_PLTMGT_SWAT[0], "SetScenario",
                             "The Scenario data can not to be nullptr.");
    }
    if (!m_mgtFactory.empty()) { return; } // Avoid repeatly set!

    map<int, BMPFactory *>& tmpBMPFactories = sce->GetBMPFactories();
    for (auto it = tmpBMPFactories.begin(); it != tmpBMPFactories.end(); ++it) {
        /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
        if (it->first / 100000 != BMP_TYPE_PLANT_MGT) { continue; }
        /// calculate unique index for the key of m_mgtFactory, using Landuse_ID * 100 + subScenario
        // Bugs fixed when using AppleClang here. 05/22/2023. By LJ.
        //   BMPFactory should have a destructor defined and implemented in cpp, although nothing to do!
        BMPPlantMgtFactory* tmpPltFactory = dynamic_cast<BMPPlantMgtFactory *>(it->second);
        assert (nullptr != tmpPltFactory);
        if (m_subSceneID < 0) m_subSceneID = it->second->GetSubScenarioId();
        int uniqueIdx = tmpPltFactory->GetLUCCID() * 100 + m_subSceneID;
        m_mgtFactory[uniqueIdx] = tmpPltFactory;
        m_mgtOpSequences[uniqueIdx] = tmpPltFactory->GetOperationSequence();
        m_mgtOpSeqCount[uniqueIdx] = CVT_INT(m_mgtOpSequences[uniqueIdx].size());
        m_pltMgtOps[uniqueIdx] = tmpPltFactory->GetOperations();
        /// Set plant management spatial units
        if (nullptr == m_mgtFields) {
            m_mgtFields = tmpPltFactory->GetRasterData();
        }
    }
}

void MGTOpt_SWAT::SetSubbasins(clsSubbasins* subbasins) {
    if (nullptr == subbasins) {
        throw ModelException(M_PLTMGT_SWAT[0], "SetSubbasins",
                             "The Subbasins data can not to be nullptr.");
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
    CHECK_POSITIVE(M_PLTMGT_SWAT[0], m_nCells);
    CHECK_POSITIVE(M_PLTMGT_SWAT[0], m_cellWth);
    CHECK_POSITIVE(M_PLTMGT_SWAT[0], m_maxSoilLyrs);
    CHECK_NONNEGATIVE(M_PLTMGT_SWAT[0], m_cbnModel);
    if (m_cbnModel == 2) {
        /// Check for the CENTURY required initialized variables
        CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilHSN);
        CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilLM);
        CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilLMC);
        CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilLMN);
        CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilLSC);
        CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilLSN);
        CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilLS);
        CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilLSL);
        CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilLSLC);
        CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilLSLNC);
    }
    /// DT_Raster
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_subbsnID);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_landUse);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_landCover);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_mgtFields);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_pgTempBase);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_nSoilLyrs);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilMaxRootD);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilSumFC);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_cn2);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_igro);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_landCoverCls);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_curYrMat);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_wtrStrsHvst);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_lai);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_phuBase);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_phuAccum);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_phuPlt);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_dormFlag);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_hvstIdx);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_hvstIdxAdj);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_laiMaxFr);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_oLai);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_frPltN);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_frPltP);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_totActPltET);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_totPltPET);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_frRoot);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_biomass);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_stoSoilRootD);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_deepWaterDepth);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_shallowWaterDepth);
    /// DT_Raster2D
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilDepth);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilThick);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilBD);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilN);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilCbn);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilClay);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilSand);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilSilt);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilRock);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilActvOrgN);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilFrshOrgN);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilFrshOrgP);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilNO3);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilStabOrgN);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilHumOrgP);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilSolP);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilActvMinP);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilStabMinP);
    CHECK_POINTER(M_PLTMGT_SWAT[0], m_soilRsd);
    return true;
}

bool MGTOpt_SWAT::GetOperationCode(const int i, const int factoryID, vector<int>& nOps) {
    /// Figure out if any management operation should be applied, i.e., find sequence IDs (nOps)
    // vector<int>& tmpOpSeqences = m_mgtFactory[factoryID]->GetOperationSequence();
    // map<int, PltMgtOp *>& tmpOperations = m_mgtFactory[factoryID]->GetOperations();
    // get the next should be done sequence number
    int nextSeq = -1;
    if (m_doneOpSequence[i] == -1 || m_doneOpSequence[i] == m_mgtOpSeqCount[factoryID] - 1) {
        nextSeq = 0;
    } else {
        nextSeq = m_doneOpSequence[i] + 1;
    }
    // int opCode = tmpOpSeqences[nextSeq];
    int opCode = m_mgtOpSequences[factoryID][nextSeq];
    // figure out the nextSeq is satisfied or not. // this find() statement will always be true? by LJ
    //if (tmpOperations.find(opCode) == tmpOperations.end()) {
    //    return false;
    //}
    PltMgtOp*& tmpOperation = m_pltMgtOps[factoryID][opCode];
    bool dateDepent = false;
    bool huscDepent = false;
    /// If operation applied date (month and day) are defined
    if (m_month == tmpOperation->GetMonth() && m_day == tmpOperation->GetDay()) {
        dateDepent = true;
    }
    /// If husc is defined
    if (tmpOperation->GetHUFraction() >= 0.) {
        FLTPT aphu = NODATA_VALUE; /// fraction of total heat units accumulated
        if (!FloatEqual(m_dormFlag[i], 1.)) {
            if (tmpOperation->UseBaseHUSC() && FloatEqual(m_igro[i], 0.)) { // use base hu
                aphu = m_phuBase[i];
                if (aphu >= tmpOperation->GetHUFraction()) {
                    huscDepent = true;
                }
            } else { // use accumulated plant hu
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
        throw ModelException(M_PLTMGT_SWAT[0], "CheckInputData",
                             "Landuse lookup array must not be nullptr");
    }
    if (m_landuseNum <= 0) {
        throw ModelException(M_PLTMGT_SWAT[0], "CheckInputData",
                             "Landuse number must be greater than 0");
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
        throw ModelException(M_PLTMGT_SWAT[0], "CheckInputData",
                             "Crop lookup array must not be nullptr");
    }
    if (m_cropNum <= 0) throw ModelException(M_PLTMGT_SWAT[0], "CheckInputData",
                                             "Crop number must be greater than 0");

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
        throw ModelException(M_PLTMGT_SWAT[0], "CheckInputData",
                             "Fertilizer lookup array must not be nullptr");
    }
    if (m_fertNum <= 0) {
        throw ModelException(M_PLTMGT_SWAT[0], "CheckInputData",
                             "Fertilizer number must be greater than 0");
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
        throw ModelException(M_PLTMGT_SWAT[0], "CheckInputData",
                             "Tillage lookup array must not be nullptr");
    }
    if (m_tillageNum <= 0) {
        throw ModelException(M_PLTMGT_SWAT[0], "CheckInputData",
                             "Tillage number must be greater than 0");
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
    PltOp* curOperation = dynamic_cast<PltOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    /// initialize parameters
    m_igro[i] = 1;
    m_HvstIdxTrgt[i] = curOperation->HITarg();
    m_biomTrgt[i] = curOperation->BIOTarg(); /// kg/ha
    m_curYrMat[i] = curOperation->CurYearMaturity();
    int newPlantID = curOperation->PlantID();
    m_landCover[i] = newPlantID;
    m_phuPlt[i] = curOperation->HeatUnits();
    m_dormFlag[i] = 0;
    m_phuAccum[i] = 0.;
    m_pltN[i] = 0.;
    m_pltP[i] = 0.;
    m_totActPltET[i] = 0.;
    m_totPltPET[i] = 0.;
    m_laiMaxFr[i] = 0.;
    m_hvstIdxAdj[i] = 0.;
    m_oLai[i] = 0.;
    m_frRoot[i] = 0.;
    /// update crop-related parameters in order to calculate phuAcc. by LJ
    if (m_cropLookupMap.find(newPlantID) == m_cropLookupMap.end()) {
        throw ModelException(M_PLTMGT_SWAT[0], "ExecutePlantOperation",
                             "The new plant ID: " + ValueToString(newPlantID) +
                             " is not prepared in cropLookup table!");
    }
    // update IDC
    m_landCoverCls[i] = CVT_INT(m_cropLookupMap.at(newPlantID)[CROP_PARAM_IDX_IDC]);
    m_pgTempBase[i] = m_cropLookupMap.at(newPlantID)[CROP_PARAM_IDX_T_BASE];
    /// initialize transplant variables
    if (curOperation->LAIInit() > 0.) {
        m_lai[i] = curOperation->LAIInit();
        m_biomass[i] = curOperation->BIOInit();
    }
    /// compare maximum rooting depth in soil to maximum rooting depth of plant
    m_soilMaxRootD[i] = m_soilDepth[i][CVT_INT(m_nSoilLyrs[i] - 1)];
    /// if the land cover does existed, throw an exception.
    if (m_landuseLookupMap.find(CVT_INT(m_landCover[i])) == m_landuseLookupMap.end()) {
        throw ModelException(M_PLTMGT_SWAT[0], "ExecutePlantOperation",
                             "Land use ID: " + ValueToString(CVT_INT(m_landCover[i])) +
                             " does not existed in Landuse lookup table, please check and retry!");
    }
    FLTPT pltRootDepth = m_landuseLookupMap[CVT_INT(m_landCover[i])][LANDUSE_PARAM_ROOT_DEPTH_IDX];
    m_soilMaxRootD[i] = Min(m_soilMaxRootD[i], pltRootDepth);
    /// reset curve number if necessary
    if (curOperation->CNOP() > 0.) {
        /// curno.f
        FLTPT cnn = curOperation->CNOP();
        m_cn2[i] = cnn;
    }
}

void MGTOpt_SWAT::ExecuteIrrigationOperation(const int i, const int factoryID, const int nOp) {
    IrrOp* curOperation = dynamic_cast<IrrOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    /// initialize parameters
    /// irrigation source
    int m_irrSource = curOperation->IRRSource();
    /// irrigation source location code
    int m_irrNo = curOperation->IRRNo() <= 0 ? CVT_INT(m_subbsnID[i]) : curOperation->IRRNo();
    /// irrigation apply depth (mm)
    FLTPT m_irrApplyDepth = curOperation->IRRApplyDepth();
    /// FLTPT* m_irrSalt; /// currently not used
    /// irrigation efficiency
    FLTPT m_irrEfficiency = curOperation->IRREfficiency();

    m_irrFlag[i] = 1;
    int tmpSubbsnID = m_subbsnID[i];
    if (m_irrSource > IRR_SRC_RES) {
        /// irrigation from reach and reservoir are irr_rch.f and irr_res.f, respectively
               /// call irrsub.f
               /// Performs the irrigation operation when the source is the shallow or deep aquifer or a source outside the watershed
        FLTPT vmma = 0.; /// amount of water in source, mm
        FLTPT vmm = 0.;  ///maximum amount of water to be applied, mm
        FLTPT cnv = 0.;  /// conversion factor (mm/ha => m^3)
        FLTPT vmxi = 0.; /// amount of water specified in irrigation operation, mm
        FLTPT vol = 0.;  /// volume of water to be applied in irrigation, m^3
        FLTPT vmms = 0.; /// amount of water in shallow aquifer, m^3
        FLTPT vmmd = 0.; /// amount of water in deep aquifer, m^3
        /// Whether m_irrNo is valid
        if (m_nCellsSubbsn.find(m_irrNo) == m_nCellsSubbsn.end()) {
            m_irrNo = CVT_INT(m_subbsnID[i]);
        }

        cnv = m_nAreaSubbsn[m_irrNo] * 10.; /// area of current subbasin
        switch (m_irrSource) {
                /// in SEIMS, we hypothesis that shallow aquifer and deep aquifer is consistent within subbasin.
            case IRR_SRC_SHALLOW:
                if (m_shallowWaterDepth[tmpSubbsnID] < UTIL_ZERO) {
                    m_shallowWaterDepth[tmpSubbsnID] = 0.;
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
        if (vmm > 0.) {
            cnv = m_cellArea * 10.;
            vmxi = m_irrApplyDepth < UTIL_ZERO ? m_soilSumFC[i] : m_irrApplyDepth;
            if (vmm > vmxi) vmm = vmxi;
            vol = vmm * cnv;
            FLTPT pot_fr = 0.;
            if (m_potVol != nullptr && m_impndTrig != nullptr && FloatEqual(m_impndTrig[i], 0.)) {
                /// m_impoundTrig equals to 0 means pot_fr is 1.
                /// and m_impoundArea is set to m_cellArea.
                pot_fr = 1.;
                if (m_potArea != nullptr) {
                    m_potVol[i] += vol / (10. * m_potArea[i]);
                } else {
                    m_potVol[i] += vol / (10. * m_cellArea);
                }
                m_irrWtrAmt[i] = vmm; ///added rice irrigation 11/10/11
            } else {
                pot_fr = 0.;
                /// Call irrigate(i, vmm) /// irrigate.f
                m_irrWtrAmt[i] = vmm * (1. - curOperation->IRRSQfrac());
                m_irrWtr2SurfqAmt[i] = vmm * curOperation->IRRSQfrac();
            }
            /// subtract irrigation from shallow or deep aquifer
            if (pot_fr > UTIL_ZERO) {
                vol = m_irrWtrAmt[i] * cnv * m_irrEfficiency;
            }
            switch (m_irrSource) {
                case IRR_SRC_SHALLOW: cnv = m_nAreaSubbsn[m_irrNo] * 10.;
                    vmma = 0.;
                    if (vmms > -0.01) {
                        vmma = vol * m_shallowWaterDepth[tmpSubbsnID] * cnv / vmms;
                    }
                    vmma /= cnv;
                    m_shallowWaterDepth[tmpSubbsnID] -= vmma;
                    if (m_shallowWaterDepth[tmpSubbsnID] < 0.) {
                        vmma += m_shallowWaterDepth[tmpSubbsnID];
                        m_shallowWaterDepth[tmpSubbsnID] = 0.;
                    }
                    m_shallowIrrWater[i] += vmma;
                    break;
                case IRR_SRC_DEEP: cnv = m_nAreaSubbsn[m_irrNo] * 10.;
                    vmma = 0.;
                    if (vmmd > 0.01) {
                        vmma = vol * (m_deepWaterDepth[tmpSubbsnID] * cnv / vmmd);
                    }
                    vmma /= cnv;
                    m_deepWaterDepth[tmpSubbsnID] -= vmma;
                    if (m_deepWaterDepth[tmpSubbsnID] < 0.) {
                        vmma += m_deepWaterDepth[tmpSubbsnID];
                        m_deepWaterDepth[tmpSubbsnID] = 0.;
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
    FertOp* curOperation = dynamic_cast<FertOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    assert(nullptr != curOperation);
    /// fertilizer type, ifrt
    int fertilizerID = curOperation->FertilizerID();
    /// kg/ha         |amount of fertilizer applied to HRU
    FLTPT fertilizerKgHa = curOperation->FertilizerKg_per_ha();
    /* fraction of fertilizer which is applied to the top 10 mm of soil
	 * the remaining fraction:
     *  - is dissolved in the impounded water, if current landcover is paddy rice
	 *  - is applied to first soil layer defined by user, otherwise
	 */
    FLTPT fertilizerSurfFrac = curOperation->FertilizerSurfaceFrac();
    /// if the fertilizerID is not existed in lookup table, then throw an exception
    if (m_fertilizerLookupMap.find(fertilizerID) == m_fertilizerLookupMap.end()) {
        throw ModelException(M_PLTMGT_SWAT[0], "ExecuteFertilizerOperation", "Fertilizer ID " +
                             ValueToString(fertilizerID) +
                             " is not existed in Fertilizer Database!");
    }
    /**fertilizer paramters derived from lookup table**/
    //!!    fminn(:)      |kg minN/kg frt|fraction of fertilizer that is mineral N (NO3 + NH4)
    FLTPT fertMinN = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_FMINN_IDX];
    //!!    fminp(:)      |kg minP/kg frt|fraction of fertilizer that is mineral P
    FLTPT fertMinP = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_FMINP_IDX];
    //!!    forgn(:)      |kg orgN/kg frt|fraction of fertilizer that is organic N
    FLTPT fertOrgN = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_FORGN_IDX];
    //!!    forgp(:)      |kg orgP/kg frt|fraction of fertilizer that is organic P
    FLTPT fertOrgP = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_FORGP_IDX];
    //!!    fnh4n(:)      |kg NH4-N/kgminN|fraction of mineral N in fertilizer that is NH4-N
    FLTPT fertNH4N = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_FNH4N_IDX];
    //!!    bactpdb(:)    |# cfu/g   frt |concentration of persistent bacteria in fertilizer
    //FLTPT bactPDB = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_BACTPDB_IDX];
    //!!    bactlpdb(:)   |# cfu/g   frt |concentration of less persistent bacteria in fertilizer
    //FLTPT bactLPDB = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_BATTLPDB_IDX];
    //!!    bactkddb(:)   |none          |fraction of bacteria in solution (the remaining fraction is sorbed to soil particles)
    //FLTPT bactKDDB = m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_BACKTKDDB_IDX];
    // commercial fertilizer (0) or manure (1)
    int fertype = CVT_INT(m_fertilizerLookupMap[fertilizerID][FERTILIZER_PARAM_MANURE_IDX]);
    /**summary output**/
    //!!    fertn         |kg N/ha       |total amount of nitrogen applied to soil in cell on day
    //FLTPT fertN = 0.;
    //!!    fertp         |kg P/ha       |total amount of phosphorus applied to soil in cell on day
    //FLTPT fertP = 0.;
    //FLTPT fertSolP = 0.;
    /// cfertn       |kg N/ha       |total amount of nitrogen applied to soil during continuous fertilizer operation in cell on day
    //FLTPT cFertN = 0.;
    /// cfertp       |kg P/ha       |total amount of phosphorus applied to soil during continuous fertilizer operation in cell on day
    //FLTPT cFertP = 0.;
    /// weighting factor used to partition the organic N & P content of the fertilizer
    /// between the fresh organic and the active organic pools
    FLTPT rtof = 0.5;
    FLTPT xx = 0.; /// fraction of fertilizer applied to layer
    FLTPT gc = 0.; //, gc1 = 0.f;
    /// if current landcover is paddy rice, then apply the commercial fertilizer to the top surface and pothole.
    int lyrs = 2;
    if (m_potVol != nullptr) {
        if (FloatEqual(CVT_INT(m_landCover[i]), CROP_PADDYRICE) && fertype == 0 && m_potVol[i] > 0.) {
            lyrs = 1;
            xx = 1. - fertilizerSurfFrac;
            m_potNo3[i] += xx * fertilizerKgHa * (1. - fertNH4N) * fertMinN * m_cellArea; /// kg/ha * ha ==> kg
            m_potNH4[i] += xx * fertilizerKgHa * fertNH4N * fertMinN * m_cellArea;
            m_potSolP[i] += xx * fertilizerKgHa * fertMinP * m_cellArea;
        }
    }
    for (int l = 0; l < lyrs; l++) {
        /// top surface and first layer
        if (l == 0) xx = fertilizerSurfFrac;
        if (l == 1) xx = 1. - fertilizerSurfFrac;
        m_soilNO3[i][l] += xx * fertilizerKgHa * (1. - fertNH4N) * fertMinN;
        if (m_cbnModel == 0) {
            /// Static model
            m_soilFrshOrgN[i][l] += rtof * xx * fertilizerKgHa * fertOrgN;
            m_soilActvOrgN[i][l] += (1. - rtof) * xx * fertilizerKgHa * fertOrgN;
            m_soilFrshOrgP[i][l] += rtof * xx * fertilizerKgHa * fertOrgP;
            m_soilHumOrgP[i][l] += (1. - rtof) * xx * fertilizerKgHa * fertOrgP;
        } else if (m_cbnModel == 1) {
            /// C-FARM one carbon pool model
            m_soilManC[i][l] += xx * fertilizerKgHa * fertOrgN * 10.; /// assume C:N = 10:1
            m_soilManN[i][l] += xx * fertilizerKgHa * fertOrgN;
            m_soilManP[i][l] += xx * fertilizerKgHa * fertOrgP;
        } else if (m_cbnModel == 2) {
            FLTPT X1 = 0.;
            FLTPT X8 = 0.;
            FLTPT X10 = 0.;
            FLTPT XXX = 0.;
            FLTPT YY = 0.;
            FLTPT ZZ = 0.;
            FLTPT XZ = 0.;
            FLTPT YZ = 0.;
            FLTPT RLN = 0.;
            /// the fraction of organic carbon in fertilizer, for most fertilizers orgc_f is set to 0.
            FLTPT orgc_f = 0.;
            m_soilFrshOrgP[i][l] += rtof * xx * fertilizerKgHa * fertOrgP;
            m_soilHumOrgP[i][l] += (1. - rtof) * xx * fertilizerKgHa * fertOrgP;
            /// allocate organic fertilizer to slow (SWAT active) N pool, i.e., m_soilActiveOrgN
            m_soilHSN[i][l] += (1. - rtof) * xx * fertilizerKgHa * fertOrgN;
            m_soilActvOrgN[i][l] = m_soilHSN[i][l];
            /// X1 is fertilizer applied to layer (kg/ha)
            X1 = xx * fertilizerKgHa;
            // X8 is organic carbon applied (kg C/ha)
            X8 = X1 * orgc_f;
            /// RLN is calculated as a function of C:N ration in fertilizer
            RLN = 0.175 * orgc_f / (fertMinN + fertOrgN + 1.e-5);
            /// X10 is the fraction of carbon in fertilizer that is allocated to metabolic litter C pool
            X10 = 0.85 - 0.018 * RLN;
            if (X10 < 0.01) { X10 = 0.01; } else if (X10 > 0.7) X10 = 0.7;

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
            FLTPT lignin_C_frac = 0.175;
            /// updating lignin amount in structural litter pool
            m_soilLSLC[i][l] += XZ * lignin_C_frac;
            /// non-lignin part of the structural litter C is also updated;
            m_soilLSLNC[i][l] += XZ * (1. - lignin_C_frac);
            /// YZ is the amount of fertilizer (including C and N) allocated into structural litter SOM pool
            YZ = X1 - YY;
            m_soilLS[i][l] += YZ;
            /// assuming lignin fraction of the organic fertilizer allocated into structure litter SOM pool to be 0.175
            FLTPT lingnin_SOM_frac = 0.175;
            /// update lignin weight in structural litter.
            m_soilLSL[i][l] += YZ * lingnin_SOM_frac;
            m_soilFrshOrgN[i][l] = m_soilLMN[i][l] + m_soilLSN[i][l];
        }
        m_soilNH4[i][l] += xx * fertilizerKgHa * fertNH4N * fertMinN;
        m_soilSolP[i][l] += xx * fertilizerKgHa * fertMinP;
    }
    /// add bacteria - #cfu/g * t(manure)/ha * 1.e6g/t * ha/10,000m^2 = 100.
    /// calculate ground cover
    gc = (1.99532 - Erfc(1.333 * m_lai[i] - 2.)) / 2.1;
    if (gc < 0.) gc = 0.;
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
    PestOp* curOperation = dynamic_cast<PestOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
}

void MGTOpt_SWAT::ExecuteHarvestKillOperation(const int i, const int factoryID, const int nOp) {
    //// TODO: Yield is not set as outputs yet. by LJ
    /// harvkillop.f
    HvstKillOp* curOperation = dynamic_cast<HvstKillOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    /// initialize parameters
    FLTPT cnop = curOperation->CNOP();
    FLTPT wur = 0.;
    FLTPT hiad1 = 0.;
    if (m_cropLookupMap.find(CVT_INT(m_landCover[i])) == m_cropLookupMap.end()) {
        throw ModelException(M_PLTMGT_SWAT[0], "ExecuteHarvestKillOperation",
                             "The landcover ID " + ValueToString(m_landCover[i])
                             + " is not existed in crop lookup table!");
    }
    /// Get some parameters of current crop / landcover
    FLTPT hvsti = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_HVSTI];
    FLTPT wsyf = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_WSYF];
    int idc = CVT_INT(m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_IDC]);
    FLTPT bio_leaf = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_BIO_LEAF];
    FLTPT cnyld = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_CNYLD];
    FLTPT cpyld = m_cropLookupMap[CVT_INT(m_landCover[i])][CROP_PARAM_IDX_CPYLD];

    /// calculate modifier for autofertilization target nitrogen content
    // TODO
    //tnyld(j) = 0.
    //tnyld(j) = (1. - rwt(j)) * bio_ms(j) * pltfr_n(j) * auto_eff(j)

    if (m_HvstIdxTrgt[i] > 0.) {
        hiad1 = m_HvstIdxTrgt[i];
    } else {
        if (m_totPltPET[i] < 10.) {
            wur = 100.;
        } else {
            wur = 100. * m_totActPltET[i] / m_totPltPET[i];
        }
        hiad1 = (m_hvstIdxAdj[i] - wsyf) * (wur / (wur + CalExp(6.13 - 0.0883 * wur))) + wsyf;
        if (hiad1 > hvsti) hiad1 = hvsti;
    }
    FLTPT yield = 0.;
    FLTPT resnew = 0.;
    FLTPT rtresnew = 0.;

    /// stover fraction during harvest and kill operation
    FLTPT hi_ovr = curOperation->HarvestIndexOverride();
    FLTPT xx = curOperation->StoverFracRemoved();
    if (xx < UTIL_ZERO) {
        xx = hi_ovr;
    }
    if (hi_ovr > UTIL_ZERO) {
        yield = m_biomass[i] * hi_ovr;
        resnew = m_biomass[i] - yield;
    } else {
        if (idc == CROP_IDC_TREES) {
            yield = m_biomass[i] * (1. - bio_leaf);
            resnew = m_biomass[i] - yield;
        } else {
            if (hvsti > 1.001) {
                yield = m_biomass[i] * (1. - 1. / (1. + hiad1));
                resnew = m_biomass[i] / (1. + hiad1);
                resnew *= 1. - xx;
            } else {
                yield = (1. - m_frRoot[i]) * m_biomass[i] * hiad1;
                resnew = (1. - m_frRoot[i]) * (1. - hiad1) * m_biomass[i];
                /// remove stover during harvestKill operation
                resnew *= 1. - xx;
                rtresnew = m_frRoot[i] * m_biomass[i];
            }
        }
    }
    if (yield < 0.) yield = 0.;
    if (resnew < 0.) resnew = 0.;
    if (rtresnew < 0.) rtresnew = 0.;

    if (m_cbnModel == 2) {
        m_grainc_d[i] += yield * 0.42;
        m_stoverc_d[i] += (m_biomass[i] - yield - rtresnew) * 0.42 * xx;
        m_rsdc_d[i] += resnew * 0.42;
        m_rsdc_d[i] += rtresnew * 0.42;
    }
    /// calculate nutrient removed with yield
    FLTPT yieldn = 0., yieldp = 0.;
    yieldn = yield * cnyld;
    yieldp = yield * cpyld;
    yieldn = Min(yieldn, 0.80 * m_pltN[i]);
    yieldp = Min(yieldp, 0.80 * m_pltP[i]);

    /// call rootfr.f to distributes dead root mass through the soil profile
    /// i.e., derive fraction of roots in each layer
    for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) tmp_rtfr[i][j] = 0.;
    RootFraction(i, tmp_rtfr[i]);

    /// fraction of N, P in residue (ff1) or roots (ff2)
    FLTPT ff1 = (1. - hiad1) / (1. - hiad1 + m_frRoot[i]);
    FLTPT ff2 = 1. - ff1;
    /// update residue, N, P on soil surface
    m_soilRsd[i][0] += resnew;
    m_soilFrshOrgN[i][0] += ff1 * (m_pltN[i] - yieldn);
    m_soilFrshOrgP[i][0] += ff1 * (m_pltP[i] - yieldp);
    m_soilRsd[i][0] = Max(m_soilRsd[i][0], 0.);
    m_soilFrshOrgN[i][0] = Max(m_soilFrshOrgN[i][0], 0.);
    m_soilFrshOrgP[i][0] = Max(m_soilFrshOrgP[i][0], 0.);

    FLTPT BLG1 = 0.;
    FLTPT BLG2 = 0.;
    FLTPT BLG3 = 0.;
    FLTPT CLG = 0.;
    FLTPT sf = 0.;
    FLTPT sol_min_n = 0.;
    FLTPT resnew_n = 0.;
    FLTPT resnew_ne = 0.;
    FLTPT LMF = 0.;
    FLTPT LSF = 0.;
    FLTPT RLN = 0.;
    FLTPT RLR = 0.;
    /// insert new biomass of CENTURY model
    if (m_cbnModel == 2) {
        BLG1 = 0.01 / 0.1;
        BLG2 = 0.99;
        BLG3 = 0.10;
        FLTPT XX = CalLn(0.5 / BLG1 - 0.5);
        BLG2 = (XX - CalLn(1. / BLG2 - 1.)) / (1. - 0.5);
        BLG1 = XX + 0.5 * BLG2;
        CLG = BLG3 * m_phuAccum[i] / (m_phuAccum[i] + CalExp(BLG1 - BLG2 * m_phuAccum[i]));
        sf = 0.05;
        sol_min_n = m_soilNO3[i][0] + m_soilNH4[i][0];
        // resnew = resnew; // why do this?
        resnew_n = ff1 * (m_pltN[i] - yieldn);
        resnew_ne = resnew_n + sf * sol_min_n;

        RLN = resnew * CLG / (resnew_n + 1.e-5);
        RLR = Min(0.8, resnew * CLG / (resnew + 1.e-5));
        LMF = 0.85 - 0.018 * RLN;
        if (LMF < 0.01) { LMF = 0.01; } else if (LMF > 0.7) LMF = 0.7;
        LSF = 1. - LMF;
        m_soilLM[i][0] += LMF * resnew;
        m_soilLS[i][0] += LSF * resnew;

        m_soilLSL[i][0] += RLR * resnew;
        m_soilLSC[i][0] += 0.42 * LSF * resnew;

        m_soilLSLC[i][0] += RLR * 0.42 * resnew;
        m_soilLSLNC[i][0] = m_soilLSC[i][0] - m_soilLSLC[i][0];

        if (resnew_n > 0.42 * LSF * resnew / 150.) {
            m_soilLSN[i][0] += 0.42 * LSF * resnew / 150.;
            m_soilLMN[i][0] += resnew_n - 0.42f * LSF * resnew / 150. + 1.e-25;
        } else {
            m_soilLSN[i][0] += resnew_n;
            m_soilLMN[i][0] += 1.e-25;
        }
        m_soilLMC[i][0] += 0.42 * LMF * resnew;
        /// update no3 and nh4 in soil
        m_soilNO3[i][0] *= 1. - sf;
        m_soilNH4[i][0] *= 1. - sf;
    }
    /// end insert new biomass of CENTURY model

    /// allocate dead roots, N, P to soil layers
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        m_soilRsd[i][l] += tmp_rtfr[i][l] * rtresnew;
        m_soilFrshOrgN[i][l] += tmp_rtfr[i][l] * ff2 * (m_pltN[i] - yieldn);
        m_soilFrshOrgP[i][l] += tmp_rtfr[i][l] * ff2 * (m_pltP[i] - yieldp);

        /// insert new biomass of CENTURY model
        if (m_cbnModel == 2) {
            if (l == 1) { sf = 0.05; } else { sf = 0.1; }

            sol_min_n = m_soilNO3[i][l] + m_soilNH4[i][l]; // kg/ha
            resnew = tmp_rtfr[i][l] * rtresnew;
            resnew_n = tmp_rtfr[i][l] * ff2 * (m_pltN[i] - yieldn);
            resnew_ne = resnew_n + sf * sol_min_n;

            RLN = resnew * CLG / (resnew_n + 1.e-5);
            RLR = Min(0.8, resnew * CLG / 1000. / (resnew / 1000. + 1.e-5));
            LMF = 0.85 - 0.018 * RLN;
            if (LMF < 0.01) { LMF = 0.01; } else if (LMF > 0.7) LMF = 0.7;

            LSF = 1. - LMF;
            m_soilLM[i][l] += LMF * resnew;
            m_soilLS[i][l] += LSF * resnew;

            /// here a simplified assumption of 0.5 LSL
            //LSLF = 0.;
            //LSLF = CLG;

            m_soilLSL[i][l] += RLR * LSF * resnew;
            m_soilLSC[i][l] += 0.42 * LSF * resnew;

            m_soilLSLC[i][l] += RLR * 0.42 * LSF * resnew;
            m_soilLSLNC[i][l] = m_soilLSC[i][l] - m_soilLSLC[i][l];

            if (resnew_ne > 0.42 * LSF * resnew / 150.) {
                m_soilLSN[i][l] += 0.42 * LSF * resnew / 150.;
                m_soilLMN[i][l] += resnew_ne - 0.42 * LSF * resnew / 150. + 1.e-25;
            } else {
                m_soilLSN[i][l] += resnew_ne;
                m_soilLMN[i][l] += 1.e-25;
            }
            m_soilLMC[i][l] += 0.42 * LMF * resnew;
            /// update no3 and nh4 in soil
            m_soilNO3[i][l] *= 1. - sf;
            m_soilNH4[i][l] *= 1. - sf;
        }
        /// end insert new biomass of CENTURY model
    }
    if (cnop > 0.) {
        m_cn2[i] = cnop;
    } /// TODO: Is necessary to isolate the curno.f function for SUR_CN?
    /// reset variables
    m_igro[i] = 0;
    m_dormFlag[i] = 0;
    m_biomass[i] = 0.;
    m_frRoot[i] = 0.;
    m_pltN[i] = 0.;
    m_pltP[i] = 0.;
    m_frStrsWtr[i] = 1.;
    m_lai[i] = 0.;
    m_hvstIdxAdj[i] = 0.;
    m_phuAccum[i] = 0.;
    m_phuPlt[i] = 0.;
}

void MGTOpt_SWAT::RootFraction(const int i, FLTPT*& root_fr) {
    FLTPT cum_rd = 0.;
    FLTPT cum_d = 0.;
    FLTPT cum_rf = 0.;
    FLTPT x1 = 0.;
    FLTPT x2 = 0.;
    if (m_stoSoilRootD[i] < UTIL_ZERO) {
        root_fr[0] = 1.;
        return;
    }
    /// Normalized Root Density = 1.15*exp[-11.7*NRD] + 0.022, where NRD = normalized rooting depth
    /// Parameters of Normalized Root Density Function from Dwyer et al 19xx
    FLTPT a = 1.15;
    FLTPT b = 11.7;
    FLTPT c = 0.022;
    FLTPT d = 0.12029; /// Integral of Normalized Root Distribution Function  from 0 to 1 (normalized depth) = 0.12029
    int k = 0;          /// used as layer identifier
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        cum_d += m_soilThick[i][l];
        if (cum_d >= m_stoSoilRootD[i]) cum_rd = m_stoSoilRootD[i];
        if (cum_d < m_stoSoilRootD[i]) cum_rd = cum_d;
        x1 = (cum_rd - m_soilThick[i][l]) / m_stoSoilRootD[i];
        x2 = cum_rd / m_stoSoilRootD[i];
        FLTPT xx1 = -b * x1;
        if (xx1 > 20.) xx1 = 20.;
        FLTPT xx2 = -b * x2;
        if (xx2 > 20.) xx2 = 20.;
        root_fr[l] = (a / b * (CalExp(xx1) - CalExp(xx2)) + c * (x2 - x1)) / d;
        FLTPT xx = cum_rf;
        cum_rf += root_fr[l];
        if (cum_rf > 1.) {
            root_fr[l] = 1. - xx;
            cum_rf = 1.;
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
    TillOp* curOperation = dynamic_cast<TillOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    /// initialize parameters
    int tillID = curOperation->TillageID();
    FLTPT cnop = curOperation->CNOP();
    if (m_tillageLookupMap.find(tillID) == m_tillageLookupMap.end()) {
        throw ModelException(M_PLTMGT_SWAT[0], "ExecuteTillageOperation", "The tillage ID " + ValueToString(tillID)
                             + " is not existed in tillage lookup table!");
    }
    FLTPT deptil = m_tillageLookupMap[tillID][TILLAGE_PARAM_DEPTIL_IDX];
    FLTPT effmix = m_tillageLookupMap[tillID][TILLAGE_PARAM_EFFMIX_IDX];
    FLTPT bmix = 0.;
    FLTPT emix = 0.;
    FLTPT dtil = 0.;
    FLTPT XX = 0.;
    FLTPT WW1 = 0.;
    FLTPT WW2 = 0.;
    FLTPT WW3 = 0.;
    FLTPT WW4 = 0.;
    FLTPT maxmix = 0.;

    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        tmp_soilMass[i][l] = 0.;
        tmp_soilMixedMass[i][l] = 0.;
        tmp_soilNotMixedMass[i][l] = 0.;
    }

    if (bmix > UTIL_ZERO) {
        /// biological mixing, TODO, in SWAT, this occurs at the end of year process.
        emix = bmix;
        dtil = Min(m_soilDepth[i][CVT_INT(m_nSoilLyrs[i]) - 1], 50.);
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
    for (int ii = 0; ii < 22 + npmx + 12; ii++) tmp_smix[i][ii] = 0.;

    /// incorporate bacteria, no mixing, and lost from transport
    if (dtil > 10.) {
        //m_bactPersistSol[i] *= (1. - emix);
        //m_bactPersistParticle[i] *= (1. - emix);
        //m_bactLessPersistSol[i] *= (1. - emix);
        //m_bactLessPersistParticle[i] *= (1. - emix);
    }
    /// calculate max mixing to preserve target surface residue
    /// Assume residue in all other layers is negligible to simplify calculation and remove depth dependency
    /// TODO, m_minResidue is defined in OPS files in SWAT as residue management operation,
    ///       which should be implemented later as BMP operation. By LJ
    FLTPT m_minResidue = 10.; /// currently, set m_minResidue to 10 as default.
    if (m_minResidue > 1. && bmix < 0.001) {
        maxmix = 1. - m_minResidue / m_soilRsd[i][0];
        if (maxmix < 0.05) maxmix = 0.05;
        if (emix > maxmix) emix = maxmix;
    }
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        tmp_soilMass[i][l] = 10000. * m_soilThick[i][l] * m_soilBD[i][l] * (1 - m_soilRock[i][l] / 100.);
        tmp_soilMixedMass[i][l] = 0.;
        tmp_soilNotMixedMass[i][l] = 0.;
    }
    if (dtil > 0.) {
        if (dtil < 10.) dtil = 11.;
        for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
            if (m_soilDepth[i][l] <= dtil) {
                tmp_soilMixedMass[i][l] = emix * tmp_soilMass[i][l];
                tmp_soilNotMixedMass[i][l] = tmp_soilMass[i][l] - tmp_soilMixedMass[i][l];
            } else if (m_soilDepth[i][l] > dtil && m_soilDepth[i][l - 1] < dtil) {
                tmp_soilMixedMass[i][l] = emix * tmp_soilMass[i][l] *
                        (dtil - m_soilDepth[i][l - 1]) / m_soilThick[i][l];
                tmp_soilNotMixedMass[i][l] = tmp_soilMass[i][l] - tmp_soilMixedMass[i][l];
            } else {
                tmp_soilMixedMass[i][l] = 0.;
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
            if (m_soilRsd[i][l] < 1.e-10) m_soilRsd[i][l] = 1.e-10;
            if (m_cbnModel == 1) {
                m_soilManC[i][l] = m_soilManC[i][l] * WW3 + tmp_smix[i][11] * WW4;
                m_soilManN[i][l] = m_soilManN[i][l] * WW3 + tmp_smix[i][12] * WW4;
                m_soilManP[i][l] = m_soilManP[i][l] * WW3 + tmp_smix[i][13] * WW4;
            }
            m_soilCbn[i][l] = (m_soilCbn[i][l] * tmp_soilNotMixedMass[i][l] + tmp_smix[i][14] *
                tmp_soilMixedMass[i][l]) / tmp_soilMass[i][l];
            m_soilN[i][l] = (m_soilN[i][l] * tmp_soilNotMixedMass[i][l] + tmp_smix[i][15] *
                tmp_soilMixedMass[i][l]) /tmp_soilMass[i][l];
            m_soilClay[i][l] = (m_soilClay[i][l] * tmp_soilNotMixedMass[i][l] + tmp_smix[i][16] *
                tmp_soilMixedMass[i][l]) / tmp_soilMass[i][l];
            m_soilSilt[i][l] = (m_soilSilt[i][l] * tmp_soilNotMixedMass[i][l] + tmp_smix[i][17] *
                tmp_soilMixedMass[i][l]) / tmp_soilMass[i][l];
            m_soilSand[i][l] = (m_soilSand[i][l] * tmp_soilNotMixedMass[i][l] + tmp_smix[i][18] *
                tmp_soilMixedMass[i][l]) / tmp_soilMass[i][l];

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
    if (cnop > 1.e-4) m_cn2[i] = cnop;
}

void MGTOpt_SWAT::ExecuteHarvestOnlyOperation(const int i, const int factoryID, const int nOp) {
    /// TODO to be implemented!
    /// harvestop.f
    HvstOnlyOp* curOperation = dynamic_cast<HvstOnlyOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    //  /// initialize parameters
    //  float hi_bms = curOperation->HarvestIndexBiomass();
    //  float hi_rsd = curOperation->HarvestIndexResidue();
    //  float harveff = curOperation->HarvestEfficiency();
    //  if (m_cropLookupMap.find(CVT_INT(m_landCover[i])) == m_cropLookupMap.end())
    //throw ModelException(M_PLTMGT_SWAT[0], "ExecuteHarvestOnlyOperation", "The landcover ID " + ValueToString(m_landCover[i])
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
    //  hiad1 = (m_havstIdxAdj[i] - wsyf) * (wur / (wur + CalExp(6.13f - 0.0883f * wur))) + wsyf;
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
    FLTPT resnew = 0.;
    FLTPT rtresnew = 0.;
    resnew = m_biomass[i] * (1. - m_frRoot[i]);
    rtresnew = m_biomass[i] * m_frRoot[i];
    /// call rootfr.f to distributes dead root mass through the soil profile
    /// i.e., derive fraction of roots in each layer
    for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) tmp_rtfr[i][j] = 0.;
    RootFraction(i, tmp_rtfr[i]);
    /// update residue, N, P on soil surface
    m_soilRsd[i][0] += resnew;
    m_soilFrshOrgN[i][0] += m_pltN[i] * (1. - m_frRoot[i]);
    m_soilFrshOrgP[i][0] += m_pltP[i] * (1. - m_frRoot[i]);
    m_soilRsd[i][0] = Max(m_soilRsd[i][0], 0.);
    m_soilFrshOrgN[i][0] = Max(m_soilFrshOrgN[i][0], 0.);
    m_soilFrshOrgP[i][0] = Max(m_soilFrshOrgP[i][0], 0.);

    /// allocate dead roots, N and P to soil layers
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        m_soilRsd[i][l] += tmp_rtfr[i][l] * rtresnew;
        m_soilFrshOrgN[i][l] += tmp_rtfr[i][l] * m_pltN[i] * m_frRoot[i];
        m_soilFrshOrgP[i][l] += tmp_rtfr[i][l] * m_pltP[i] * m_frRoot[i];
    }
    /// reset variables
    m_igro[i] = 0;
    m_dormFlag[i] = 0;
    m_biomass[i] = 0.;
    m_frRoot[i] = 0.;
    m_pltN[i] = 0.;
    m_pltP[i] = 0.;
    m_frStrsWtr[i] = 1.;
    m_lai[i] = 0.;
    m_hvstIdxAdj[i] = 0.;
    m_phuAccum[i] = 0.;
}

void MGTOpt_SWAT::ExecuteGrazingOperation(const int i, const int factoryID, const int nOp) {
    /// TODO, graze.f, simulate biomass lost to grazing
    GrazOp* curOperation = dynamic_cast<GrazOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    //int manureID = curOperation->ManureID();
    //int grzDays = curOperation->GrazingDays();
    //float bioEat = curOperation->BiomassConsumed();
    //float bioTrmp = curOperation->BiomassTrampled();
    //float manueKg = curOperation->ManureDeposited();
}

void MGTOpt_SWAT::ExecuteAutoIrrigationOperation(const int i, const int factoryID, const int nOp) {
    AutoIrrOp* curOperation = dynamic_cast<AutoIrrOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    m_autoIrrSrc[i] = curOperation->AutoIrrSrcCode();
    m_autoIrrLocNo[i] = curOperation->AutoIrrSrcLocs() <= 0
                            ? m_subbsnID[i]
                            : curOperation->AutoIrrSrcLocs();
    m_wtrStrsID[i] = curOperation->WaterStrsIdent();
    m_autoWtrStrsTrig[i] = curOperation->AutoWtrStrsThrsd();
    m_autoIrrEff[i] = curOperation->IrrigationEfficiency();
    m_autoIrrWtrD[i] = curOperation->IrrigationWaterApplied();
    m_autoIrrWtr2SurfqR[i] = curOperation->SurfaceRunoffRatio();
    m_irrFlag[i] = 1;
    /// call autoirr.f
    /// TODO, this will be implemented as an isolated module in the near future.
}

void MGTOpt_SWAT::ExecuteAutoFertilizerOperation(const int i, const int factoryID, const int nOp) {
    AutoFertOp* curOperation = dynamic_cast<AutoFertOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    m_fertID[i] = curOperation->FertilizerID();
    m_NStrsMeth[i] = curOperation->NitrogenMethod();
    m_autoNStrsTrig[i] = curOperation->NitrogenStrsFactor();
    m_autoFertMaxApldN[i] = curOperation->MaxMineralN();
    m_autoFertMaxAnnApldMinN[i] = curOperation->MaxMineralNYearly();
    m_autoFertEff[i] = curOperation->FertEfficiency();
    m_autoFertSurfFr[i] = curOperation->SurfaceFracApplied();
    if (m_cropLookupMap.find(m_landCover[i]) == m_cropLookupMap.end()) {
        return;
    }
    FLTPT cnyld = m_cropLookupMap[m_landCover[i]][CROP_PARAM_IDX_CNYLD];
    FLTPT bio_e = m_cropLookupMap[m_landCover[i]][CROP_PARAM_IDX_BIO_E];
    /// calculate modifier for auto fertilization target nitrogen content'
    if (m_autoFertNtrgtMod[i] < UTIL_ZERO) {
        m_autoFertNtrgtMod[i] = 150. * cnyld * bio_e;
    }
    /// call anfert.f
    /// TODO, this will be implemented as an isolated module in the near future.
}

void MGTOpt_SWAT::ExecuteReleaseImpoundOperation(const int i, const int factoryID, const int nOp) {
    /// No more executable code here.
    RelImpndOp* curOperation = dynamic_cast<RelImpndOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
    m_impndTrig[i] = curOperation->ImpoundTriger();
    /// pothole.f and potholehr.f for sub-daily timestep simulation, TODO
    /// IF IMP_SWAT module is not configured, then this operation will be ignored. By LJ
    if (m_potVol == nullptr) {
        return;
    }
    /// 1. pothole module has been added by LJ, 2016-9-6, IMP_SWAT
    /// paddy rice module should be added!
    m_potVolMax[i] = curOperation->MaxPondDepth();
    m_potVolLow[i] = curOperation->MinFitDepth();
    if (FloatEqual(m_impndTrig[i], 0.)) {
        /// Currently, add pothole volume (mm) to the max depth directly (in case of infiltration).
        /// TODO, autoirrigation operations should be triggered. BY lj
        m_potVol[i] = curOperation->MaxPondDepth();
        /// force the soil water storage to field capacity
        for (int ly = 0; ly < m_nSoilLyrs[i]; ly++) {
            // float dep2cap = m_sol_sat[i][ly] - m_soilStorage[i][ly];
            FLTPT dep2cap = m_soilFC[i][ly] - m_soilWtrSto[i][ly];
            if (dep2cap > 0.) {
                dep2cap = Min(dep2cap, m_potVol[i]);
                m_soilWtrSto[i][ly] += dep2cap;
                m_potVol[i] -= dep2cap;
            }
        }
        if (m_potVol[i] < curOperation->MaxFitDepth()) {
            m_potVol[i] = curOperation->MaxFitDepth();
        } /// force to reach the up depth.
        /// recompute total soil water storage
        m_soilWtrStoPrfl[i] = 0.;
        for (int ly = 0; ly < m_nSoilLyrs[i]; ly++) {
            m_soilWtrStoPrfl[i] += m_soilWtrSto[i][ly];
        }
    } else {
        m_potVolMax[i] = 0.;
        m_potVolLow[i] = 0.;
    }
}

void MGTOpt_SWAT::ExecuteContinuousFertilizerOperation(const int i, const int factoryID, const int nOp) {
    // TODO
    ContFertOp* curOperation = dynamic_cast<ContFertOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
}

void MGTOpt_SWAT::ExecuteContinuousPesticideOperation(const int i, const int factoryID, const int nOp) {
    /// TODO
    ContPestOp* curOperation = dynamic_cast<ContPestOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
}

void MGTOpt_SWAT::ExecuteBurningOperation(const int i, const int factoryID, const int nOp) {
    BurnOp* curOperation = dynamic_cast<BurnOp *>(m_mgtFactory[factoryID]->GetOperations().at(nOp));
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
        if (m_grainc_d != nullptr) m_grainc_d[i] = 0.;
        if (m_stoverc_d != nullptr) m_stoverc_d[i] = 0.;
        if (m_rsdc_d != nullptr) m_rsdc_d[i] = 0.;
    }

    if (m_mgtFactory.empty()) return 0; /// Nothing to do

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_doneOpSequence[i] == -9999) { continue; }
        int curFactoryID = m_landUse[i] * 100 + m_subSceneID;
        /// Check if there are suitable operations, and execute them.
        vector<int> curOps;
        if (GetOperationCode(i, curFactoryID, curOps)) {
            for (auto it = curOps.begin(); it != curOps.end(); ++it) {
                ScheduledManagement(i, curFactoryID, *it);
            }
        }
    }
    return 0;
}

void MGTOpt_SWAT::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    /// plant operation
    if (StringMatch(sk, VAR_HITARG[0])) {
        *data = m_HvstIdxTrgt;
    } else if (StringMatch(sk, VAR_BIOTARG[0])) {
        *data = m_biomTrgt;
        /// auto irrigation operation
    } else if (StringMatch(sk, VAR_IRR_WTR[0])) {
        *data = m_irrWtrAmt;
    } else if (StringMatch(sk, VAR_IRR_SURFQ[0])) {
        *data = m_irrWtr2SurfqAmt;
    } else if (StringMatch(sk, VAR_AWTR_STRS_TRIG[0])) {
        *data = m_autoWtrStrsTrig;
    } else if (StringMatch(sk, VAR_AIRR_EFF[0])) {
        *data = m_autoIrrEff;
    } else if (StringMatch(sk, VAR_AIRRWTR_DEPTH[0])) {
        *data = m_autoIrrWtrD;
    } else if (StringMatch(sk, VAR_AIRRSURF_RATIO[0])) {
        *data = m_autoIrrWtr2SurfqR;
        /// fertilizer / auto fertilizer operation
    } else if (StringMatch(sk, VAR_AFERT_NSTRS[0])) {
        *data = m_autoNStrsTrig;
    } else if (StringMatch(sk, VAR_AFERT_MAXN[0])) {
        *data = m_autoFertMaxApldN;
    } else if (StringMatch(sk, VAR_AFERT_AMAXN[0])) {
        *data = m_autoFertMaxAnnApldMinN;
    } else if (StringMatch(sk, VAR_AFERT_NYLDT[0])) {
        *data = m_autoFertNtrgtMod;
    } else if (StringMatch(sk, VAR_AFERT_FRTEFF[0])) {
        *data = m_autoFertEff;
    } else if (StringMatch(sk, VAR_AFERT_FRTSURF[0])) {
        *data = m_autoFertSurfFr;
        /// Grazing operation
    } else if (StringMatch(sk, VAR_GRZ_DAYS[0])) {
        *data = m_nGrazDays;
    }
    /// Impound/Release operation
    else if (StringMatch(sk, VAR_POT_VOLMAXMM[0])) {
        *data = m_potVolMax;
    } else if (StringMatch(sk, VAR_POT_VOLLOWMM[0])) {
        *data = m_potVolLow;
        /// tillage operation of CENTURY model
    } else if (StringMatch(sk, VAR_TILLAGE_DAYS[0])) {
        *data = m_tillDays;
    } else if (StringMatch(sk, VAR_TILLAGE_DEPTH[0])) {
        *data = m_tillDepth;
    } else if (StringMatch(sk, VAR_TILLAGE_FACTOR[0])) {
        *data = m_tillFactor;
    } else {
        throw ModelException(M_PLTMGT_SWAT[0], "Get1DData",
                             "Parameter " + sk + " is not existed!");
    }
}


void MGTOpt_SWAT::Get1DData(const char* key, int* n, int** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    /// plant operation
    if (StringMatch(sk, VAR_IRR_FLAG[0])) {
        *data = m_irrFlag;
    } else if (StringMatch(sk, VAR_AWTR_STRS_ID[0])) {
        *data = m_wtrStrsID;
    } else if (StringMatch(sk, VAR_AFERT_ID[0])) {
        *data = m_fertID;
    } else if (StringMatch(sk, VAR_AFERT_NSTRSID[0])) {
        *data = m_NStrsMeth;
    } else if (StringMatch(sk, VAR_GRZ_FLAG[0])) {
        *data = m_grazFlag;
    }
    /// auto irrigation
    else if (StringMatch(sk, VAR_AIRR_SOURCE[0])) {
        *data = m_autoIrrSrc;
    }
    else if (StringMatch(sk, VAR_AIRR_LOCATION[0])) {
        *data = m_autoIrrLocNo;
    }
    /// Impound/Release operation
    else if (StringMatch(sk, VAR_IMPOUND_TRIG[0])) {
        *data = m_impndTrig;
    } else if (StringMatch(sk, VAR_TILLAGE_SWITCH[0])) {
        *data = m_tillSwitch;
    } else {
        throw ModelException(M_PLTMGT_SWAT[0], "Get1DData",
                             "Integer Parameter " + sk + " is not existed!");
    }
}

void MGTOpt_SWAT::Get2DData(const char* key, int* nRows, int* nCols, FLTPT*** data) {
    InitialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_maxSoilLyrs;
    /// fertilizer operation
    if (StringMatch(sk, VAR_SOL_MC[0])) {
        *data = m_soilManC;
    } else if (StringMatch(sk, VAR_SOL_MN[0])) {
        *data = m_soilManN;
    } else if (StringMatch(sk, VAR_SOL_MP[0])) {
        *data = m_soilManP;
    } else {
        throw ModelException(M_PLTMGT_SWAT[0], "Get2DData",
                             "Parameter " + sk + " is not existed!");
    }
}

void MGTOpt_SWAT::InitialOutputs() {
    if (m_initialized) return;
    CHECK_POSITIVE(M_PLTMGT_SWAT[0], m_nCells);
    if (m_cellArea < 0.) m_cellArea = m_cellWth * m_cellWth * 0.0001; // unit: ha
    /// figure out all the management codes, and initialize the corresponding variables, aimed to save memory. By LJ
    vector<int> defined_mgt_codes;
    for (auto it = m_mgtFactory.begin(); it != m_mgtFactory.end(); ++it) {
        int factory_id = it->first;
        vector<int>& tmp_op_seqences = m_mgtFactory[factory_id]->GetOperationSequence();
        for (auto seq_iter = tmp_op_seqences.begin(); seq_iter != tmp_op_seqences.end(); ++seq_iter) {
            /// *seq_iter is calculated by: seqNo. * 1000 + operationCode
            int cur_mgt_code = *seq_iter % 1000;
            if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), cur_mgt_code)
                == defined_mgt_codes.end()) {
                defined_mgt_codes.emplace_back(cur_mgt_code);
            }
        }
    }
    /// plant operation
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_Plant)
        != defined_mgt_codes.end()) {
        if (m_HvstIdxTrgt == nullptr) Initialize1DArray(m_nCells, m_HvstIdxTrgt, 0.);
        if (m_biomTrgt == nullptr) Initialize1DArray(m_nCells, m_biomTrgt, 0.);
    }
    /// irrigation / auto irrigation operations
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_Irrigation)
        != defined_mgt_codes.end() ||
        find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_AutoIrrigation)
        != defined_mgt_codes.end()) {
        if (m_irrWtrAmt == nullptr) Initialize1DArray(m_nCells, m_irrWtrAmt, 0.);
        if (m_irrWtr2SurfqAmt == nullptr) Initialize1DArray(m_nCells, m_irrWtr2SurfqAmt, 0.);
        if (m_irrFlag == nullptr) Initialize1DArray(m_nCells, m_irrFlag, 0);
        if (m_autoIrrSrc == nullptr) Initialize1DArray(m_nCells, m_autoIrrSrc, IRR_SRC_OUTWTSD);
        if (m_autoIrrLocNo == nullptr) Initialize1DArray(m_nCells, m_autoIrrLocNo, -1.);
        if (m_wtrStrsID == nullptr) Initialize1DArray(m_nCells, m_wtrStrsID, 1.); /// By default, plant water demand
        if (m_autoWtrStrsTrig == nullptr) Initialize1DArray(m_nCells, m_autoWtrStrsTrig, 0.);
        if (m_autoIrrEff == nullptr) Initialize1DArray(m_nCells, m_autoIrrEff, 0.);
        if (m_autoIrrWtrD == nullptr) Initialize1DArray(m_nCells, m_autoIrrWtrD, 0.);
        if (m_autoIrrWtr2SurfqR == nullptr) Initialize1DArray(m_nCells, m_autoIrrWtr2SurfqR, 0.);
    }
    /// fertilizer / auto fertilizer operations
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_Fertilizer)
        != defined_mgt_codes.end() ||
        find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_AutoFertilizer)
        != defined_mgt_codes.end()) {
        if (m_fertID == nullptr) Initialize1DArray(m_nCells, m_fertID, -1);
        if (m_NStrsMeth == nullptr) Initialize1DArray(m_nCells, m_NStrsMeth, 0);
        if (m_autoNStrsTrig == nullptr) Initialize1DArray(m_nCells, m_autoNStrsTrig, 0.);
        if (m_autoFertMaxApldN == nullptr) Initialize1DArray(m_nCells, m_autoFertMaxApldN, 0.);
        if (m_autoFertNtrgtMod == nullptr) Initialize1DArray(m_nCells, m_autoFertNtrgtMod, 0.);
        if (m_autoFertMaxAnnApldMinN == nullptr) Initialize1DArray(m_nCells, m_autoFertMaxAnnApldMinN, 0.);
        if (m_autoFertEff == nullptr) Initialize1DArray(m_nCells, m_autoFertEff, 0.);
        if (m_autoFertSurfFr == nullptr) Initialize1DArray(m_nCells, m_autoFertSurfFr, 0.);

        if (m_cbnModel == 1) {
            if (m_soilManC == nullptr) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilManC, 0.);
            if (m_soilManN == nullptr) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilManN, 0.);
            if (m_soilManP == nullptr) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilManP, 0.);
        }
    }
    /// impound/release operation
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_ReleaseImpound)
        != defined_mgt_codes.end()) {
        if (m_impndTrig == nullptr) Initialize1DArray(m_nCells, m_impndTrig, 1);
        if (m_potVolMax == nullptr) Initialize1DArray(m_nCells, m_potVolMax, 0.);
        if (m_potVolLow == nullptr) Initialize1DArray(m_nCells, m_potVolLow, 0.);
    }
    /// tillage
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_Tillage)
        != defined_mgt_codes.end()) {
        if (m_cbnModel == 2) {
            if (m_tillDays == nullptr) Initialize1DArray(m_nCells, m_tillDays, 0.);
            if (m_tillSwitch == nullptr) Initialize1DArray(m_nCells, m_tillSwitch, 0);
            if (m_tillDepth == nullptr) Initialize1DArray(m_nCells, m_tillDepth, 0.);
            if (m_tillFactor == nullptr) Initialize1DArray(m_nCells, m_tillFactor, 0.);
        }
    }
    /// harvestkill
    if (find(defined_mgt_codes.begin(), defined_mgt_codes.end(), BMP_PLTOP_HarvestKill)
        != defined_mgt_codes.end()) {
        if (m_cbnModel == 2) {
            if (m_grainc_d == nullptr) Initialize1DArray(m_nCells, m_grainc_d, 0.);
            if (m_stoverc_d == nullptr) Initialize1DArray(m_nCells, m_stoverc_d, 0.);
            if (m_rsdc_d == nullptr) Initialize1DArray(m_nCells, m_rsdc_d, 0.);
        }
    }
    /// temporary variables
    /// Initial locations that will be applied with plant management operations
    // if (nullptr == m_doneOpSequence) Initialize1DArray(m_nCells, m_doneOpSequence, -9999);
    m_doneOpSequence = new(nothrow)int[m_nCells];
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        int curFactoryID = CVT_INT(m_landUse[i]) * 100 + m_subSceneID;
        /// Use nested IF-ELSE to make the logic more clear!
        if (m_mgtFactory.find(curFactoryID) == m_mgtFactory.end()) { // no BMPs for landuse
            m_doneOpSequence[i] = -9999;
        } else {
            if (m_mgtFieldIDs[curFactoryID].empty()) { // operation be applied to all cells in the Field
                m_doneOpSequence[i] = -1;
            } else {
                if (m_mgtFieldIDs[curFactoryID].find(CVT_INT(m_mgtFields[i])) ==
                    m_mgtFieldIDs[curFactoryID].end()) { // not applied on this Field
                    m_doneOpSequence[i] = -9999;
                }
                else {
                    m_doneOpSequence[i] = -1;
                }
            }
        }
    }

    if (nullptr == tmp_rtfr) Initialize2DArray(m_nCells, m_maxSoilLyrs, tmp_rtfr, 0.);
    if (nullptr == tmp_soilMass) Initialize2DArray(m_nCells, m_maxSoilLyrs, tmp_soilMass, 0.);
    if (nullptr == tmp_soilMixedMass) Initialize2DArray(m_nCells, m_maxSoilLyrs, tmp_soilMixedMass, 0.);
    if (nullptr == tmp_soilNotMixedMass) Initialize2DArray(m_nCells, m_maxSoilLyrs, tmp_soilNotMixedMass, 0.);
    if (nullptr == tmp_smix) Initialize2DArray(m_nCells, 22 + 12, tmp_smix, 0.);
    m_initialized = true;
}

FLTPT MGTOpt_SWAT::Erfc(const FLTPT xx) {
    FLTPT c1 = .19684;
    FLTPT c2 = .115194;
    FLTPT c3 = .00034;
    FLTPT c4 = .019527;
    FLTPT x = 0.;
    FLTPT erf = 0.;
    FLTPT erfc = 0.;
    x = Abs(CalSqrt(2.) * xx);
    erf = 1. - CalPow(CVT_FLT(1. + c1 * x + c2 * x * x + c3 * x * x * x + c4 * x * x * x * x), -4.);
    if (xx < 0.) erf = -erf;
    erfc = 1. - erf;
    return erfc;
}
