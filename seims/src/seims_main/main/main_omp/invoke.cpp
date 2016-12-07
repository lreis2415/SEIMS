/*!
 * \brief Implementation of invoking SEIMS
 * \author Junzhi Liu
 * \date May 2010
 *
 * 
 */
#include "invoke.h"

#define BUFSIZE 255

string _GetApplicationPath()
{
    string RootPath;
#ifndef linux
    TCHAR buffer[BUFSIZE];
    GetModuleFileName(NULL, buffer, BUFSIZE);
    RootPath = string((char *) buffer);
#else
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
    static char buf[PATH_MAX];
    int rslt = readlink("/proc/self/exe", buf, PATH_MAX);
    if(rslt < 0 || rslt >= PATH_MAX)
        buf[0] = '\0';
    else
        buf[rslt] = '\0';
    RootPath = buf;
#endif
    basic_string<char>::size_type idx = RootPath.find_last_of(SEP);
    RootPath = RootPath.substr(0, idx + 1);
    return RootPath;
}


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
    vector<string> tableNames;
    GetCollectionNames(conn, dbName, tableNames);
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
    utils util;
    string checkFilePath = projectPath + File_Config;
    if (!util.FileExists(checkFilePath))
        throw ModelException("ModelMain", "checkProject", checkFilePath +
                                                          " does not exist or has not the read permission!");

    checkFilePath = projectPath + File_Input;
    if (!util.FileExists(checkFilePath))
        throw ModelException("ModelMain", "checkProject", checkFilePath +
                                                          " does not exist or has not the read permission!");

    checkFilePath = projectPath + File_Output;
    if (!util.FileExists(checkFilePath))
        throw ModelException("ModelMain", "checkProject", checkFilePath +
                                                          " does not exist or has not the read permission!");
	// create OUTPUT folder and empty it
	
}

bool isIPAddress(const char *ip)
{
    const char *pChar;
    bool rv = true;
    int tmp1, tmp2, tmp3, tmp4, i;

    while (1)
    {
#ifdef MSVC
        i = sscanf_s(ip,"%d.%d.%d.%d",&tmp1, &tmp2, &tmp3, &tmp4);
#else
        i = sscanf(ip, "%d.%d.%d.%d", &tmp1, &tmp2, &tmp3, &tmp4);
#endif

        if (i != 4)
        {
            rv = false;
            cout << "IP Address format is not correct!" << endl;
            break;
        }

        if ((tmp1 > 255) || (tmp2 > 255) || (tmp3 > 255) || (tmp4 > 255) || (tmp1 < 0) || (tmp2 < 0) || (tmp3 < 0) ||
            (tmp4 < 0))
        {
            rv = false;
            cout << "IP Address format is not correct!" << endl;
            break;
        }

        for (pChar = ip; *pChar != 0; pChar++)
        {
            if ((*pChar != '.') && ((*pChar < '0') || (*pChar > '9')))
            {
                rv = false;
                cout << "IP Address format is not correct!" << endl;
                break;
            }
        }
        break;
    }
    return rv;
}

bool isPathExists(const char *path)
{
    bool isExists;
#ifndef linux
    struct _stat fileStat;
    isExists = (_stat(path, &fileStat) == 0) && (fileStat.st_mode & _S_IFDIR);
#else
    struct stat fileStat;
    isExists = (stat(path, &fileStat) == 0) && S_ISDIR(fileStat.st_mode);
#endif
    return isExists;
}

void MainMongoDB(string modelPath, char *host, int port, int scenarioID, int numThread, LayeringMethod layeringMethod)
{
    try
    {
        string exePath = _GetApplicationPath();
        string projectPath = modelPath + SEP;
        string modulePath = exePath + SEP;
        checkProject(projectPath);

        size_t nameIdx = modelPath.rfind(SEP);
        string dbName = modelPath.substr(nameIdx + 1);
		string configFile = projectPath + File_Config;
        mongoc_client_t *conn;
        if (!isIPAddress(host))
            throw ModelException("MainMongoDB", "Connect to MongoDB",
                                 "IP address: " + string(host) + "is invalid, Please check!\n");
        mongoc_init();
        mongoc_uri_t *uri = mongoc_uri_new_for_host_port(host, port);
        conn = mongoc_client_new_from_uri(uri);
        /// Check the connection to MongoDB is success or not
        bson_t *reply = bson_new();
        bson_error_t *err = NULL;
        if (!mongoc_client_get_server_status(conn, NULL, reply, err))
            throw ModelException(MODEL_NAME, "MainMongoDB", "Failed to connect to MongoDB!\n");
        bson_destroy(reply);

        /// TODO: ADD CHECK DATABASE AND TABLE, LJ.
        char **dbnames;
        unsigned i;
        bool dbExist = false;
        dbnames = mongoc_client_get_database_names(conn, err);
        if (err == NULL)
        {
            for (i = 0; dbnames[i]; i++)
            {
                if (StringMatch(string(dbnames[i]), dbName))
                {
                    dbExist = true;
                    break;
                }
            }
            bson_strfreev(dbnames);
        }
        if (!dbExist)
            throw ModelException(MODEL_NAME, "MainMongoDB", "Database: " + dbName + " is not existed in MongoDB!\n");
        /// CHECK FINISHED

        int nSubbasin = 0; // updated 2016-10-28, 0 means the whole basin. By LJ
		/// Load Setting Input from file.in, which is deprecated now! By LJ
        /// SettingsInput *input = new SettingsInput(projectPath + File_Input, conn, dbName, nSubbasin);
        SettingsInput *input = new SettingsInput(conn, dbName, nSubbasin);
        ModuleFactory *factory = new ModuleFactory(configFile, modulePath, conn, dbName, nSubbasin, layeringMethod, scenarioID);
		/// Setting Output is loaded in ModelMain. 
        ModelMain main(conn, dbName, projectPath, input, factory, nSubbasin, scenarioID, numThread, layeringMethod);
        main.Execute();
		main.Output();

        mongoc_uri_destroy(uri);
        mongoc_cleanup();
    }
    catch (ModelException e)
    {
        cout << e.toString() << endl;
    }
    catch (exception e)
    {
        cout << e.what() << endl;
    }
}