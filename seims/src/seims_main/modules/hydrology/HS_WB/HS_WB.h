/*!
 * \brief Hillslope water balance
 * \author Junzhi Liu
 * \date May 2011
 */
#ifndef SEIMS_MODULE_HS_WB_H
#define SEIMS_MODULE_HS_WB_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

/** \defgroup HS_WB
 * \ingroup Hydrology
 * \brief
 */
/*!
 * \class HS_WB
 * \ingroup HS_WB
 *
 * \brief
 *
 */
class HS_WB : public SimulationModule {
private:

    int m_nCells;
    time_t m_date;

    /// for routing calculation
    /// inputs
    float m_dtHs;
    float m_dtCh;
    float *m_streamLink;
    float *m_qs;
    float *m_qi;
    int m_nReaches;
    /// outputs
    float *m_qsInput;
    float *m_qiInput;
    /// temporary
    float *m_qsTemp;
    float *m_qiTemp;
    //////////////////////////////////////////////////

    /// for water balance calculation
    /// inputs
    int m_subbasinID;
    float *m_subbasin;
    float *m_pNet;
    float *m_rootdepth;
    float *m_infil;
    float *m_es;
    float *m_ri;
    float *m_percolation;
    float *m_revap;
    float *m_porosity;
    float *m_fieldCapacity;

    //result
    float *m_soilMoisture;            //distribution of soil moisture
    float **m_soilWaterBalance;    //time series result

    //variables used to output
    //float* m_precipitation;
    //float* m_interception;
    //float* m_depression;
    //float* m_ei;
    //float* m_ed;

    //float* m_se;
    //float* m_tMax;
    //float* m_tMin;
    //float* m_soilT;


public:
    //! Constructor
    HS_WB(void);

    //! Destructor
    ~HS_WB(void);

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int nRows, float *data);

    //virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    virtual void Get1DData(const char *key, int *nRows, float **data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

    virtual void SetReaches(clsReaches *reaches);

    virtual int Execute(void);

    bool CheckInputData(void);

    void InitialOutputs(void);

    /**
    *	@brief check the input size. Make sure all the input data have same dimension.
    *
    *	@param key The key of the input data
    *	@param n The input data dimension
    *	@return bool The validity of the dimension
    */
    bool CheckInputSize(const char *, int);
};

#endif /* SEIMS_MODULE_HS_WB_H */
