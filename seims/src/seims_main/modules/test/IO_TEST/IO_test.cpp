#include "api.h"
#include <iostream>
#include "IO_test.h"
#include "MetadataInfo.h"
#include "ModelException.h"

using namespace std;

IO_TEST::IO_TEST(void) : m_nCells(-1), m_soilLayers(-1), m_raster1D(NULL), m_raster2D(NULL),
                         m_output1Draster(NULL), m_output2Draster(NULL), m_nSoilLayrs(NULL),
						 m_scenario(NULL), m_reaches(NULL)
{
}

IO_TEST::~IO_TEST(void)
{
    if (m_output1Draster != NULL)
        Release1DArray(m_output1Draster);
    if (m_output2Draster != NULL)
		Release2DArray(m_nCells, m_output2Draster);
    if (m_scenario != NULL)
        delete m_scenario;
    if (m_reaches != NULL)
        delete m_reaches;
}

void IO_TEST::Set1DData(const char *key, int n, float *data)
{
    if (!this->CheckInputSize(key, n)) return;
    string sk(key);
    if (StringMatch(sk, VAR_CN2)) this->m_raster1D = data;
	if (StringMatch(sk, VAR_SOILLAYERS))this->m_nSoilLayrs = data;
}

void IO_TEST::Set2DData(const char *key, int n, int col, float **data)
{
    string sk(key);
    if (StringMatch(sk, VAR_CONDUCT))
    {
        this->m_raster2D = data;
        this->m_soilLayers = col;
    }
	
}

void IO_TEST::SetScenario(MainBMP::Scenario *sce)
{
    if (NULL != sce)
        m_scenario = sce;
}

void IO_TEST::SetReaches(clsReaches *reaches)
{
    if (NULL != reaches)
        m_reaches = reaches;
}

bool IO_TEST::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException("IO_TEST", "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
            throw ModelException("IO_TEST", "CheckInputSize", "Input data for " + string(key) +
                                                              " is invalid. All the input data should have same size.");
    }
    return true;
}

bool IO_TEST::CheckInputData()
{
    if (this->m_date == -1) /// m_date is protected variable member in base class SimulationModule.
        throw ModelException("IO_TEST", "CheckInputData", "You have not set the time.");
    if (m_nCells <= 0)
        throw ModelException("IO_TEST", "CheckInputData", "The dimension of the input data can not be less than zero.");
    if (m_raster1D == NULL)
        throw ModelException("IO_TEST", "CheckInputData", "The 1D raster input data can not be NULL.");
    if (m_raster2D == NULL)
		throw ModelException("IO_TEST", "CheckInputData", "The 2D raster input data can not be NULL.");
	if (m_nSoilLayrs == NULL)
		throw ModelException("IO_TEST", "CheckInputData", "The soil layers of raster data can not be NULL.");
    return true;
}

int IO_TEST::Execute()
{
    /// Initialize output variables
    if (m_output1Draster == NULL)
		Initialize1DArray(m_nCells, m_output1Draster, 0.f);
    
    if (m_output2Draster == NULL)
		Initialize2DArray(m_nCells, m_soilLayers, m_output2Draster, NODATA_VALUE);
    
    /// Execute function
#pragma omp parallel for
    for (int i = 0; i < m_nCells; ++i)
    {
        m_output1Draster[i] = m_raster1D[i] * 0.5f;
        for (int j = 0; j < m_nSoilLayrs[i]; j++)
            m_output2Draster[i][j] = m_raster2D[i][j] + 2.f;
    }
    /// Write Scenario Information
   // m_scenario->Dump("e:\\test\\bmpScenario2.txt");
    int nReaches = m_reaches->GetReachNumber();
    vector<int> reachIDs = m_reaches->GetReachIDs();
    /// Get reach information by subbasin ID
    for (vector<int>::iterator it = reachIDs.begin(); it != reachIDs.end(); it++)
    {
        clsReach *reachTest = m_reaches->GetReachByID(*it);
        /// Get any fields of reach
        float slope = reachTest->GetSlope();
        //cout<<"Reach ID: "<<*it<<", Slope: "<<slope<<endl;
    }
    return 0;
}

void IO_TEST::Get1DData(const char *key, int *n, float **data)
{
    string sk(key);
    if (StringMatch(sk, "CN2_M"))
    {
        *data = this->m_output1Draster;
        *n = this->m_nCells;
    }
}

void IO_TEST::Get2DData(const char *key, int *n, int *col, float ***data)
{
    string sk(key);
    if (StringMatch(sk, "K_M"))
    {
        *data = this->m_output2Draster;
        *n = this->m_nCells;
        *col = this->m_soilLayers;
    }
}