

#include "basic.h"
#include "ModuleFactory.h"
#include "utils_filesystem.h"
#include "Logging.h"

INITIALIZE_EASYLOGGINGPP

void Usage(const string &error_msg = "")
{
    if (!error_msg.empty())
    {
        cout << "FAILURE: " << error_msg << endl
             << endl;
    }
    cout << " Usage: extract_metadata <output_path>" << endl
         << endl;
    exit(1);
}

int main(const int argc, const char **argv)
{
    /// Parse input arguments
    if (argc != 2)
    {
        Usage();
    }
    string output_path = argv[1];

    /// Initialize easylogging++
    START_EASYLOGGINGPP(argc, argv);
    Logging::init();
    //Logging::setLoggingToFile(output_path + SEP + "metadata_extract.log");
    Logging::setLogLevel(el::Level::Info, nullptr);

    string module_path = GetAppPath();

    vector<string> dll_paths;
    FindFiles(module_path.c_str(), "*.dll", dll_paths);
    vector<DLLINSTANCE> dllHandles;              // dynamic library handles (.dll in Windows, .so in Linux, and .dylib in macOS)
    map<string, InstanceFunction> instanceFuncs; // map of modules instance
    map<string, MetadataFunction> metadataFuncs; // Metadata map of modules
    for (auto dll_path : dll_paths)
    {
        string dll_name = GetCoreFileName(dll_path);
        cout << dll_name << endl;
        ModuleFactory::ReadDLL(module_path, dll_name, dll_name, dllHandles, instanceFuncs, metadataFuncs);
        MetadataFunction metadataInfo = metadataFuncs[dll_name];
        const char *current_metadata = metadataInfo();
        std::ostringstream oss;
        oss << ConcatFullName(output_path, dll_name, ".xml");

        std::ofstream myfile;
        myfile.open(oss.str().c_str());
        myfile << current_metadata;
        myfile.close();

         //parse the metadata. Not used since some xml output is not well-formed.
         //TiXmlDocument doc;
         //doc.Parse(current_metadata);
         //doc.Print();
         //doc.SaveFile(oss.str().c_str());
    }

    return 0;
}
