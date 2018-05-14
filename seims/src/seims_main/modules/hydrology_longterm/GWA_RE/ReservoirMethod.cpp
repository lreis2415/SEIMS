#include "ReservoirMethod.h"

#include "text.h"

ReservoirMethod::ReservoirMethod() : m_TimeStep(-1), m_nCells(-1), m_CellWidth(NODATA_VALUE), m_nMaxSoilLayers(-1),
                                     m_soilLayers(nullptr), m_soilThick(nullptr),
                                     m_dp_co(NODATA_VALUE), m_Kg(NODATA_VALUE), m_Base_ex(NODATA_VALUE),
                                     m_perc(nullptr), m_D_EI(nullptr), m_D_ED(nullptr),
                                     m_D_ES(nullptr), m_plantEP(nullptr),
                                     m_D_PET(nullptr), m_GW0(NODATA_VALUE), m_GWMAX(NODATA_VALUE),
                                     m_petSubbasin(nullptr), m_gwStore(nullptr), m_Slope(nullptr), m_soilStorage(nullptr),
                                     m_soilDepth(nullptr),
                                     m_VgroundwaterFromBankStorage(nullptr), m_T_Perco(nullptr),
                                     /// intermediate
                                     m_T_PerDep(nullptr), m_T_RG(nullptr),
                                     /// outputs
                                     m_T_QG(nullptr), m_D_Revap(nullptr), m_T_Revap(nullptr), m_T_GWWB(nullptr),
                                     m_nSubbasins(-1),
                                     m_subbasinID(-1), m_firstRun(true), m_subbasinsInfo(nullptr) {
}

ReservoirMethod::~ReservoirMethod() {
    if (m_T_Perco != nullptr) Release1DArray(m_T_Perco);
    if (m_T_PerDep != nullptr) Release1DArray(m_T_PerDep);
    if (m_D_Revap != nullptr) Release1DArray(m_D_Revap);
    if (m_T_Revap != nullptr) Release1DArray(m_T_Revap);
    if (m_T_RG != nullptr) Release1DArray(m_T_RG);
    if (m_T_QG != nullptr) Release1DArray(m_T_QG);
    if (m_petSubbasin != nullptr) Release1DArray(m_petSubbasin);
    if (m_gwStore != nullptr) Release1DArray(m_gwStore);
    if (m_T_GWWB != nullptr) Release2DArray(m_nSubbasins + 1, m_T_GWWB);
}

void ReservoirMethod:: InitialOutputs() {
    if (m_firstRun) {
        SetSubbasinInfos();
        m_firstRun = false;
    }
    CHECK_POSITIVE(MID_GWA_RE, m_nSubbasins);
    int nLen = m_nSubbasins + 1;
    if (m_T_Perco == nullptr) Initialize1DArray(nLen, m_T_Perco, 0.f);
    if (m_T_Revap == nullptr) Initialize1DArray(nLen, m_T_Revap, 0.f);
    if (m_T_PerDep == nullptr) Initialize1DArray(nLen, m_T_PerDep, 0.f);
    if (m_T_RG == nullptr) Initialize1DArray(nLen, m_T_RG, 0.f);
    if (m_T_QG == nullptr) Initialize1DArray(nLen, m_T_QG, 0.f);
    if (m_petSubbasin == nullptr) Initialize1DArray(nLen, m_petSubbasin, 0.f);
    if (m_gwStore == nullptr) Initialize1DArray(nLen, m_gwStore, m_GW0);
    if (m_D_Revap == nullptr) Initialize1DArray(m_nCells, m_D_Revap, 0.f);
    if (m_T_GWWB == nullptr) Initialize2DArray(nLen, 6, m_T_GWWB, 0.f);
}

int ReservoirMethod::Execute() {
    CheckInputData();
    InitialOutputs();
    float QGConvert = 1.f * m_CellWidth * m_CellWidth / m_TimeStep * 0.001f; // mm ==> m3/s
    for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); ++it) {
        int subID = *it;
        Subbasin* curSub = m_subbasinsInfo->GetSubbasinByID(subID);

        // get percolation from the bottom soil layer at the subbasin scale
        int curCellsNum = curSub->GetCellCount();
        int* curCells = curSub->GetCells();
        float perco = 0.f;
#pragma omp parallel for reduction(+:perco)
        for (int i = 0; i < curCellsNum; i++) {
            int index = 0;
            index = curCells[i];
            float tmp_perc = m_perc[index][CVT_INT(m_soilLayers[index]) - 1];
            if (tmp_perc > 0) perco += tmp_perc;
            else m_perc[index][CVT_INT(m_soilLayers[index]) - 1] = 0.f;
        }
        perco /= curCellsNum; // mean mm
        /// percolated water ==> vadose zone ==> shallow aquifer ==> deep aquifer
        /// currently, for convenience, we assume a small portion of the percolated water
        /// will enter groundwater. By LJ. 2016-9-2
        float ratio2gw = 1.f;
        perco *= ratio2gw;
        curSub->SetPerco(perco);
        //calculate EG, i.e. Revap
        float revap = 0.f;
        float fPET = 0.f;
        float fEI = 0.f;
        float fED = 0.f;
        float fES = 0.f;
        float plantEP = 0.f;
        fPET = Sum(curCellsNum, curCells, m_D_PET) / curCellsNum;
        fEI = Sum(curCellsNum, curCells, m_D_EI) / curCellsNum;
        fED = Sum(curCellsNum, curCells, m_D_ED) / curCellsNum;
        fES = Sum(curCellsNum, curCells, m_D_ES) / curCellsNum;
        plantEP = Sum(curCellsNum, curCells, m_plantEP) / curCellsNum;

        curSub->SetPet(fPET);

        //if percolation < 0.01, EG will be 0. if percolation >= 0.01, EG will be calculated by equation (why? this is not used currently. Junzhi Liu 2016-08-14).
        //if (perco >= 0.01f)
        //{
        revap = (fPET - fEI - fED - fES - plantEP) * m_gwStore[subID] / m_GWMAX;
        if (revap != revap) {
            cout << "fPET: " << fPET << ", fEI: " << fEI << ", fED: " << fED << ", fES: " << fES << ", plantEP: "
                    << plantEP << ", " << " subbasin ID: " << subID << ", gwStore: " << m_gwStore[subID] << endl;
            throw ModelException(MID_GWA_RE, "Execute", "revap calculation wrong!");
        }
        revap = Max(revap, 0.f);
        revap = Min(revap, perco);
        //}
        //float prevRevap = curSub->getEG();
        //if (prevRevap != revap)
        //{
        //	curSub->setEG(revap);
        //	curSub->setIsRevapChanged(true);
        //}
        //else
        //	curSub->setIsRevapChanged(false);
        curSub->SetEg(revap);

        //deep percolation
        float percoDeep = perco * m_dp_co;
        curSub->SetPerde(percoDeep);

        // groundwater runoff (mm)
        float slopeCoef = curSub->GetSlopeCoef();
        float kg = m_Kg * slopeCoef;
        float groundRunoff = kg * pow(m_gwStore[subID], m_Base_ex); //mm
        if (groundRunoff != groundRunoff) {
            cout << groundRunoff;
        }

        float groundQ = groundRunoff * curCellsNum * QGConvert; // groundwater discharge (m3/s)

        float groundStorage = m_gwStore[subID];
        groundStorage += (perco - revap - percoDeep - groundRunoff);

        //add the ground water from bank storage, 2011-3-14
        float gwBank = 0.f;
        // at the first time step m_VgroundwaterFromBankStorage is nullptr
        if (m_VgroundwaterFromBankStorage != nullptr) {
            gwBank = m_VgroundwaterFromBankStorage[subID];
        }
        groundStorage += gwBank / curSub->GetArea() * 1000.f;

        groundStorage = Max(groundStorage, 0.f);
        if (groundStorage > m_GWMAX) {
            groundRunoff += (groundStorage - m_GWMAX);
            groundQ = groundRunoff * curCellsNum * QGConvert; // groundwater discharge (m3/s)
            groundStorage = m_GWMAX;
        }
        curSub->SetRg(groundRunoff);
        curSub->SetGw(groundStorage);
        curSub->SetQg(groundQ);
        if (groundStorage != groundStorage) {
            std::ostringstream oss;
            oss << perco << "\t" << revap << "\t" << percoDeep << "\t" << groundRunoff << "\t" << m_gwStore[subID]
                    << "\t" << m_Kg << "\t" <<
                    m_Base_ex << "\t" << slopeCoef << endl;
            throw ModelException("Subbasin", "setInputs", oss.str());
        }
        m_T_Perco[subID] = curSub->GetPerco();
        m_T_Revap[subID] = curSub->GetEg();
        m_T_PerDep[subID] = curSub->GetPerde();
        m_T_RG[subID] = curSub->GetRg(); //get rg of specific subbasin
        m_T_QG[subID] = curSub->GetQg(); //get qg of specific subbasin
        m_petSubbasin[subID] = curSub->GetPet();
        m_gwStore[subID] = curSub->GetGw();
    }

    m_T_Perco[0] = m_subbasinsInfo->Subbasin2Basin(VAR_PERCO);
    m_T_Revap[0] = m_subbasinsInfo->Subbasin2Basin(VAR_REVAP);
    m_T_PerDep[0] = m_subbasinsInfo->Subbasin2Basin(VAR_PERDE);
    m_T_RG[0] = m_subbasinsInfo->Subbasin2Basin(VAR_RG); // get rg of entire watershed
    m_gwStore[0] = m_subbasinsInfo->Subbasin2Basin(VAR_GW_Q);
    m_T_QG[0] = m_subbasinsInfo->Subbasin2Basin(VAR_QG); // get qg of entire watershed

    // output to GWWB
    for (int i = 0; i <= m_nSubbasins; i++) {
        m_T_GWWB[i][0] = m_T_Perco[i];
        m_T_GWWB[i][1] = m_T_Revap[i];
        m_T_GWWB[i][2] = m_T_PerDep[i];
        m_T_GWWB[i][3] = m_T_RG[i];
        m_T_GWWB[i][4] = m_gwStore[i];
        m_T_GWWB[i][5] = m_T_QG[i];
    }

    // update soil moisture
    for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); ++it) {
        Subbasin* sub = m_subbasinsInfo->GetSubbasinByID(*it);
        int* cells = sub->GetCells();
        int nCells = sub->GetCellCount();
        int index = 0;
        for (int i = 0; i < nCells; i++) {
            index = cells[i];
            m_soilStorage[index][static_cast<int>(m_soilLayers[index]) - 1] += sub->GetEg();
            // TODO: Is it need to allocate revap to each soil layers??? By LJ
        }
    }
    // DEBUG
    //cout << "GWA_RE, cell id 17842, m_soilStorage: ";
    //for (int i = 0; i < (int)m_soilLayers[17842]; i++)
    //    cout << m_soilStorage[17842][i] << ", ";
    //cout << endl;
    // END OF DEBUG
    return 0;
}

bool ReservoirMethod::CheckInputData() {
    CHECK_POSITIVE(MID_GWA_RE, m_nCells);
    CHECK_POSITIVE(MID_GWA_RE, m_nSubbasins);
    CHECK_POSITIVE(MID_GWA_RE, m_CellWidth);
    CHECK_POSITIVE(MID_GWA_RE, m_TimeStep);
    CHECK_POSITIVE(MID_GWA_RE, m_nMaxSoilLayers);
    CHECK_NODATA(MID_GWA_RE, m_dp_co);
    CHECK_NODATA(MID_GWA_RE, m_Kg);
    CHECK_NODATA(MID_GWA_RE, m_Base_ex);
    CHECK_POINTER(MID_GWA_RE, m_perc);
    CHECK_POINTER(MID_GWA_RE, m_D_EI);
    CHECK_POINTER(MID_GWA_RE, m_D_ED);
    CHECK_POINTER(MID_GWA_RE, m_D_ES);
    CHECK_POINTER(MID_GWA_RE, m_plantEP);
    CHECK_POINTER(MID_GWA_RE, m_D_PET);
    CHECK_POINTER(MID_GWA_RE, m_Slope);
    CHECK_POINTER(MID_GWA_RE, m_soilStorage);
    CHECK_POINTER(MID_GWA_RE, m_soilLayers);
    CHECK_POINTER(MID_GWA_RE, m_soilThick);
    CHECK_POINTER(MID_GWA_RE, m_subbasinsInfo);
    return true;
}

bool ReservoirMethod::CheckInputSize(const char* key, const int n) {
    if (n <= 0) {
        throw ModelException(MID_GWA_RE, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException(MID_GWA_RE, "CheckInputSize", "Input data for " + string(key) +
                                 " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

// set value
void ReservoirMethod::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_TimeStep)) m_TimeStep = CVT_INT(value);
    else if (StringMatch(sk, VAR_SUBBSNID_NUM)) m_nSubbasins = CVT_INT(value);
    else if (StringMatch(sk, Tag_SubbasinId)) m_subbasinID = CVT_INT(value);
    else if (StringMatch(sk, Tag_CellWidth)) m_CellWidth = value;
    else if (StringMatch(sk, VAR_KG)) m_Kg = value;
    else if (StringMatch(sk, VAR_Base_ex)) m_Base_ex = value;
    else if (StringMatch(sk, VAR_DF_COEF)) m_dp_co = value;
    else if (StringMatch(sk, VAR_GW0)) m_GW0 = value;
    else if (StringMatch(sk, VAR_GWMAX)) m_GWMAX = value;
    else {
        throw ModelException(MID_GWA_RE, "SetValue", "Parameter " + sk + " does not exist in current module.");
    }
}

void ReservoirMethod::Set1DData(const char* key, const int n, float* data) {
    string sk(key);
    if (StringMatch(sk, VAR_GWNEW)) {
        m_VgroundwaterFromBankStorage = data;
        return;
    }
    //check the input data
    if (!CheckInputSize(key, n)) return;

    //set the value
    if (StringMatch(sk, VAR_INET)) {
        m_D_EI = data;
    } else if (StringMatch(sk, VAR_DEET)) {
        m_D_ED = data;
    } else if (StringMatch(sk, VAR_SOET)) {
        m_D_ES = data;
    } else if (StringMatch(sk, VAR_AET_PLT)) {
        m_plantEP = data;
    } else if (StringMatch(sk, VAR_PET)) {
        m_D_PET = data;
    } else if (StringMatch(sk, VAR_SLOPE)) {
        m_Slope = data;
    } else if (StringMatch(sk, VAR_SOILLAYERS)) {
        m_soilLayers = data;
    } else {
        throw ModelException(MID_GWA_RE, "Set1DData", "Parameter " + sk + " does not exist in current module.");
    }
}

void ReservoirMethod::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    string sk(key);
    CheckInputSize(key, nrows);
    m_nMaxSoilLayers = ncols;

    if (StringMatch(sk, VAR_PERCO)) {
        m_perc = data;
    } else if (StringMatch(sk, VAR_SOL_ST)) {
        m_soilStorage = data;
    } else if (StringMatch(sk, VAR_SOILDEPTH)) {
        m_soilDepth = data;
    } else if (StringMatch(sk, VAR_SOILTHICK)) {
        m_soilThick = data;
    } else {
        throw ModelException(MID_GWA_RE, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void ReservoirMethod::SetSubbasins(clsSubbasins* subbasins) {
    if (m_subbasinsInfo == nullptr) {
        m_subbasinsInfo = subbasins;
        // m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber(); // Set in SetValue()! lj
        m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
    }
}

void ReservoirMethod::GetValue(const char* key, float* value) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_RG) && m_subbasinID > 0) *value = m_T_RG[m_subbasinID];
    else if (StringMatch(sk, VAR_SBQG) && m_subbasinID > 0) *value = m_T_QG[m_subbasinID];
    else if (StringMatch(sk, VAR_SBGS) && m_subbasinID > 0) *value = m_gwStore[m_subbasinID];
    else if (StringMatch(sk, VAR_SBPET) && m_subbasinID > 0) *value = m_petSubbasin[m_subbasinID];
    else {
        throw ModelException(MID_GWA_RE, "GetValue", "Parameter " + sk + " does not exist.");
    }
}

void ReservoirMethod::Get1DData(const char* key, int* nRows, float** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_REVAP)) {
        *data = m_D_Revap;
        *nRows = m_nCells;
    } else if (StringMatch(sk, VAR_RG)) {
        *data = m_T_RG;
        *nRows = m_nSubbasins + 1;
    } else if (StringMatch(sk, VAR_SBQG)) {
        *data = m_T_QG;
        *nRows = m_nSubbasins + 1;
    } else if (StringMatch(sk, VAR_SBGS)) {
        *data = m_gwStore;
        *nRows = m_nSubbasins + 1;
    } else if (StringMatch(sk, VAR_SBPET)) {
        *data = m_petSubbasin;
        *nRows = m_nSubbasins + 1;
    } else {
        throw ModelException(MID_GWA_RE, "Get1DData", "Parameter " + sk + " does not exist.");
    }
}

void ReservoirMethod::Get2DData(const char* key, int* nRows, int* nCols, float*** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_GWWB)) {
        *data = m_T_GWWB;
        *nRows = m_nSubbasins + 1;
        *nCols = 6;
    } else {
        throw ModelException(MID_GWA_RE, "Get2DData", "Parameter " + sk + " does not exist in current module.");
    }
}

void ReservoirMethod::SetSubbasinInfos() {
    for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); ++it) {
        Subbasin* curSub = m_subbasinsInfo->GetSubbasinByID(*it);
        if (curSub->GetSlope() <= 0.f) {
            curSub->SetSlope(m_Slope);
        }
    }
    m_subbasinsInfo->SetSlopeCoefficient();
}
