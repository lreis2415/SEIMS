#include "NutrientTransportSediment.h"

#include "text.h"
#include "NutrientCommon.h"

NutrientTransportSediment::NutrientTransportSediment() :
    m_nSubbsns(-1), m_inputSubbsnID(-1), m_cellWth(-1.f), m_cellArea(-1.f), m_nCells(-1),
    m_nSoilLyrs(nullptr), m_maxSoilLyrs(-1),
    m_soilRock(nullptr), m_soilSat(nullptr), m_cbnModel(0), m_enratio(nullptr),
    m_olWtrEroSed(nullptr), m_surfRf(nullptr), m_soilBD(nullptr), m_soilThk(nullptr), m_soilMass(nullptr),
    m_subbsnID(nullptr), m_subbasinsInfo(nullptr), m_surfRfSedOrgN(nullptr),
    m_surfRfSedOrgP(nullptr), m_surfRfSedAbsorbMinP(nullptr), m_surfRfSedSorbMinP(nullptr),
    m_surfRfSedOrgNToCh(nullptr), m_surfRfSedOrgPToCh(nullptr),
    /// for CENTURY C/N cycling model inputs
    m_surfRfSedAbsorbMinPToCh(nullptr), m_surfRfSedSorbMinPToCh(nullptr), m_soilActvOrgN(nullptr),
    m_soilFrshOrgN(nullptr),
    m_soilStabOrgN(nullptr),
    m_soilHumOrgP(nullptr), m_soilFrshOrgP(nullptr), m_soilStabMinP(nullptr), m_soilActvMinP(nullptr),
    m_soilManP(nullptr),
    m_sol_LSN(nullptr), m_sol_LMN(nullptr), m_sol_HPN(nullptr), m_sol_HSN(nullptr), m_sol_HPC(nullptr),
    m_sol_HSC(nullptr), m_sol_LMC(nullptr), m_sol_LSC(nullptr),
    /// for C-FARM one carbon model input
    m_sol_LS(nullptr),
    /// for CENTURY C/N cycling model outputs
    m_sol_LM(nullptr), m_sol_LSL(nullptr), m_sol_LSLC(nullptr), m_sol_LSLNC(nullptr), m_sol_BMC(nullptr),
    //outputs
    m_sol_WOC(nullptr), m_soilPerco(nullptr), m_subSurfRf(nullptr), m_soilIfluCbn(nullptr),
    m_soilPercoCbn(nullptr), m_soilIfluCbnPrfl(nullptr), m_soilPercoCbnPrfl(nullptr), m_sedLossCbn(nullptr) {
}

NutrientTransportSediment::~NutrientTransportSediment() {
    if (m_soilMass != nullptr) Release2DArray(m_nCells, m_soilMass);
    if (m_enratio != nullptr) Release1DArray(m_enratio);

    if (m_surfRfSedOrgP != nullptr) Release1DArray(m_surfRfSedOrgP);
    if (m_surfRfSedOrgN != nullptr) Release1DArray(m_surfRfSedOrgN);
    if (m_surfRfSedAbsorbMinP != nullptr) Release1DArray(m_surfRfSedAbsorbMinP);
    if (m_surfRfSedSorbMinP != nullptr) Release1DArray(m_surfRfSedSorbMinP);

    if (m_surfRfSedOrgNToCh != nullptr) Release1DArray(m_surfRfSedOrgNToCh);
    if (m_surfRfSedOrgPToCh != nullptr) Release1DArray(m_surfRfSedOrgPToCh);
    if (m_surfRfSedAbsorbMinPToCh != nullptr) Release1DArray(m_surfRfSedAbsorbMinPToCh);
    if (m_surfRfSedSorbMinPToCh != nullptr) Release1DArray(m_surfRfSedSorbMinPToCh);

    /// for CENTURY C/N cycling model outputs
    if (m_soilIfluCbn != nullptr) Release2DArray(m_nCells, m_soilPercoCbn);
    if (m_soilPercoCbn != nullptr) Release2DArray(m_nCells, m_soilPercoCbn);
    if (m_soilIfluCbnPrfl != nullptr) Release1DArray(m_soilIfluCbnPrfl);
    if (m_soilPercoCbnPrfl != nullptr) Release1DArray(m_soilPercoCbnPrfl);
    if (m_sedLossCbn != nullptr) Release1DArray(m_sedLossCbn);
}

bool NutrientTransportSediment::CheckInputData() {
    CHECK_POSITIVE(MID_NUTRSED, m_nCells);
    CHECK_POSITIVE(MID_NUTRSED, m_nSubbsns);
    CHECK_POSITIVE(MID_NUTRSED, m_cellWth);
    CHECK_POSITIVE(MID_NUTRSED, m_maxSoilLyrs);
    CHECK_POINTER(MID_NUTRSED, m_nSoilLyrs);
    CHECK_POINTER(MID_NUTRSED, m_olWtrEroSed);
    CHECK_POINTER(MID_NUTRSED, m_surfRf);
    CHECK_POINTER(MID_NUTRSED, m_soilBD);
    CHECK_POINTER(MID_NUTRSED, m_soilActvMinP);
    CHECK_POINTER(MID_NUTRSED, m_soilStabOrgN);
    CHECK_POINTER(MID_NUTRSED, m_soilHumOrgP);
    CHECK_POINTER(MID_NUTRSED, m_soilStabMinP);
    CHECK_POINTER(MID_NUTRSED, m_soilActvOrgN);
    CHECK_POINTER(MID_NUTRSED, m_soilFrshOrgN);
    CHECK_POINTER(MID_NUTRSED, m_soilFrshOrgP);
    CHECK_POINTER(MID_NUTRSED, m_subbsnID);
    CHECK_POINTER(MID_NUTRSED, m_subbasinsInfo);
    if (!(m_cbnModel == 0 || m_cbnModel == 1 || m_cbnModel == 2)) {
        throw ModelException(MID_NUTRSED, "CheckInputData", "Carbon modeling method must be 0, 1, or 2.");
    }
    return true;
}

bool NutrientTransportSediment::CheckInputDataCenturyModel() {
    CHECK_POINTER(MID_NUTRSED, m_sol_LSN);
    CHECK_POINTER(MID_NUTRSED, m_sol_LMN);
    CHECK_POINTER(MID_NUTRSED, m_sol_HPN);
    CHECK_POINTER(MID_NUTRSED, m_sol_HSN);
    CHECK_POINTER(MID_NUTRSED, m_sol_HPC);
    CHECK_POINTER(MID_NUTRSED, m_sol_HSC);
    CHECK_POINTER(MID_NUTRSED, m_sol_LMC);
    CHECK_POINTER(MID_NUTRSED, m_sol_LSC);
    CHECK_POINTER(MID_NUTRSED, m_sol_LS);
    CHECK_POINTER(MID_NUTRSED, m_sol_LM);
    CHECK_POINTER(MID_NUTRSED, m_sol_LSL);
    CHECK_POINTER(MID_NUTRSED, m_sol_LSLC);
    CHECK_POINTER(MID_NUTRSED, m_sol_LSLNC);
    CHECK_POINTER(MID_NUTRSED, m_sol_BMC);
    CHECK_POINTER(MID_NUTRSED, m_sol_WOC);
    CHECK_POINTER(MID_NUTRSED, m_soilPerco);
    CHECK_POINTER(MID_NUTRSED, m_subSurfRf);
    return true;
}

bool NutrientTransportSediment::CheckInputDataCFarmModel() {
    CHECK_POINTER(MID_NUTRSED, m_soilManP);
    return true;
}

void NutrientTransportSediment::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSNID_NUM)) m_nSubbsns = CVT_INT(value);
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnID = CVT_INT(value);
    else if (StringMatch(sk, Tag_CellWidth)) m_cellWth = value;
    else if (StringMatch(sk, VAR_CSWAT)) m_cbnModel = CVT_INT(value);
    else {
        throw ModelException(MID_NUTRSED, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void NutrientTransportSediment::Set1DData(const char* key, const int n, float* data) {
    CheckInputSize(MID_NUTRSED, key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN)) {
        m_subbsnID = data;
    } else if (StringMatch(sk, VAR_SOILLAYERS)) {
        m_nSoilLyrs = data;
    } else if (StringMatch(sk, VAR_SEDYLD)) {
        m_olWtrEroSed = data;
    } else if (StringMatch(sk, VAR_OLFLOW)) {
        m_surfRf = data;
    } else {
        throw ModelException(MID_NUTRSED, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void NutrientTransportSediment::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    CheckInputSize2D(MID_NUTRSED, key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    string sk(key);
    if (StringMatch(sk, VAR_SOILTHICK)) m_soilThk = data;
    else if (StringMatch(sk, VAR_SOL_BD)) m_soilBD = data;
    else if (StringMatch(sk, VAR_SOL_AORGN)) m_soilActvOrgN = data;
    else if (StringMatch(sk, VAR_SOL_SORGN)) m_soilStabOrgN = data;
    else if (StringMatch(sk, VAR_SOL_HORGP)) m_soilHumOrgP = data;
    else if (StringMatch(sk, VAR_SOL_FORGP)) m_soilFrshOrgP = data;
    else if (StringMatch(sk, VAR_SOL_FORGN)) m_soilFrshOrgN = data;
    else if (StringMatch(sk, VAR_SOL_ACTP)) m_soilActvMinP = data;
    else if (StringMatch(sk, VAR_SOL_STAP)) m_soilStabMinP = data;
        /// for CENTURY C/Y cycling model, optional inputs
    else if (StringMatch(sk, VAR_ROCK)) m_soilRock = data;
    else if (StringMatch(sk, VAR_SOL_UL)) m_soilSat = data;
    else if (StringMatch(sk, VAR_SOL_LSN)) m_sol_LSN = data;
    else if (StringMatch(sk, VAR_SOL_LMN)) m_sol_LMN = data;
    else if (StringMatch(sk, VAR_SOL_HPN)) m_sol_HPN = data;
    else if (StringMatch(sk, VAR_SOL_HSN)) m_sol_HSN = data;
    else if (StringMatch(sk, VAR_SOL_HPC)) m_sol_HPC = data;
    else if (StringMatch(sk, VAR_SOL_HSC)) m_sol_HSC = data;
    else if (StringMatch(sk, VAR_SOL_LMC)) m_sol_LMC = data;
    else if (StringMatch(sk, VAR_SOL_LSC)) m_sol_LSC = data;
    else if (StringMatch(sk, VAR_SOL_LS)) m_sol_LS = data;
    else if (StringMatch(sk, VAR_SOL_LM)) m_sol_LM = data;
    else if (StringMatch(sk, VAR_SOL_LSL)) m_sol_LSL = data;
    else if (StringMatch(sk, VAR_SOL_LSLC)) m_sol_LSLC = data;
    else if (StringMatch(sk, VAR_SOL_LSLNC)) m_sol_LSLNC = data;
    else if (StringMatch(sk, VAR_SOL_BMC)) m_sol_BMC = data;
    else if (StringMatch(sk, VAR_SOL_WOC)) m_sol_WOC = data;
    else if (StringMatch(sk, VAR_PERCO)) m_soilPerco = data;
    else if (StringMatch(sk, VAR_SSRU)) m_subSurfRf = data;
        /// for C-FARM one carbon model
    else if (StringMatch(sk, VAR_SOL_MP)) m_soilManP = data;
    else {
        throw ModelException(MID_NUTRSED, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void NutrientTransportSediment::InitialOutputs() {
    CHECK_POSITIVE(MID_NUTRSED, m_nCells);
    // initial enrichment ratio
    if (nullptr == m_enratio) {
        Initialize1DArray(m_nCells, m_enratio, 0.f);
    }
    if (m_cellArea < 0) {
        m_cellArea = m_cellWth * m_cellWth * 0.0001f; //Unit is ha
    }
    /// initialize m_soilMass
    if (m_soilMass == nullptr) {
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilMass, 0.f);
    }
    // allocate the output variables
    if (nullptr == m_surfRfSedOrgN) {
        Initialize1DArray(m_nCells, m_surfRfSedOrgN, 0.f);
        Initialize1DArray(m_nCells, m_surfRfSedOrgP, 0.f);
        Initialize1DArray(m_nCells, m_surfRfSedAbsorbMinP, 0.f);
        Initialize1DArray(m_nCells, m_surfRfSedSorbMinP, 0.f);

        Initialize1DArray(m_nSubbsns + 1, m_surfRfSedOrgNToCh, 0.f);
        Initialize1DArray(m_nSubbsns + 1, m_surfRfSedOrgPToCh, 0.f);
        Initialize1DArray(m_nSubbsns + 1, m_surfRfSedAbsorbMinPToCh, 0.f);
        Initialize1DArray(m_nSubbsns + 1, m_surfRfSedSorbMinPToCh, 0.f);
    }
    /// for CENTURY C/N cycling model outputs
    if (m_cbnModel == 2 && nullptr == m_soilIfluCbn) {
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilIfluCbn, 0.f);
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilPercoCbn, 0.f);
        Initialize1DArray(m_nCells, m_soilIfluCbnPrfl, 0.f);
        Initialize1DArray(m_nCells, m_soilPercoCbnPrfl, 0.f);
        Initialize1DArray(m_nCells, m_sedLossCbn, 0.f);
    }
}

void NutrientTransportSediment::InitializeIntermediateVariables() {
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        for (int k = 0; k < CVT_INT(m_nSoilLyrs[i]); k++) {
            m_soilMass[i][k] = 10000.f * m_soilThk[i][k] * m_soilBD[i][k] * (1.f - m_soilRock[i][k] * 0.01f);
        }
    }

    m_needReCalIntermediateParams = false;
}

void NutrientTransportSediment::SetSubbasins(clsSubbasins* subbasins) {
    if (nullptr == m_subbasinsInfo) {
        m_subbasinsInfo = subbasins;
        // m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber(); // Set in SetValue()
        m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
    }
}

int NutrientTransportSediment::Execute() {
    CheckInputData();
    if (m_cbnModel == 1) {
        if (!CheckInputDataCFarmModel()) return false;
    }
    if (m_cbnModel == 2) {
        if (!CheckInputDataCenturyModel()) return false;
    }
    InitialOutputs();
    if (m_needReCalIntermediateParams) InitializeIntermediateVariables();
    // initial nutrient to channel for each day
    for (int i = 0; i < m_nSubbsns + 1; i++) {
        m_surfRfSedOrgNToCh[i] = 0.f;
        m_surfRfSedOrgPToCh[i] = 0.f;
        m_surfRfSedAbsorbMinPToCh[i] = 0.f;
        m_surfRfSedSorbMinPToCh[i] = 0.f;
    }

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_olWtrEroSed[i] < 1.e-4f) m_olWtrEroSed[i] = 0.f;
        // CREAMS method for calculating enrichment ratio
        m_enratio[i] = CalEnrichmentRatio(m_olWtrEroSed[i], m_surfRf[i], m_cellArea);

        //Calculates the amount of organic nitrogen removed in surface runoff
        if (m_cbnModel == 0) {
            OrgNRemovedInRunoffStaticMethod(i);
        } else if (m_cbnModel == 1) {
            OrgNRemovedInRunoffCFarmOneCarbonModel(i);
        } else if (m_cbnModel == 2) {
            OrgNRemovedInRunoffCenturyModel(i);
        }
        //Calculates the amount of organic and mineral phosphorus attached to sediment in surface runoff. psed.f of SWAT
        OrgPAttachedtoSed(i);
    }
    // sum by subbasin
    // See https://github.com/lreis2415/SEIMS/issues/36 for more descriptions. By lj
#pragma omp parallel
    {
        float* tmp_orgn2ch = new(nothrow) float[m_nSubbsns + 1];
        float* tmp_orgp2ch = new(nothrow) float[m_nSubbsns + 1];
        float* tmp_minpa2ch = new(nothrow) float[m_nSubbsns + 1];
        float* tmp_minps2ch = new(nothrow) float[m_nSubbsns + 1];
        for (int i = 0; i <= m_nSubbsns; i++) {
            tmp_orgn2ch[i] = 0.f;
            tmp_orgp2ch[i] = 0.f;
            tmp_minpa2ch[i] = 0.f;
            tmp_minps2ch[i] = 0.f;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            tmp_orgn2ch[CVT_INT(m_subbsnID[i])] += m_surfRfSedOrgN[i];
            tmp_orgp2ch[CVT_INT(m_subbsnID[i])] += m_surfRfSedOrgP[i];
            tmp_minpa2ch[CVT_INT(m_subbsnID[i])] += m_surfRfSedAbsorbMinP[i];
            tmp_minps2ch[CVT_INT(m_subbsnID[i])] += m_surfRfSedSorbMinP[i];
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbsns; i++) {
                m_surfRfSedOrgNToCh[i] += tmp_orgn2ch[i];
                m_surfRfSedOrgPToCh[i] += tmp_orgp2ch[i];
                m_surfRfSedAbsorbMinPToCh[i] += tmp_minpa2ch[i];
                m_surfRfSedSorbMinPToCh[i] += tmp_minps2ch[i];
            }
        }
        delete[] tmp_orgn2ch;
        delete[] tmp_orgp2ch;
        delete[] tmp_minpa2ch;
        delete[] tmp_minps2ch;
        tmp_orgn2ch = nullptr;
        tmp_orgp2ch = nullptr;
        tmp_minpa2ch = nullptr;
        tmp_minps2ch = nullptr;
    } /* END of #pragma omp parallel */
    // sum all the subbasins and put the sum value in the zero-index of the array
    for (int i = 1; i < m_nSubbsns + 1; i++) {
        m_surfRfSedOrgNToCh[i] *= m_cellArea;
        m_surfRfSedOrgPToCh[i] *= m_cellArea;
        m_surfRfSedAbsorbMinPToCh[i] *= m_cellArea;
        m_surfRfSedSorbMinPToCh[i] *= m_cellArea;
        m_surfRfSedOrgNToCh[0] += m_surfRfSedOrgNToCh[i];
        m_surfRfSedOrgPToCh[0] += m_surfRfSedOrgPToCh[i];
        m_surfRfSedAbsorbMinPToCh[0] += m_surfRfSedAbsorbMinPToCh[i];
        m_surfRfSedSorbMinPToCh[0] += m_surfRfSedSorbMinPToCh[i];
    }
    return 0;
}

void NutrientTransportSediment::OrgNRemovedInRunoffStaticMethod(const int i) {
    //amount of organic N in first soil layer (orgninfl)
    float orgninfl = 0.f;
    //conversion factor (wt)
    float wt = 0.f;
    orgninfl = m_soilStabOrgN[i][0] + m_soilActvOrgN[i][0] + m_soilFrshOrgN[i][0];
    wt = m_soilBD[i][0] * m_soilThk[i][0] * 0.01f;
    //concentration of organic N in soil (concn)
    float concn = orgninfl * m_enratio[i] / wt;
    //Calculate the amount of organic nitrogen transported with sediment to the stream, equation 4:2.2.1 in SWAT Theory 2009, p271
    m_surfRfSedOrgN[i] = 0.001f * concn * m_olWtrEroSed[i] * 0.001f / m_cellArea; /// kg/ha
    //update soil nitrogen pools
    if (orgninfl > 1.e-6f) {
        m_soilActvOrgN[i][0] = m_soilActvOrgN[i][0] - m_surfRfSedOrgN[i] * (m_soilActvOrgN[i][0] / orgninfl);
        m_soilStabOrgN[i][0] = m_soilStabOrgN[i][0] - m_surfRfSedOrgN[i] * (m_soilStabOrgN[i][0] / orgninfl);
        m_soilFrshOrgN[i][0] = m_soilFrshOrgN[i][0] - m_surfRfSedOrgN[i] * (m_soilFrshOrgN[i][0] / orgninfl);
        if (m_soilActvOrgN[i][0] < 0.f) {
            m_surfRfSedOrgN[i] = m_surfRfSedOrgN[i] + m_soilActvOrgN[i][0];
            m_soilActvOrgN[i][0] = 0.f;
        }
        if (m_soilStabOrgN[i][0] < 0.f) {
            m_surfRfSedOrgN[i] = m_surfRfSedOrgN[i] + m_soilStabOrgN[i][0];
            m_soilStabOrgN[i][0] = 0.f;
        }
        if (m_soilFrshOrgN[i][0] < 0.f) {
            m_surfRfSedOrgN[i] = m_surfRfSedOrgN[i] + m_soilFrshOrgN[i][0];
            m_soilFrshOrgN[i][0] = 0.f;
        }
    }
}

void NutrientTransportSediment::OrgNRemovedInRunoffCFarmOneCarbonModel(const int i) {
    /// TODO
}

void NutrientTransportSediment::OrgNRemovedInRunoffCenturyModel(const int i) {
    float totOrgN_lyr0 = 0.f; /// kg N/ha, amount of organic N in first soil layer, i.e., xx in SWAT src.
    float wt1 = 0.f;          /// conversion factor, mg/kg => kg/ha
    float er = 0.f;           /// enrichment ratio
    float conc = 0.f;         /// concentration of organic N in soil
    float QBC = 0.f;          /// C loss with runoff or lateral flow
    float VBC = 0.f;          /// C loss with vertical flow
    float YBC = 0.f;          /// BMC loss with sediment
    float YOC = 0.f;          /// Organic C loss with sediment
    float YW = 0.f;           /// Wind erosion, kg
    float TOT = 0.f;          /// total organic carbon in layer 1
    float YEW = 0.f;          /// fraction of soil erosion of total soil mass
    float X1 = 0.f, PRMT_21 = 0.f;
    float PRMT_44 = 0.f; /// ratio of soluble C concentration in runoff to percolate (0.1 - 1.0)
    float XX = 0.f, DK = 0.f, V = 0.f, X3 = 0.f;
    float CO = 0.f; /// the vertical concentration
    float CS = 0.f; /// the horizontal concentration
    float perc_clyr = 0.f, latc_clyr = 0.f;

    totOrgN_lyr0 = m_sol_LSN[i][0] + m_sol_LMN[i][0] + m_sol_HPN[i][0] + m_sol_HSN[i][0];
    wt1 = m_soilBD[i][0] * m_soilThk[i][0] * 0.01f;
    er = m_enratio[i];
    conc = totOrgN_lyr0 * er / wt1;
    m_surfRfSedOrgN[i] = 0.001f * conc * m_olWtrEroSed[i] * 0.001f / m_cellArea;
    /// update soil nitrogen pools
    if (totOrgN_lyr0 > UTIL_ZERO) {
        float xx1 = 1.f - m_surfRfSedOrgN[i] / totOrgN_lyr0;
        m_sol_LSN[i][0] *= xx1;
        m_sol_LMN[i][0] *= xx1;
        m_sol_HPN[i][0] *= xx1;
        m_sol_HSN[i][0] *= xx1;
    }
    /// Calculate runoff and leached C&N from micro-biomass
    /// total organic carbon in layer 1
    TOT = m_sol_HPC[i][0] + m_sol_HSC[i][0] + m_sol_LMC[i][0] + m_sol_LSC[i][0];
    /// fraction of soil erosion of total soil mass
    YEW = Min((m_olWtrEroSed[i] / m_cellArea + YW / m_cellArea) / m_soilMass[i][0], 0.9f);

    X1 = 1.f - YEW;
    YOC = YEW * TOT;
    m_sol_HSC[i][0] *= X1;
    m_sol_HPC[i][0] *= X1;
    m_sol_LS[i][0] *= X1;
    m_sol_LM[i][0] *= X1;
    m_sol_LSL[i][0] *= X1;
    m_sol_LSC[i][0] *= X1;
    m_sol_LMC[i][0] *= X1;
    m_sol_LSLC[i][0] *= X1;
    m_sol_LSLNC[i][0] = m_sol_LSC[i][0] - m_sol_LSLC[i][0];

    if (m_sol_BMC[i][0] > 0.01f) {
        ///KOC FOR CARBON LOSS IN WATER AND SEDIMENT(500._1500.) KD = KOC * C
        PRMT_21 = 1000.f;
        m_sol_WOC[i][0] = m_sol_LSC[i][0] + m_sol_LMC[i][0] + m_sol_HPC[i][0] + m_sol_HSC[i][0] + m_sol_BMC[i][0];
        DK = 0.0001f * PRMT_21 * m_sol_WOC[i][0];
        X1 = m_soilSat[i][0];
        if (X1 <= 0.f) X1 = 0.01f;
        XX = X1 + DK;
        V = m_surfRf[i] + m_soilPerco[i][0] + m_subSurfRf[i][0];
        if (V > 1.e-10f) {
            X3 = m_sol_BMC[i][0] * (1.f - exp(-V / XX)); /// loss of biomass C
            PRMT_44 = 0.5;
            CO = X3 / (m_soilPerco[i][0] + PRMT_44 * (m_surfRf[i] + m_subSurfRf[i][0]));
            CS = PRMT_44 * CO;
            VBC = CO * m_soilPerco[i][0];
            m_sol_BMC[i][0] -= X3;
            QBC = CS * (m_surfRf[i] + m_subSurfRf[i][0]);
            /// Compute WBMC loss with sediment
            if (YEW > 0.f) {
                CS = DK * m_sol_BMC[i][0] / XX;
                YBC = YEW * CS;
            }
        }
    }
    m_sol_BMC[i][0] -= YBC;
    /// surfqc_d(j) = QBC*(surfq(j)/(surfq(j)+flat(1,j)+1.e-6))  is for print purpose, thus not implemented.
    m_soilIfluCbn[i][0] = QBC * (m_subSurfRf[i][0] / (m_surfRf[i] + m_subSurfRf[i][0] + UTIL_ZERO));
    m_soilPercoCbn[i][0] = VBC;
    m_sedLossCbn[i] = YOC + YBC;

    latc_clyr += m_soilIfluCbn[i][0];
    for (int k = 1; k < CVT_INT(m_nSoilLyrs[i]); k++) {
        m_sol_WOC[i][k] = m_sol_LSC[i][k] + m_sol_LMC[i][k] + m_sol_HPC[i][k] + m_sol_HSC[i][k];
        float Y1 = m_sol_BMC[i][k] + VBC;
        VBC = 0.f;
        if (Y1 > 0.01f) {
            V = m_soilPerco[i][k] + m_subSurfRf[i][k];
            if (V > 0.f) {
                VBC = Y1 * (1.f - exp(-V / (m_soilSat[i][k] + 0.0001f * PRMT_21 * m_sol_WOC[i][k])));
            }
        }
        m_soilIfluCbn[i][k] = VBC * (m_subSurfRf[i][k] / (m_subSurfRf[i][k] + m_soilPerco[i][k] + UTIL_ZERO));
        m_soilPercoCbn[i][k] = VBC - m_soilIfluCbn[i][k];
        m_sol_BMC[i][k] = Y1 - VBC;

        /// calculate nitrate in percolate and lateral flow
        perc_clyr += m_soilPercoCbn[i][k];
        latc_clyr += m_soilIfluCbn[i][k];
    }
    m_soilIfluCbnPrfl[i] = latc_clyr;
    m_soilPercoCbnPrfl[i] = perc_clyr;
}

void NutrientTransportSediment::OrgPAttachedtoSed(const int i) {
    //amount of phosphorus attached to sediment in soil (sol_attp)
    float sol_attp = 0.f;
    //fraction of active mineral/organic/stable mineral phosphorus in soil (sol_attp_o, sol_attp_a, sol_attp_s)
    float sol_attp_o = 0.f;
    float sol_attp_a = 0.f;
    float sol_attp_s = 0.f;
    //Calculate sediment
    sol_attp = m_soilHumOrgP[i][0] + m_soilFrshOrgP[i][0] + m_soilActvMinP[i][0] + m_soilStabMinP[i][0];
    if (m_soilManP != nullptr) {
        sol_attp += m_soilManP[i][0];
    }
    if (sol_attp > 1.e-3f) {
        sol_attp_o = (m_soilHumOrgP[i][0] + m_soilFrshOrgP[i][0]) / sol_attp;
        if (m_soilManP != nullptr) {
            sol_attp_o += m_soilManP[i][0] / sol_attp;
        }
        sol_attp_a = m_soilActvMinP[i][0] / sol_attp;
        sol_attp_s = m_soilStabMinP[i][0] / sol_attp;
    }
    //conversion factor (mg/kg => kg/ha) (wt)
    float wt = m_soilBD[i][0] * m_soilThk[i][0] * 0.01f;
    //concentration of organic P in soil (concp)
    float concp = 0.f;
    concp = sol_attp * m_enratio[i] / wt; /// mg/kg
    //total amount of P removed in sediment erosion (sedp)
    float sedp = 1.e-6f * concp * m_olWtrEroSed[i] / m_cellArea; /// kg/ha
    m_surfRfSedOrgP[i] = sedp * sol_attp_o;
    m_surfRfSedAbsorbMinP[i] = sedp * sol_attp_a;
    m_surfRfSedSorbMinP[i] = sedp * sol_attp_s;

    //if(i==100)cout << "sedp: " << sedp<< ",sol_attp_o: "  << sol_attp_o << endl;
    //modify phosphorus pools

    //total amount of P in mineral sediment pools prior to sediment removal (psedd)		// Not used
    //float psedd = 0.f;
    //psedd = m_sol_actp[i][0] + m_sol_stap[i][0];

    //total amount of P in organic pools prior to sediment removal (porgg)
    float porgg = 0.f;
    porgg = m_soilHumOrgP[i][0] + m_soilFrshOrgP[i][0];
    if (porgg > 1.e-3f) {
        m_soilHumOrgP[i][0] = m_soilHumOrgP[i][0] - m_surfRfSedOrgP[i] * (m_soilHumOrgP[i][0] / porgg);
        m_soilFrshOrgP[i][0] = m_soilFrshOrgP[i][0] - m_surfRfSedOrgP[i] * (m_soilFrshOrgP[i][0] / porgg);
    }
    m_soilActvMinP[i][0] = m_soilActvMinP[i][0] - m_surfRfSedAbsorbMinP[i];
    m_soilStabMinP[i][0] = m_soilStabMinP[i][0] - m_surfRfSedSorbMinP[i];
    if (m_soilHumOrgP[i][0] < 0.f) {
        m_surfRfSedOrgP[i] = m_surfRfSedOrgP[i] + m_soilHumOrgP[i][0];
        m_soilHumOrgP[i][0] = 0.f;
    }
    if (m_soilFrshOrgP[i][0] < 0.f) {
        m_surfRfSedOrgP[i] = m_surfRfSedOrgP[i] + m_soilFrshOrgP[i][0];
        m_soilFrshOrgP[i][0] = 0.f;
    }
    if (m_soilActvMinP[i][0] < 0.f) {
        m_surfRfSedAbsorbMinP[i] = m_surfRfSedAbsorbMinP[i] + m_soilActvMinP[i][0];
        m_soilActvMinP[i][0] = 0.f;
    }
    if (m_soilStabMinP[i][0] < 0.f) {
        m_surfRfSedSorbMinP[i] = m_surfRfSedSorbMinP[i] + m_soilStabMinP[i][0];
        m_soilStabMinP[i][0] = 0.f;
    }
}

void NutrientTransportSediment::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SEDORGN)) {
        *data = m_surfRfSedOrgN;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDORGP)) {
        *data = m_surfRfSedOrgP;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDMINPA)) {
        *data = m_surfRfSedAbsorbMinP;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDMINPS)) {
        *data = m_surfRfSedSorbMinP;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDORGN_TOCH)) {
        *data = m_surfRfSedOrgNToCh;
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SEDORGP_TOCH)) {
        *data = m_surfRfSedOrgPToCh;
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SEDMINPA_TOCH)) {
        *data = m_surfRfSedAbsorbMinPToCh;
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SEDMINPS_TOCH)) {
        *data = m_surfRfSedSorbMinPToCh;
        *n = m_nSubbsns + 1;
    }
        /// outputs of CENTURY C/N cycling model
    else if (StringMatch(sk, VAR_LATERAL_C)) {
        *data = m_soilIfluCbnPrfl;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_PERCO_C)) {
        *data = m_soilPercoCbnPrfl;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDLOSS_C)) {
        *data = m_sedLossCbn;
        *n = m_nCells;
    } else {
        throw ModelException(MID_NUTRSED, "Get1DData", "Parameter " + sk + " does not exist");
    }
}

void NutrientTransportSediment::Get2DData(const char* key, int* nrows, int* ncols, float*** data) {
    InitialOutputs();
    string sk(key);
    *nrows = m_nCells;
    *ncols = m_maxSoilLyrs;
    if (StringMatch(sk, VAR_SOL_AORGN)) *data = m_soilActvOrgN;
    else if (StringMatch(sk, VAR_SOL_FORGN)) *data = m_soilFrshOrgN;
    else if (StringMatch(sk, VAR_SOL_SORGN)) *data = m_soilStabOrgN;
    else if (StringMatch(sk, VAR_SOL_HORGP)) *data = m_soilHumOrgP;
    else if (StringMatch(sk, VAR_SOL_FORGP)) *data = m_soilFrshOrgP;
    else if (StringMatch(sk, VAR_SOL_STAP)) *data = m_soilStabMinP;
    else if (StringMatch(sk, VAR_SOL_ACTP)) *data = m_soilActvMinP;
        /// outputs of CENTURY C/N cycling model
    else if (StringMatch(sk, VAR_SOL_LATERAL_C)) *data = m_soilIfluCbn;
    else if (StringMatch(sk, VAR_SOL_PERCO_C)) *data = m_soilPercoCbn;
    else {
        throw ModelException(MID_NUTRSED, "Get2DData", "Output " + sk + " does not exist.");
    }
}
