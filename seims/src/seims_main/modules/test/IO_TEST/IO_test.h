/*!
 * \brief A IO test demo of developing module for SEIMS.
 *        Refer https://github.com/lreis2415/SEIMS/wiki/Module-demo for more details.
 * \author Liangjun Zhu
 * \date last updated 2018-02-07
 */

#ifndef SEIMS_MODULE_IO_TEST_H
#define SEIMS_MODULE_IO_TEST_H

#include "SimulationModule.h"
#include "Scenario.h"
#include "clsReach.h"

using namespace bmps;

class IO_TEST : public SimulationModule {
public:
    IO_TEST();

    virtual ~IO_TEST();

    int Execute() OVERRIDE;

    void Set1DData(const char *key, int n, float *data) OVERRIDE;

    void Get1DData(const char *key, int *n, float **data) OVERRIDE;

    void Set2DData(const char *key, int n, int col, float **data) OVERRIDE;

    void Get2DData(const char *key, int *n, int *col, float ***data) OVERRIDE;

    void SetScenario(Scenario *sce) OVERRIDE;

    void SetReaches(clsReaches *reaches) OVERRIDE;

private:
    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData();

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *key, int n);

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
};
#endif /* SEIMS_MODULE_IO_TEST_H */
