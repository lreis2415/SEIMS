#pragma once

#include <string>
#include "api.h"
#include "util.h"
#include "SimulationModule.h"
#include "Scenario.h"
#include "clsReach.h"

using namespace MainBMP;
using namespace std;

class IO_TEST : public SimulationModule
{
private:
    /// valid cells number
    int m_nCells;
    /// input 1D raster data
    float *m_raster1D;
    /// maximum number of soil layers
    int m_soilLayers;
	/// soil layers
	float *m_nSoilLayrs;
    /// input 2D raster data
    float **m_raster2D;
    /// output 1D raster data
    float *m_output1Draster;
    /// output 2D raster data
    float **m_output2Draster;
    /// BMPs Scenario data
    Scenario *m_scenario;
    /// Reach information
    clsReaches *m_reaches;
public:
    IO_TEST(void);

    ~IO_TEST(void);

    int Execute();

    void Set1DData(const char *key, int n, float *data);

    void Get1DData(const char *key, int *n, float **data);

    void Set2DData(const char *key, int n, int col, float **data);

    void Get2DData(const char *key, int *n, int *col, float ***data);

    void SetScenario(Scenario *sce);

    void SetReaches(clsReaches *reaches);

private:
    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData(void);

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);
};

