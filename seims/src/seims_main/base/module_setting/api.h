/*!
 * \brief API definition of SEIMS modules
 * \author Junzhi Liu
 * \date 2011
 */
#ifndef SEIMS_MODULE_API_H
#define SEIMS_MODULE_API_H
#ifndef SEIMS_MODULE_API
#ifdef MSVC
#ifdef MODULE_EXPORTS
#define SEIMS_MODULE_API __declspec(dllexport)
#else
#define SEIMS_MODULE_API __declspec(dllimport)
#endif
#else
#define SEIMS_MODULE_API
#endif

#include "SimulationModule.h"

extern "C" SEIMS_MODULE_API const char* MetadataInformation();
extern "C" SEIMS_MODULE_API SimulationModule* GetInstance();

#endif
#endif /* SEIMS_MODULE_API_H */
