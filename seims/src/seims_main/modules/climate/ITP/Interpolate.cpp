#include "Interpolate.h"

#include "utils_time.h"
#include "text.h"

Interpolate::Interpolate() : m_dataType(0), m_nStatioins(-1),
                             m_stationData(nullptr), m_nCells(-1), m_itpWeights(nullptr), m_itpVertical(false),
                             m_hStations(nullptr), m_dem(nullptr), m_lapseRate(nullptr),
                             m_itpOutput(nullptr) {
}

void Interpolate::SetClimateDataType(const float value) {
    if (FloatEqual(value, 1.0f)) {
        m_dataType = 0; /// Precipitation
    } else if (FloatEqual(value, 2.0f) || FloatEqual(value, 3.0f) || FloatEqual(value, 4.0f)) {
        m_dataType = 1; /// Temperature
    } else if (FloatEqual(value, 5.0f)) {
        m_dataType = 2; /// PET
    } else if (FloatEqual(value, 6.0f) || FloatEqual(value, 7.0f) || FloatEqual(value, 8.0f)) {
        m_dataType = 3; /// Meteorology
    }
}

Interpolate::~Interpolate() {
    if (m_itpOutput != nullptr) Release1DArray(m_itpOutput);
}

int Interpolate::Execute() {
    CheckInputData();
    if (nullptr == m_itpOutput) { Initialize1DArray(m_nCells, m_itpOutput, 0.f); }
    size_t err_count = 0;
#pragma omp parallel for reduction(+: err_count)
    for (int i = 0; i < m_nCells; i++) {
        int index = 0;
        float value = 0.f;
        for (int j = 0; j < m_nStatioins; j++) {
            index = i * m_nStatioins + j;
            value += m_stationData[j] * m_itpWeights[index];
            if (value != value) {
                err_count++;
                cout << "CELL:" << i << ", Site: " << j << ", Weight: " << m_itpWeights[index] <<
                        ", siteData: " << m_stationData[j] << ", Value:" << value << ";" << endl;
            }
            if (m_itpVertical) {
                float delta = m_dem[i] - m_hStations[j];
                float factor = m_lapseRate[m_month - 1][m_dataType];
                float adjust = m_itpWeights[index] * delta * factor * 0.01f;
                value += adjust;
            }
        }
        m_itpOutput[i] = value;
    }
    if (err_count > 0) {
        throw ModelException(M_ITP[0], "Execute", "Error occurred in weight data!");
    }
    return true;
}

void Interpolate::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, VAR_TSD_DT[0])) {
        SetClimateDataType(value);
    } else if (StringMatch(sk, Tag_VerticalInterpolation[0])) {
        m_itpVertical = value > 0;
    } else {
        throw ModelException(M_ITP[0], "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void Interpolate::Set2DData(const char* key, const int n_rows, const int n_cols, float** data) {
    string sk(key);
    if (StringMatch(sk, Tag_LapseRate)) {
        if (m_itpVertical) {
            int n_month = 12;
            CheckInputSize(M_ITP[0], key, n_rows, n_month);
            m_lapseRate = data;
        }
    } else {
        throw ModelException(M_ITP[0], "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void Interpolate::Set1DData(const char* key, const int n, float* data) {
    string sk(key);
    if (StringMatch(sk, Tag_DEM)) {
        if (m_itpVertical) {
            CheckInputSize(M_ITP[0], key, n, m_nCells);
            m_dem = data;
        }
    } else if (StringMatch(sk, Tag_Weight[0])) {
        CheckInputSize(M_ITP[0], key, n, m_nCells);
        m_itpWeights = data;
    } else if (StringMatch(sk, Tag_Elevation_Precipitation) || StringMatch(sk, Tag_Elevation_Meteorology)
        || StringMatch(sk, Tag_Elevation_Temperature) || StringMatch(sk, Tag_Elevation_PET)) {
        if (m_itpVertical) {
            CheckInputSize(M_ITP[0], key, n, m_nStatioins);
            m_hStations = data;
        }
    } else {
        string prefix = sk.substr(0, 1);
        if (StringMatch(prefix, DataType_Prefix_TS)) {
            CheckInputSize(M_ITP[0], key, n, m_nStatioins);
            m_stationData = data;
        } else {
            throw ModelException(M_ITP[0], "Set1DData", "Parameter " + sk + " does not exist.");
        }
    }
}

bool Interpolate::CheckInputData() {
    CHECK_NONNEGATIVE(M_ITP[0], m_dataType);
    CHECK_NONNEGATIVE(M_ITP[0], m_month);
    CHECK_POINTER(M_ITP[0], m_itpWeights);
    if (m_itpVertical) {
        CHECK_POINTER(M_ITP[0], m_lapseRate);
        CHECK_POINTER(M_ITP[0], m_dem);
        CHECK_POINTER(M_ITP[0], m_hStations);
    }
    CHECK_POINTER(M_ITP[0], m_stationData);
    return true;
}

void Interpolate::Get1DData(const char* key, int* n, float** data) {
    string sk(key);
    if (StringMatch(sk, Tag_DEM)) {
        *n = m_nCells;
        *data = m_dem;
    } else if (StringMatch(sk, Tag_StationElevation)) {
        *n = m_nStatioins;
        *data = m_hStations;
    } else if (StringMatch(sk, DataType_Prefix_TS)) {
        *n = m_nStatioins;
        *data = m_stationData;
    } else {
        *n = m_nCells;
        *data = m_itpOutput;
    }
}
