#include "SEIMS_ModuleSetting.h"

#include "utils_string.h"
#include "text.h"

using namespace utils_string;

SEIMSModuleSetting::SEIMSModuleSetting(string& module_id, string& setting) :
    m_moduleId(module_id), m_settingString(setting) {
    m_settings = SplitString(m_settingString, '_');
}

float SEIMSModuleSetting::dataType() {
    if (m_moduleId.find(M_ITP[0]) == string::npos && m_moduleId.find(M_TSD_RD[0]) == string::npos) {
        return -1.0f;
    }
    if (m_settings.size() < 2) {
        throw ModelException("SEIMSModuleSetting", "dataType", "Module " + m_moduleId +
                             " does not appoint data type in the second column.");
    }
    float data_type = dataTypeString2Float(m_settings.at(1));
    if (data_type == -1.0f) {
        throw ModelException("SEIMSModuleSetting", "dataType",
                             "The data type of module " + m_moduleId +
                             " is not correct. It must be P, TMEAN, TMIN, TMAX or PET.");
    }
    return data_type;
}

bool SEIMSModuleSetting::needDoVerticalInterpolation() {
    if (m_moduleId.find(M_ITP[0]) == string::npos) {
        return false;
    }
    if (m_settings.size() < 3) {
        throw ModelException("SEIMSModuleSetting", "needDoVerticalInterpolation",
                             "Module " + m_moduleId +
                             " does not appoint vertical interpolation in the third column.");
    }
    char* strend = nullptr;
    errno = 0;
    int iIsDoVerticalInterpolation = strtol(m_settings.at(2).c_str(), &strend, 10); // deprecate C-style atoi
    if (errno != 0) {
        throw ModelException("SEIMSModuleSetting", "needDoVerticalInterpolation",
                             "iIsDoVerticalInterpolation field converted to integer failed!");
    }
    return iIsDoVerticalInterpolation != 0;
}

float SEIMSModuleSetting::dataTypeString2Float(const string& data_type) {
    if (StringMatch(data_type, DataType_Precipitation)) return 1.0f;
    if (StringMatch(data_type, DataType_MeanTemperature)) return 2.0f;
    if (StringMatch(data_type, DataType_MinimumTemperature)) return 3.0f;
    if (StringMatch(data_type, DataType_MaximumTemperature)) return 4.0f;
    if (StringMatch(data_type, DataType_PotentialEvapotranspiration)) return 5.0f;
    if (StringMatch(data_type, DataType_SolarRadiation)) return 6.0f;
    if (StringMatch(data_type, DataType_WindSpeed)) return 7.0f;
    if (StringMatch(data_type, DataType_RelativeAirMoisture)) return 8.0f;
    return -1.0f;
}

string SEIMSModuleSetting::dataType2String(const float data_type) {
    switch (CVT_INT(data_type)) {
        case 1: return DataType_Precipitation;
        case 2: return DataType_MeanTemperature;
        case 3: return DataType_MinimumTemperature;
        case 4: return DataType_MaximumTemperature;
        case 5: return DataType_PotentialEvapotranspiration;
        case 6: return DataType_SolarRadiation;
        case 7: return DataType_WindSpeed;
        case 8: return DataType_RelativeAirMoisture;
        default: return "";
    }
}
