#include "seims.h"
#include "template.h"

ModulesTest::ModulesTest() : m_nCells(-1) {
}

ModulesTest::~ModulesTest() {
}

void ModulesTest::Set1DData(const char *key, int n, float *data) {
}

void ModulesTest::Set2DData(const char *key, int n, int col, float **data) {
}

bool ModulesTest::CheckInputData() {
    return true;
}

bool ModulesTest::CheckInputSize(const char *, int) {
    return true;
}

int ModulesTest::Execute() {
    return true;
}

void ModulesTest::Get1DData(const char *key, int *n, float **data) {
}

void ModulesTest::Get2DData(const char *key, int *n, int *col, float ***data) {
}