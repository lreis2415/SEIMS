#pragma once

#include <string>
#include <vector>
#include <string>
#include <sstream>
#include "api.h"
#include "SimulationModule.h"

using namespace std;


class UnsaturatedFlow : public SimulationModule
{
private:

    int m_cellSize;

    float *m_Moist;
    float *m_FieldCap;
    float *m_WiltPoint;
    float *m_PET;
    float *m_EI;
    float *m_ED;
    float *m_rootDepth;
    float *m_infiltration;
    float *m_percolation;
    float *m_interflow;

    float *m_SoilT;
    float m_ForzenT;

    float *m_D_SOET;

public:
    UnsaturatedFlow(void);

    ~UnsaturatedFlow(void);

    virtual int Execute();

    virtual void Set1DData(const char *key, int nRows, float *data);

    virtual void SetValue(const char *key, float data);

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

    static string toString(float value);
};

