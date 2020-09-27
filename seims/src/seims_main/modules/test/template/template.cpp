#include "template.h"
#include "text.h"

// #include "Logging.h"

// DLL export not working with easyloggingpp?
// https://github.com/amrayn/easyloggingpp/issues/386
// INITIALIZE_NULL_EASYLOGGINGPP
//
// After a long time digging, I decided to not using
//   Easyloggingpp in the modulues (dynamic/shared libs)
//   and some classes (e.g., clsReach and clsSubbasin)
//   that defined in base libs (e.g., the data lib) and
//   invoked in several modules.
//   
//   Hope to get this issue solved in the future. -LJ.

ModuleTemplate::ModuleTemplate() :
    m_nCells(-1) {
}

ModuleTemplate::~ModuleTemplate() {
}

void ModuleTemplate::SetValue(const char* key, float value) {
}

void ModuleTemplate::SetValueByIndex(const char* key, int index, float value) {
}

void ModuleTemplate::Set1DData(const char* key, int n, float* data) {
}

void ModuleTemplate::Set2DData(const char* key, int n, int col, float** data) {
}

void ModuleTemplate::SetReaches(clsReaches* rches) {
}

void ModuleTemplate::SetSubbasins(clsSubbasins* subbsns) {
}

void ModuleTemplate::SetScenario(Scenario* sce) {
}

bool ModuleTemplate::CheckInputData() {
    return true;
}

void ModuleTemplate::InitialOutputs() {
}

int ModuleTemplate::Execute() {
    return 0;
}

TimeStepType ModuleTemplate::GetTimeStepType() {
    return TIMESTEP_HILLSLOPE;
}


void ModuleTemplate::GetValue(const char* key, float* value) {
}

void ModuleTemplate::Get1DData(const char* key, int* n, float** data) {
}

void ModuleTemplate::Get2DData(const char* key, int* n, int* col, float*** data) {
}
