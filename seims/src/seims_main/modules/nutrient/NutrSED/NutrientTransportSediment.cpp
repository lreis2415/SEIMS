#include "NutrientTransportSediment.h"

#include "text.h"
#include "NutrientCommon.h"

NutrientTransportSediment::NutrientTransportSediment() :
    //input
    m_nSubbasins(-1), m_subbasinID(-1), m_cellWidth(-1.f), m_cellArea(-1.f), m_nCells(-1),
    m_nSoilLyrs(nullptr), m_maxSoilLyrs(-1),
    m_soilRock(nullptr), m_soilSat(nullptr), m_cbnModel(0), m_enratio(nullptr),
    m_sedEroded(nullptr), m_surfRf(nullptr), m_soilBD(nullptr), m_soilThk(nullptr), m_soilMass(nullptr),
    m_subbasin(nullptr), m_subbasinsInfo(nullptr), m_sedorgn(nullptr),
    m_sedorgp(nullptr), m_sedminpa(nullptr), m_sedminps(nullptr),
    m_sedorgnToCh(nullptr), m_sedorgpToCh(nullptr),
    /// for CENTURY C/N cycling model inputs
    m_sedminpaToCh(nullptr), m_sedminpsToCh(nullptr), m_soilActvOrgN(nullptr), m_soilFrshOrgN(nullptr),
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
    m_sol_WOC(nullptr), m_soilPerco(nullptr), m_subSurfRf(nullptr), m_sol_latC(nullptr),
    m_sol_percoC(nullptr), m_laterC(nullptr), m_percoC(nullptr), m_sedCLoss(nullptr) {
}

NutrientTransportSediment::~NutrientTransportSediment() {
    if (m_soilMass != nullptr) Release2DArray(m_nCells, m_soilMass);
    if (m_enratio != nullptr) Release1DArray(m_enratio);

    if (m_sedorgp != nullptr) Release1DArray(m_sedorgp);
    if (m_sedorgn != nullptr) Release1DArray(m_sedorgn);
    if (m_sedminpa != nullptr) Release1DArray(m_sedminpa);
    if (m_sedminps != nullptr) Release1DArray(m_sedminps);

    if (m_sedorgnToCh != nullptr) Release1DArray(m_sedorgnToCh);
    if (m_sedorgpToCh != nullptr) Release1DArray(m_sedorgpToCh);
    if (m_sedminpaToCh != nullptr) Release1DArray(m_sedminpaToCh);
    if (m_sedminpsToCh != nullptr) Release1DArray(m_sedminpsToCh);

    /// for CENTURY C/N cycling model outputs
    if (m_sol_latC != nullptr) Release2DArray(m_nCells, m_sol_percoC);
    if (m_sol_percoC != nullptr) Release2DArray(m_nCells, m_sol_percoC);
    if (m_laterC != nullptr) Release1DArray(m_laterC);
    if (m_percoC != nullptr) Release1DArray(m_percoC);
    if (m_sedCLoss != nullptr) Release1DArray(m_sedCLoss);
}

bool NutrientTransportSediment::CheckInputSize(const char* key, int n) {
    if (n <= 0) {
        throw ModelException(MID_NUTRSED, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.\n");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException(MID_NUTRSED, "CheckInputSize",
                                 "Input data for " + string(key) + " is invalid with size: " + ValueToString(n) +
                                 ". The origin size is " + ValueToString(m_nCells) + ".\n");
        }
    }
    return true;
}

bool NutrientTransportSediment::CheckInputData() {
    CHECK_POSITIVE(MID_NUTRSED, m_nCells);
    CHECK_POSITIVE(MID_NUTRSED, m_nSubbasins);
    CHECK_POSITIVE(MID_NUTRSED, m_cellWidth);
    CHECK_POSITIVE(MID_NUTRSED, m_maxSoilLyrs);
    CHECK_POINTER(MID_NUTRSED, m_nSoilLyrs);
    CHECK_POINTER(MID_NUTRSED, m_sedEroded);
    CHECK_POINTER(MID_NUTRSED, m_surfRf);
    CHECK_POINTER(MID_NUTRSED, m_soilBD);
    CHECK_POINTER(MID_NUTRSED, m_soilActvMinP);
    CHECK_POINTER(MID_NUTRSED, m_soilStabOrgN);
    CHECK_POINTER(MID_NUTRSED, m_soilHumOrgP);
    CHECK_POINTER(MID_NUTRSED, m_soilStabMinP);
    CHECK_POINTER(MID_NUTRSED, m_soilActvOrgN);
    CHECK_POINTER(MID_NUTRSED, m_soilFrshOrgN);
    CHECK_POINTER(MID_NUTRSED, m_soilFrshOrgP);
    CHECK_POINTER(MID_NUTRSED, m_subbasin);
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

void NutrientTransportSediment::SetValue(const char* key, float value) {
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSNID_NUM)) m_nSubbasins = CVT_INT(value);
    else if (StringMatch(sk, Tag_SubbasinId)) m_subbasinID = CVT_INT(value);
    else if (StringMatch(sk, Tag_CellWidth)) m_cellWidth = value;
    else if (StringMatch(sk, VAR_CSWAT)) m_cbnModel = CVT_INT(value);
    else {
        throw ModelException(MID_NUTRSED, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void NutrientTransportSediment::Set1DData(const char* key, int n, float* data) {
    CheckInputSize(key, n);
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN)) {
        m_subbasin = data;
    } else if (StringMatch(sk, VAR_SOILLAYERS)) {
        m_nSoilLyrs = data;
    } else if (StringMatch(sk, VAR_SEDYLD)) {
        m_sedEroded = data;
    } else if (StringMatch(sk, VAR_OLFLOW)) {
        m_surfRf = data;
    } else {
        throw ModelException(MID_NUTRSED, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void NutrientTransportSediment::Set2DData(const char* key, int nRows, int nCols, float** data) {
    CheckInputSize(key, nRows);
    string sk(key);
    m_maxSoilLyrs = nCols;
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
        m_cellArea = m_cellWidth * m_cellWidth * 0.0001f; //Unit is ha
    }
    /// initialize m_soilMass
    if (m_soilMass == nullptr) {
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilMass, 0.f);
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            for (int k = 0; k < CVT_INT(m_nSoilLyrs[i]); k++) {
                m_soilMass[i][k] = 10000.f * m_soilThk[i][k] * m_soilBD[i][k] * (1.f - m_soilRock[i][k] * 0.01f);
            }
        }
    }
    // allocate the output variables
    if (nullptr == m_sedorgn) {
        Initialize1DArray(m_nCells, m_sedorgn, 0.f);
        Initialize1DArray(m_nCells, m_sedorgp, 0.f);
        Initialize1DArray(m_nCells, m_sedminpa, 0.f);
        Initialize1DArray(m_nCells, m_sedminps, 0.f);

        Initialize1DArray(m_nSubbasins + 1, m_sedorgnToCh, 0.f);
        Initialize1DArray(m_nSubbasins + 1, m_sedorgpToCh, 0.f);
        Initialize1DArray(m_nSubbasins + 1, m_sedminpaToCh, 0.f);
        Initialize1DArray(m_nSubbasins + 1, m_sedminpsToCh, 0.f);
    }
    /// for CENTURY C/N cycling model outputs
    if (m_cbnModel == 2 && nullptr == m_sol_latC) {
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_sol_latC, 0.f);
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_sol_percoC, 0.f);
        Initialize1DArray(m_nCells, m_laterC, 0.f);
        Initialize1DArray(m_nCells, m_percoC, 0.f);
        Initialize1DArray(m_nCells, m_sedCLoss, 0.f);
    }
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
    // initial nutrient to channel for each day
    for (int i = 0; i < m_nSubbasins + 1; i++) {
        m_sedorgnToCh[i] = 0.f;
        m_sedorgpToCh[i] = 0.f;
        m_sedminpaToCh[i] = 0.f;
        m_sedminpsToCh[i] = 0.f;
    }

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_sedEroded[i] < 1.e-4f) m_sedEroded[i] = 0.f;
        // CREAMS method for calculating enrichment ratio
        m_enratio[i] = CalEnrichmentRatio(m_sedEroded[i], m_surfRf[i], m_cellArea);

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
        float* tmp_orgn2ch = new(nothrow) float[m_nSubbasins + 1];
        float* tmp_orgp2ch = new(nothrow) float[m_nSubbasins + 1];
        float* tmp_minpa2ch = new(nothrow) float[m_nSubbasins + 1];
        float* tmp_minps2ch = new(nothrow) float[m_nSubbasins + 1];
        for (int i = 0; i <= m_nSubbasins; i++) {
            tmp_orgn2ch[i] = 0.f;
            tmp_orgp2ch[i] = 0.f;
            tmp_minpa2ch[i] = 0.f;
            tmp_minps2ch[i] = 0.f;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            tmp_orgn2ch[CVT_INT(m_subbasin[i])] += m_sedorgn[i];
            tmp_orgp2ch[CVT_INT(m_subbasin[i])] += m_sedorgp[i];
            tmp_minpa2ch[CVT_INT(m_subbasin[i])] += m_sedminpa[i];
            tmp_minps2ch[CVT_INT(m_subbasin[i])] += m_sedminps[i];
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbasins; i++) {
                m_sedorgnToCh[i] += tmp_orgn2ch[i];
                m_sedorgpToCh[i] += tmp_orgp2ch[i];
                m_sedminpaToCh[i] += tmp_minpa2ch[i];
                m_sedminpsToCh[i] += tmp_minps2ch[i];
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
    for (int i = 1; i < m_nSubbasins + 1; i++) {
        m_sedorgnToCh[i] *= m_cellArea;
        m_sedorgpToCh[i] *= m_cellArea;
        m_sedminpaToCh[i] *= m_cellArea;
        m_sedminpsToCh[i] *= m_cellArea;
        m_sedorgnToCh[0] += m_sedorgnToCh[i];
        m_sedorgpToCh[0] += m_sedorgpToCh[i];
        m_sedminpaToCh[0] += m_sedminpaToCh[i];
        m_sedminpsToCh[0] += m_sedminpsToCh[i];
    }
    return 0;
}

void NutrientTransportSediment::OrgNRemovedInRunoffStaticMethod(int i) {
    //amount of organic N in first soil layer (orgninfl)
    float orgninfl = 0.f;
    //conversion factor (wt)
    float wt = 0.f;
    orgninfl = m_soilStabOrgN[i][0] + m_soilActvOrgN[i][0] + m_soilFrshOrgN[i][0];
    wt = m_soilBD[i][0] * m_soilThk[i][0] * 0.01f;
    //concentration of organic N in soil (concn)
    float concn = 0.f;
    concn = orgninfl * m_enratio[i] / wt;
    //Calculate the amount of organic nitrogen transported with sediment to the stream, equation 4:2.2.1 in SWAT Theory 2009, p271
    m_sedorgn[i] = 0.001f * concn * m_sedEroded[i] * 0.001f / m_cellArea; /// kg/ha
    //update soil nitrogen pools
    if (orgninfl > 1.e-6f) {
        m_soilActvOrgN[i][0] = m_soilActvOrgN[i][0] - m_sedorgn[i] * (m_soilActvOrgN[i][0] / orgninfl);
        m_soilStabOrgN[i][0] = m_soilStabOrgN[i][0] - m_sedorgn[i] * (m_soilStabOrgN[i][0] / orgninfl);
        m_soilFrshOrgN[i][0] = m_soilFrshOrgN[i][0] - m_sedorgn[i] * (m_soilFrshOrgN[i][0] / orgninfl);
        if (m_soilActvOrgN[i][0] < 0.f) {
            m_sedorgn[i] = m_sedorgn[i] + m_soilActvOrgN[i][0];
            m_soilActvOrgN[i][0] = 0.f;
        }
        if (m_soilStabOrgN[i][0] < 0.f) {
            m_sedorgn[i] = m_sedorgn[i] + m_soilStabOrgN[i][0];
            m_soilStabOrgN[i][0] = 0.f;
        }
        if (m_soilFrshOrgN[i][0] < 0.f) {
            m_sedorgn[i] = m_sedorgn[i] + m_soilFrshOrgN[i][0];
            m_soilFrshOrgN[i][0] = 0.f;
        }
    }
}

void NutrientTransportSediment::OrgNRemovedInRunoffCFarmOneCarbonModel(int i) {
    /// TODO
}

void NutrientTransportSediment::OrgNRemovedInRunoffCenturyModel(int i) {
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
    m_sedorgn[i] = 0.001f * conc * m_sedEroded[i] * 0.001f / m_cellArea;
    /// update soil nitrogen pools
    if (totOrgN_lyr0 > UTIL_ZERO) {
        float xx1 = 1.f - m_sedorgn[i] / totOrgN_lyr0;
        m_sol_LSN[i][0] *= xx1;
        m_sol_LMN[i][0] *= xx1;
        m_sol_HPN[i][0] *= xx1;
        m_sol_HSN[i][0] *= xx1;
    }
    /// Calculate runoff and leached C&N from micro-biomass
    /// total organic carbon in layer 1
    TOT = m_sol_HPC[i][0] + m_sol_HSC[i][0] + m_sol_LMC[i][0] + m_sol_LSC[i][0];
    /// fraction of soil erosion of total soil mass
    YEW = Min((m_sedEroded[i] / m_cellArea + YW / m_cellArea) / m_soilMass[i][0], 0.9f);

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
    m_sol_latC[i][0] = QBC * (m_subSurfRf[i][0] / (m_surfRf[i] + m_subSurfRf[i][0] + UTIL_ZERO));
    m_sol_percoC[i][0] = VBC;
    m_sedCLoss[i] = YOC + YBC;

    latc_clyr += m_sol_latC[i][0];
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
        m_sol_latC[i][k] = VBC * (m_subSurfRf[i][k] / (m_subSurfRf[i][k] + m_soilPerco[i][k] + UTIL_ZERO));
        m_sol_percoC[i][k] = VBC - m_sol_latC[i][k];
        m_sol_BMC[i][k] = Y1 - VBC;

        /// calculate nitrate in percolate and lateral flow
        perc_clyr += m_sol_percoC[i][k];
        latc_clyr += m_sol_latC[i][k];
    }
    m_laterC[i] = latc_clyr;
    m_percoC[i] = perc_clyr;
}

void NutrientTransportSediment::OrgPAttachedtoSed(int i) {
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
    float sedp = 1.e-6f * concp * m_sedEroded[i] / m_cellArea; /// kg/ha
    m_sedorgp[i] = sedp * sol_attp_o;
    m_sedminpa[i] = sedp * sol_attp_a;
    m_sedminps[i] = sedp * sol_attp_s;

    //if(i==100)cout << "sedp: " << sedp<< ",sol_attp_o: "  << sol_attp_o << endl;
    //modify phosphorus pools

    //total amount of P in mineral sediment pools prior to sediment removal (psedd)		// Not used
    //float psedd = 0.f;
    //psedd = m_sol_actp[i][0] + m_sol_stap[i][0];

    //total amount of P in organic pools prior to sediment removal (porgg)
    float porgg = 0.f;
    porgg = m_soilHumOrgP[i][0] + m_soilFrshOrgP[i][0];
    if (porgg > 1.e-3f) {
        m_soilHumOrgP[i][0] = m_soilHumOrgP[i][0] - m_sedorgp[i] * (m_soilHumOrgP[i][0] / porgg);
        m_soilFrshOrgP[i][0] = m_soilFrshOrgP[i][0] - m_sedorgp[i] * (m_soilFrshOrgP[i][0] / porgg);
    }
    m_soilActvMinP[i][0] = m_soilActvMinP[i][0] - m_sedminpa[i];
    m_soilStabMinP[i][0] = m_soilStabMinP[i][0] - m_sedminps[i];
    if (m_soilHumOrgP[i][0] < 0.f) {
        m_sedorgp[i] = m_sedorgp[i] + m_soilHumOrgP[i][0];
        m_soilHumOrgP[i][0] = 0.f;
    }
    if (m_soilFrshOrgP[i][0] < 0.f) {
        m_sedorgp[i] = m_sedorgp[i] + m_soilFrshOrgP[i][0];
        m_soilFrshOrgP[i][0] = 0.f;
    }
    if (m_soilActvMinP[i][0] < 0.f) {
        m_sedminpa[i] = m_sedminpa[i] + m_soilActvMinP[i][0];
        m_soilActvMinP[i][0] = 0.f;
    }
    if (m_soilStabMinP[i][0] < 0.f) {
        m_sedminps[i] = m_sedminps[i] + m_soilStabMinP[i][0];
        m_soilStabMinP[i][0] = 0.f;
    }
}

void NutrientTransportSediment::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SEDORGN)) {
        *data = m_sedorgn;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDORGP)) {
        *data = m_sedorgp;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDMINPA)) {
        *data = m_sedminpa;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDMINPS)) {
        *data = m_sedminps;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDORGN_TOCH)) {
        *data = m_sedorgnToCh;
        *n = m_nSubbasins + 1;
    } else if (StringMatch(sk, VAR_SEDORGP_TOCH)) {
        *data = m_sedorgpToCh;
        *n = m_nSubbasins + 1;
    } else if (StringMatch(sk, VAR_SEDMINPA_TOCH)) {
        *data = m_sedminpaToCh;
        *n = m_nSubbasins + 1;
    } else if (StringMatch(sk, VAR_SEDMINPS_TOCH)) {
        *data = m_sedminpsToCh;
        *n = m_nSubbasins + 1;
    }
        /// outputs of CENTURY C/N cycling model
    else if (StringMatch(sk, VAR_LATERAL_C)) {
        *data = m_laterC;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_PERCO_C)) {
        *data = m_percoC;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDLOSS_C)) {
        *data = m_sedCLoss;
        *n = m_nCells;
    } else {
        throw ModelException(MID_NUTRSED, "Get1DData", "Parameter " + sk + " does not exist");
    }
}

void NutrientTransportSediment::Get2DData(const char* key, int* nRows, int* nCols, float*** data) {
    InitialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_maxSoilLyrs;
    if (StringMatch(sk, VAR_SOL_AORGN)) *data = m_soilActvOrgN;
    else if (StringMatch(sk, VAR_SOL_FORGN)) *data = m_soilFrshOrgN;
    else if (StringMatch(sk, VAR_SOL_SORGN)) *data = m_soilStabOrgN;
    else if (StringMatch(sk, VAR_SOL_HORGP)) *data = m_soilHumOrgP;
    else if (StringMatch(sk, VAR_SOL_FORGP)) *data = m_soilFrshOrgP;
    else if (StringMatch(sk, VAR_SOL_STAP)) *data = m_soilStabMinP;
    else if (StringMatch(sk, VAR_SOL_ACTP)) *data = m_soilActvMinP;
        /// outputs of CENTURY C/N cycling model
    else if (StringMatch(sk, VAR_SOL_LATERAL_C)) *data = m_sol_latC;
    else if (StringMatch(sk, VAR_SOL_PERCO_C)) *data = m_sol_percoC;
    else {
        throw ModelException(MID_NUTRSED, "Get2DData", "Output " + sk + " does not exist.");
    }
}
