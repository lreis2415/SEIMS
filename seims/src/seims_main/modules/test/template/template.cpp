#include "api.h"
#include <iostream>
#include "template.h"
#include "MetadataInfo.h"
#include "ModelException.h"

using namespace std;

ModulesTest::ModulesTest(void) : m_nCells(-1)
{
}


ModulesTest::~ModulesTest(void)
{
}

void ModulesTest::Set1DData(const char *key, int n, float *data)
{
}

void ModulesTest::Set2DData(const char *key, int n, int col, float **data)
{
}

int ModulesTest::Execute()
{
    return true;
}

void ModulesTest::Get1DData(const char *key, int *n, float **data)
{
}

void ModulesTest::Get2DData(const char *key, int *n, int *col, float ***data)
{
}