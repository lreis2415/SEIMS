#include "SOL_WB.h"

#include "text.h"

SOL_WB::SOL_WB() : m_nCells(-1), m_nSoilLayers(-1), m_soilLayers(nullptr), m_soilThick(nullptr), m_soilZMX(nullptr),
                   m_pNet(nullptr), m_Infil(nullptr), m_ES(nullptr), m_Revap(nullptr),
                   m_RI(nullptr), m_Perco(nullptr), m_soilStorage(nullptr),
                   m_subbasinsInfo(nullptr), m_nSubbasins(-1),
                   m_PCP(nullptr), m_Interc(nullptr), m_EI(nullptr), m_Dep(nullptr), m_ED(nullptr),
                   m_RS(nullptr), m_RG(nullptr), m_SE(nullptr), m_tMean(nullptr), m_SoilT(nullptr),
                   m_soilWaterBalance(nullptr) {
}

SOL_WB::~SOL_WB() {
    if (m_soilWaterBalance != nullptr) Release2DArray(m_nSubbasins + 1, m_soilWaterBalance);
}

void SOL_WB:: InitialOutputs() {
    CHECK_POSITIVE(MID_SOL_WB, m_nSubbasins);
    if (m_soilWaterBalance == nullptr) Initialize2DArray(m_nSubbasins + 1, 16, m_soilWaterBalance, 0.f);
}

int SOL_WB::Execute() {
    CheckInputData();
    return 0;
}

void SOL_WB::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, VAR_SUBBSNID_NUM)) { m_nSubbasins = int(data); }
    else {
        throw ModelException(MID_SOL_WB, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void SOL_WB::SetValueToSubbasins() {
    if (m_subbasinsInfo != nullptr) {
        for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); it++) {
            Subbasin *curSub = m_subbasinsInfo->GetSubbasinByID(*it);
            int *cells = curSub->GetCells();
            int cellsNum = curSub->GetCellCount();
            float ri = 0.f; // total subsurface runoff of soil profile (mm)
            float sm = 0.f; // total soil moisture of soil profile (mm)
            float pcp = 0.f, netPcp = 0.f, depET = 0.f, infil = 0.f;
            float itpET = 0.f, netPerc = 0.f, r = 0.f, revap = 0.f;
            float rs = 0.f, meanT = 0.f, soilT = 0.f, es = 0.f, totalET = 0.f;

            for (int i = 0; i < cellsNum; i++) // loop cells of current subbasin
            {
                int cell = cells[i];
                ri = 0.f;
                sm = 0.f;
                for (int j = 0; j < (int) m_soilLayers[cell]; j++) {
                    ri += m_RI[cell][j] / float(cellsNum);
                    sm += m_soilStorage[cell][j] / float(cellsNum);
                }
                pcp += m_PCP[cell] / float(cellsNum);
                meanT += m_tMean[cell] / float(cellsNum);
                soilT += m_SoilT[cell] / float(cellsNum);
                netPcp += m_pNet[cell] / float(cellsNum);
                itpET += m_EI[cell] / float(cellsNum);
                depET += m_ED[cell] / float(cellsNum);
                infil += m_Infil[cell] / float(cellsNum);
                totalET += (m_EI[cell] + m_ED[cell] + m_ES[cell]) / float(cellsNum); // add plant et?
                es += m_ES[cell] / float(cellsNum);
                netPerc += (m_Perco[cell][(int) m_soilLayers[cell] - 1] - m_Revap[cell]) / float(cellsNum);
                revap += m_Revap[cell] / float(cellsNum);
                rs += m_RS[cell] / float(cellsNum);
                r += (m_RS[cell] + ri) / float(cellsNum);
            }
            float rg = m_RG[*it];
            r += rg;

            m_soilWaterBalance[*it][0] = pcp;
            m_soilWaterBalance[*it][1] = meanT;
            m_soilWaterBalance[*it][2] = soilT;
            m_soilWaterBalance[*it][3] = netPcp;
            m_soilWaterBalance[*it][4] = itpET;
            m_soilWaterBalance[*it][5] = depET;
            m_soilWaterBalance[*it][6] = infil;
            m_soilWaterBalance[*it][7] = totalET;
            m_soilWaterBalance[*it][8] = es;
            m_soilWaterBalance[*it][9] = netPerc;
            m_soilWaterBalance[*it][10] = revap;
            m_soilWaterBalance[*it][11] = rs;
            m_soilWaterBalance[*it][12] = ri;
            m_soilWaterBalance[*it][13] = rg;
            m_soilWaterBalance[*it][14] = r;
            m_soilWaterBalance[*it][15] = sm;
        }
    }
}

void SOL_WB::Set1DData(const char *key, int nRows, float *data) {
    string s(key);
    if (StringMatch(s, VAR_RG)) {
        m_RG = data;
        if (m_nSubbasins != nRows - 1) {
            throw ModelException(MID_SOL_WB, "Set1DData",
                                 "The size of groundwater runoff should be equal to (subbasin number + 1)!");
        }
        return;
    }
    CheckInputSize(key, nRows);
    if (StringMatch(s, VAR_SOILLAYERS)) {
        m_soilLayers = data;
    } else if (StringMatch(s, VAR_SOL_ZMX)) {
        m_soilZMX = data;
    } else if (StringMatch(s, VAR_NEPR)) {
        m_pNet = data;
    } else if (StringMatch(s, VAR_INFIL)) {
        m_Infil = data;
    } else if (StringMatch(s, VAR_SOET)) {
        m_ES = data;
    } else if (StringMatch(s, VAR_REVAP)) {
        m_Revap = data;
    } else if (StringMatch(s, VAR_PCP)) {
        m_PCP = data;
    } else if (StringMatch(s, VAR_INLO)) {
        m_Interc = data;
    } else if (StringMatch(s, VAR_INET)) {
        m_EI = data;
    } else if (StringMatch(s, VAR_DEET)) {
        m_ED = data;
    } else if (StringMatch(s, VAR_DPST)) {
        m_Dep = data;
    } else if (StringMatch(s, VAR_SURU)) {
        m_RS = data;
    } else if (StringMatch(s, VAR_SNSB)) {
        m_SE = data;
    } else if (StringMatch(s, VAR_TMEAN)) {
        m_tMean = data;
    } else if (StringMatch(s, VAR_SOTE)) {
        m_SoilT = data;
    } else {
        throw ModelException(MID_SOL_WB, "Set1DData", "Parameter " + s + " does not exist in current module.");
    }
}

void SOL_WB::Set2DData(const char *key, int nrows, int ncols, float **data) {
    CheckInputSize(key, nrows);
    string s(key);
    m_nSoilLayers = ncols;
    if (StringMatch(s, VAR_PERCO)) {
        m_Perco = data;
    } else if (StringMatch(s, VAR_SSRU)) {
        m_RI = data;
    } else if (StringMatch(s, VAR_SOL_ST)) {
        m_soilStorage = data;
    } else if (StringMatch(s, VAR_SOILTHICK)) {
        m_soilThick = data;
    } else {
        throw ModelException(MID_SOL_WB, "Set2DData", "Parameter " + s + " does not exist in current module.");
    }
}

void SOL_WB::SetSubbasins(clsSubbasins *subbasins) {
    if (m_subbasinsInfo == nullptr) {
        m_subbasinsInfo = subbasins;
        // m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber(); // Set in SetValue()
        m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
    }
}

void SOL_WB::Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
     InitialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SOWB)) {
        SetValueToSubbasins();
        *nRows = m_nSubbasins + 1;
        *nCols = 16;
        *data = m_soilWaterBalance;
    } else {
        throw ModelException(MID_SOL_WB, "Get2DData", "Result " + s + " does not exist in current module.");
    }
}

void SOL_WB::CheckInputData() {
    CHECK_POSITIVE(MID_SOL_WB, m_nCells);
    CHECK_POSITIVE(MID_SOL_WB, m_nSubbasins);
    CHECK_POINTER(MID_SOL_WB, m_soilLayers);
    CHECK_POINTER(MID_SOL_WB, m_soilZMX);
    CHECK_POINTER(MID_SOL_WB, m_soilThick);
    CHECK_POINTER(MID_SOL_WB, m_pNet);
    CHECK_POINTER(MID_SOL_WB, m_Infil);
    CHECK_POINTER(MID_SOL_WB, m_ES);
    CHECK_POINTER(MID_SOL_WB, m_Revap);
    CHECK_POINTER(MID_SOL_WB, m_RI);
    CHECK_POINTER(MID_SOL_WB, m_Perco);
    CHECK_POINTER(MID_SOL_WB, m_soilStorage);
    CHECK_POINTER(MID_SOL_WB, m_PCP);
    CHECK_POINTER(MID_SOL_WB, m_Interc);
    CHECK_POINTER(MID_SOL_WB, m_Dep);
    CHECK_POINTER(MID_SOL_WB, m_ED);
    CHECK_POINTER(MID_SOL_WB, m_EI);
    CHECK_POINTER(MID_SOL_WB, m_RG);
    CHECK_POINTER(MID_SOL_WB, m_RS);
    CHECK_POINTER(MID_SOL_WB, m_tMean);
    CHECK_POINTER(MID_SOL_WB, m_SoilT);
    CHECK_POINTER(MID_SOL_WB, m_subbasinsInfo);
}

bool SOL_WB::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_SOL_WB, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_SOL_WB, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}
