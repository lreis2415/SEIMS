#include <stdio.h>
#include <string>
#include "util.h"
#include "api.h"
#include "template.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new ModulesTest();
}

/// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;
    string res;

    mdi.SetAuthor("Name");
    mdi.SetClass("TEST", "Base functionality test!");
    mdi.SetDescription("Module test.");
    mdi.SetID("moduletest");
    mdi.SetName("moduletest");
    mdi.SetVersion("1.0");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    /// set parameters from database
    mdi.AddParameter("VAR_NAME", "UNIT", "DESC", "Source", "DT");
    /// set input from other modules
    mdi.AddInput("VAR_NAME", "UNIT", "DESC", "Source", "DT");
    /// set the output variables
    mdi.AddOutput("VAR_NAME", "UNIT", "DESC", "DT");
    /// write out the XML file.
    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}