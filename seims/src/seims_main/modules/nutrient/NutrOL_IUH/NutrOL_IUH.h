
/*!
 * \file NutrOL_IUH.h
 * \brief IUH overland method to calculates the concentration of nutrient in overland flow.
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
/** \defgroup NutrOL_IUH
 * \ingroup Nutrient
 * \brief IUH overland method to calculates nutrient transformations in overland flow.
 */

/*!
 * \class NutrOL_IUH
 * \ingroup NutrOL_IUH
 *
 * \brief IUH overland method to calculates the concentration of nutrient in overland flow.
 *
 */

class NutrOL_IUH : public SimulationModule
{
public:
    NutrOL_IUH(void);

    ~NutrOL_IUH(void);

    virtual void SetValue(const char *key, float value);

    virtual void Set1DData(const char *key, int n, float *data);
    //virtual void Set2DData(const char *key, int nRows, int nCols, float **data);
    virtual int Execute();
    //virtual void GetValue(const char* key, float* value);
    virtual void Get1DData(const char *key, int *n, float **data);
    //virtual void Get2DData(const char* key, int* nRows, int* nCols, float*** data);
private:

    /// cell width of grid map (m)
    float m_cellWidth;
    /// number of cells
    int m_nCells;
    //int m_nLayers;
    ///  Routing layers according to the flow direction. There are not flow relationships within each layer. The first element in each layer is the number of cells in the layer
    //float **m_routingLayers;
    /// 2d array of flow in cells. The first element in each sub-array is the number of flow in cells in this sub-array
    //float **m_flowInIndex;
    /// length of time step (s)
    float m_TimeStep;

    // input data
    /// flow width (m) of overland plane
    //float *m_FlowWidth;
    /// channel width (zero for non-channel cells)
    //float *m_chWidth;
    /// stream link
	//float *m_streamLink;

	/// IUH of each grid cell (1/s)
	float **m_iuhCell;
	/// the number of columns of Ol_iuh
	int m_iuhCols;
	/// the total number of sub-basins
	int m_nsub;
	/// subbasin grid ( sub-watersheds ID)
	float *m_subbasin;

	//temporary
	int m_cellNutrCols;
	//float **m_cellFlow;
	float **m_cellsurqno3;
	float **m_celllatno3;
	float **m_cellno3gw;
	float **m_cellsurqsolp;
	float **m_cellminpgw;
	float **m_cellsedorgn;
	float **m_cellsedorgp;
	float **m_cellsedminpa;
	float **m_cellsedminps;
	//float **m_cellammo;
	//float **m_cellnitrite;
	float **m_cellcod;

    /// amount of nitrate transported with lateral flow
    float *m_latno3;
    /// amount of nitrate transported with surface runoff
    float *m_surqno3;
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
    /// amount of ammonium transported with lateral flow	// No calculation before
    //float* m_ammo;
	/// amount of nitrite transported with lateral flow	// No calculation before
	//float* m_nitrite;
    /// carbonaceous oxygen demand of surface runoff
    float *m_cod;

	// Variables to calculate the ratio that flow and sediment into channel
	/// sediment from soil erosion (kg)
	float *m_Soer;
	/// sediment in flow (kg)
	float *m_Sed_kg;
	/// sediment from soil erosion (mm)
	float *m_Suru;
	/// sediment in flow (mm)
	float *m_Q_Flow;

    /// output data
	////////////////////// For each bassin/////////////////////
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

    void initialOutputs();

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);

//     /*!
//     *	\brief calculate the sediment routing of overland flow.
//     *
//     *	\param ID The id of cell in grid map
//     *	\return the nutrient of flowing to channel for each channel cell, kg
//     */
//     float NutToChannel(int id, float nut);

	/*!
    *	\brief Calculate the ratio between sediment in flow and soil loss caused by water erosion.
    *
    *	\param ID The id of cell in grid map
    *	\return the ratio between sediment in flow and soil loss caused by water erosion
    */
	float CalculateSedinFlowFraction(int id);

	float CalculateFlowFraction(int id);

	/*!
    *	\brief Calculate the ratio between sediment in flow and soil loss caused by water erosion.
    *
	*	\param id, The id of cell in grid map
	*	\param nutr, nutrient in surface runoff, lateral flow or groundwater.
	*	\param cellNutr, temporary variable
	*	\param ratio, ratio that flow and sediment into channel
	*
    *	\return the new nutrient value
    */
	float **GetnewNutrient(int id, float *nutr, float **cellNutr, float ratio);
	
	float GettotalNutr(float *NutrToCh);
};

#endif





