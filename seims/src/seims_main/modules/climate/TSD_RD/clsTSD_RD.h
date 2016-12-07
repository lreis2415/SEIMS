/** \defgroup TSD_RD
 * \ingroup Climate
 * \brief Read Time Series Data
 *  
 */
/*!
 * \file clsTSD_RD.h
 * \ingroup TSD_RD
 * \author Zhiqiang Yu
 * \date Apr. 2010
 *
 * 
 */
#pragma once

#include <string>
#include "api.h"
#include "util.h"
#include "SimulationModule.h"

using namespace std;

/*!
 * \class clsTSD_RD
 * \ingroup TSD_RD
 *
 * \brief Read Time Series Data, e.g., Maximum temperature.
 *
 */
class clsTSD_RD : public SimulationModule
{
private:
    /// data row number
    int m_Rows;
    /// time series data
    float *m_Data;
	int counter;
public:
    clsTSD_RD(void);

    ~clsTSD_RD(void);

    void Set1DData(const char *key, int n, float *data);

    void Get1DData(const char *key, int *n, float **data);
};

