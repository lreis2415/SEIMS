/*!
 * \brief Generate parameters, inputs, and outputs lists of SEIMS Modules
 *
 * \author Liangjun Zhu
 * \version 1.0
 * \date July 2015
 */

#include <iostream>
//#include <string>
//#include <stdio.h>
#include "util.h"
#include "utils.h"
#include "ModulesIOList.h"

using namespace std;

int main(int argc, char *argv[])
{
    string modulesListFile = "";
    string storePath = "";
    utils util;
    string moduleIOListFile = "", paramListFile = "", moduleParamListFile = "", dllPath = "";

    ModulesIOList *modulesIO;
    if (argc == 2)
    {
        if (util.FileExists(argv[1]))
        {
            modulesListFile = argv[1];
        }
        else
            goto errexit;
    }
    else
        goto errexit;
    storePath = GetPathFromFullName(modulesListFile);
    moduleIOListFile = storePath +
                       "module_IO_List.txt";  ///< parameters from Database, inputs from other modules or files, and outputs
    paramListFile = storePath + "params_List.txt";  ///< parameters from Database as input for modules
    moduleParamListFile = storePath + "module_Params_List.txt";
    dllPath = _GetApplicationPath();
    modulesIO = new ModulesIOList(modulesListFile, dllPath);
    writeText(moduleIOListFile, modulesIO->moduleInfoList);
    writeText(moduleParamListFile, modulesIO->moduleParamsInfoList);
    writeText(paramListFile, modulesIO->paramsInfoList);
    //system("pause");
    return 0;
    errexit:
    cout << "Error invoke format!" << endl;
    exit(0);
}