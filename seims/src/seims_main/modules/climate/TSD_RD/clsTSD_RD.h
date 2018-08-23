/*!
 * \file clsTSD_RD.h
 * \brief Read hydroclimate data
 * \author Zhiqiang Yu
 * \date Apr. 2010
 */
#ifndef SEIMS_MODULE_TSD_RD_H
#define SEIMS_MODULE_TSD_RD_H

#include "SimulationModule.h"

/** \defgroup TSD_RD
 * \ingroup Climate
 * \brief Read Time Series Data
 */
/*!
 * \class clsTSD_RD
 * \ingroup TSD_RD
 * \brief Read Time Series Data, e.g., Maximum temperature.
 */
class clsTSD_RD: public SimulationModule {
public:
    clsTSD_RD();

    ~clsTSD_RD();

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

private:
    /// data row number, i.e., number of stations
    int m_nStations;
    /// time series data
    float* m_stationData;
};
#endif /* SEIMS_MODULE_TSD_RD_H */
