#include "SettingsInput.h"

#include "easylogging++.h"
#include "utils_time.h"
#include "text.h"

using namespace utils_time;

const char* SimuModeToString(SimulationMode mode) {
    switch (mode) {
    case DAILY: return "DAILY";
    case STORM: return "STORM";
    case UNDEFINED: return "UNDEFINED";
    default: return "UNDEFINED";
    }
}

const SimulationMode StringToSimuMode(string mode) {
    if (StringMatch(mode, "DAILY")) return DAILY;
    else if (StringMatch(mode, "STORM")) return STORM;
    else return UNDEFINED;
}

SettingsInput::SettingsInput(vector<string>& stringvector)
    : m_startDate(0), m_endDate(0), m_dtHs(3600), m_dtCh(86400), m_mode(UNDEFINED),
      m_stormMode(false) {
    Settings::SetSettingTagStrings(stringvector);

    if (!readSimulationPeriodDate()) {
        throw ModelException("SettingInput", "Constructor",
                             "The start time and end time in file.in is invalid or missing."
                             "The format would be YYYY/MM/DD/HH. Please check it.");
    }
    m_mode = StringToSimuMode(GetValue(Tag_Mode));

    //read interval
    vector<string> dtList = SplitString(GetValue(Tag_Interval), ',');
    char* strend = nullptr;
    errno = 0;
    m_dtHs = strtol(dtList[0].c_str(), &strend, 10);
    m_dtCh = m_dtHs;
    if (dtList.size() > 1) {
        m_dtCh = strtol(dtList[1].c_str(), &strend, 10);
    }
    // The timestep's unit is FIXED as second!
    // convert the time interval to seconds to conform to time_t struct
    //if (m_mode == DAILY) {
    //    m_dtHs *= 86400; // 86400 secs is 1 day
    //    m_dtCh *= 86400;
    //    m_stormMode = false;
    //}
    //else { m_stormMode = true; }
    m_stormMode = m_mode == STORM;
}

SettingsInput* SettingsInput::Init(const InputArgs* input_args) {
    string model_cfgpath = input_args->model_path;
    if (!input_args->model_cfgname.empty() && !StringMatch(input_args->model_cfgname, "_BASE_")) {
        model_cfgpath += SEP + input_args->model_cfgname;
    }
    string file_in = model_cfgpath + SEP + File_Input;
    if (!FileExists(file_in)) {
        LOG(ERROR) << file_in << " does not exist!";
        return nullptr;
    }
    vector<string> stringvector;
    if (!LoadPlainTextFile(file_in, stringvector) || stringvector.empty()) {
        LOG(ERROR) << file_in << " is not loaded!";
        return nullptr;
    }
    return new SettingsInput(stringvector);
}


SettingsInput* SettingsInput::Init(vector<string>& stringvector) {
    if (stringvector.empty()) {
        return nullptr;
    }
    return new SettingsInput(stringvector);
}

bool SettingsInput::readSimulationPeriodDate() {
    //read start and end time
    m_startDate = ConvertToTime(GetValue(Tag_StartTime), "%d-%d-%d %d:%d:%d", true);
    m_endDate = ConvertToTime(GetValue(Tag_EndTime), "%d-%d-%d %d:%d:%d", true);

    if (m_startDate <= 0 || m_endDate <= 0) return false;

    // make sure the start and end times are in the proper order
    if (m_endDate < m_startDate) {
        time_t tmp = m_startDate;
        m_startDate = m_endDate;
        m_endDate = tmp;
    }
    return true;
}

void SettingsInput::Dump(const string& fileName) {
    std::ofstream fs;
    fs.open(fileName.c_str(), std::ios::out);
    if (fs.is_open()) {
        fs << "Start Date :" << ConvertToString2(m_startDate) << endl;
        fs << "End Date :" << ConvertToString2(m_endDate) << endl;
        fs << "Interval :" << m_dtHs << "\t" << m_dtCh << endl;
        fs.close();
    }
}
