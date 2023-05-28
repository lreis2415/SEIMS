#include "GR4J.h"
#include "text.h"

GR4J::GR4J() :
    m_nCells(-1), m_isInitialized(false),m_subbasins(nullptr),
    //infiltration
    m_pcp(nullptr), m_soilThickness(nullptr), m_soilPorosity(nullptr), m_soilCapacity(nullptr),
    m_infil(nullptr), m_soilWtrSto(nullptr), m_pcpExcess(nullptr),
    //soil evaporation
    m_pet(nullptr), m_soilET(nullptr),
    //percolation
    m_GR4J_X2(nullptr), m_GR4J_X3(nullptr),
    //split and convolve
    m_GR4J_X4(nullptr), m_unitHydro(nullptr), m_convEntering1(nullptr), m_convEntering2(nullptr), m_convTransport1(nullptr), m_convTransport2(nullptr),
    //routing
    m_Q_SBOF(nullptr), m_Q_SB_ZEROS(nullptr) {
}

GR4J::~GR4J() {
    Release1DArray(m_soilPorosity);
    Release1DArray(m_soilCapacity);
    Release1DArray(m_infil);
    Release2DArray(m_soilWtrSto);
    Release1DArray(m_pcpExcess);
    Release1DArray(m_pet);
    Release1DArray(m_soilET);
    Release2DArray(m_GR4J_X2);
    Release2DArray(m_GR4J_X3);
    Release1DArray(m_GR4J_X4);
    Release2DArray(m_unitHydro);
    Release1DArray(m_convEntering1);
    Release1DArray(m_convEntering2);
    Release2DArray(m_convTransport1);
    Release2DArray(m_convTransport2);
}

void GR4J::SetValue(const char* key, int value) {
    string sk(key);
    if (StringMatch(sk, Tag_CellSize[0])) m_nCells = value;
    else if (StringMatch(sk, VAR_SUBBSNID_NUM[0])) m_nSubbasins = value;
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnId = value;
    else {
        throw ModelException(M_IUH_OL[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

// void GR4J::SetValue(const char* key, FLTPT value) {
//     string sk(key);
//     if (StringMatch(sk, Tag_CellSize[0])) m_nCells = value;
//     else if (StringMatch(sk, VAR_SUBBSNID_NUM[0])) m_nSubbasins = value;
//     else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnId = value;
//     else {
//         throw ModelException(M_IUH_OL[0], "SetValue",
//                              "Integer Parameter " + sk + " does not exist.");
//     }
// }
void GR4J::Set1DData(const char* key, int n, int* data) {
    CheckInputSize(key, n);
    string sk(key);

    if (StringMatch(sk, VAR_SUBBSN[0])) m_cellsMappingToSubbasinId = data;
    else {
        throw ModelException(CM_GR4J[0], "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}
void GR4J::Set1DData(const char* key, int n, FLTPT* data) {
    CheckInputSize(key, n);
    string sk(key);

    if (StringMatch(sk, VAR_PCP[0])) { m_pcp = data; }
    else if (StringMatch(sk, VAR_PET[0])) { m_pet = data; }
    else if (StringMatch(sk, "GR4J_X4")) { m_GR4J_X4 = data; }
    else {
        throw ModelException(CM_GR4J[0], "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

void GR4J::Set2DData(const char* key, int nrows, int ncols, FLTPT** data) {
    string sk(key);
    CheckInputSize2D(CM_GR4J[0], key, nrows, ncols, m_nCells, N_SOIL_LAYERS);
    if (StringMatch(sk, VAR_SOILTHICK[0])) { m_soilThickness = data; }
    else if (StringMatch(sk, VAR_POROST[0])) { m_soilPorosity = data; }
    else if (StringMatch(sk, "GR4J_X2")) { m_GR4J_X2 = data; }
    else if (StringMatch(sk, "GR4J_X3")) { m_GR4J_X3 = data; }
    else {
        throw ModelException(CM_GR4J[0], "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}

bool GR4J::CheckInputSize(const char* key, int n) {
    if (n <= 0) {
        throw ModelException("GR4J", "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException("GR4J", "CheckInputSize", "Input data for " + string(key) +
                                 " is invalid. All the input data should have same size.");
            return false;
        }
    }
    return true;
}

bool GR4J::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException("GR4J", "CheckInputData", "Input data is invalid. The size could not be less than zero.");
        return false;
    }
    CHECK_POINTER(CM_GR4J[0], m_pcp);
    CHECK_POINTER(CM_GR4J[0], m_soilThickness);
    CHECK_POINTER(CM_GR4J[0], m_soilPorosity);
    CHECK_POSITIVE(CM_GR4J[0], m_GR4J_X4);
    return true;
}

void GR4J::InitialOutputs() {
    if(m_isInitialized) {
        return;
    }
    Initialize1DArray(m_nCells, m_pcpExcess, 0.);
    Initialize2DArray(m_nCells, N_SOIL_LAYERS, m_soilCapacity, 0.);
    Initialize1DArray(m_nCells, m_infil, 0.);
    Initialize2DArray(m_nCells, N_SOIL_LAYERS, m_soilWtrSto, 0.);
    Initialize1DArray(m_nCells, m_infil, 0.);
    Initialize1DArray(m_nCells, m_soilET, 0.);
    Initialize1DArray(m_nCells, m_convEntering1, 0.);
    Initialize1DArray(m_nCells, m_convEntering2, 0.);
    Initialize2DArray(m_nCells, N_SOIL_LAYERS, m_unitHydro, 0.);
    Initialize2DArray(m_nCells, N_SOIL_LAYERS, m_convTransport1, 0.);
    Initialize2DArray(m_nCells, N_SOIL_LAYERS, m_convTransport2, 0.);

    Initialize1DArray(m_nSubbasins + 1, m_Q_SBOF, 0.);
    Initialize1DArray(m_nSubbasins + 1, m_Q_SB_ZEROS, 0.);

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        for (int j = 0; j < N_SOIL_LAYERS; ++j) {
            m_soilCapacity[i][j] = m_soilPorosity[i][j] * m_soilThickness[i][j];
        }
    }

    m_isInitialized = true;
}

void GR4J::SetSubbasins(clsSubbasins* subbsns) {
    if (m_subbasins == nullptr) {
        m_subbasins = subbsns;
        m_subbasinIds = m_subbasins->GetSubbasinIDs();
    }
}

void GR4J::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    if (StringMatch(sk, VAR_SBOF[0])) { *data = m_Q_SBOF; }
    else if (StringMatch(sk, VAR_SBIF[0])) { *data = m_Q_SB_ZEROS; }
    else if (StringMatch(sk, VAR_SBQG[0])) { *data = m_Q_SB_ZEROS; }
    else {
        throw ModelException("GR4J", "Get1DData",
                             "Result " + sk +
                             " does not exist in current module. Please contact the module developer.");
    }
    *n = m_nSubbasins + 1;
}

void GR4J::CalculateSoilCapacity() {
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        for (int j = 0; j < N_SOIL_LAYERS; j++) {
            m_soilCapacity[i][j] = m_soilThickness[i][j] * m_soilPorosity[i][j];
        }
    }
}


void GR4J::Infiltration() {
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        // Calculate X1
        ///Parameter X1 of GR4J model. X1 = SoilCapacity = Thickness * Porosity * (1-StoneFraction). 
        ///StoneFraction is not considered in this implementation, assumed to be 0.
        FLTPT x1 = m_soilCapacity[i][SOIL_PRODUCT_LAYER];

        FLTPT store = m_infil[i]; // infiltration to soil layer 0 is all stored
        FLTPT sat = store / x1; // saturation
        FLTPT tmp = tanh(m_pcp[i] / x1);
        FLTPT infil = x1 * (1.0 - (sat * sat)) * tmp / (1.0 + sat * tmp);
        FLTPT impF = 0; // m_impermeableFraction[i]
        infil = (1.0 - impF) * infil;

        //output
        m_infil[i] += infil;
        m_pcpExcess[i] += m_pcp[i] - infil;
        m_soilWtrSto[i][SOIL_PRODUCT_LAYER] += m_infil[i];
    }
}

void GR4J::SoilEvaporation() {
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT maxStor = m_soilCapacity[i][SOIL_PRODUCT_LAYER];
        FLTPT stor = m_soilWtrSto[i][SOIL_PRODUCT_LAYER];
        FLTPT sat = stor / maxStor;
        FLTPT tmp = tanh(Max(m_pet[i], 0.0) / maxStor);
        m_soilET[i] += stor * (2.0 - sat) * tmp / (1.0 + (1.0 - sat) * tmp);
        m_soilWtrSto[i][SOIL_PRODUCT_LAYER] -= m_soilET[i];
        // AET = usedPET = m_soilET[i];  // AET is not needed as an output here?
    }
}

void GR4J::Percolation(int fromSoilLayer, int toSoilLayer) {
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT stor = m_soilWtrSto[i][fromSoilLayer];
        FLTPT maxStor = m_soilCapacity[i][fromSoilLayer];
        //TODO: In Raven, it is:
        //    rates[0]=stor*(1.0-pow(1.0+pow(4.0/9.0*Max(stor/max_stor,0.0),4),-0.25))/Options.timestep;
        FLTPT perc = stor * (1.0 - pow(1.0 + pow(4.0 / 9.0 * Max(stor / maxStor, 0.0), 4), -0.25));

        m_soilWtrSto[i][fromSoilLayer] -= perc;
        m_soilWtrSto[i][toSoilLayer] += perc;
    }
}

void GR4J::PercolationExch(int fromSoilLayer, int toSoilLayer) {
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT stor = m_soilWtrSto[i][fromSoilLayer];
        FLTPT x2 = m_GR4J_X2[i][fromSoilLayer];
        FLTPT x3 = m_GR4J_X3[i][fromSoilLayer];
        FLTPT perc = -x2 * pow(Max(Min(stor / x3, 1.0), 0.0), 3.5);

        m_soilWtrSto[i][fromSoilLayer] -= perc;
        m_soilWtrSto[i][toSoilLayer] += perc;
    }
}

void GR4J::PercolationExch2(int fromSoilLayer, int toSoilLayer) {
    // TODO: difference between exch exch2? Is Raven Exch2 correct?
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT stor = m_soilWtrSto[i][SOIL_ROUTING_LAYER];
        FLTPT x2 = m_GR4J_X2[i][SOIL_ROUTING_LAYER];
        FLTPT x3 = m_GR4J_X3[i][SOIL_ROUTING_LAYER];

        FLTPT perc = -x2 * pow(Max(Min(stor / x3, 1.0), 0.0), 3.5);
        m_soilWtrSto[i][fromSoilLayer] -= perc;
        m_soilWtrSto[i][toSoilLayer] += perc;
    }
}


void GR4J::Flush(FLTPT* fromStore, int toSoilLayer) {
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_soilWtrSto[i][toSoilLayer] += fromStore[i];
        fromStore[i] = 0;
    }
}

void GR4J::Flush(int fromSoilLayer, FLTPT* toStore) {
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        toStore[i] += m_soilWtrSto[i][fromSoilLayer];
        m_soilWtrSto[i][fromSoilLayer] = 0;
    }
}

//Split RAVEN_DEFAULT TEMP_STORE CONVOLUTION[0] CONVOLUTION[1] 0.9
void GR4J::Split() {
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_convEntering1[i] = m_soilWtrSto[i][SOIL_TEMP_LAYER] * 0.9;
        m_convEntering2[i] = m_soilWtrSto[i][SOIL_TEMP_LAYER] * 0.1;
        m_soilWtrSto[i][SOIL_TEMP_LAYER] = 0;
    }
}

void GR4J::Convolve(ConvoleType t) {
    int N = 0;
    GenerateUnitHydrograph(t, N);
    if (m_convTransport1 == nullptr)
        Initialize2DArray(m_nCells, N, m_convTransport1, 0);
    if (m_convTransport2 == nullptr)
        Initialize2DArray(m_nCells, N, m_convTransport2, 0);
    FLTPT* convEntering = nullptr;
    FLTPT** convTransport = nullptr;
    int toSoilLayer = 0;
    if (t == CONVOL_GR4J_1) {
        convEntering = m_convEntering1;
        convTransport = m_convTransport1;
        toSoilLayer = SOIL_ROUTING_LAYER;
    }
    else if (t == CONVOL_GR4J_2) {
        convEntering = m_convEntering2;
        convTransport = m_convTransport2;
        toSoilLayer = SOIL_TEMP_LAYER;
    }
    else {
        throw ModelException(CM_GR4J[0], "Convolve", "unknown convolution type " + std::to_string(t));
    }
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        // moving entering water into transport water array
        convTransport[i][0] = convEntering[i];
        // moving every transport water element to the out soil layer (conceptual store),
        // with a proportion corresponding to the unit hydrograph
        for (int n = 0; n < N; n++) {
            m_soilWtrSto[i][toSoilLayer] += convTransport[i][n] * m_unitHydro[i][n];
        }
        // moving every transport water element to the next index (next timestep).
        for (int n = N - 1; n > 0; ++n) {
            convTransport[i][n] = convTransport[i][n - 1];
        }
    }
}

void GR4J::GenerateUnitHydrograph(ConvoleType t, int& N) {
    if (m_unitHydro != nullptr) return;
    FLTPT tstep = 1; // days
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT maxTime = 0;
        FLTPT x4 = m_GR4J_X4[i];

        if (t == CONVOL_GR4J_1) {
            maxTime = x4;
        }
        else if (t == CONVOL_GR4J_2) {
            maxTime = 2 * x4;
        }
        else {
            throw ModelException(CM_GR4J[0], "GenerateUnitHydrograph", "unknown convolution type " + std::to_string(t));
        }

        N = ceil(maxTime / tstep);
        if (N == 0) { N = 1; }
        if (N > 50) { throw ModelException("GR4J", "GenerateUnitHydrograph", "unit hydrograph duration for convolution too long"); }

        FLTPT sum = 0.0;
        for (int n = 0; n < N; ++n) {
            FLTPT h = 0.0;
            if (t == CONVOL_GR4J_1) {
                h = Min(pow((n+1)*tstep/x4,2.5), 1.0) - Min(pow(n*tstep/x4,2.5), 1.0);
            }
            else if (t == CONVOL_GR4J_2) {
                h = GR4J_SH2((n + 1) * tstep, x4) - GR4J_SH2(n * tstep, x4);
            }
            m_unitHydro[i][n] = h;
            sum += h;
        }
        if (sum == 0.0) { throw ModelException("GR4J", "GenerateUnitHydrograph", "bad unit hydrograph constructed"); }
        for (int n = 0; n < N; ++n) { m_unitHydro[i][n] /= sum; }
    }
}

FLTPT GR4J::GR4J_SH2(const FLTPT& t, const FLTPT& x4) {
    if (t / x4 < 1.0) {
        return 0.5 * pow(t / x4, 2.5);
    }
    else if (t / x4 < 2.0) {
        return 1.0 - 0.5 * pow(2 - t / x4, 2.5);
    }
    else {
        return 1.0;
    }
}

void GR4J::Baseflow(int fromSoilLayer) {
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT x3 = m_GR4J_X3[i][fromSoilLayer];
        FLTPT stor = m_soilWtrSto[i][fromSoilLayer];
        FLTPT q = stor * (1.0 - pow(1.0 + pow(Max(stor/x3, 0.0), 4), -0.25));
        m_pcpExcess[i] += q;
    }
}

// directly add pcp excess to subbasin outlet
void GR4J::OverlandRouting() {
#pragma omp parallel
    {
        FLTPT* tmp_qsSub = new FLTPT[m_nSubbasins + 1];
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            tmp_qsSub[CVT_INT(m_cellsMappingToSubbasinId[i])] = m_pcpExcess[i]; //get new value
            m_pcpExcess[i] = 0.0;
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbasins; i++) {
                m_Q_SBOF[i] += tmp_qsSub[i];
            }
        }
        delete[] tmp_qsSub;
        tmp_qsSub = nullptr;
    } /* END of #pragma omp parallel */

    for (int n = 1; n <= m_nSubbasins; n++) {
        //get overland flow routing for entire watershed.
        m_Q_SBOF[0] += m_Q_SBOF[n];
    }
    
}

int GR4J::Execute() {
    InitialOutputs();
    CheckInputData();

    Infiltration();
    SoilEvaporation();
    Percolation(SOIL_PRODUCT_LAYER, SOIL_TEMP_LAYER);
    Flush(m_pcpExcess, SOIL_TEMP_LAYER);
    Split();
    Convolve(CONVOL_GR4J_1);
    Convolve(CONVOL_GR4J_2);
    PercolationExch(SOIL_ROUTING_LAYER, SOIL_GW_LAYER);
    PercolationExch2(SOIL_TEMP_LAYER, SOIL_GW_LAYER);
    Flush(SOIL_TEMP_LAYER, m_pcpExcess);
    Baseflow(SOIL_ROUTING_LAYER);
    OverlandRouting();
    return 0;
}

