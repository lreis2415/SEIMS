#include "Interpolate.h"

#include "utils_time.h"
#include "text.h"

Interpolate::Interpolate() :
m_dataType(0), m_nStations(-1),
m_stationData(nullptr), m_nCells(-1), m_itpWeights(nullptr), m_itpVertical(false),
m_hStations(nullptr), m_dem(nullptr), m_lapseRate(nullptr),
m_itpOutput(nullptr) {
    SetModuleName(M_ITP[0]);
}

void Interpolate::SetClimateDataType(const int data_type) {
    if (data_type == 1) {
        m_dataType = 0; /// Precipitation
    } else if (data_type == 2 || data_type == 3 || data_type == 4) {
        m_dataType = 1; /// Temperature
    } else if (data_type == 5) {
        m_dataType = 2; /// PET
    } else if (data_type == 6 || data_type == 7 || data_type == 8) {
        m_dataType = 3; /// Meteorology
    } else {
        m_dataType = -1; /// Unknown
    }
}

Interpolate::~Interpolate() {
    if (m_itpOutput != nullptr) { Release1DArray(m_itpOutput); }
}

int Interpolate::Execute() {
    CheckInputData();
    if (nullptr == m_itpOutput) {
        Initialize1DArray(m_nCells, m_itpOutput, 0.);
    }
    size_t err_count = 0;
#pragma omp parallel for reduction(+: err_count)
    for (int i = 0; i < m_nCells; i++) {
        FLTPT value = 0.;
        for (int j = 0; j < m_nStations; j++) {
            value += m_stationData[j] * m_itpWeights[i][j];
            if (value != value) {
                err_count++;
                cout << "CELL:" << i << ", Site: " << j << ", Weight: " << m_itpWeights[i][j] <<
                        ", siteData: " << m_stationData[j] << ", Value:" << value << ";" << endl;
            }
            if (m_itpVertical) {
                FLTPT delta = m_dem[i] - m_hStations[j];
                FLTPT factor = m_lapseRate[m_month - 1][m_dataType];
                FLTPT adjust = m_itpWeights[i][j] * delta * factor * 0.01;
                value += adjust;
            }
        }
        m_itpOutput[i] = value;
    }
    if (err_count > 0) {
        throw ModelException(GetModuleName(), "Execute",
                             "Error occurred in interpolation based on weight data of stations!");
    }
    return true;
}

void Interpolate::SetValue(const char* key, const int value) {
    string sk(key);
    if (StringMatch(sk, VAR_TSD_DT[0])) {
        SetClimateDataType(value);
    } else if (StringMatch(sk, Tag_VerticalInterpolation[0])) {
        m_itpVertical = value > 0;
    } else {
        throw ModelException(GetModuleName(), "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void Interpolate::Set2DData(const char* key, const int n_rows, const int n_cols, FLTPT** data) {
    string sk(key);
    if (StringMatch(sk, Tag_LapseRate)) {
        if (m_itpVertical) {
            int n_month = 12;
            CheckInputSize(key, n_rows, n_month);
            m_lapseRate = data;
        }
    } else if (StringMatch(sk, Tag_Weight[0])) {
        CheckInputSize2D(key, n_rows, n_cols, m_nCells, m_nStations);
        m_itpWeights = data;
    } else {
        throw ModelException(GetModuleName(), "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void Interpolate::Set1DData(const char* key, const int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_DEM[0])) {
        if (m_itpVertical) {
            CheckInputSize(key, n, m_nCells);
            m_dem = data;
        }
    } else if (StringMatch(sk, Tag_Elevation_Precipitation) || StringMatch(sk, Tag_Elevation_Meteorology)
        || StringMatch(sk, Tag_Elevation_Temperature) || StringMatch(sk, Tag_Elevation_PET)) {
        if (m_itpVertical) {
            CheckInputSize(key, n, m_nStations);
            m_hStations = data;
        }
    } else {
        string prefix = sk.substr(0, 1);
        if (StringMatch(prefix, DataType_Prefix_TS)) {
            CheckInputSize(key, n, m_nStations);
            m_stationData = data;
        } else {
            throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk + " does not exist.");
        }
    }
}

bool Interpolate::CheckInputData() {
    CHECK_NONNEGATIVE(GetModuleName(), m_dataType);
    CHECK_NONNEGATIVE(GetModuleName(), m_month);
    CHECK_POINTER(GetModuleName(), m_itpWeights);
    if (m_itpVertical) {
        CHECK_POINTER(GetModuleName(), m_lapseRate);
        CHECK_POINTER(GetModuleName(), m_dem);
        CHECK_POINTER(GetModuleName(), m_hStations);
    }
    CHECK_POINTER(GetModuleName(), m_stationData);
    return true;
}

void Interpolate::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    if (StringMatch(sk, VAR_DEM[0])) {
        *n = m_nCells;
        *data = m_dem;
    } else if (StringMatch(sk, Tag_StationElevation)) {
        *n = m_nStations;
        *data = m_hStations;
    } else if (StringMatch(sk, DataType_Prefix_TS)) {
        *n = m_nStations;
        *data = m_stationData;
    } else { // DataType_Prefix_DIS
        *n = m_nCells;
        *data = m_itpOutput;
    }
}
