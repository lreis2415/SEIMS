#include "Biomass_EPIC.h"

#include <cmath>

#include "PlantGrowthCommon.h"
#include "text.h"


Biomass_EPIC::Biomass_EPIC() :
    m_nCells(-1), m_co2Conc(NODATA_VALUE), m_meanTemp(nullptr), m_minTemp(nullptr),
    m_SR(nullptr), m_annMeanTemp(nullptr), m_dayLenMin(nullptr),
    m_dormHr(nullptr), m_nSoilLyrs(nullptr), m_maxSoilLyrs(-1),
    m_soilMaxRootD(nullptr),
    m_soilAlb(nullptr), m_soilDepth(nullptr), m_soilThk(nullptr),
    m_soilFC(nullptr), m_soilSumFC(nullptr), m_soilSumSat(nullptr), m_soilWtrSto(nullptr),
    m_soilWtrStoPrfl(nullptr), m_rsdInitSoil(nullptr), m_rsdCovSoil(nullptr),
    m_soilRsd(nullptr), m_biomTrgt(nullptr),
    m_igro(nullptr), m_landCoverCls(nullptr), m_minLaiDorm(nullptr),
    m_biomEnrgRatio(nullptr),
    m_biomEnrgRatio2ndPt(nullptr),
    m_biomDropFr(nullptr), m_maxLai(nullptr), m_maxBiomTree(nullptr),
    m_biomNFr1(nullptr), m_biomNFr2(nullptr),
    m_biomNFr3(nullptr), m_biomPFr1(nullptr), m_biomPFr2(nullptr),
    m_biomPFr3(nullptr), m_maxCanHgt(nullptr), m_co2Conc2ndPt(nullptr), m_dormPHUFr(nullptr),
    m_epco(nullptr), m_lightExtCoef(nullptr),
    m_frGrow1stPt(nullptr), m_frGrow2ndPt(nullptr), m_hvstIdx(nullptr),
    m_frMaxLai1stPt(nullptr), m_frMaxLai2ndPt(nullptr),
    m_matYrs(nullptr), m_pgTempBase(nullptr),
    m_pgOptTemp(nullptr), m_wavp(nullptr), m_canHgt(nullptr), m_alb(nullptr),
    m_curYrMat(nullptr),
    m_initBiom(nullptr), m_initLai(nullptr),
    m_phuPlt(nullptr), m_dormFlag(nullptr), m_totActPltET(nullptr), m_totPltPET(nullptr),
    m_upTkDistN(NODATA_VALUE), m_upTkDistP(NODATA_VALUE), m_NFixCoef(NODATA_VALUE),
    m_NFixMax(NODATA_VALUE),
    m_dayLen(nullptr), m_vpd(nullptr), m_pet(nullptr), m_maxPltET(nullptr),
    m_soilET(nullptr),
    m_soilNO3(nullptr), m_soilSolP(nullptr), m_snowAccum(nullptr),
    ubw(10.f), uobw(NODATA_VALUE), m_pltRootD(nullptr), m_wuse(nullptr),
    m_lai(nullptr),
    m_phuAccum(nullptr), m_maxLaiYr(nullptr), m_hvstIdxAdj(nullptr),
    m_LaiMaxFr(nullptr), m_oLai(nullptr), m_stoSoilRootD(nullptr),
    m_actPltET(nullptr), m_frRoot(nullptr), m_fixN(nullptr),
    m_plantUpTkN(nullptr), m_plantUpTkP(nullptr), m_pltN(nullptr),
    m_pltP(nullptr), m_frPltN(nullptr), m_frPltP(nullptr), m_NO3Defic(nullptr),
    m_frStrsAe(nullptr), m_frStrsN(nullptr),
    m_frStrsP(nullptr), m_frStrsTmp(nullptr), m_frStrsWtr(nullptr),
    m_biomassDelta(nullptr), m_biomass(nullptr) {
}

Biomass_EPIC::~Biomass_EPIC() {
    if (m_rsdCovSoil != nullptr) Release1DArray(m_rsdCovSoil);
    if (m_soilRsd != nullptr) Release2DArray(m_nCells, m_soilRsd);
    if (m_lai != nullptr) Release1DArray(m_lai);
    if (m_maxLaiYr != nullptr) Release1DArray(m_maxLaiYr);
    if (m_phuAccum != nullptr) Release1DArray(m_phuAccum);
    if (m_totActPltET != nullptr) Release1DArray(m_totActPltET);
    if (m_totPltPET != nullptr) Release1DArray(m_totPltPET);
    if (m_hvstIdxAdj != nullptr) Release1DArray(m_hvstIdxAdj);
    if (m_LaiMaxFr != nullptr) Release1DArray(m_LaiMaxFr);
    if (m_oLai != nullptr) Release1DArray(m_oLai);
    if (m_stoSoilRootD != nullptr) Release1DArray(m_stoSoilRootD);
    if (m_actPltET != nullptr) Release1DArray(m_actPltET);
    if (m_frRoot != nullptr) Release1DArray(m_frRoot);
    if (m_fixN != nullptr) Release1DArray(m_fixN);
    if (m_plantUpTkN != nullptr) Release1DArray(m_plantUpTkN);
    if (m_plantUpTkP != nullptr) Release1DArray(m_plantUpTkP);
    if (m_pltN != nullptr) Release1DArray(m_pltN);
    if (m_pltP != nullptr) Release1DArray(m_pltP);
    if (m_frPltN != nullptr) Release1DArray(m_frPltN);
    if (m_frPltP != nullptr) Release1DArray(m_frPltP);
    if (m_NO3Defic != nullptr) Release1DArray(m_NO3Defic);
    if (m_frStrsAe != nullptr) Release1DArray(m_frStrsAe);
    if (m_frStrsN != nullptr) Release1DArray(m_frStrsN);
    if (m_frStrsP != nullptr) Release1DArray(m_frStrsP);
    if (m_frStrsTmp != nullptr) Release1DArray(m_frStrsTmp);
    if (m_frStrsWtr != nullptr) Release1DArray(m_frStrsWtr);
    if (m_biomassDelta != nullptr) Release1DArray(m_biomassDelta);
    if (m_biomass != nullptr) Release1DArray(m_biomass);

    if (m_wuse != nullptr) Release2DArray(m_nCells, m_wuse);
}

void Biomass_EPIC::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, VAR_CO2[0])) m_co2Conc = value;
    else if (StringMatch(sk, VAR_NUPDIS[0])) m_upTkDistN = value;
    else if (StringMatch(sk, VAR_PUPDIS[0])) m_upTkDistP = value;
    else if (StringMatch(sk, VAR_NFIXCO[0])) m_NFixCoef = value;
    else if (StringMatch(sk, VAR_NFIXMX[0])) m_NFixMax = value;
    else {
        throw ModelException(M_PG_EPIC[0], "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void Biomass_EPIC::Set1DData(const char* key, const int n, float* data) {
    string sk(key);
    CheckInputSize(M_PG_EPIC[0], key, n, m_nCells);
    //// climate
    if (StringMatch(sk, VAR_TMEAN[0])) m_meanTemp = data;
    else if (StringMatch(sk, VAR_TMIN[0])) m_minTemp = data;
    else if (StringMatch(sk, DataType_SolarRadiation)) m_SR = data;
    else if (StringMatch(sk, VAR_DAYLEN_MIN[0])) m_dayLenMin = data;
    else if (StringMatch(sk, VAR_TMEAN_ANN[0])) m_annMeanTemp = data;
    else if (StringMatch(sk, VAR_DORMHR[0])) m_dormHr = data;
    else if (StringMatch(sk, VAR_DAYLEN[0])) m_dayLen = data;
        //// soil properties and water related
    else if (StringMatch(sk, VAR_SOILLAYERS[0])) m_nSoilLyrs = data;
    else if (StringMatch(sk, VAR_SOL_ZMX[0])) m_soilMaxRootD = data;
    else if (StringMatch(sk, VAR_SOL_ALB[0])) m_soilAlb = data;
    else if (StringMatch(sk, VAR_SOL_SW[0])) m_soilWtrStoPrfl = data;
    else if (StringMatch(sk, VAR_SOL_SUMAWC[0])) m_soilSumFC = data;
    else if (StringMatch(sk, VAR_SOL_SUMSAT[0])) m_soilSumSat = data;
    else if (StringMatch(sk, VAR_PET[0])) m_pet = data;
    else if (StringMatch(sk, VAR_VPD[0])) m_vpd = data;
    else if (StringMatch(sk, VAR_PPT[0])) m_maxPltET = data;
    else if (StringMatch(sk, VAR_SOET[0])) m_soilET = data;
    else if (StringMatch(sk, VAR_SOL_COV[0])) m_rsdCovSoil = data;
    else if (StringMatch(sk, VAR_BIOTARG[0])) m_biomTrgt = data;
    else if (StringMatch(sk, VAR_SNAC[0])) m_snowAccum = data;
    else if (StringMatch(sk, VAR_SOL_RSDIN[0])) m_rsdInitSoil = data;
    else if (StringMatch(sk, VAR_IGRO[0])) m_igro = data;
    else if (StringMatch(sk, VAR_IDC[0])) m_landCoverCls = data;
    else if (StringMatch(sk, VAR_ALAIMIN[0])) m_minLaiDorm = data;
    else if (StringMatch(sk, VAR_BIO_E[0])) m_biomEnrgRatio = data;
    else if (StringMatch(sk, VAR_BIOEHI[0])) m_biomEnrgRatio2ndPt = data;
    else if (StringMatch(sk, VAR_BIOLEAF[0])) m_biomDropFr = data;
    else if (StringMatch(sk, VAR_BLAI[0])) m_maxLai = data;
    else if (StringMatch(sk, VAR_BMX_TREES[0])) m_maxBiomTree = data;
    else if (StringMatch(sk, VAR_BN1[0])) m_biomNFr1 = data;
    else if (StringMatch(sk, VAR_BN2[0])) m_biomNFr2 = data;
    else if (StringMatch(sk, VAR_BN3[0])) m_biomNFr3 = data;
    else if (StringMatch(sk, VAR_BP1[0])) m_biomPFr1 = data;
    else if (StringMatch(sk, VAR_BP2[0])) m_biomPFr2 = data;
    else if (StringMatch(sk, VAR_BP3[0])) m_biomPFr3 = data;
    else if (StringMatch(sk, VAR_CHTMX[0])) m_maxCanHgt = data;
    else if (StringMatch(sk, VAR_CO2HI[0])) m_co2Conc2ndPt = data;
    else if (StringMatch(sk, VAR_DLAI[0])) m_dormPHUFr = data;
    else if (StringMatch(sk, VAR_EXT_COEF[0])) m_lightExtCoef = data;
    else if (StringMatch(sk, VAR_FRGRW1[0])) m_frGrow1stPt = data;
    else if (StringMatch(sk, VAR_FRGRW2[0])) m_frGrow2ndPt = data;
    else if (StringMatch(sk, VAR_HVSTI[0])) m_hvstIdx = data;
    else if (StringMatch(sk, VAR_LAIMX1[0])) m_frMaxLai1stPt = data;
    else if (StringMatch(sk, VAR_LAIMX2[0])) m_frMaxLai2ndPt = data;
    else if (StringMatch(sk, VAR_MAT_YRS[0])) m_matYrs = data;
    else if (StringMatch(sk, VAR_T_BASE[0])) m_pgTempBase = data;
    else if (StringMatch(sk, VAR_T_OPT[0])) m_pgOptTemp = data;
    else if (StringMatch(sk, VAR_WAVP[0])) m_wavp = data;
    else if (StringMatch(sk, VAR_EPCO[0])) m_epco = data;
    else if (StringMatch(sk, VAR_TREEYRS[0])) m_curYrMat = data;
    else if (StringMatch(sk, VAR_LAIINIT[0])) m_initLai = data;
    else if (StringMatch(sk, VAR_BIOINIT[0])) m_initBiom = data;
    else if (StringMatch(sk, VAR_PHUPLT[0])) m_phuPlt = data;
    else if (StringMatch(sk, VAR_CHT[0])) m_canHgt = data;
    else if (StringMatch(sk, VAR_DORMI[0])) m_dormFlag = data;
    else {
        throw ModelException(M_PG_EPIC[0], "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void Biomass_EPIC::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    string sk(key);
    CheckInputSize2D(M_PG_EPIC[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    if (StringMatch(sk, VAR_SOILDEPTH[0])) m_soilDepth = data;
    else if (StringMatch(sk, VAR_SOILTHICK[0])) m_soilThk = data;
    else if (StringMatch(sk, VAR_SOL_RSD[0])) m_soilRsd = data;
    else if (StringMatch(sk, VAR_SOL_AWC[0])) m_soilFC = data;
    else if (StringMatch(sk, VAR_SOL_ST[0])) m_soilWtrSto = data;
    else if (StringMatch(sk, VAR_SOL_NO3[0])) m_soilNO3 = data;
    else if (StringMatch(sk, VAR_SOL_SOLP[0])) m_soilSolP = data;
    else {
        throw ModelException(M_PG_EPIC[0], "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

bool Biomass_EPIC::CheckInputData() {
    /// DT_Single
    CHECK_POSITIVE(M_PG_EPIC[0], m_nCells);
    CHECK_POSITIVE(M_PG_EPIC[0], m_maxSoilLyrs);
    CHECK_NODATA(M_PG_EPIC[0], m_co2Conc);
    CHECK_NODATA(M_PG_EPIC[0], m_upTkDistN);
    CHECK_NODATA(M_PG_EPIC[0], m_upTkDistP);
    CHECK_NODATA(M_PG_EPIC[0], m_NFixCoef);
    CHECK_NODATA(M_PG_EPIC[0], m_NFixMax);
    /// DT_Raster1D
    CHECK_POINTER(M_PG_EPIC[0], m_annMeanTemp);
    CHECK_POINTER(M_PG_EPIC[0], m_minTemp);
    CHECK_POINTER(M_PG_EPIC[0], m_meanTemp);
    CHECK_POINTER(M_PG_EPIC[0], m_SR);
    CHECK_POINTER(M_PG_EPIC[0], m_dayLen);
    CHECK_POINTER(M_PG_EPIC[0], m_dayLenMin);
    CHECK_POINTER(M_PG_EPIC[0], m_dormHr);
    CHECK_POINTER(M_PG_EPIC[0], m_nSoilLyrs);
    CHECK_POINTER(M_PG_EPIC[0], m_soilMaxRootD);
    CHECK_POINTER(M_PG_EPIC[0], m_soilAlb);
    CHECK_POINTER(M_PG_EPIC[0], m_soilWtrStoPrfl);
    CHECK_POINTER(M_PG_EPIC[0], m_soilSumFC);
    CHECK_POINTER(M_PG_EPIC[0], m_soilSumSat);
    CHECK_POINTER(M_PG_EPIC[0], m_pet);
    CHECK_POINTER(M_PG_EPIC[0], m_vpd);
    CHECK_POINTER(M_PG_EPIC[0], m_maxPltET);
    CHECK_POINTER(M_PG_EPIC[0], m_soilET);
    CHECK_POINTER(M_PG_EPIC[0], m_rsdInitSoil);
    CHECK_POINTER(M_PG_EPIC[0], m_igro);
    CHECK_POINTER(M_PG_EPIC[0], m_landCoverCls);
    CHECK_POINTER(M_PG_EPIC[0], m_minLaiDorm);
    CHECK_POINTER(M_PG_EPIC[0], m_biomEnrgRatio);
    CHECK_POINTER(M_PG_EPIC[0], m_biomEnrgRatio2ndPt);
    CHECK_POINTER(M_PG_EPIC[0], m_biomDropFr);
    CHECK_POINTER(M_PG_EPIC[0], m_maxLai);
    CHECK_POINTER(M_PG_EPIC[0], m_maxBiomTree);
    CHECK_POINTER(M_PG_EPIC[0], m_biomNFr1);
    CHECK_POINTER(M_PG_EPIC[0], m_biomNFr2);
    CHECK_POINTER(M_PG_EPIC[0], m_biomNFr3);
    CHECK_POINTER(M_PG_EPIC[0], m_biomPFr1);
    CHECK_POINTER(M_PG_EPIC[0], m_biomPFr2);
    CHECK_POINTER(M_PG_EPIC[0], m_biomPFr3);
    CHECK_POINTER(M_PG_EPIC[0], m_maxCanHgt);
    CHECK_POINTER(M_PG_EPIC[0], m_co2Conc2ndPt);
    CHECK_POINTER(M_PG_EPIC[0], m_dormPHUFr);
    CHECK_POINTER(M_PG_EPIC[0], m_lightExtCoef);
    CHECK_POINTER(M_PG_EPIC[0], m_frGrow1stPt);
    CHECK_POINTER(M_PG_EPIC[0], m_frGrow2ndPt);
    CHECK_POINTER(M_PG_EPIC[0], m_hvstIdx);
    CHECK_POINTER(M_PG_EPIC[0], m_frMaxLai1stPt);
    CHECK_POINTER(M_PG_EPIC[0], m_frMaxLai2ndPt);
    CHECK_POINTER(M_PG_EPIC[0], m_matYrs);
    CHECK_POINTER(M_PG_EPIC[0], m_pgTempBase);
    CHECK_POINTER(M_PG_EPIC[0], m_pgOptTemp);
    CHECK_POINTER(M_PG_EPIC[0], m_wavp);
    CHECK_POINTER(M_PG_EPIC[0], m_epco);
    CHECK_POINTER(M_PG_EPIC[0], m_curYrMat);
    CHECK_POINTER(M_PG_EPIC[0], m_initLai);
    CHECK_POINTER(M_PG_EPIC[0], m_initBiom);
    CHECK_POINTER(M_PG_EPIC[0], m_phuPlt);
    CHECK_POINTER(M_PG_EPIC[0], m_canHgt);
    CHECK_POINTER(M_PG_EPIC[0], m_dormFlag);
    /// DT_Raster2D
    CHECK_POINTER(M_PG_EPIC[0], m_soilDepth);
    CHECK_POINTER(M_PG_EPIC[0], m_soilThk);
    CHECK_POINTER(M_PG_EPIC[0], m_soilFC);
    CHECK_POINTER(M_PG_EPIC[0], m_soilWtrSto);
    CHECK_POINTER(M_PG_EPIC[0], m_soilNO3);
    CHECK_POINTER(M_PG_EPIC[0], m_soilSolP);
    return true;
}

void Biomass_EPIC::InitialOutputs() {
    if (FloatEqual(uobw, NODATA_VALUE)) {
        ubw = 10.f; /// the uptake distribution for water is hardwired, users are not allowed to modify
        uobw = 0.f;
        uobw = GetNormalization(ubw);
    }
    if (m_alb == nullptr) Initialize1DArray(m_nCells, m_alb, 0.f);
    if (m_rsdCovSoil == nullptr || m_soilRsd == nullptr) {
        Initialize1DArray(m_nCells, m_rsdCovSoil, m_rsdInitSoil);
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilRsd, 0.f);
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            m_soilRsd[i][0] = m_rsdCovSoil[i];
        }
    }
    if (m_biomTrgt == nullptr) {
        Initialize1DArray(m_nCells, m_biomTrgt, 0.f);
    }
    if (m_lai == nullptr) {
        if (m_initLai != nullptr) {
            Initialize1DArray(m_nCells, m_lai, m_initLai);
        } else {
            Initialize1DArray(m_nCells, m_lai, 0.f);
        }
    }
    if (m_maxLaiYr == nullptr) {
        m_maxLaiYr = new(nothrow) float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            if (IsTree(CVT_INT(m_landCoverCls[i]))) {
                m_maxLaiYr[i] *= m_curYrMat[i] / m_matYrs[i];
            } else {
                m_maxLaiYr[i] = m_lai[i];
            }
        }
    }
    if (m_phuAccum == nullptr) {
        Initialize1DArray(m_nCells, m_phuAccum, 0.f);
    }
    if (m_totActPltET == nullptr) {
        Initialize1DArray(m_nCells, m_totActPltET, 0.f);
    }
    if (m_totPltPET == nullptr) {
        Initialize1DArray(m_nCells, m_totPltPET, 0.f);
    }
    if (m_hvstIdxAdj == nullptr) {
        Initialize1DArray(m_nCells, m_hvstIdxAdj, 0.f);
    }
    if (m_LaiMaxFr == nullptr) {
        Initialize1DArray(m_nCells, m_LaiMaxFr, 0.f);
    }
    if (m_oLai == nullptr) {
        Initialize1DArray(m_nCells, m_oLai, 0.f);
    }
    if (m_stoSoilRootD == nullptr) {
        Initialize1DArray(m_nCells, m_stoSoilRootD, 10.f);
    }
    if (m_pltRootD == nullptr) {
        Initialize1DArray(m_nCells, m_pltRootD, 10.f);
    }
    if (nullptr == m_wuse) {
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_wuse, 0.f);
    }
    if (m_actPltET == nullptr) {
        Initialize1DArray(m_nCells, m_actPltET, 0.f);
    }
    if (m_frRoot == nullptr) {
        Initialize1DArray(m_nCells, m_frRoot, 0.f);
    }
    if (m_fixN == nullptr) {
        Initialize1DArray(m_nCells, m_fixN, 0.f);
    }
    if (m_plantUpTkN == nullptr) {
        Initialize1DArray(m_nCells, m_plantUpTkN, 0.f);
    }
    if (m_plantUpTkP == nullptr) {
        Initialize1DArray(m_nCells, m_plantUpTkP, 0.f);
    }
    if (m_pltN == nullptr) {
        Initialize1DArray(m_nCells, m_pltN, 0.f);
    }
    if (m_pltP == nullptr) {
        Initialize1DArray(m_nCells, m_pltP, 0.f);
    }
    if (m_frPltN == nullptr) {
        Initialize1DArray(m_nCells, m_frPltN, 0.f);
    }
    if (m_frPltP == nullptr) {
        Initialize1DArray(m_nCells, m_frPltP, 0.f);
    }

    if (m_NO3Defic == nullptr) {
        Initialize1DArray(m_nCells, m_NO3Defic, 0.f);
    }
    // initialize these stress factors according to sim_iniday.f of SWAT
    if (m_frStrsAe == nullptr) {
        Initialize1DArray(m_nCells, m_frStrsAe, 1.f);
    }
    if (m_frStrsN == nullptr) {
        Initialize1DArray(m_nCells, m_frStrsN, 1.f);
    }
    if (m_frStrsP == nullptr) {
        Initialize1DArray(m_nCells, m_frStrsP, 1.f);
    }
    if (m_frStrsTmp == nullptr) {
        Initialize1DArray(m_nCells, m_frStrsTmp, 1.f);
    }
    // according to zero.f of SWAT
    if (m_frStrsWtr == nullptr) {
        Initialize1DArray(m_nCells, m_frStrsWtr, 1.f);
    }
    if (m_biomassDelta == nullptr) {
        Initialize1DArray(m_nCells, m_biomassDelta, 0.f);
    }
    if (m_biomass == nullptr) {
        if (m_initBiom != nullptr) {
            Initialize1DArray(m_nCells, m_biomass, m_initBiom);
        } else {
            Initialize1DArray(m_nCells, m_biomass, 0.f);
        }
    }
}

void Biomass_EPIC::DistributePlantET(const int i) {
    /// swu.f of SWAT
    float sum = 0.f, sump = 0.f, gx = 0.f;
    /// fraction of water uptake by plants achieved
    /// where the reduction is caused by low water content
    float reduc = 0.f;
    /// water uptake by plants in each soil layer

    /*
     * Initialize1DArray should not be used inside a OMP codeblock, which is nested omp for loops.
     * Although in most case, this would be compiled and run successfully,
     *   the nested omp for loops still be problematic potentially.
     * See https://github.com/lreis2415/SEIMS/issues/36 for more information.
     * By lj, 06/27/18.
     */
    // 1. Nested omp for loops
    //float* wuse = nullptr;
    //Initialize1DArray(CVT_INT(m_nSoilLyrs[i]), wuse, 0.f);
    // 2. No nested omp for loops
    //float *wuse = new(nothrow) float[CVT_INT(m_nSoilLyrs[i])];
    //for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) wuse[j] = 0.f;
    // 3. Create *wuse for each unit in each simulation loop may cause necessary costs,
    //    so, use a temporary 2D array instead.
    for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) m_wuse[i][j] = 0.f;

    /// water uptake by plants from all layers
    float xx = 0.f;
    int ir = -1;
    int idc = CVT_INT(m_landCoverCls[i]);
    if (idc == CROP_IDC_WARM_SEASON_ANNUAL_LEGUME ||
        idc == CROP_IDC_COLD_SEASON_ANNUAL_LEGUME ||
        idc == CROP_IDC_WARM_SEASON_ANNUAL ||
        idc == CROP_IDC_COLD_SEASON_ANNUAL) {
        m_pltRootD[i] = 2.5f * m_phuAccum[i] * m_soilMaxRootD[i];
        if (m_pltRootD[i] > m_soilMaxRootD[i]) m_pltRootD[i] = m_soilMaxRootD[i];
        if (m_pltRootD[i] < 10.f) m_pltRootD[i] = 10.f; /// minimum root depth is 10mm
    } else {
        m_pltRootD[i] = m_soilMaxRootD[i];
    }
    m_stoSoilRootD[i] = m_pltRootD[i];
    if (m_maxPltET[i] <= 0.01f) {
        m_frStrsWtr[i] = 1.f;
        m_actPltET[i] = 0.f;
        return;
    }
    /// initialize variables
    ir = 0;
    sump = 0.f;
    xx = 0.f;
    // update soil storage profile just in case
    m_soilWtrStoPrfl[i] = 0.f;
    for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
        m_soilWtrStoPrfl[i] += m_soilWtrSto[i][ly];
    }
    /// compute aeration stress
    if (m_soilWtrStoPrfl[i] >= m_soilSumFC[i]) {
        float satco = (m_soilWtrStoPrfl[i] - m_soilSumFC[i]) / (m_soilSumSat[i] - m_soilSumFC[i]);
        float pl_aerfac = 0.85f;
        float scparm = 100.f * (satco - pl_aerfac) / (1.0001f - pl_aerfac);
        if (scparm > 0.f) {
            m_frStrsAe[i] = 1.f - (scparm / (scparm + exp(2.9014f - 0.03867f * scparm)));
        } else {
            m_frStrsAe[i] = 1.f;
        }
    }
    for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
        if (ir > 0) break;
        if (m_pltRootD[i] <= m_soilDepth[i][j]) {
            gx = m_pltRootD[i];
            ir = j;
        } else {
            gx = m_soilDepth[i][j];
        }
        if (m_pltRootD[i] <= 0.01f) {
            sum = m_maxPltET[i] / uobw;
        } else {
            sum = m_maxPltET[i] * (1.f - exp(-ubw * gx / m_pltRootD[i])) / uobw;
        }
        m_wuse[i][j] = sum - sump + 1.f * m_epco[i];
        m_wuse[i][j] = sum - sump + (sump - xx) * m_epco[i];
        sump = sum;
        /// adjust uptake if sw is less than 25% of plant available water
        if (m_soilWtrSto[i][j] < m_soilFC[i][j] * 0.25f) {
            reduc = exp(5.f * (4.f * m_soilWtrSto[i][j] / m_soilFC[i][j] - 1.f));
        } else {
            reduc = 1.f;
        }
        // reduc = 1.f;  /// TODO, Is SWAT wrong here? by LJ
        m_wuse[i][j] *= reduc;
        if (m_soilWtrSto[i][j] < m_wuse[i][j]) {
            m_wuse[i][j] = m_soilWtrSto[i][j];
        }
        m_soilWtrSto[i][j] = Max(UTIL_ZERO, m_soilWtrSto[i][j] - m_wuse[i][j]);
        xx += m_wuse[i][j];
    }
    /// update total soil water in profile
    m_soilWtrStoPrfl[i] = 0.f;
    for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
        m_soilWtrStoPrfl[i] += m_soilWtrSto[i][ly];
    }
    m_frStrsWtr[i] = xx / m_maxPltET[i];
    m_actPltET[i] = xx;
}

void Biomass_EPIC::CalTempStress(const int i) {
    float tgx = 0.f, rto = 0.f;
    tgx = m_meanTemp[i] - m_pgTempBase[i];
    if (tgx <= 0.f) {
        m_frStrsTmp[i] = 0.f;
    } else if (m_meanTemp[i] > m_pgOptTemp[i]) {
        tgx = 2.f * m_pgOptTemp[i] - m_pgTempBase[i] - m_meanTemp[i];
    }
    rto = (m_pgOptTemp[i] - m_pgTempBase[i]) / pow(tgx + UTIL_ZERO, 2.f);
    if (rto <= 200.f && tgx > 0.f) {
        m_frStrsTmp[i] = exp(-0.1054f * rto);
    } else {
        m_frStrsTmp[i] = 0.f;
    }
    if (m_minTemp[i] <= m_annMeanTemp[i] - 15.f) {
        m_frStrsTmp[i] = 0.f;
    }
}

void Biomass_EPIC::AdjustPlantGrowth(const int i) {
    /// Update accumulated heat units for the plant
    float delg = 0.f;
    if (m_phuPlt[i] > 0.1f) {
        delg = (m_meanTemp[i] - m_pgTempBase[i]) / m_phuPlt[i];
    }
    if (delg < 0.f) {
        delg = 0.f;
    }
    m_phuAccum[i] += delg;

    /// If plant hasn't reached maturity
    if (m_phuAccum[i] <= 1.f) {
        ///compute temperature stress - strstmp(j) , tstr.f in SWAT
        CalTempStress(i);
        /// Calculate optimal biomass
        /// 1. calculate photosynthetically active radiation
        float activeRadiation = 0.f;
        activeRadiation = 0.5f * m_SR[i] * (1.f - exp(-m_lightExtCoef[i] * (m_lai[i] + 0.05f)));
        /// 2. Adjust radiation-use efficiency for CO2
        ////  determine shape parameters for the radiation use efficiency equation, readplant.f in SWAT
        if (FloatEqual(m_co2Conc2ndPt[i], 330.0f)) m_co2Conc2ndPt[i] = 660.f;
        float m_RadUseEffiShpCoef1 = 0.f;
        float m_RadUseEffiShpCoef2 = 0.f;
        GetScurveShapeParameter(m_biomEnrgRatio[i] * 0.01f, m_biomEnrgRatio2ndPt[i] * 0.01f, m_co2Conc,
                                m_co2Conc2ndPt[i],
                                &m_RadUseEffiShpCoef1, &m_RadUseEffiShpCoef2);

        float beadj = 0.f;
        if (m_co2Conc > 330.f) {
            beadj = 100.f * m_co2Conc / (m_co2Conc + exp(m_RadUseEffiShpCoef1 - m_co2Conc * m_RadUseEffiShpCoef2));
        } else {
            beadj = m_biomEnrgRatio[i];
        }
        /// 3. adjust radiation-use efficiency for vapor pressure deficit
        ///     assumes vapor pressure threshold of 1.0 kPa
        if (m_vpd[i] > 1.f) {
            beadj -= m_wavp[i] * (m_vpd[i] - 1.f);
            beadj = Max(beadj, 0.27f * m_biomEnrgRatio[i]);
        }
        m_biomassDelta[i] = Max(0.f, beadj * activeRadiation);
        /// 4. Calculate plant uptake of N and P to make sure no plant N and P uptake under temperature, water and aeration stress
        /// m_frStrsWa and m_frStrsAe are derived from DistributePlantET()
        float reg = Min(Min(m_frStrsWtr[i], m_frStrsTmp[i]), m_frStrsAe[i]);
        if (reg < 0.f) reg = 0.f;
        if (reg > 0.) {
            //cout<<"Begin Uptake N and P"<<endl;
            /// call nup to calculates plant nitrogen uptake
            PlantNitrogenUptake(i);
            /// call npup to calculates plant phosphorus uptake
            PlantPhosphorusUptake(i);
        } else {
            m_frStrsN[i] = 1.f;
            m_frStrsP[i] = 1.f;
        }
        /// 5. auto fertilization-nitrogen demand (non-legumes only)
        int idc = CVT_INT(m_landCoverCls[i]);
        //if((idc == 4 || idc == 5 || idc == 6 || idc == 7) && auto_nstrs[i] > 0.)
        /// call anfert
        ///////// TODO: Finish auto fertilization-nitrogen later. by LJ
        /// 6. reduce predicted biomass due to stress on plant
        reg = Min(m_frStrsWtr[i], Min(m_frStrsTmp[i], Min(m_frStrsN[i], m_frStrsP[i])));
        if (reg < 0.f) reg = 0.f;
        if (reg > 1.f) reg = 1.f;
        if (m_biomTrgt[i] > 0.01f) {
            m_biomassDelta[i] *= (m_biomTrgt[i] - m_biomass[i]) / m_biomTrgt[i];
        }
        m_biomass[i] += m_biomassDelta[i] * reg;
        float rto = 1.f;
        if (idc == CROP_IDC_TREES) {
            if (m_matYrs[i] > 0.) {
                float curYrMat = m_curYrMat[i] + m_yearIdx;
                rto = curYrMat / m_matYrs[i];
                m_biomass[i] = Min(m_biomass[i], rto * m_maxBiomTree[i] * 1000.f); /// convert tons/ha -> kg/ha
            } else {
                rto = 1.f;
            }
        }
        m_biomass[i] = Max(m_biomass[i], 0.f);
        //if(i == 5) cout << m_biomass[i] << ", \n";
        /// 7. calculate fraction of total biomass that is in the roots
        float m_rootShootRatio1 = 0.4f;
        float m_rootShootRatio2 = 0.2f;
        m_frRoot[i] = m_rootShootRatio1 * (m_rootShootRatio1 - m_rootShootRatio2) * m_phuAccum[i];
        float LAIShpCoef1 = 0.f;
        float LAIShpCoef2 = 0.f;
        GetScurveShapeParameter(m_frMaxLai1stPt[i], m_frMaxLai2ndPt[i], m_frGrow1stPt[i], m_frGrow2ndPt[i],
                                &LAIShpCoef1, &LAIShpCoef2);
        float f = m_phuAccum[i] / (m_phuAccum[i] + exp(LAIShpCoef1 - LAIShpCoef2 * m_phuAccum[i]));
        float ff = f - m_LaiMaxFr[i];
        m_LaiMaxFr[i] = f;
        /// 8. calculate new canopy height
        if (idc == CROP_IDC_TREES) {
            m_canHgt[i] = rto * m_maxCanHgt[i];
        } else {
            m_canHgt[i] = m_maxCanHgt[i] * sqrt(f);
        }
        /// 9. calculate new leaf area index (LAI)
        float laiMax = 0.f;
        float laiDelta = 0.f;
        if (m_phuAccum[i] <= m_dormPHUFr[i]) {
            laiMax = 0.f;
            laiDelta = 0.f;
            if (idc == CROP_IDC_TREES) {
                laiMax = rto * m_maxLai[i];
            } else {
                laiMax = m_maxLai[i];
            }
            if (m_lai[i] > laiMax) m_lai[i] = laiMax;
            laiDelta = ff * laiMax * (1.f - exp(5.f * (m_lai[i] - laiMax))) * sqrt(reg);
            m_lai[i] += laiDelta;
            if (m_lai[i] > laiMax) m_lai[i] = laiMax;
            m_oLai[i] = m_lai[i];
            if (m_lai[i] > m_maxLaiYr[i]) m_maxLaiYr[i] = m_lai[i];
        } else {
            m_lai[i] = m_oLai[i] * (1.f - m_phuAccum[i]) / (1.f - m_dormPHUFr[i]);
        }
        if (m_lai[i] < m_minLaiDorm[i]) {
            m_lai[i] = m_minLaiDorm[i];
        }
        /// 10. calculate plant ET values
        if (m_phuAccum[i] > 0.5f && m_phuAccum[i] < m_dormPHUFr[i]) {
            m_totActPltET[i] += m_actPltET[i] + m_soilET[i];
            m_totPltPET[i] += m_pet[i];
        }
        m_hvstIdxAdj[i] = m_hvstIdx[i] * 100.f * m_phuAccum[i] / (100.f * m_phuAccum[i] +
            exp(11.1f - 10.f * m_phuAccum[i]));
    } else {
        if (m_dormPHUFr[i] > 1.f) {
            if (m_phuAccum[i] > m_dormPHUFr[i]) {
                m_lai[i] = m_oLai[i] * (1.f - (m_phuAccum[i] - m_dormPHUFr[i]) / (1.2f - m_dormPHUFr[i]));
            }
        }
        if (m_lai[i] < 0.f) m_lai[i] = 0.f;
    }
}

void Biomass_EPIC::PlantNitrogenUptake(const int i) {
    float uobn = GetNormalization(m_upTkDistN);
    float n_reduc = 300.f; /// nitrogen uptake reduction factor (not currently used; defaulted 300.)
    float tno3 = 0.f;
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        tno3 += m_soilNO3[i][l];
    }
    tno3 /= n_reduc;
    // float up_reduc = tno3 / (tno3 + exp(1.56f - 4.5f * tno3)); /// However, up_reduc is not used hereafter.
    /// icrop is land cover code in SWAT.
    /// in SEIMS, it is no need to use it.
    //// determine shape parameters for plant nitrogen uptake equation, from readplant.f
    m_frPltN[i] = NPBiomassFraction(m_biomNFr1[i], m_biomNFr2[i], m_biomNFr3[i], m_phuAccum[i]);
    float un2 = 0.f; /// ideal (or optimal) plant nitrogen content (kg/ha)
    un2 = m_frPltN[i] * m_biomass[i];
    if (un2 < m_pltN[i]) un2 = m_pltN[i];
    m_NO3Defic[i] = un2 - m_pltN[i];
    m_NO3Defic[i] = Min(4.f * m_biomNFr3[i] * m_biomassDelta[i], m_NO3Defic[i]);
    m_frStrsN[i] = 1.f;
    int ir = 0;
    if (m_NO3Defic[i] < UTIL_ZERO) {
        return;
    }
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        if (ir > 0) {
            break;
        }
        float gx = 0.f;
        if (m_pltRootD[i] <= m_soilDepth[i][l]) {
            gx = m_pltRootD[i];
            ir = 1;
        } else {
            gx = m_soilDepth[i][l];
        }
        float unmx = 0.f;
        float uno3l = 0.f; /// plant nitrogen demand (kg/ha)
        unmx = m_NO3Defic[i] * (1.f - exp(-m_upTkDistN * gx / m_pltRootD[i])) / uobn;
        uno3l = Min(unmx - m_plantUpTkN[i], m_soilNO3[i][l]);
        //if (uno3l != uno3l)
        //	cout<<"cellid: "<<i<<", lyr: "<<l<<",m_NO3Defic: "<<m_NO3Defic[i]<<
        //	", UpDis: "<<m_NUpDis<<", gx: "<<gx<<", msoilrd:"<<m_soilRD[i]<<
        //	", uobn:"<<uobn<<", plantUpTkN: "<<m_plantUpTkN[i]<<
        //	", soilNo3: "<<m_soilNO3[i][l]<<", unmx: "<<unmx<<
        //	", uno3l: "<<uno3l<<endl;
        m_plantUpTkN[i] += uno3l;
        m_soilNO3[i][l] -= uno3l;
    }
    if (m_plantUpTkN[i] < 0.f) m_plantUpTkN[i] = 0.f;
    /// If crop is a legume, call nitrogen fixation routine
    int idc = CVT_INT(m_landCoverCls[i]);
    if (idc == CROP_IDC_WARM_SEASON_ANNUAL_LEGUME ||
        idc == CROP_IDC_COLD_SEASON_ANNUAL_LEGUME ||
        idc == CROP_IDC_PERENNIAL_LEGUME) {
        PlantNitrogenFixed(i);
    }
    m_plantUpTkN[i] += m_fixN[i];
    m_pltN[i] += m_plantUpTkN[i];
    //if (m_plantN[i] > 0.f)
    //	cout<<"cell ID: "<<i<<", plantN: "<<m_plantN[i]<<endl;
    /// compute nitrogen stress
    if (idc == CROP_IDC_WARM_SEASON_ANNUAL_LEGUME ||
        idc == CROP_IDC_COLD_SEASON_ANNUAL_LEGUME ||
        idc == CROP_IDC_PERENNIAL_LEGUME) {
        m_frStrsN[i] = 1.f;
    } else {
        CalPlantStressByLimitedNP(m_pltN[i], un2, &m_frStrsN[i]);
        float xx = 0.f;
        if (m_NO3Defic[i] > 1.e-5f) {
            xx = m_plantUpTkN[i] / m_NO3Defic[i];
        } else {
            xx = 1.f;
        }
        m_frStrsN[i] = Max(m_frStrsN[i], xx);
        m_frStrsN[i] = Min(m_frStrsN[i], 1.f);
    }
}

void Biomass_EPIC::PlantNitrogenFixed(const int i) {
    /// compute the difference between supply and demand
    float uno3l = 0.f;
    if (m_NO3Defic[i] > m_plantUpTkN[i]) {
        uno3l = m_NO3Defic[i] - m_plantUpTkN[i];
    } else /// if supply is being met, fixation = 0 and return
    {
        m_fixN[i] = 0.f;
        return;
    }
    /// compute fixation as a function of no3, soil water, and growth stage
    //// 1. compute soil water factor
    float fxw = 0.f;
    fxw = m_soilWtrStoPrfl[i] / (0.85f * m_soilSumFC[i]);
    //// 2. compute no3 factor
    float sumn = 0.f; /// total amount of nitrate stored in soil profile (kg/ha)
    float fxn = 0.f;
    for (int l = 0; l < m_nSoilLyrs[i]; l++) {
        sumn += m_soilNO3[i][l];
    }
    if (sumn > 300.f) fxn = 0.f;
    if (sumn > 100.f && sumn <= 300.f) fxn = 1.5f - 0.0005f * sumn;
    if (sumn <= 100.f) fxn = 1.f;
    //// 3. compute growth stage factor
    float fxg = 0.f;
    if (m_phuAccum[i] > 0.15f && m_phuAccum[i] <= 0.30f) {
        fxg = 6.67f * m_phuAccum[i] - 1.f;
    }
    if (m_phuAccum[i] > 0.30f && m_phuAccum[i] <= 0.55f) {
        fxg = 1.f;
    }
    if (m_phuAccum[i] > 0.55f && m_phuAccum[i] <= 0.75f) {
        fxg = 3.75f - 5.f * m_phuAccum[i];
    }
    float fxr = Min(1.f, Min(fxw, fxn)) * fxg;
    fxr = Max(0.f, fxr);
    if (m_NFixCoef <= 0.f) m_NFixCoef = 0.5f;
    if (m_NFixMax <= 0.f) m_NFixMax = 20.f;
    m_fixN[i] = Min(6.f, fxr * m_NO3Defic[i]);
    m_fixN[i] = m_NFixCoef * m_fixN[i] + (1.f - m_NFixCoef) * uno3l;
    m_fixN[i] = Min(m_fixN[i], uno3l);
    m_fixN[i] = Min(m_NFixMax, m_fixN[i]);
}

void Biomass_EPIC::PlantPhosphorusUptake(const int i) {
    float uobp = GetNormalization(m_upTkDistP);
    //// determine shape parameters for plant phosphorus uptake equation, from readplant.f
    m_frPltP[i] = NPBiomassFraction(m_biomPFr1[i], m_biomPFr2[i], m_biomPFr3[i], m_phuAccum[i]);
    float up2 = 0.f;  /// optimal plant phosphorus content
    float uapd = 0.f; /// plant demand of phosphorus
    float upmx = 0.f; /// maximum amount of phosphorus that can be removed from the soil layer
    float uapl = 0.f; /// amount of phosphorus removed from layer
    float gx = 0.f;   /// lowest depth in layer from which phosphorus may be removed
    up2 = m_frPltP[i] * m_biomass[i];
    if (up2 < m_pltP[i]) up2 = m_pltP[i];
    uapd = up2 - m_pltP[i];
    uapd *= 1.5f; /// luxury p uptake
    m_frStrsP[i] = 1.f;
    int ir = 0;
    if (uapd < UTIL_ZERO) return;
    for (int l = 0; l < m_nSoilLyrs[i]; l++) {
        if (ir > 0) break;
        if (m_pltRootD[i] <= m_soilDepth[i][l]) {
            gx = m_pltRootD[i];
            ir = 1;
        } else {
            gx = m_soilDepth[i][l];
        }
        upmx = uapd * (1.f - exp(-m_upTkDistP * gx / m_pltRootD[i])) / uobp;
        uapl = Min(upmx - m_plantUpTkP[i], m_soilSolP[i][l]);
        m_plantUpTkP[i] += uapl;
        m_soilSolP[i][l] -= uapl;
    }
    if (m_plantUpTkP[i] < 0.f) m_plantUpTkP[i] = 0.f;
    m_pltP[i] += m_plantUpTkP[i];
    /// compute phosphorus stress
    CalPlantStressByLimitedNP(m_pltP[i], up2, &m_frStrsP[i]);
}

void Biomass_EPIC::CheckDormantStatus(const int i) {
    /// TODO
    return;
}

int Biomass_EPIC::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        /// calculate albedo in current day, albedo.f of SWAT
        float cej = -5.e-5f;
        float eaj = exp(cej * (m_rsdCovSoil[i] + 0.1f)); // eq. 1:1.2.16 in SWAT theory 2009
        if (m_snowAccum[i] < 0.5f) {
            m_alb[i] = m_soilAlb[i]; // eq. 1:1.2.14
            if (m_lai[i] > 0.f) {
                // eq. 1:1.2.15, 0.23 is plant albedo set by SWAT
                m_alb[i] = 0.23f * (1.f - eaj) + m_soilAlb[i] * eaj;
            }
        } else {
            m_alb[i] = 0.8f; // eq. 1:1.2.13
        }
        /// reWrite from plantmod.f of SWAT
        /// calculate residue on soil surface for current day
        m_rsdCovSoil[i] = Max(0.8f * m_biomass[i] + m_soilRsd[i][0], 0.f);
        if (FloatEqual(m_igro[i], 1.f)) {
            /// land cover growing
            DistributePlantET(i); /// swu.f
            if (FloatEqual(m_dormFlag[i], 0.f)) {
                /// plant will not undergo stress if dormant
                AdjustPlantGrowth(i); /// grow.f
            }
            CheckDormantStatus(i); /// dormant.f
        }
    }
    return 0;
}

void Biomass_EPIC::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_BIOMASS[0])) *data = m_biomass;
    else if (StringMatch(sk, VAR_LAST_SOILRD[0])) *data = m_stoSoilRootD;
    else if (StringMatch(sk, VAR_PLANT_P[0])) *data = m_pltP;
    else if (StringMatch(sk, VAR_PLANT_N[0])) *data = m_pltN;
    else if (StringMatch(sk, VAR_FR_PLANT_N[0])) *data = m_frPltN;
    else if (StringMatch(sk, VAR_FR_PLANT_P[0])) *data = m_frPltP;
    else if (StringMatch(sk, VAR_AET_PLT[0])) *data = m_actPltET;
    else if (StringMatch(sk, VAR_PLTPET_TOT[0])) *data = m_totPltPET;
    else if (StringMatch(sk, VAR_PLTET_TOT[0])) *data = m_totActPltET;
    else if (StringMatch(sk, VAR_FR_PHU_ACC[0])) *data = m_phuAccum;
    else if (StringMatch(sk, VAR_ROOTDEPTH[0])) *data = m_pltRootD;
    else if (StringMatch(sk, VAR_LAIDAY[0])) *data = m_lai;
    else if (StringMatch(sk, VAR_LAIYRMAX[0])) *data = m_maxLaiYr;
    else if (StringMatch(sk, VAR_LAIMAXFR[0])) *data = m_LaiMaxFr;
    else if (StringMatch(sk, VAR_OLAI[0])) *data = m_oLai;
    else if (StringMatch(sk, VAR_ALBDAY[0])) *data = m_alb;
    else if (StringMatch(sk, VAR_DORMI[0])) *data = m_dormFlag;
    else if (StringMatch(sk, VAR_IGRO[0])) *data = m_igro;
    else if (StringMatch(sk, VAR_HVSTI_ADJ[0])) *data = m_hvstIdxAdj;
    else if (StringMatch(sk, VAR_FR_ROOT[0])) *data = m_frRoot;
    else if (StringMatch(sk, VAR_FR_STRSWTR[0])) *data = m_frStrsWtr;
    else if (StringMatch(sk, VAR_SOL_COV[0])) *data = m_rsdCovSoil;
    else if (StringMatch(sk, VAR_SOL_SW[0])) *data = m_soilWtrStoPrfl;
    else {
        throw ModelException(M_PG_EPIC[0], "Get1DData", "Result " + sk + " does not exist.");
    }
}

void Biomass_EPIC::Get2DData(const char* key, int* nrows, int* ncols, float*** data) {
    InitialOutputs();
    string sk(key);
    *nrows = m_nCells;
    *ncols = m_maxSoilLyrs;
    if (StringMatch(sk, VAR_SOL_RSD[0])) *data = m_soilRsd;
    else {
        throw ModelException(M_PG_EPIC[0], "Get2DData", "Result " + sk + " does not exist.");
    }
}
