/*!
 * \brief Implementation of invoking SEIMS
 * \author Junzhi Liu, Liangjun Zhu
 * \date May 2010
 * \revised Feb 2017
 */
#include "invoke.h"

void checkTable(vector<string> &tableNameList, string dbName, const char *tableName)
{
    //ostringstream oss;
    //oss << dbName << "." << tableName;
    //if (tableNameList.find(oss.str()) == tableNameList.end())
    vector<string>::iterator it = find(tableNameList.begin(), tableNameList.end(), string(tableName));
    if (it == tableNameList.end())
    {
        cerr << "The " << tableName << " table does not exist in database: " << dbName.c_str() << "\n";
        exit(-1);
    }
}

void checkDatabase(mongoc_client_t *conn, string dbName)
{
	vector<string> tableNames = MongoDatabase(conn, dbName).getCollectionNames();
    checkTable(tableNames, dbName, DB_TAB_PARAMETERS);
#ifndef MULTIPLY_REACHES
    checkTable(tableNames, dbName, DB_TAB_REACH);
#endif
    checkTable(tableNames, dbName, DB_TAB_SPATIAL);
    //checkTable(tableNames, dbName, DB_TAB_LOOKUP_LANDUSE);
    //checkTable(tableNames, dbName, DB_TAB_LOOKUP_SOIL);
    //checkTable(tableNames, dbName, DB_TAB_SITES);
}

void checkProject(string projectPath)
{
    string checkFilePath = projectPath + File_Config;
    if (!FileExists(checkFilePath))
        throw ModelException("ModelMain", "checkProject", checkFilePath +
                                                          " does not exist or has not the read permission!");

    checkFilePath = projectPath + File_Input;
    if (!FileExists(checkFilePath))
        throw ModelException("ModelMain", "checkProject", checkFilePath +
                                                          " does not exist or has not the read permission!");

    checkFilePath = projectPath + File_Output;
    if (!FileExists(checkFilePath))
        throw ModelException("ModelMain", "checkProject", checkFilePath +
                                                          " does not exist or has not the read permission!");
	// create OUTPUT folder and empty it

}

void MainMongoDB(string modelPath, char *host, int port, int scenarioID, int numThread, LayeringMethod layeringMethod)
{
    try{
		/// 1. Get paths and model name
		string exePath = GetAppPath();
        string modulePath = exePath + SEP;
        string projectPath = modelPath + SEP;
		string configFile = projectPath + File_Config;
		checkProject(projectPath);
		size_t nameIdx = modelPath.rfind(SEP);
		string dbName = modelPath.substr(nameIdx + 1);

		/// 2. Connect to MongoDB database, and make sure the required data is available.
		MongoClient dbclient = MongoClient(host, port);
		mongoc_client_t *conn = dbclient.getConn();

        /// TODO: ADD CHECK DATABASE AND TABLE, LJ.
		vector<string> dbnames = dbclient.getDatabaseNames();
		if (!ValueInVector(dbName, dbnames))
			throw ModelException(MODEL_NAME, "MainMongoDB", "Database: " + dbName + " is not existed in MongoDB!\n");
        /// CHECK FINISHED

		/// 3. Create main model according to subbasin number, 0 means the whole basin.
        int nSubbasin = 0;
		/// 3.1 Load model basic Input (e.g. simulation period) from "file.in" file or MongoDB
        /// SettingsInput *input = new SettingsInput(projectPath + File_Input, conn, dbName, nSubbasin);
        SettingsInput *input = new SettingsInput(conn, dbName, nSubbasin);
		/// 3.2 Constructor module factories by "config.fig" file
        ModuleFactory *factory = new ModuleFactory(configFile, modulePath, conn, dbName, nSubbasin, layeringMethod, scenarioID);
		/// 3.3 Constructor SEIMS model, BTW, SettingsOutput is created in ModelMain.
        ModelMain main(conn, dbName, projectPath, input, factory, nSubbasin, scenarioID, numThread, layeringMethod);
        
		/// 4. Run SEIMS model and export outputs.
		main.Execute();
		main.Output();
    }
    catch (ModelException e){
        cout << e.toString() << endl;
	}
    catch (exception e){
        cout << e.what() << endl;
	}
}
