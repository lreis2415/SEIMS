#include "seims.h"
#include "Biomass_EPIC.h"

using namespace std;

Biomass_EPIC::Biomass_EPIC() : m_nCells(-1), m_nClimDataYrs(-1), m_co2(NODATA_VALUE), m_tMean(nullptr), m_tMin(nullptr),
                               m_SR(nullptr), m_dayLenMin(nullptr), m_dormHr(nullptr),
                               m_nMaxSoilLayers(-1), m_NUpDis(NODATA_VALUE), m_PUpDis(NODATA_VALUE),
                               m_NFixCoef(NODATA_VALUE),
                               m_NFixMax(NODATA_VALUE), m_soilRD(nullptr), m_tMeanAnn(nullptr),
                               m_nSoilLayers(nullptr), m_soilZMX(nullptr), m_soilALB(nullptr), m_soilDepth(nullptr),
                               m_soilAWC(nullptr), m_totSoilAWC(nullptr), m_totSoilSat(nullptr),
                               m_soilStorage(nullptr), m_soilStorageProfile(nullptr), m_sol_rsdin(nullptr), m_sol_cov(nullptr),
                               m_sol_rsd(nullptr),
                               m_igro(nullptr), m_landCoverCls(nullptr), m_aLAIMin(nullptr), m_BIOE(nullptr), m_BIOEHI(nullptr),
                               m_frBioLeafDrop(nullptr), m_maxLAI(nullptr), m_maxBiomass(nullptr),
                               m_frPlantN1(nullptr), m_frPlantN2(nullptr), m_frPlantN3(nullptr), m_frPlantP1(nullptr),
                               m_frPlantP2(nullptr), m_frPlantP3(nullptr),
                               m_chtMax(nullptr), m_co2Hi(nullptr), m_frDLAI(nullptr), m_epco(nullptr), m_lightExtCoef(nullptr),
                               m_frGrowOptLAI1(nullptr), m_frGrowOptLAI2(nullptr),
                               m_hvstIdx(nullptr), m_frMaxLAI1(nullptr), m_frMaxLAI2(nullptr), m_matYrs(nullptr), m_tBase(nullptr),
                               m_tOpt(nullptr), m_wavp(nullptr),
                               m_cht(nullptr), m_initTreeMatYr(nullptr), m_initBiomass(nullptr), m_initLAI(nullptr),
                               m_PHUPlt(nullptr), m_dormFlag(nullptr), m_pltET(nullptr), m_pltPET(nullptr),
                               m_dayLen(nullptr), m_VPD(nullptr), m_PET(nullptr), m_ppt(nullptr), m_soilESDay(nullptr),
                               m_soilNO3(nullptr), m_soilPsol(nullptr), m_snowAcc(nullptr),
                               m_LAIDay(nullptr), m_frPHUacc(nullptr), m_LAIYrMax(nullptr), m_hvstIdxAdj(nullptr),
                               m_LAIMaxFr(nullptr), m_oLAI(nullptr), m_lastSoilRootDepth(nullptr),
                               m_plantEPDay(nullptr), m_frRoot(nullptr), m_fixN(nullptr),
                               m_plantUpTkN(nullptr), m_plantN(nullptr), m_frPlantN(nullptr),
                               m_plantUpTkP(nullptr), m_plantP(nullptr), m_frPlantP(nullptr),
                               m_NO3Defic(nullptr), m_frStrsAe(nullptr), m_frStrsN(nullptr), m_frStrsP(nullptr),
                               m_frStrsTmp(nullptr), m_frStrsWa(nullptr),
                               m_biomassDelta(nullptr), m_biomass(nullptr), m_albedo(nullptr),
    // parameters not allowed to modify
                               ubw(10.f), uobw(NODATA_VALUE) {
}

Biomass_EPIC::~Biomass_EPIC() {
    if (m_sol_cov != nullptr) Release1DArray(m_sol_cov);
    if (m_sol_rsd != nullptr) Release2DArray(m_nCells, m_sol_rsd);
    if (m_LAIDay != nullptr) Release1DArray(m_LAIDay);
    if (m_LAIYrMax != nullptr) Release1DArray(m_LAIYrMax);
    if (m_frPHUacc != nullptr) Release1DArray(m_frPHUacc);
    if (m_pltET != nullptr) Release1DArray(m_pltET);
    if (m_pltPET != nullptr) Release1DArray(m_pltPET);
    if (m_hvstIdxAdj != nullptr) Release1DArray(m_hvstIdxAdj);
    if (m_LAIMaxFr != nullptr) Release1DArray(m_LAIMaxFr);
    if (m_oLAI != nullptr) Release1DArray(m_oLAI);
    if (m_lastSoilRootDepth != nullptr) Release1DArray(m_lastSoilRootDepth);
    if (m_plantEPDay != nullptr) Release1DArray(m_plantEPDay);
    if (m_frRoot != nullptr) Release1DArray(m_frRoot);
    if (m_fixN != nullptr) Release1DArray(m_fixN);
    if (m_plantUpTkN != nullptr) Release1DArray(m_plantUpTkN);
    if (m_plantUpTkP != nullptr) Release1DArray(m_plantUpTkP);
    if (m_plantN != nullptr) Release1DArray(m_plantN);
    if (m_plantP != nullptr) Release1DArray(m_plantP);
    if (m_frPlantN != nullptr) Release1DArray(m_frPlantN);
    if (m_frPlantP != nullptr) Release1DArray(m_frPlantP);
    if (m_NO3Defic != nullptr) Release1DArray(m_NO3Defic);
    if (m_frStrsAe != nullptr) Release1DArray(m_frStrsAe);
    if (m_frStrsN != nullptr) Release1DArray(m_frStrsN);
    if (m_frStrsP != nullptr) Release1DArray(m_frStrsP);
    if (m_frStrsTmp != nullptr) Release1DArray(m_frStrsTmp);
    if (m_frStrsWa != nullptr) Release1DArray(m_frStrsWa);
    if (m_biomassDelta != nullptr) Release1DArray(m_biomassDelta);
    if (m_biomass != nullptr) Release1DArray(m_biomass);
}

void Biomass_EPIC::SetValue(const char *key, float value) {
    string sk(key);
    if (StringMatch(sk, VAR_CO2)) { m_co2 = value; }
    else if (StringMatch(sk, VAR_OMP_THREADNUM)) { SetOpenMPThread((int) value); }
    else if (StringMatch(sk, VAR_NUPDIS)) { m_NUpDis = value; }
    else if (StringMatch(sk, VAR_PUPDIS)) { m_PUpDis = value; }
    else if (StringMatch(sk, VAR_NFIXCO)) { m_NFixCoef = value; }
    else if (StringMatch(sk, VAR_NFIXMX)) { m_NFixMax = value; }
    else {
        throw ModelException(MID_BIO_EPIC, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

bool Biomass_EPIC::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_BIO_EPIC, "CheckInputSize", "Input data for " + string(key) +
            " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException(MID_BIO_EPIC, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input raster data should have same size.");
        }
    }
    return true;
}

void Biomass_EPIC::Set1DData(const char *key, int n, float *data) {
    string sk(key);
    CheckInputSize(key, n);
    //// climate
    if (StringMatch(sk, DataType_MeanTemperature)) { m_tMean = data; }
    else if (StringMatch(sk, DataType_MinimumTemperature)) { m_tMin = data; }
    else if (StringMatch(sk, DataType_SolarRadiation)) { m_SR = data; }
    else if (StringMatch(sk, VAR_DAYLEN_MIN)) { m_dayLenMin = data; }
    else if (StringMatch(sk, VAR_TMEAN_ANN)) { m_tMeanAnn = data; }
    else if (StringMatch(sk, VAR_DORMHR)) { m_dormHr = data; }
    else if (StringMatch(sk, VAR_DAYLEN)) { m_dayLen = data; } //// soil properties and water related
    else if (StringMatch(sk, VAR_SOILLAYERS)) { m_nSoilLayers = data; }
    else if (StringMatch(sk, VAR_SOL_ZMX)) { m_soilZMX = data; }
    else if (StringMatch(sk, VAR_SOL_ALB)) { m_soilALB = data; }
    else if (StringMatch(sk, VAR_SOL_SW)) { m_soilStorageProfile = data; }
    else if (StringMatch(sk, VAR_SOL_SUMAWC)) { m_totSoilAWC = data; }
    else if (StringMatch(sk, VAR_SOL_SUMSAT)) { m_totSoilSat = data; }
    else if (StringMatch(sk, VAR_PET)) { m_PET = data; }
    else if (StringMatch(sk, VAR_VPD)) { m_VPD = data; }
    else if (StringMatch(sk, VAR_PPT)) { m_ppt = data; }
    else if (StringMatch(sk, VAR_SOET)) { m_soilESDay = data; }
    else if (StringMatch(sk, VAR_SOL_COV)) { m_sol_cov = data; }
    else if (StringMatch(sk, VAR_SNAC)) { m_snowAcc = data; } 
    else if (StringMatch(sk, VAR_SOL_RSDIN)) { m_sol_rsdin = data; }
    else if (StringMatch(sk, VAR_IGRO)) { m_igro = data; }
    else if (StringMatch(sk, VAR_IDC)) { m_landCoverCls = data; }
    else if (StringMatch(sk, VAR_ALAIMIN)) { m_aLAIMin = data; }
    else if (StringMatch(sk, VAR_BIO_E)) { m_BIOE = data; }
    else if (StringMatch(sk, VAR_BIOEHI)) { m_BIOEHI = data; }
    else if (StringMatch(sk, VAR_BIOLEAF)) { m_frBioLeafDrop = data; }
    else if (StringMatch(sk, VAR_BLAI)) { m_maxLAI = data; }
    else if (StringMatch(sk, VAR_BMX_TREES)) { m_maxBiomass = data; }
    else if (StringMatch(sk, VAR_BN1)) { m_frPlantN1 = data; }
    else if (StringMatch(sk, VAR_BN2)) { m_frPlantN2 = data; }
    else if (StringMatch(sk, VAR_BN3)) { m_frPlantN3 = data; }
    else if (StringMatch(sk, VAR_BP1)) { m_frPlantP1 = data; }
    else if (StringMatch(sk, VAR_BP2)) { m_frPlantP2 = data; }
    else if (StringMatch(sk, VAR_BP3)) { m_frPlantP3 = data; }
    else if (StringMatch(sk, VAR_CHTMX)) { m_chtMax = data; }
    else if (StringMatch(sk, VAR_CO2HI)) { m_co2Hi = data; }
    else if (StringMatch(sk, VAR_DLAI)) { m_frDLAI = data; }
    else if (StringMatch(sk, VAR_EXT_COEF)) { m_lightExtCoef = data; }
    else if (StringMatch(sk, VAR_FRGRW1)) { m_frGrowOptLAI1 = data; }
    else if (StringMatch(sk, VAR_FRGRW2)) { m_frGrowOptLAI2 = data; }
    else if (StringMatch(sk, VAR_HVSTI)) { m_hvstIdx = data; }
    else if (StringMatch(sk, VAR_LAIMX1)) { m_frMaxLAI1 = data; }
    else if (StringMatch(sk, VAR_LAIMX2)) { m_frMaxLAI2 = data; }
    else if (StringMatch(sk, VAR_MAT_YRS)) { m_matYrs = data; }
    else if (StringMatch(sk, VAR_T_BASE)) { m_tBase = data; }
    else if (StringMatch(sk, VAR_T_OPT)) { m_tOpt = data; }
    else if (StringMatch(sk, VAR_WAVP)) { m_wavp = data; } 
    else if (StringMatch(sk, VAR_EPCO)) { m_epco = data; }
    else if (StringMatch(sk, VAR_TREEYRS)) { m_initTreeMatYr = data; }
    else if (StringMatch(sk, VAR_LAIINIT)) { m_initLAI = data; }
    else if (StringMatch(sk, VAR_BIOINIT)) { m_initBiomass = data; }
    else if (StringMatch(sk, VAR_PHUPLT)) { m_PHUPlt = data; }
    else if (StringMatch(sk, VAR_CHT)) { m_cht = data; }
    else if (StringMatch(sk, VAR_DORMI)) { m_dormFlag = data; }
    else {
        throw ModelException(MID_BIO_EPIC, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

bool Biomass_EPIC::CheckInputSize2D(const char *key, int n, int col) {
    CheckInputSize(key, n);
    if (col <= 0) {
        throw ModelException(MID_BIO_EPIC, "CheckInputSize2D", "Input data for " + string(key) +
            " is invalid. The layer number could not be less than zero.");
    }
    if (m_nMaxSoilLayers != col) {
        if (m_nMaxSoilLayers <= 0) {
            m_nMaxSoilLayers = col;
        } else {
            throw ModelException(MID_BIO_EPIC, "CheckInputSize2D", "Input data for " + string(key) +
                " is invalid. All the layers of input 2D raster data should have same size.");
        }
    }
    return true;
}

void Biomass_EPIC::Set2DData(const char *key, int nRows, int nCols, float **data) {
    string sk(key);
    CheckInputSize2D(key, nRows, nCols);
    if (StringMatch(sk, VAR_SOILDEPTH)) { m_soilDepth = data; }
    else if (StringMatch(sk, VAR_SOILTHICK)) { m_soilThick = data; }
    else if (StringMatch(sk, VAR_SOL_RSD)) { m_sol_rsd = data; }
    else if (StringMatch(sk, VAR_SOL_AWC)) { m_soilAWC = data; }
    else if (StringMatch(sk, VAR_SOL_ST)) { m_soilStorage = data; }
    else if (StringMatch(sk, VAR_SOL_NO3)) { m_soilNO3 = data; }
    else if (StringMatch(sk, VAR_SOL_SOLP)) { m_soilPsol = data; }
    else {
        throw ModelException(MID_BIO_EPIC, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

bool Biomass_EPIC::CheckInputData(void) {
    /// DT_Single
    CHECK_POSITIVE(MID_BIO_EPIC, m_nCells);
    CHECK_POSITIVE(MID_BIO_EPIC, m_nMaxSoilLayers);
    CHECK_NODATA(MID_BIO_EPIC, m_co2);
    CHECK_NODATA(MID_BIO_EPIC, m_NUpDis);
    CHECK_NODATA(MID_BIO_EPIC, m_PUpDis);
    CHECK_NODATA(MID_BIO_EPIC, m_NFixCoef);
    CHECK_NODATA(MID_BIO_EPIC, m_NFixMax);
    /// DT_Raster1D
    CHECK_POINTER(MID_BIO_EPIC, m_tMeanAnn);
    CHECK_POINTER(MID_BIO_EPIC, m_tMin);
    CHECK_POINTER(MID_BIO_EPIC, m_tMean);
    CHECK_POINTER(MID_BIO_EPIC, m_SR);
    CHECK_POINTER(MID_BIO_EPIC, m_dayLen);
    CHECK_POINTER(MID_BIO_EPIC, m_dayLenMin);
    CHECK_POINTER(MID_BIO_EPIC, m_dormHr);
    CHECK_POINTER(MID_BIO_EPIC, m_nSoilLayers);
    CHECK_POINTER(MID_BIO_EPIC, m_soilZMX);
    CHECK_POINTER(MID_BIO_EPIC, m_soilALB);
    CHECK_POINTER(MID_BIO_EPIC, m_soilStorageProfile);
    CHECK_POINTER(MID_BIO_EPIC, m_totSoilAWC);
    CHECK_POINTER(MID_BIO_EPIC, m_totSoilSat);
    CHECK_POINTER(MID_BIO_EPIC, m_PET);
    CHECK_POINTER(MID_BIO_EPIC, m_VPD);
    CHECK_POINTER(MID_BIO_EPIC, m_ppt);
    CHECK_POINTER(MID_BIO_EPIC, m_soilESDay);
    CHECK_POINTER(MID_BIO_EPIC, m_sol_rsdin);
    CHECK_POINTER(MID_BIO_EPIC, m_igro);
    CHECK_POINTER(MID_BIO_EPIC, m_landCoverCls);
    CHECK_POINTER(MID_BIO_EPIC, m_aLAIMin);
    CHECK_POINTER(MID_BIO_EPIC, m_BIOE);
    CHECK_POINTER(MID_BIO_EPIC, m_BIOEHI);
    CHECK_POINTER(MID_BIO_EPIC, m_frBioLeafDrop);
    CHECK_POINTER(MID_BIO_EPIC, m_maxLAI);
    CHECK_POINTER(MID_BIO_EPIC, m_maxBiomass);
    CHECK_POINTER(MID_BIO_EPIC, m_frPlantN1);
    CHECK_POINTER(MID_BIO_EPIC, m_frPlantN2);
    CHECK_POINTER(MID_BIO_EPIC, m_frPlantN3);
    CHECK_POINTER(MID_BIO_EPIC, m_frPlantP1);
    CHECK_POINTER(MID_BIO_EPIC, m_frPlantP2);
    CHECK_POINTER(MID_BIO_EPIC, m_frPlantP3);
    CHECK_POINTER(MID_BIO_EPIC, m_chtMax);
    CHECK_POINTER(MID_BIO_EPIC, m_co2Hi);
    CHECK_POINTER(MID_BIO_EPIC, m_frDLAI);
    CHECK_POINTER(MID_BIO_EPIC, m_lightExtCoef);
    CHECK_POINTER(MID_BIO_EPIC, m_frGrowOptLAI1);
    CHECK_POINTER(MID_BIO_EPIC, m_frGrowOptLAI2);
    CHECK_POINTER(MID_BIO_EPIC, m_hvstIdx);
    CHECK_POINTER(MID_BIO_EPIC, m_frMaxLAI1);
    CHECK_POINTER(MID_BIO_EPIC, m_frMaxLAI2);
    CHECK_POINTER(MID_BIO_EPIC, m_matYrs);
    CHECK_POINTER(MID_BIO_EPIC, m_tBase);
    CHECK_POINTER(MID_BIO_EPIC, m_tOpt);
    CHECK_POINTER(MID_BIO_EPIC, m_wavp);
    CHECK_POINTER(MID_BIO_EPIC, m_epco);
    CHECK_POINTER(MID_BIO_EPIC, m_initTreeMatYr);
    CHECK_POINTER(MID_BIO_EPIC, m_initLAI);
    CHECK_POINTER(MID_BIO_EPIC, m_initBiomass);
    CHECK_POINTER(MID_BIO_EPIC, m_PHUPlt);
    CHECK_POINTER(MID_BIO_EPIC, m_cht);
    CHECK_POINTER(MID_BIO_EPIC, m_dormFlag);
    /// DT_Raster2D
    CHECK_POINTER(MID_BIO_EPIC, m_soilDepth);
    CHECK_POINTER(MID_BIO_EPIC, m_soilThick);
    CHECK_POINTER(MID_BIO_EPIC, m_soilAWC);
    CHECK_POINTER(MID_BIO_EPIC, m_soilStorage);
    CHECK_POINTER(MID_BIO_EPIC, m_soilNO3);
    CHECK_POINTER(MID_BIO_EPIC, m_soilPsol);
    return true;
}

void Biomass_EPIC::initialOutputs() {
    if (FloatEqual(uobw, NODATA_VALUE)) {
        ubw = 10.f; /// the uptake distribution for water is hardwired, users are not allowed to modify
        uobw = 0.f;
        uobw = PGCommon::getNormalization(ubw);
    }
    if (m_albedo == nullptr) Initialize1DArray(m_nCells, m_albedo, 0.f);
    if (m_sol_cov == nullptr || m_sol_rsd == nullptr) {
        Initialize1DArray(m_nCells, m_sol_cov, m_sol_rsdin);
        Initialize2DArray(m_nCells, m_nMaxSoilLayers, m_sol_rsd, 0.f);
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            m_sol_rsd[i][0] = m_sol_cov[i];
        }
    }
    if (m_LAIDay == nullptr) {
        if (m_initLAI != nullptr) {
            Initialize1DArray(m_nCells, m_LAIDay, m_initLAI);
        } else {
            Initialize1DArray(m_nCells, m_LAIDay, 0.f);
        }
    }
    if (m_LAIYrMax == nullptr) {
        m_LAIYrMax = new float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            if (PGCommon::IsTree((int) (m_landCoverCls[i]))) {
                m_LAIYrMax[i] *= m_initTreeMatYr[i] / m_matYrs[i];
            } else {
                m_LAIYrMax[i] = m_LAIDay[i];
            }
        }
    }
    if (m_frPHUacc == nullptr) {
        Initialize1DArray(m_nCells, m_frPHUacc, 0.f);
    }
    if (m_pltET == nullptr) {
        Initialize1DArray(m_nCells, m_pltET, 0.f);
    }
    if (m_pltPET == nullptr) {
        Initialize1DArray(m_nCells, m_pltPET, 0.f);
    }
    if (m_hvstIdxAdj == nullptr) {
        Initialize1DArray(m_nCells, m_hvstIdxAdj, 0.f);
    }
    if (m_LAIMaxFr == nullptr) {
        Initialize1DArray(m_nCells, m_LAIMaxFr, 0.f);
    }
    if (m_oLAI == nullptr) {
        Initialize1DArray(m_nCells, m_oLAI, 0.f);
    }
    if (m_lastSoilRootDepth == nullptr) {
        Initialize1DArray(m_nCells, m_lastSoilRootDepth, 10.f);
    }
    if (m_soilRD == nullptr) {
        Initialize1DArray(m_nCells, m_soilRD, 10.f);
    }
    if (m_plantEPDay == nullptr) {
        Initialize1DArray(m_nCells, m_plantEPDay, 0.f);
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
    if (m_plantN == nullptr) {
        Initialize1DArray(m_nCells, m_plantN, 0.f);
    }
    if (m_plantP == nullptr) {
        Initialize1DArray(m_nCells, m_plantP, 0.f);
    }
    if (m_frPlantN == nullptr) {
        Initialize1DArray(m_nCells, m_frPlantN, 0.f);
    }
    if (m_frPlantP == nullptr) {
        Initialize1DArray(m_nCells, m_frPlantP, 0.f);
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
    if (m_frStrsWa == nullptr) {
        Initialize1DArray(m_nCells, m_frStrsWa, 1.f);
    }
    if (m_biomassDelta == nullptr) {
        Initialize1DArray(m_nCells, m_biomassDelta, 0.f);
    }
    if (m_biomass == nullptr) {
        if (m_initBiomass != nullptr) {
            Initialize1DArray(m_nCells, m_biomass, m_initBiomass);
        } else {
            Initialize1DArray(m_nCells, m_biomass, 0.f);
        }
    }
}

void Biomass_EPIC::DistributePlantET(int i) {    /// swu.f of SWAT
    float sum = 0.f, sump = 0.f, gx = 0.f;
    /// fraction of water uptake by plants achieved
    /// where the reduction is caused by low water content
    float reduc = 0.f;
    /// water uptake by plants in each soil layer
    /*
     * Initialize1DArray should not be used inside a OMP codeblock
     * In VS this would be fine, but in linux, may be problematic.*/
    float *wuse(nullptr);
    Initialize1DArray((int) m_nSoilLayers[i], wuse, 0.f);

    //float *wuse = new float[(int) m_nSoilLayers[i]];
    //for (int j = 0; j < (int) m_nSoilLayers[i]; j++)
    //	wuse[j] = 0.f;
    /// water uptake by plants from all layers
    float xx = 0.f;
    int ir = -1;
    int idc = int(m_landCoverCls[i]);
    if (idc == 1 || idc == 2 || idc == 4 || idc == 5) {
        m_soilRD[i] = 2.5f * m_frPHUacc[i] * m_soilZMX[i];
        if (m_soilRD[i] > m_soilZMX[i]) m_soilRD[i] = m_soilZMX[i];
        if (m_soilRD[i] < 10.f) m_soilRD[i] = 10.f;   /// minimum root depth is 10mm
    } else {
        m_soilRD[i] = m_soilZMX[i];
    }
    m_lastSoilRootDepth[i] = m_soilRD[i];
    if (m_ppt[i] <= 0.01f) {
        m_frStrsWa[i] = 1.f;
    } else {
        /// initialize variables
        gx = 0.f;
        ir = 0;
        sump = 0.f;
        xx = 0.f;
        // update soil storage profile just in case
        m_soilStorageProfile[i] = 0.f;
        for (int ly = 0; ly < (int) m_nSoilLayers[i]; ly++) {
            m_soilStorageProfile[i] += m_soilStorage[i][ly];
        }
        /// compute aeration stress
        if (m_soilStorageProfile[i] >= m_totSoilAWC[i]) // mm
        {
            float satco = (m_soilStorageProfile[i] - m_totSoilAWC[i]) / (m_totSoilSat[i] - m_totSoilAWC[i]);
            float pl_aerfac = 0.85f;
            float scparm = 100.f * (satco - pl_aerfac) / (1.0001f - pl_aerfac);
            if (scparm > 0.f) {
                m_frStrsAe[i] = 1.f - (scparm / (scparm + exp(2.9014f - 0.03867f * scparm)));
            } else {
                m_frStrsAe[i] = 1.f;
            }
        }
        for (int j = 0; j < (int) m_nSoilLayers[i]; j++) {
            if (ir > 0) break;
            if (m_soilRD[i] <= m_soilDepth[i][j]) {
                gx = m_soilRD[i];
                ir = j;
            } else {
                gx = m_soilDepth[i][j];
            }
            sum = 0.f;
            if (m_soilRD[i] <= 0.01f) {
                sum = m_ppt[i] / uobw;
            } else {
                sum = m_ppt[i] * (1.f - exp(-ubw * gx / m_soilRD[i])) / uobw;
            }
            wuse[j] = sum - sump + 1.f * m_epco[i];
            wuse[j] = sum - sump + (sump - xx) * m_epco[i];
            sump = sum;
            /// adjust uptake if sw is less than 25% of plant available water
            reduc = 0.f;
            if (m_soilStorage[i][j] < m_soilAWC[i][j] / 4.f) {
                reduc = exp(5.f * (4.f * m_soilStorage[i][j] / m_soilAWC[i][j] - 1.f));
            } else {
                reduc = 1.f;
            }
            // reduc = 1.f;  /// TODO, Is SWAT wrong here? by LJ
            wuse[j] *= reduc;
            if (m_soilStorage[i][j] < wuse[j]) {
                wuse[j] = m_soilStorage[i][j];
            }
            m_soilStorage[i][j] = max(UTIL_ZERO, m_soilStorage[i][j] - wuse[j]);
            xx += wuse[j];
        }
        /// update total soil water in profile
        m_soilStorageProfile[i] = 0.f;
        for (int ly = 0; ly < (int) m_nSoilLayers[i]; ly++) {
            m_soilStorageProfile[i] += m_soilStorage[i][ly];
        }
        m_frStrsWa[i] = xx / m_ppt[i];
        m_plantEPDay[i] = xx;
    }
    Release1DArray(wuse);
}

void Biomass_EPIC::CalTempStress(int i) {
    float tgx = 0.f, rto = 0.f;
    tgx = m_tMean[i] - m_tBase[i];
    if (tgx <= 0.f) {
        m_frStrsTmp[i] = 0.f;
    } else if (m_tMean[i] > m_tOpt[i]) {
        tgx = 2.f * m_tOpt[i] - m_tBase[i] - m_tMean[i];
    }
    rto = (m_tOpt[i] - m_tBase[i]) / pow((tgx + UTIL_ZERO), 2.f);
    if (rto <= 200.f && tgx > 0.f) {
        m_frStrsTmp[i] = exp(-0.1054f * rto);
    } else {
        m_frStrsTmp[i] = 0.f;
    }
    if (m_tMin[i] <= m_tMeanAnn[i] - 15.f) {
        m_frStrsTmp[i] = 0.f;
    }
}

void Biomass_EPIC::AdjustPlantGrowth(int i) {
    /// Update accumulated heat units for the plant
    float delg = 0.f;
    if (m_PHUPlt[i] > 0.1) {
        delg = (m_tMean[i] - m_tBase[i]) / m_PHUPlt[i];
    }
    if (delg < 0.f) {
        delg = 0.f;
    }
    m_frPHUacc[i] += delg;
    //if(i == 5) cout << m_biomassDelta[i] << ", \n";
    //if(i == 0) cout << m_frPHUacc[i] << ", \n";
    /// If plant hasn't reached maturity
    if (m_frPHUacc[i] <= 1.f) {
        ///compute temperature stress - strstmp(j) , tstr.f in SWAT
        CalTempStress(i);
        /// Calculate optimal biomass
        /// 1. calculate photosynthetically active radiation
        float activeRadiation = 0.f;
        activeRadiation = 0.5f * m_SR[i] * (1.f - exp(-m_lightExtCoef[i] * (m_LAIDay[i] + 0.05f)));
        /// 2. Adjust radiation-use efficiency for CO2
        ////  determine shape parameters for the radiation use efficiency equation, readplant.f in SWAT
        if (FloatEqual(m_co2Hi[i], 330.0f)) m_co2Hi[i] = 660.f;
        float m_RadUseEffiShpCoef1 = 0.f;
        float m_RadUseEffiShpCoef2 = 0.f;
        PGCommon::getScurveShapeParameter(m_BIOE[i] * 0.01f, m_BIOEHI[i] * 0.01f, m_co2, m_co2Hi[i],
                                          &m_RadUseEffiShpCoef1, &m_RadUseEffiShpCoef2);

        float beadj = 0.f;
        if (m_co2 > 330.f) {
            beadj = 100.f * m_co2 / (m_co2 + exp(m_RadUseEffiShpCoef1 - m_co2 * m_RadUseEffiShpCoef2));
        } else {
            beadj = m_BIOE[i];
        }
        /// 3. adjust radiation-use efficiency for vapor pressure deficit
        ///     assumes vapor pressure threshold of 1.0 kPa
        float ruedecl = 0.f;
        if (m_VPD[i] > 1.0) {
            ruedecl = m_VPD[i] - 1.f;
            beadj -= m_wavp[i] * ruedecl;
            beadj = max(beadj, 0.27f * m_BIOE[i]);
        }
        m_biomassDelta[i] = max(0.f, beadj * activeRadiation);
        /// 4. Calculate plant uptake of N and P to make sure no plant N and P uptake under temperature, water and aeration stress   
        /// m_frStrsWa and m_frStrsAe are derived from DistributePlantET()
        //if (i == 2000 && (m_frStrsWa[i] > 0.f || m_frStrsTmp[i] > 0.f || m_frStrsAe[i] > 0.f))
        //	cout<<"water stress frac: "<<m_frStrsWa[i]<<", tmp: "<<m_frStrsTmp[i]<<", Ae: "<<m_frStrsAe[i]<<endl;
        float reg = min(min(m_frStrsWa[i], m_frStrsTmp[i]), m_frStrsAe[i]);
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
        int idc = int(m_landCoverCls[i]);
        //if((idc == 4 || idc == 5 || idc == 6 || idc == 7) && auto_nstrs[i] > 0.)
        /// call anfert
        ///////// TODO: Finish auto fertilization-nitrogen later. by LJ
        /// 6. reduce predicted biomass due to stress on plant
        reg = min(m_frStrsWa[i], min(m_frStrsTmp[i], min(m_frStrsN[i], m_frStrsP[i])));
        if (reg < 0.f) reg = 0.f;
        if (reg > 1.f) reg = 1.f;
        //// TODO bio_targ in SWAT is not incorporated in SEIMS.
        m_biomass[i] += m_biomassDelta[i] * reg;
        float rto = 1.f;
        if (idc == CROP_IDC_TREES) {
            if (m_matYrs[i] > 0.) {
                float curYrMat = m_initTreeMatYr[i] + m_yearIdx;
                rto = curYrMat / m_matYrs[i];
                m_biomass[i] = min(m_biomass[i], rto * m_maxBiomass[i] * 1000.f);  /// convert tons/ha -> kg/ha
            } else {
                rto = 1.f;
            }
        }
        m_biomass[i] = max(m_biomass[i], 0.f);
        //if(i == 5) cout << m_biomass[i] << ", \n";
        /// 7. calculate fraction of total biomass that is in the roots
        float m_rootShootRatio1 = 0.4f;
        float m_rootShootRatio2 = 0.2f;
        m_frRoot[i] = m_rootShootRatio1 * (m_rootShootRatio1 - m_rootShootRatio2) * m_frPHUacc[i];
        float LAIShpCoef1 = 0.f, LAIShpCoef2 = 0.f;
        PGCommon::getScurveShapeParameter(m_frMaxLAI1[i], m_frMaxLAI2[i], m_frGrowOptLAI1[i], m_frGrowOptLAI2[i],
                                          &LAIShpCoef1, &LAIShpCoef2);
        float f = m_frPHUacc[i] / (m_frPHUacc[i] + exp(LAIShpCoef1 - LAIShpCoef2 * m_frPHUacc[i]));
        float ff = f - m_LAIMaxFr[i];
        m_LAIMaxFr[i] = f;
        /// 8. calculate new canopy height
        if (idc == CROP_IDC_TREES) {
            m_cht[i] = rto * m_chtMax[i];
        } else {
            m_cht[i] = m_chtMax[i] * sqrt(f);
        }
        /// 9. calculate new leaf area index (LAI)
        float laiMax = 0.f;
        float laiDelta = 0.f;
        if (m_frPHUacc[i] <= m_frDLAI[i]) {
            laiMax = 0.f;
            laiDelta = 0.f;
            if (idc == CROP_IDC_TREES) {
                laiMax = rto * m_maxLAI[i];
            } else {
                laiMax = m_maxLAI[i];
            }
            if (m_LAIDay[i] > laiMax) m_LAIDay[i] = laiMax;
            laiDelta = ff * laiMax * (1.f - exp(5.f * (m_LAIDay[i] - laiMax))) * sqrt(reg);
            m_LAIDay[i] += laiDelta;
            if (m_LAIDay[i] > laiMax) m_LAIDay[i] = laiMax;
            m_oLAI[i] = m_LAIDay[i];
            if (m_LAIDay[i] > m_LAIYrMax[i]) m_LAIYrMax[i] = m_LAIDay[i];
        } else {
            m_LAIDay[i] = m_oLAI[i] * (1.f - m_frPHUacc[i]) / (1.f - m_frDLAI[i]);
        }
        if (m_LAIDay[i] < m_aLAIMin[i]) {
            m_LAIDay[i] = m_aLAIMin[i];
        }
        /// 10. calculate plant ET values
        if (m_frPHUacc[i] > 0.5f && m_frPHUacc[i] < m_frDLAI[i]) {
            m_pltET[i] += (m_plantEPDay[i] + m_soilESDay[i]);
            m_pltPET[i] += m_PET[i];
        }
        m_hvstIdxAdj[i] = m_hvstIdx[i] * 100.f * m_frPHUacc[i] / (100.f * m_frPHUacc[i] +
            exp(11.1f - 10.f * m_frPHUacc[i]));
    } else {
        if (m_frDLAI[i] > 1.f) {
            if (m_frPHUacc[i] > m_frDLAI[i]) {
                m_LAIDay[i] = m_oLAI[i] * (1.f - (m_frPHUacc[i] - m_frDLAI[i]) / (1.2f - m_frDLAI[i]));
            }
        }
        if (m_LAIDay[i] < 0.f) m_LAIDay[i] = 0.f;
    }
}

void Biomass_EPIC::PlantNitrogenUptake(int i) {
    float uobn = PGCommon::getNormalization(m_NUpDis);
    float n_reduc = 300.f; /// nitrogen uptake reduction factor (not currently used; defaulted 300.)
    float tno3 = 0.f;
    for (int l = 0; l < (int) m_nSoilLayers[i]; l++) {
        tno3 += m_soilNO3[i][l];
    }
    tno3 /= n_reduc;
    // float up_reduc = tno3 / (tno3 + exp(1.56f - 4.5f * tno3)); /// However, up_reduc is not used hereafter.
    /// icrop is land cover code in SWAT.
    /// in SEIMS, it is no need to use it.
    //// determine shape parameters for plant nitrogen uptake equation, from readplant.f
    m_frPlantN[i] = PGCommon::NPBiomassFraction(m_frPlantN1[i], m_frPlantN2[i], m_frPlantN3[i], m_frPHUacc[i]);
    float un2 = 0.f; /// ideal (or optimal) plant nitrogen content (kg/ha)
    un2 = m_frPlantN[i] * m_biomass[i];
    if (un2 < m_plantN[i]) un2 = m_plantN[i];
    m_NO3Defic[i] = un2 - m_plantN[i];
    m_NO3Defic[i] = min(4.f * m_frPlantN3[i] * m_biomassDelta[i], m_NO3Defic[i]);
    m_frStrsN[i] = 1.f;
    int ir = 0;
    if (m_NO3Defic[i] < UTIL_ZERO) {
        return;
    }
    for (int l = 0; l < (int) m_nSoilLayers[i]; l++) {
        if (ir > 0) {
            break;
        }
        float gx = 0.f;
        if (m_soilRD[i] <= m_soilDepth[i][l]) {
            gx = m_soilRD[i];
            ir = 1;
        } else {
            gx = m_soilDepth[i][l];
        }
        float unmx = 0.f;
        float uno3l = 0.f; /// plant nitrogen demand (kg/ha)
        unmx = m_NO3Defic[i] * (1.f - exp(-m_NUpDis * gx / m_soilRD[i])) / uobn;
        uno3l = min(unmx - m_plantUpTkN[i], m_soilNO3[i][l]);
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
    if (FloatEqual(m_landCoverCls[i], 1.f) || FloatEqual(m_landCoverCls[i], 2.f) ||
        FloatEqual(m_landCoverCls[i], 3.f)) {
            PlantNitrogenFixed(i);
    }
    m_plantUpTkN[i] += m_fixN[i];
    m_plantN[i] += m_plantUpTkN[i];
    //if (m_plantN[i] > 0.f)
    //	cout<<"cell ID: "<<i<<", plantN: "<<m_plantN[i]<<endl;
    /// compute nitrogen stress
    if (FloatEqual(m_landCoverCls[i], 1.f) || FloatEqual(m_landCoverCls[i], 2.f) ||
        FloatEqual(m_landCoverCls[i], 3.f)) {
            m_frStrsN[i] = 1.f;
    } else {
        PGCommon::calPlantStressByLimitedNP(m_plantN[i], un2, &m_frStrsN[i]);
        float xx = 0.f;
        if (m_NO3Defic[i] > 1.e-5f) {
            xx = m_plantUpTkN[i] / m_NO3Defic[i];
        } else {
            xx = 1.f;
        }
        m_frStrsN[i] = max(m_frStrsN[i], xx);
        m_frStrsN[i] = min(m_frStrsN[i], 1.f);
    }
}

void Biomass_EPIC::PlantNitrogenFixed(int i) {
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
    fxw = m_soilStorageProfile[i] / (0.85f * m_totSoilAWC[i]);
    //// 2. compute no3 factor
    float sumn = 0.f; /// total amount of nitrate stored in soil profile (kg/ha)
    float fxn = 0.f;
    for (int l = 0; l < m_nSoilLayers[i]; l++) {
        sumn += m_soilNO3[i][l];
    }
    if (sumn > 300.f) fxn = 0.f;
    if (sumn > 100.f && sumn <= 300.f) fxn = 1.5f - 0.0005f * sumn;
    if (sumn <= 100.f) fxn = 1.f;
    //// 3. compute growth stage factor
    float fxg = 0.f;
    if (m_frPHUacc[i] > 0.15f && m_frPHUacc[i] <= 0.30f) {
        fxg = 6.67f * m_frPHUacc[i] - 1.f;
    }
    if (m_frPHUacc[i] > 0.30f && m_frPHUacc[i] <= 0.55f) {
        fxg = 1.f;
    }
    if (m_frPHUacc[i] > 0.55f && m_frPHUacc[i] <= 0.75f) {
        fxg = 3.75f - 5.f * m_frPHUacc[i];
    }
    float fxr = min(1.f, min(fxw, fxn)) * fxg;
    fxr = max(0.f, fxr);
    if (m_NFixCoef <= 0.f) m_NFixCoef = 0.5f;
    if (m_NFixMax <= 0.f) m_NFixMax = 20.f;
    m_fixN[i] = min(6.f, fxr * m_NO3Defic[i]);
    m_fixN[i] = m_NFixCoef * m_fixN[i] + (1.f - m_NFixCoef) * uno3l;
    m_fixN[i] = min(m_fixN[i], uno3l);
    m_fixN[i] = min(m_NFixMax, m_fixN[i]);
}

void Biomass_EPIC::PlantPhosphorusUptake(int i) {
    float uobp = PGCommon::getNormalization(m_PUpDis);
    //// determine shape parameters for plant phosphorus uptake equation, from readplant.f
    m_frPlantP[i] = PGCommon::NPBiomassFraction(m_frPlantP1[i], m_frPlantP2[i], m_frPlantP3[i], m_frPHUacc[i]);
    float up2 = 0.f; /// optimal plant phosphorus content
    float uapd = 0.f; /// plant demand of phosphorus
    float upmx = 0.f; /// maximum amount of phosphorus that can be removed from the soil layer
    float uapl = 0.f; /// amount of phosphorus removed from layer
    float gx = 0.f; /// lowest depth in layer from which phosphorus may be removed
    up2 = m_frPlantP[i] * m_biomass[i];
    if (up2 < m_plantP[i]) up2 = m_plantP[i];
    uapd = up2 - m_plantP[i];
    uapd *= 1.5f;   /// luxury p uptake
    m_frStrsP[i] = 1.f;
    int ir = 0;
    if (uapd < UTIL_ZERO) return;
    for (int l = 0; l < m_nSoilLayers[i]; l++) {
        if (ir > 0) break;
        if (m_soilRD[i] <= m_soilDepth[i][l]) {
            gx = m_soilRD[i];
            ir = 1;
        } else {
            gx = m_soilDepth[i][l];
        }
        upmx = uapd * (1.f - exp(-m_PUpDis * gx / m_soilRD[i])) / uobp;
        uapl = min(upmx - m_plantUpTkP[i], m_soilPsol[i][l]);
        m_plantUpTkP[i] += uapl;
        m_soilPsol[i][l] -= uapl;
    }
    if (m_plantUpTkP[i] < 0.f) m_plantUpTkP[i] = 0.f;
    m_plantP[i] += m_plantUpTkP[i];
    /// compute phosphorus stress
    PGCommon::calPlantStressByLimitedNP(m_plantP[i], up2, &m_frStrsP[i]);
}

void Biomass_EPIC::CheckDormantStatus(int i) {
    /// TODO
    return;
}

int Biomass_EPIC::Execute() {
    CheckInputData();
    initialOutputs();
    //cout<<"BIOEPIC, pre solno3: ";
    //for (int i = 0; i < m_nCells; i++)
    //{
    //	for (int j = 0; j < (int)m_nSoilLayers[i]; j++){
    //		if (m_soilNO3[i][j] != m_soilNO3[i][j])
    //			cout<<"cellid: "<<i<<"lyr: "<<j<<", "<<m_soilNO3[i][j]<<endl;
    //	}
    //}
    //cout<<endl;
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        /// calculate albedo in current day, albedo.f of SWAT
        float cej = -5.e-5f, eaj = 0.f;
        eaj = exp(cej * (m_sol_cov[i] + 0.1f));
        if (m_snowAcc[i] < 0.5f) {
            m_albedo[i] = m_soilALB[i];
            if (m_LAIDay[i] > 0.f) {
                m_albedo[i] = 0.23f * (1.f - eaj) + m_soilALB[i] * eaj;
            }
        } else {
            m_albedo[i] = 0.8f;
        }
        /// reWrite from plantmod.f of SWAT
        /// calculate residue on soil surface for current day
        m_sol_cov[i] = max(0.8f * m_biomass[i] + m_sol_rsd[i][0], 0.f);
        if (FloatEqual(m_igro[i], 1.f))            /// land cover growing
        {
            DistributePlantET(i);                  /// swu.f
            if (FloatEqual(m_dormFlag[i], 0.f)) {    /// plant will not undergo stress if dormant
                AdjustPlantGrowth(i);
            }              /// plantmod.f
            CheckDormantStatus(i);                 /// dormant.f
        }
    }
    // DEBUG
    //cout << "BIO_EPIC, cell id 14377, m_soilStorage: ";
    //for (int i = 0; i < (int)m_nSoilLayers[14377]; i++)
    //    cout << m_soilStorage[14377][i] << ", ";
    //cout << endl;
    // END OF DEBUG
    return 0;
}

void Biomass_EPIC::Get1DData(const char *key, int *n, float **data) {
    initialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_BIOMASS)) { *data = m_biomass; }
    else if (StringMatch(sk, VAR_LAST_SOILRD)) { *data = m_lastSoilRootDepth; }
    else if (StringMatch(sk, VAR_PLANT_P)) { *data = m_plantP; }
    else if (StringMatch(sk, VAR_PLANT_N)) { *data = m_plantN; }
    else if (StringMatch(sk, VAR_FR_PLANT_N)) { *data = m_frPlantN; }
    else if (StringMatch(sk, VAR_FR_PLANT_P)) { *data = m_frPlantP; }
    else if (StringMatch(sk, VAR_AET_PLT)) { *data = m_plantEPDay; }
    else if (StringMatch(sk, VAR_PLTPET_TOT)) { *data = m_pltPET; }
    else if (StringMatch(sk, VAR_PLTET_TOT)) { *data = m_pltET; }
    else if (StringMatch(sk, VAR_FR_PHU_ACC)) { *data = m_frPHUacc; }
    else if (StringMatch(sk, VAR_ROOTDEPTH)) { *data = m_soilRD; }
    else if (StringMatch(sk, VAR_LAIDAY)) { *data = m_LAIDay; }
    else if (StringMatch(sk, VAR_LAIYRMAX)) { *data = m_LAIYrMax; }
    else if (StringMatch(sk, VAR_LAIMAXFR)) { *data = m_LAIMaxFr; }
    else if (StringMatch(sk, VAR_OLAI)) { *data = m_oLAI; }
    else if (StringMatch(sk, VAR_ALBDAY)) { *data = m_albedo; }
    else if (StringMatch(sk, VAR_DORMI)) { *data = m_dormFlag; }
    else if (StringMatch(sk, VAR_IGRO)) { *data = m_igro; }
    else if (StringMatch(sk, VAR_HVSTI_ADJ)) { *data = m_hvstIdxAdj; }
    //else if (StringMatch(sk, VAR_CHT)) { *data = m_cht; }
    else if (StringMatch(sk, VAR_FR_ROOT)) { *data = m_frRoot; }
    else if (StringMatch(sk, VAR_FR_STRSWTR)) { *data = m_frStrsWa; }
    else if (StringMatch(sk, VAR_SOL_COV)) { *data = m_sol_cov; }
    else if (StringMatch(sk, VAR_SOL_SW)) { *data = m_soilStorageProfile; }
    else {
        throw ModelException(MID_BIO_EPIC, "Get1DData", "Result " + sk +
            " does not exist in current module. Please contact the module developer.");
    }
}

void Biomass_EPIC::Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
    initialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_nMaxSoilLayers;
    if (StringMatch(sk, VAR_SOL_RSD)) { *data = m_sol_rsd; }
    else {
        throw ModelException(MID_BIO_EPIC, "Get2DData", "Result " + sk + " does not exist.");
    }
}