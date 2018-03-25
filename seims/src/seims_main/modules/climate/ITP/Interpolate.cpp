#include "seims.h"
#include "Interpolate.h"

using namespace std;

Interpolate::Interpolate() : m_nCells(-1), m_nStatioins(-1),
                             m_month(-1), m_itpOutput(nullptr), m_stationData(nullptr), m_weights(nullptr),
                             m_dem(nullptr), m_hStations(nullptr), m_lapseRate(nullptr), m_vertical(false), m_dataType(0) {
}

void Interpolate::SetClimateDataType(float value) {
    if (FloatEqual(value, 1.0f)) {
        m_dataType = 0; /// Precipitation
    } else if (FloatEqual(value, 2.0f) || FloatEqual(value, 3.0f) || FloatEqual(value, 4.0f)) {
        m_dataType = 1; /// Temperature
    } else if (FloatEqual(value, 5.0f)) {
        m_dataType = 2; /// PET
    } else if (FloatEqual(value, 6.0f) || FloatEqual(value, 7.0f) || FloatEqual(value, 8.0f)) {
        m_dataType = 3;
    } /// Meteorology
}

Interpolate::~Interpolate(void) {
    if (m_itpOutput != nullptr) Release1DArray(m_itpOutput);
};

int Interpolate::Execute() {
    CheckInputData();
    if (nullptr == m_itpOutput) { Initialize1DArray(m_nCells, m_itpOutput, 0.f); }
    size_t errCount = 0;
#pragma omp parallel for reduction(+: errCount)
    for (int i = 0; i < m_nCells; ++i) {
        int index = 0;
        float value = 0.f;
        for (int j = 0; j < m_nStatioins; ++j) {
            index = i * m_nStatioins + j;
            value += m_stationData[j] * m_weights[index];
            if (value != value) {
                errCount++;
                cout << "CELL:" << i << ", Site: " << j << ", Weight: " << m_weights[index] <<
                    ", siteData: " << m_stationData[j] << ", Value:" << value << ";" << endl;
            }
            if (m_vertical) {
                float delta = m_dem[i] - m_hStations[j];
                float factor = m_lapseRate[m_month][m_dataType];
                float adjust = m_weights[index] * delta * factor / 100.f;
                value += adjust;
            }
        }
        m_itpOutput[i] = value;
    }
    if (errCount > 0) {
        throw ModelException(MID_ITP, "Execute", "Error occurred in weight data!\n");
    }
    return true;
}

void Interpolate::SetDate(time_t date, int yearIdx) {
    m_date = date;
    m_yearIdx = yearIdx;
    struct tm t;
    LocalTime(date, &t);
    m_month = t.tm_mon;
}

void Interpolate::SetValue(const char *key, float value) {
    string sk(key);
    if (StringMatch(sk, VAR_OMP_THREADNUM)) {
        SetOpenMPThread((int) value);
    } else if (StringMatch(sk, VAR_TSD_DT)) {
        SetClimateDataType(value);
    } else if (StringMatch(sk, Tag_VerticalInterpolation)) {
        if (value > 0) {
            m_vertical = true;
        } else {
            m_vertical = false;
        }
    } else {
        throw ModelException(MID_ITP, "SetValue", "Parameter " + sk + " does not exist.");
    }

}

void Interpolate::Set2DData(const char *key, int nRows, int nCols, float **data) {
    string sk(key);
    if (StringMatch(sk, Tag_LapseRate)) {
        if (m_vertical) {
            int nMonth = 12;
            CheckInputSize(sk, nRows, nMonth);
            m_lapseRate = data;
        }
    } else {
        throw ModelException(MID_ITP, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void Interpolate::Set1DData(const char *key, int n, float *data) {
    string sk(key);
    if (StringMatch(sk, Tag_DEM)) {
        if (m_vertical) {
            CheckInputSize(sk, n, m_nCells);
            m_dem = data;
        }
    } else if (StringMatch(sk, Tag_Weight)) {
        CheckInputSize(sk, n, m_nCells);
        m_weights = data;
    } else if (StringMatch(sk, Tag_Elevation_Precipitation) || StringMatch(sk, Tag_Elevation_Meteorology)
        || StringMatch(sk, Tag_Elevation_Temperature) || StringMatch(sk, Tag_Elevation_PET)) {
        if (m_vertical) {
            CheckInputSize(sk, n, m_nStatioins);
            m_hStations = data;
        }
    } else {
        string prefix = sk.substr(0, 1);
        if (StringMatch(prefix, DataType_Prefix_TS)) {
            CheckInputSize(sk, n, m_nStatioins);
            m_stationData = data;
        } else {
            throw ModelException(MID_ITP, "Set1DData", "Parameter " + sk + " does not exist.");
        }
    }
}

bool Interpolate::CheckInputSize(string &key, int n, int &m_n) {
    if (n <= 0) {
        throw ModelException(MID_ITP, "CheckInputSize", "Input data for " + key
            + " is invalid. The size could not be less than zero.");
    }
    if (n != m_n) {
        if (m_n <= 0) {
            m_n = n;
        } else {
            throw ModelException(MID_ITP, "CheckInputSize",
                                 "Input data for " + key + " is invalid." + " The size of input data is " +
                                     ValueToString(n) +
                                     ". The number of columns in weight file and the number of stations should be same.");
        }
    }
    return true;
}

void Interpolate::CheckInputData() {
    CHECK_NONNEGATIVE(MID_ITP, m_dataType);
    CHECK_NONNEGATIVE(MID_ITP, m_month);
    CHECK_POINTER(MID_ITP, m_weights);
    if (m_vertical) {
        CHECK_POINTER(MID_ITP, m_lapseRate);
        CHECK_POINTER(MID_ITP, m_dem);
        CHECK_POINTER(MID_ITP, m_hStations);
    }
    CHECK_POINTER(MID_ITP, m_stationData);
}

void Interpolate::Get1DData(const char *key, int *n, float **data) {
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
