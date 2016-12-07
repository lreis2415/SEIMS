#ifndef linux
#ifdef MODULE_EXPORTS
#define SEIMS_MODULE_API __declspec(dllexport)
#else
#define SEIMS_MODULE_API __declspec(dllimport)
#endif
#else
#define SEIMS_MODULE_API
#endif

#include "SimulationModule.h"

extern "C" SEIMS_MODULE_API const char *MetadataInformation(void);
extern "C" SEIMS_MODULE_API SimulationModule *GetInstance();


