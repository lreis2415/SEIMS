/*!
 * \ingroup module_setting
 * \brief Settings class to store the settings information
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date June 2010
 * \revised LJ - 1. Change LoadSettingsFromFile to SetSettingTagStrings, Value to GetValue
 *               2. Decoupling with the file IO handling
 */
#ifndef SEIMS_SETTING_H
#define SEIMS_SETTING_H

#include "utilities.h"

using namespace std;

/*!
 * \ingroup module_setting
 * \class Settings
 * \brief Base class for input or output Settings
 */
class Settings {
public:
    //! Constructor
    Settings() = default;

    //! Constructor via 2D string vector
    explicit Settings(vector<vector<string> > &str2dvec) : m_Settings(str2dvec) {};

    //! Constructor via 1D string vector
    explicit Settings(vector<string> &str1dvec);

    //! Destructor
    virtual ~Settings() = default;

    //! Set Settings vector directly
    virtual void SetSettingTagStrings(vector<vector<string> > &string2dvector) {
        m_Settings = string2dvector;
    }

    //! Parse and Set Settings vector by splitting strings
    virtual void SetSettingTagStrings(vector<string> &stringvector);

    //! Return the value for the entry with the given tag, "" if not found
    string GetValue(const string &tag);

    //! Output information to plain text file
    virtual void Dump(string &filename) {};

public:
    //! Store setting key and values
    vector<vector<string> > m_Settings;
};

#endif /* SEIMS_SETTING_H */
