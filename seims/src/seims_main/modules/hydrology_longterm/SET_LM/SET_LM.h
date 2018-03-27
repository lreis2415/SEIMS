/*!
 * \brief The method of soil actual ET linearly with actual soil moisture developed by
            Thornthwaite and Mather (1955), which was also adapted by WetSpa Extension.
 * \author Chunping Ou, Liangjun Zhu
 * \revised date 2018-3-23
 * \description 1.
 */
#ifndef SEIMS_MODULE_SET_LM_H
#define SEIMS_MODULE_SET_LM_H

#include "SimulationModule.h"

using namespace std;

/*!
 * \defgroup SET_LM
 * \ingroup Hydrology_longterm
 * \brief Calculate soil Temperature according to the linearly relationship with actual soil moisture
 */

/*!
 * \class SET_LM
 * \ingroup SET_LM
 * \brief Calculate soil Temperature according to the linearly relationship with actual soil moisture
 */
class SET_LM : public SimulationModule {
public:
    SET_LM();

    ~SET_LM();

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
    bool CheckInputData();

    /**
    *	@brief check the input size. Make sure all the input data have same dimension.
    *
    *	@param key The key of the input data
    *	@param n The input data dimension
    *	@return bool The validity of the dimension
    */
    bool CheckInputSize(const char *key, int n);

    void initialOutputs();

private:
    int m_nCells;
    float *m_soilLayers; ///< Soil layers number
    float **m_soilThick; ///< Soil thickness of each layer, mm

    float **m_sm; ///< soil moisture
    float **m_fc; ///< field capacity
    float **m_wp; ///< wilting point
    float *m_PET; ///< Potential evapotranspiration
    float *m_EI;  ///< Evaporation from interception
    float *m_ED;  ///< Evaporation from depression storage
    float *m_plantET;  ///< Evaporation from plant

    float *m_soilT;  ///< Soil temperature
    float m_frozenT; ///< Freezing temperature

    float *m_soilET; ///< Output, actual soil evaporation
};
#endif /* SEIMS_MODULE_SET_LM_H */
