
/*!
 * \file NutrientOLRoute.h
 * \brief Calculates the concentration of nutrient in overland flow.
 * \author Huiran Gao
 * \date Jun 2016
 */

#pragma once
#ifndef SEIMS_NutOLRout_PARAMS_INCLUDE
#define SEIMS_NutOLRout_PARAMS_INCLUDE

#include <string>
#include <map>
#include <vector>
#include "api.h"
#include "SimulationModule.h"

using namespace std;
/** \defgroup NutOLRout
 * \ingroup Nutrient
 * \brief Calculates nutrient transformations in overland flow.
 */

/*!
 * \class NutrientOLRoute
 * \ingroup NutOLRout
 *
 * \brief Calculates the concentration of nutrient in overland flow.
 *
 */

class NutrientOL_IKW : public SimulationModule
{
public:
    NutrientOL_IKW(void);

    ~NutrientOL_IKW(void);

    virtual void SetValue(const char *key, float value);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Set2DData(const char *key, int nRows, int nCols, float **data);

    virtual int Execute();

    //virtual void GetValue(const char* key, float* value);
    virtual void Get1DData(const char *key, int *n, float **data);
    //virtual void Get2DData(const char* key, int* nRows, int* nCols, float*** data);
private:

    /// cell width of grid map (m)
    float m_cellWidth;
    /// number of cells
    int m_nCells;
    int m_nLayers;
    ///  Routing layers according to the flow direction. There are not flow relationships within each layer. The first element in each layer is the number of cells in the layer
    float **m_routingLayers;
    /// 2d array of flow in cells. The first element in each sub-array is the number of flow in cells in this sub-array
    float **m_flowInIndex;
    /// length of time step (s)
    float m_TimeStep;

    // input data
    /// flow width (m) of overland plane
    float *m_FlowWidth;
    /// channel width (zero for non-channel cells)
    float *m_chWidth;
    /// stream link
	float *m_streamLink;

    /// amount of nitrate transported with lateral flow
    float *m_latno3;
    /// amount of nitrate transported with surface runoff
    float *m_surqno3;
    /// amount of ammonium transported with lateral flow
    //float* m_ammo;
    /// amount of soluble phosphorus in surface runoff
    float *m_surqsolp;
    /// nitrate loading to reach in groundwater
    float *m_no3gw;
    /// soluble P loading to reach in groundwater
    float *m_minpgw;
    //amount of organic nitrogen in surface runoff
    float *m_sedorgn;
    //amount of organic phosphorus in surface runoff
    float *m_sedorgp;
    //amount of active mineral phosphorus absorbed to sediment in surface runoff
    float *m_sedminpa;
    //amount of stable mineral phosphorus absorbed to sediment in surface runoff
    float *m_sedminps;
    /// carbonaceous oxygen demand of surface runoff
    float *m_cod;

	/// sediment in flow (kg)
	float *m_Sed_kg;
	/// outgoing sediment flux (kg/s)
	float *m_Qsn;
	/// the distribution of overland flow detachment (kg)
	float *m_SedDet;
	/// sediment deposition (kg)
	float *m_SedDep;

    //test
    float *m_ChV;
    float *m_QV;
    float *m_fract;

    /// output data
    /// NO3-N in surface runoff (kg)
    float *m_surqno3ToCh;
    /// NO3-N in lateral flow (kg)
    float *m_latno3ToCh;
    /// nitrate loading to reach in groundwater (kg)
    float *m_no3gwToCh;
    /// soluble phosphorus in surface runoff (kg)
    float *m_surqsolpToCh;
    /// soluble P loading to reach in groundwater (kg)
    float *m_minpgwToCh;
    /// organic nitrogen in surface runoff (kg)
    float *m_sedorgnToCh;
    /// organic phosphorus in surface runoff (kg)
    float *m_sedorgpToCh;
    /// active mineral phosphorus adsorbed to sediment in surface runoff (kg)
    float *m_sedminpaToCh;
    /// stable mineral phosphorus adsorbed to sediment in surface runoff (kg)
    float *m_sedminpsToCh;
    /// ammonium to reach in surface runoff (kg)
    float *m_ammoToCh;
    /// nitrite to reach in surface runoff (kg)
    float *m_nitriteToCh;
    /// cod to reach in surface runoff (kg)
    float *m_codToCh;

private:

    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData(void);

    void initial();

    void initialOutputs();

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);

    /*!
    * \brief Nutrient transformations in overland flow.
     *
     * \return void
     */
    void NutrientinOverland(int);

    /*!
    *	\brief calculate the sediment routing of overland flow.
    *
    *	\param ID The id of cell in grid map
    *	\return the nutrient of flowing to channel for each channel cell, kg
    */
    float NutToChannel(int i, float nut);

	  /*!
    *	\brief Calculate the ratio between sediment in flow and soil loss caused by water erosion.
    *
    *	\param ID The id of cell in grid map
    *	\return the ratio between sediment in flow and soil loss caused by water erosion
    */
	float CalculateSedinFlowFraction(int id);
	
};

#endif





