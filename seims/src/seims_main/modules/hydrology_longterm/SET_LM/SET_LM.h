#pragma once

#include <string>
#include <vector>
#include <string>
#include <sstream>
#include "api.h"
#include "SimulationModule.h"

using namespace std;

/** \defgroup SET_LM
 * \ingroup Hydrology_longterm
 * \brief Calculate soil Temperature
 *
 */

/*!
 * \class SET_LM
 * \ingroup SET_LM
 * \brief Calculate soil Temperature
 * 
 */

class SET_LM : public SimulationModule
{
private:

    int m_nCells;
    int m_nSoilLayers;
    float m_upSoilDepth;

    float **m_sm;
    float **m_fc;
    float **m_wp;
    float *m_PET;
    float *m_EI;
    float *m_ED;
    float *m_rootDepth;

    float *m_soilT;
    float m_frozenT;

    float *m_soilET;

public:
    SET_LM(void);

    ~SET_LM(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int nRows, float *data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    virtual void Get1DData(const char *key, int *nRows, float **data);

private:
    /**
    *	@brief check the input data. Make sure all the input data is available.
    *
    *	@return bool The validity of the input data.
    */
    bool CheckInputData(void);

    /**
    *	@brief checke the input size. Make sure all the input data have same dimension.
    *
    *	@param key The key of the input data
    *	@param n The input data dimension
    *	@return bool The validity of the dimension
    */
    bool CheckInputSize(const char *, int);
};

