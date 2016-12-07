/*!
 * \file SEIMS_ModuleSetting.h
 * \brief
 *
 *
 *
 * \author [your name]
 * \version 
 * \date June 2015
 *
 * 
 */
#pragma once

#include <string>
#include <vector>

using namespace std;

/*!
 * \ingroup module_setting
 * \class SEIMSModuleSetting
 *
 * \brief 
 *
 *
 *
 */
class SEIMSModuleSetting
{
public:
    SEIMSModuleSetting(string moduleId, string setting);

    ~SEIMSModuleSetting(void);

    float dataType();

    ///< climate data type for TSD and ITP
    string dataTypeString();

    ///< data type
    bool needDoVerticalInterpolation();

    ///< vertical interpolation information for ITP
    string channelFlowRoutingMethod();

    ///< get channel flow routing method
    string channelSedimentRoutingMethod();

    ///< get channel sediment routing method
    string channelNutrientRoutingMethod();///< get channel nutrient routing method
private:
    string m_moduleId; ///< module's ID
    string m_settingString; ///< module setting string
    vector<string> m_settings; ///< module settings

    void getSettings();

    string channelRoutingMethod(int);

    static float dataTypeString2Float(string);

    static string dataType2String(float);
};
