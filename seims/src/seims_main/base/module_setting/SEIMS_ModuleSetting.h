/*!
 * \file SEIMS_ModuleSetting.h
 * \brief User-defined module information in config.fig
 * \author Junzhi Liu, Liang-Jun Zhu
 * \date June 2015
 */
#ifndef SEIMS_MODULE_SETTING_H
#define SEIMS_MODULE_SETTING_H

#include <vector>

#include "basic.h"

using namespace ccgl;
using std::vector;

/*!
 * \ingroup module_setting
 * \class SEIMSModuleSetting
 * \brief
 */
class SEIMSModuleSetting: Interface {
public:
    SEIMSModuleSetting(string& module_id, string& setting);

    ///< data type
    float dataType();

    ///< climate data type for TSD and ITP
    string dataTypeString() { return dataType2String(dataType()); };

    ///< vertical interpolation information for ITP
    bool needDoVerticalInterpolation();

    static float dataTypeString2Float(const string& data_type);

    static string dataType2String(float data_type);

private:
    string m_moduleId;         ///< module's ID
    string m_settingString;    ///< PROCESS NAME with suffix, e.g., Interpolation_P_0 and TimeSeries_M
    vector<string> m_settings; ///< module settings
};

#endif /* SEIMS_MODULE_SETTING_H */
