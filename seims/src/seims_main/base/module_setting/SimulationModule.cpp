#include "SimulationModule.h"

using std::string;

SimulationModule::SimulationModule() :
    m_date(-1), m_yearIdx(-1), m_year(1900), m_month(-1), m_day(-1),
    m_tsCounter(1), m_inputsSetDone(false), m_needReCalIntermediateParams(true) {
    // Do nothing
}

void SimulationModule::SetDate(const time_t t, const int year_idx) {
    m_date = t;
    m_yearIdx = year_idx;
    struct tm* date_info = new tm();
    LocalTime(m_date, date_info);
    m_year = date_info->tm_year + 1900;
    m_month = date_info->tm_mon + 1;
    m_day = date_info->tm_mday;
    m_dayOfYear = date_info->tm_yday + 1;
    delete date_info;
}


bool SimulationModule::CheckInputSize(const char* module_id, const char* key, const int nrows, int& m_nrows) {
    if (nrows <= 0) {
        throw ModelException(module_id, "CheckInputSize", "Input data for " + string(key)
                             + " is invalid. The size could not be less than zero.");
    }
    if (nrows != m_nrows) {
        if (m_nrows <= 0) {
            m_nrows = nrows;
        } else {
            throw ModelException(module_id, "CheckInputSize",
                                 "Input data for " + string(key) + " is invalid." +
                                 " The size of input data is " + ValueToString(nrows) +
                                 ", which is expected to be equal to " + ValueToString(m_nrows));
        }
    }
    return true;
}

bool SimulationModule::CheckInputSize2D(const char* module_id, const char* key, const int nrows, const int ncols,
                                        int& m_nrows, int& m_ncols) {
    return CheckInputSize(module_id, key, nrows, m_nrows) && CheckInputSize(module_id, key, ncols, m_ncols);
}
