#include "SOL_WB.h"

#include "text.h"

SOL_WB::SOL_WB() :
    m_nCells(-1), m_maxSoilLyrs(-1), m_nSoilLyrs(nullptr), m_soilThk(nullptr), m_soilMaxRootD(nullptr),
    m_netPcp(nullptr), m_infil(nullptr), m_soilET(nullptr), m_Revap(nullptr),
    m_subSurfRf(nullptr), m_soilPerco(nullptr), m_soilWtrSto(nullptr),
    m_PCP(nullptr), m_intcpLoss(nullptr),
    m_IntcpET(nullptr), m_deprSto(nullptr), m_deprStoET(nullptr), m_surfRf(nullptr), m_RG(nullptr),
    m_snowSublim(nullptr), m_meanTemp(nullptr), m_soilTemp(nullptr), m_nSubbsns(-1), m_subbasinsInfo(nullptr),
    m_soilWtrBal(nullptr) {
}

SOL_WB::~SOL_WB() {
    if (m_soilWtrBal != nullptr) Release2DArray(m_soilWtrBal);
}

void SOL_WB::InitialOutputs() {
    CHECK_POSITIVE(M_SOL_WB[0], m_nSubbsns);
    if (m_soilWtrBal == nullptr) Initialize2DArray(m_nSubbsns + 1, 16, m_soilWtrBal, 0.f);
}

int SOL_WB::Execute() {
    CheckInputData();
    return 0;
}

void SOL_WB::SetValue(const char* key, float value) {
    string s(key);
    if (StringMatch(s, VAR_SUBBSNID_NUM[0])) m_nSubbsns = CVT_INT(value);
    else {
        throw ModelException(M_SOL_WB[0], "SetValue", "Parameter " + s + " does not exist.");
    }
}

void SOL_WB::SetValueToSubbasins() {
    if (m_subbasinsInfo != nullptr) {
        for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); ++it) {
            Subbasin* curSub = m_subbasinsInfo->GetSubbasinByID(*it);
            int* cells = curSub->GetCells();
            int cellsNum = curSub->GetCellCount();
            float ratio = 1.f / CVT_FLT(cellsNum);
            float ri = 0.f; // total subsurface runoff of soil profile (mm)
            float sm = 0.f; // total soil moisture of soil profile (mm)
            float pcp = 0.f, netPcp = 0.f, depET = 0.f, infil = 0.f;
            float itpET = 0.f, netPerc = 0.f, r = 0.f, revap = 0.f;
            float rs = 0.f, meanT = 0.f, soilT = 0.f, es = 0.f, totalET = 0.f;

            for (int i = 0; i < cellsNum; i++) {
                // loop cells of current subbasin
                int cell = cells[i];
                ri = 0.f;
                sm = 0.f;
                for (int j = 0; j < CVT_INT(m_nSoilLyrs[cell]); j++) {
                    ri += m_subSurfRf[cell][j] * ratio;
                    sm += m_soilWtrSto[cell][j] * ratio;
                }
                pcp += m_PCP[cell] * ratio;
                meanT += m_meanTemp[cell] * ratio;
                soilT += m_soilTemp[cell] * ratio;
                netPcp += m_netPcp[cell] * ratio;
                itpET += m_IntcpET[cell] * ratio;
                depET += m_deprStoET[cell] * ratio;
                infil += m_infil[cell] * ratio;
                totalET += (m_IntcpET[cell] + m_deprStoET[cell] + m_soilET[cell]) * ratio; // add plant et?
                es += m_soilET[cell] * ratio;
                netPerc += (m_soilPerco[cell][CVT_INT(m_nSoilLyrs[cell]) - 1] - m_Revap[cell]) * ratio;
                revap += m_Revap[cell] * ratio;
                rs += m_surfRf[cell] * ratio;
                r += (m_surfRf[cell] + ri) * ratio;
            }
            float rg = m_RG[*it];
            r += rg;

            m_soilWtrBal[*it][0] = pcp;
            m_soilWtrBal[*it][1] = meanT;
            m_soilWtrBal[*it][2] = soilT;
            m_soilWtrBal[*it][3] = netPcp;
            m_soilWtrBal[*it][4] = itpET;
            m_soilWtrBal[*it][5] = depET;
            m_soilWtrBal[*it][6] = infil;
            m_soilWtrBal[*it][7] = totalET;
            m_soilWtrBal[*it][8] = es;
            m_soilWtrBal[*it][9] = netPerc;
            m_soilWtrBal[*it][10] = revap;
            m_soilWtrBal[*it][11] = rs;
            m_soilWtrBal[*it][12] = ri;
            m_soilWtrBal[*it][13] = rg;
            m_soilWtrBal[*it][14] = r;
            m_soilWtrBal[*it][15] = sm;
        }
    }
}

void SOL_WB::Set1DData(const char* key, const int nrows, float* data) {
    string s(key);
    if (StringMatch(s, VAR_RG[0])) {
        m_RG = data;
        if (m_nSubbsns != nrows - 1) {
            throw ModelException(M_SOL_WB[0], "Set1DData",
                                 "The size of groundwater runoff should be equal to (subbasin number + 1)!");
        }
        return;
    }
    CheckInputSize(M_SOL_WB[0], key, nrows, m_nCells);
    if (StringMatch(s, VAR_SOILLAYERS[0])) {
        m_nSoilLyrs = data;
    } else if (StringMatch(s, VAR_SOL_ZMX[0])) {
        m_soilMaxRootD = data;
    } else if (StringMatch(s, VAR_NEPR[0])) {
        m_netPcp = data;
    } else if (StringMatch(s, VAR_INFIL[0])) {
        m_infil = data;
    } else if (StringMatch(s, VAR_SOET[0])) {
        m_soilET = data;
    } else if (StringMatch(s, VAR_REVAP[0])) {
        m_Revap = data;
    } else if (StringMatch(s, VAR_PCP[0])) {
        m_PCP = data;
    } else if (StringMatch(s, VAR_INLO[0])) {
        m_intcpLoss = data;
    } else if (StringMatch(s, VAR_INET[0])) {
        m_IntcpET = data;
    } else if (StringMatch(s, VAR_DEET[0])) {
        m_deprStoET = data;
    } else if (StringMatch(s, VAR_DPST[0])) {
        m_deprSto = data;
    } else if (StringMatch(s, VAR_SURU[0])) {
        m_surfRf = data;
    } else if (StringMatch(s, VAR_SNSB[0])) {
        m_snowSublim = data;
    } else if (StringMatch(s, VAR_TMEAN[0])) {
        m_meanTemp = data;
    } else if (StringMatch(s, VAR_SOTE[0])) {
        m_soilTemp = data;
    } else {
        throw ModelException(M_SOL_WB[0], "Set1DData", "Parameter " + s + " does not exist in current module.");
    }
}

void SOL_WB::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    CheckInputSize2D(M_SOL_WB[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    string s(key);
    if (StringMatch(s, VAR_PERCO[0])) {
        m_soilPerco = data;
    } else if (StringMatch(s, VAR_SSRU[0])) {
        m_subSurfRf = data;
    } else if (StringMatch(s, VAR_SOL_ST[0])) {
        m_soilWtrSto = data;
    } else if (StringMatch(s, VAR_SOILTHICK[0])) {
        m_soilThk = data;
    } else {
        throw ModelException(M_SOL_WB[0], "Set2DData", "Parameter " + s + " does not exist in current module.");
    }
}

void SOL_WB::SetSubbasins(clsSubbasins* subbasins) {
    if (m_subbasinsInfo == nullptr) {
        m_subbasinsInfo = subbasins;
        // m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber(); // Set in SetValue()
        m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
    }
}

void SOL_WB::Get2DData(const char* key, int* nRows, int* nCols, float*** data) {
    InitialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SOWB[0])) {
        SetValueToSubbasins();
        *nRows = m_nSubbsns + 1;
        *nCols = 16;
        *data = m_soilWtrBal;
    } else {
        throw ModelException(M_SOL_WB[0], "Get2DData", "Result " + s + " does not exist in current module.");
    }
}

bool SOL_WB::CheckInputData() {
    CHECK_POSITIVE(M_SOL_WB[0], m_nCells);
    CHECK_POSITIVE(M_SOL_WB[0], m_nSubbsns);
    CHECK_POINTER(M_SOL_WB[0], m_nSoilLyrs);
    CHECK_POINTER(M_SOL_WB[0], m_soilMaxRootD);
    CHECK_POINTER(M_SOL_WB[0], m_soilThk);
    CHECK_POINTER(M_SOL_WB[0], m_netPcp);
    CHECK_POINTER(M_SOL_WB[0], m_infil);
    CHECK_POINTER(M_SOL_WB[0], m_soilET);
    CHECK_POINTER(M_SOL_WB[0], m_Revap);
    CHECK_POINTER(M_SOL_WB[0], m_subSurfRf);
    CHECK_POINTER(M_SOL_WB[0], m_soilPerco);
    CHECK_POINTER(M_SOL_WB[0], m_soilWtrSto);
    CHECK_POINTER(M_SOL_WB[0], m_PCP);
    CHECK_POINTER(M_SOL_WB[0], m_intcpLoss);
    CHECK_POINTER(M_SOL_WB[0], m_deprSto);
    CHECK_POINTER(M_SOL_WB[0], m_deprStoET);
    CHECK_POINTER(M_SOL_WB[0], m_IntcpET);
    CHECK_POINTER(M_SOL_WB[0], m_RG);
    CHECK_POINTER(M_SOL_WB[0], m_surfRf);
    CHECK_POINTER(M_SOL_WB[0], m_meanTemp);
    CHECK_POINTER(M_SOL_WB[0], m_soilTemp);
    CHECK_POINTER(M_SOL_WB[0], m_subbasinsInfo);
    return true;
}
