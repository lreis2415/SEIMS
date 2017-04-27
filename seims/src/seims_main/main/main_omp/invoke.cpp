#include "invoke.h"

#include <memory>

void checkTable(vector <string> &tableNameList, string dbName, const char *tableName) {
    //ostringstream oss;
    //oss << dbName << "." << tableName;
    //if (tableNameList.find(oss.str()) == tableNameList.end())
    vector<string>::iterator it = find(tableNameList.begin(), tableNameList.end(), string(tableName));
    if (it == tableNameList.end()) {
        cerr << "The " << tableName << " table does not exist in database: " << dbName.c_str() << "\n";
        exit(-1);
    }
}

void checkDatabase(mongoc_client_t *conn, string dbName) {
    vector <string> tableNames;
    MongoDatabase(conn, dbName).getCollectionNames(tableNames);
    checkTable(tableNames, dbName, DB_TAB_PARAMETERS);
#ifndef MULTIPLY_REACHES
    checkTable(tableNames, dbName, DB_TAB_REACH);
#endif /* not MULTIPLY_REACHES */
    checkTable(tableNames, dbName, DB_TAB_SPATIAL);
    //checkTable(tableNames, dbName, DB_TAB_LOOKUP_LANDUSE);
    //checkTable(tableNames, dbName, DB_TAB_LOOKUP_SOIL);
    //checkTable(tableNames, dbName, DB_TAB_SITES);
}

void checkProject(string projectPath) {
    string checkFilePath = projectPath + File_Config;
    if (!FileExists(checkFilePath)) {
        throw ModelException("ModelMain", "checkProject", checkFilePath +
            " does not exist or has not the read permission!");
    }

    checkFilePath = projectPath + File_Input;
    if (!FileExists(checkFilePath)) {
        throw ModelException("ModelMain", "checkProject", checkFilePath +
            " does not exist or has not the read permission!");
    }

    checkFilePath = projectPath + File_Output;
    if (!FileExists(checkFilePath)) {
        throw ModelException("ModelMain", "checkProject", checkFilePath +
            " does not exist or has not the read permission!");
    }
    // create OUTPUT folder and empty it

}

int MainMongoDB(string modelPath, char *host, int port, int scenarioID, int numThread, LayeringMethod layeringMethod) {
    /// 1. Get paths and model name
    string exePath = GetAppPath();
    string modulePath = exePath;
    string projectPath = modelPath + SEP;
    string configFile = projectPath + File_Config;
    checkProject(projectPath);
    size_t nameIdx = modelPath.rfind(SEP);
    string dbName = modelPath.substr(nameIdx + 1);

    /// 2. Connect to MongoDB database, and make sure the required data is available.
    MongoClient *dbclient = MongoClient::Init(host, port);
    if (NULL == dbclient) {
        throw ModelException(MODEL_NAME, "MainMongoDB", "Failed to connect to MongoDB!\n");
    }

    /// TODO: ADD CHECK DATABASE AND TABLE, LJ.
    vector<string> dbnames;
    dbclient->getDatabaseNames(dbnames);
    if (!ValueInVector(dbName, dbnames)) {
        throw ModelException(MODEL_NAME, "MainMongoDB", "Database: " + dbName + " is not existed in MongoDB!\n");
    }
    /// CHECK FINISHED

    /// 3. Create main model according to subbasin number, 0 means the whole basin.
    int nSubbasin = 0;
    /// old version
    /*/// 3.1 Load model basic Input (e.g. simulation period) from "file.in" file or MongoDB
    /// SettingsInput *input = new SettingsInput(projectPath + File_Input, conn, dbName, nSubbasin);
    SettingsInput *input = new SettingsInput(conn, dbName, nSubbasin);
    /// 3.2 Constructor module factories by "config.fig" file
    ModuleFactory *factory = new ModuleFactory(configFile, modulePath, conn, dbName, nSubbasin, layeringMethod,
                                                scenarioID);
    /// 3.3 Constructor SEIMS model, BTW, SettingsOutput is created in ModelMain.
    ModelMain main(conn, dbName, projectPath, input, factory, nSubbasin, scenarioID, numThread, layeringMethod);*/
    /// update by Liangjun, 4-27-2017
    unique_ptr<ModelMain> main(new ModelMain(dbclient, dbName, projectPath, modulePath, layeringMethod, nSubbasin, scenarioID, numThread));
    //ModelMain main(dbclient, dbName, projectPath, modulePath, layeringMethod, nSubbasin, scenarioID, numThread);
    /// Run SEIMS model and export outputs.
    //main.Execute();
    //main.Output();

    main->Execute();
    main->Output();

    /// Return success
    return 0;
}
