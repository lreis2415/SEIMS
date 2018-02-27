#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#ifndef USE_MONGODB
#define USE_MONGODB
#endif /* USE_MONGODB */

#include "seims.h"
#include "invoke.h"

int main(int argc, const char **argv) {
    /// Parse input arguments
    InputArgs *input_args = InputArgs::Init(argc, argv);
    if (nullptr == input_args) { exit(EXIT_FAILURE); }
    /// Register GDAL
    GDALAllRegister();
    /// Run model.
    try {
        MainMongoDB(input_args);
    }
    catch (ModelException &e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
    }
    catch (exception &e) {
        cout << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    catch (...) {
        cout << "Unknown exception occurred!" << endl;
        exit(EXIT_FAILURE);
    }
    return 0;
}
