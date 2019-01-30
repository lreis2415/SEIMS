#include "template.h"

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


bool ModuleTemplate::CheckInputSize(const char* key, const int n) {
    if (n <= 0) return false;
    if (m_nCells == n) return true;
    if (m_nCells <= 0) {
        m_nCells = n;
    } else {
        throw ModelException("ModuleTemplate", "CheckInputSize", "Input data for "
                             + string(key) + " is invalid with size: " + ValueToString(n)
                             + ". The origin size is " + ValueToString(m_nCells) + ".\n");
    }
    return true;
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
