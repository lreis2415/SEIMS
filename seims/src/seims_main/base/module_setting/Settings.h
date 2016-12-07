/*!
 * \file Settings.h
 * \ingroup module_setting
 * \brief Settings class to store the settings information from Configuration files
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date June 2010
 *
 * 
 */
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream> 
#include <algorithm> 
#include <iterator> 
#include "utils.h"
#include "util.h"
#include "ModelException.h"

using namespace std;

//! \typdef 2D stringarray (vector)  
typedef vector<vector<string> > string2DArray;

/*!
 * \ingroup module_setting
 * \class Settings
 *
 * \brief Basic setting class
 */
class Settings
{
public:
    //! Store setting key and values
    string2DArray m_Settings;

public:
    //! Constructor
    Settings(void);

    //! Destructor
    virtual ~Settings(void);

    //! Load the settings value from the given file
    virtual bool LoadSettingsFromFile(string filename);

    virtual void Dump(string);

    //! Return the value for the entry with the given tag
    string Value(string tag);

protected:
    //! input setting file path
    string m_settingFileName;
};

