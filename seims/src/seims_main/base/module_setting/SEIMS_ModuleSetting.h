/*!
 * \brief User-defined module information in config.fig
 * \author Junzhi Liu, Liang-Jun Zhu
 * \version 
 * \date June 2015
 */
#ifndef SEIMS_MODULE_SETTING_H
#define SEIMS_MODULE_SETTING_H

#include "text.h"
#include "utilities.h"

using namespace std;

/*!
 * \ingroup module_setting
 * \class SEIMSModuleSetting
 * \brief 
 */
class SEIMSModuleSetting {
public:
    SEIMSModuleSetting(string &moduleId, string &setting);

    ~SEIMSModuleSetting() = default;

    ///< data type
    float dataType();

    ///< climate data type for TSD and ITP
    string dataTypeString() { return dataType2String(dataType()); };

    ///< vertical interpolation information for ITP
    bool needDoVerticalInterpolation();

    ///< get channel flow routing method
    string channelFlowRoutingMethod();

    ///< get channel sediment routing method
    string channelSedimentRoutingMethod();

    ///< get channel nutrient routing method
    string channelNutrientRoutingMethod();
public:
    ///< Copy constructor is unusable.
    SEIMSModuleSetting(const SEIMSModuleSetting &) = delete;
    ///< Copy assignment is unusable.
    SEIMSModuleSetting &operator=(const SEIMSModuleSetting &) = delete;
private:
    string m_moduleId; ///< module's ID
    string m_settingString; ///< module setting string
    vector<string> m_settings; ///< module settings

    string channelRoutingMethod(int);

    static float dataTypeString2Float(string);

    static string dataType2String(float);
};

#endif /* SEIMS_MODULE_SETTING_H */