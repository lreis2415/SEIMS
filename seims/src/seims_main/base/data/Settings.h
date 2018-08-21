/*!
 * \file Settings.h
 * \brief Settings class to store the settings information
 *
 * Changelog:
 *   - 1. 2010-06-30 - lj - Change LoadSettingsFromFile to SetSettingTagStrings, Value to GetValue.
 *                          Decoupling with the file IO handling
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date June 2010
 */
#ifndef SEIMS_SETTING_H
#define SEIMS_SETTING_H

#include <vector>

#include "basic.h"

using namespace ccgl;
using std::vector;

/*!
 * \ingroup data
 * \class Settings
 * \brief Base class for input or output Settings
 */
class Settings: Interface {
public:
    //! Constructor
    Settings() {};

    //! Constructor via 2D string vector
    explicit Settings(vector<vector<string> >& str2dvec) : m_Settings(str2dvec) {};

    //! Constructor via 1D string vector
    explicit Settings(vector<string>& str1dvec);

    //! Set Settings vector directly
    virtual void SetSettingTagStrings(vector<vector<string> >& string2dvector) {
        m_Settings = string2dvector;
    }

    //! Parse and Set Settings vector by splitting strings
    virtual void SetSettingTagStrings(vector<string>& stringvector);

    //! Return the value for the entry with the given tag, "" if not found
    string GetValue(const string& tag);

    //! Output information to plain text file
    virtual void Dump(const string& filename) {};

public:
    //! Store setting key and values
    vector<vector<string> > m_Settings;
};

#endif /* SEIMS_SETTING_H */
