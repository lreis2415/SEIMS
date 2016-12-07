/*!
 * \file NutrientMovementViaWater.h
 * \brief Simulates the loss of nitrate and phosphorus via surface runoff, 
 *        lateral flow, tile flow, and percolation out of the profile.
 *        Method of SWAT
 * \author Huiran Gao
 * \date May 2016
 */

#pragma once

#include <string>
#include "api.h"
#include "SimulationModule.h"
#include "NutrientCommon.h"

using namespace std;

/** \defgroup NutrMV
 * \ingroup Nutrient
 * \brief Simulates the loss of nitrate and phosphorus via surface runoff, lateral flow, tile flow, and percolation out of the profile.
 */

/*!
 * \class NutrientMovementViaWater
 * \ingroup NutrMV
 *
 * \brief Nutrient removed and loss in surface runoff, lateral flow, tile flow, and percolation out of the profile
 *
 */

class NutrientMovementViaWater : public SimulationModule
{
public:
    NutrientMovementViaWater(void);
    ~NutrientMovementViaWater(void);

    virtual void Set1DData(const char *key, int n, float *data);
    virtual void Set2DData(const char *key, int nRows, int nCols, float **data);
    virtual void SetValue(const char *key, float value);
    virtual int Execute();
    virtual void GetValue(const char *key, float *value);
    virtual void Get1DData(const char *key, int *n, float **data);
    //virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);
	virtual void SetSubbasins(clsSubbasins *subbasins);

private:
    /// cell width of grid map (m)
    float m_cellWidth;
	/// cell area, ha
	float m_cellArea;
    /// number of cells
    int m_nCells;
    /// soil layers
    float *m_nSoilLayers;
    /// maximum soil layers
    int m_soiLayers;
	/// stream link
	float *m_streamLink;
	/* carbon modeling method
     *   = 0 Static soil carbon (old mineralization routines)
     *   = 1 C-FARM one carbon pool model
     *   = 2 Century model
	 */
    int m_CbnModel;

    /// input data

	/// factor which converts kg/kg soil to kg/ha
	float **m_conv_wt;
    /// drainage tile flow in soil profile
    float m_qtile;
    /// Phosphorus soil partitioning coefficient
    float m_phoskd;
    /// phosphorus percolation coefficient (0-1)
    float m_pperco;
    /// nitrate percolation coefficient (0-1)
    float m_nperco;
	/// Conversion factor from CBOD to COD
	float m_cod_n;
	/// Reaction coefficient from CBOD to COD
	float m_cod_k;

    /// distribution of soil loss caused by water erosion
    float *m_sedimentYield;
    // fraction of porosity from which anions are excluded
    float *m_anion_excl;
    // distribution of surface runoff generated
    float *m_surfr;
    /// initial septic operational condition (active-1, failing-2, non_septic-0)
    float m_isep_opt;
    /// soil layer where drainage tile is located
    float *m_ldrain;
    /// crack volume potential of soil
    float *m_sol_crk;
	/// distance to the stream
	float *m_dis_stream;
    /// amount of water held in the soil layer at saturation
    float **m_sol_wsatur;

    /// lateral flow in soil layer
    float **m_flat;
    /// percolation from soil layer
    float **m_sol_perco;
    /// bulk density of the soil
    float **m_sol_bd;
    /// depth to bottom of soil layer
    float **m_sol_z;

	/// flow out index
	float *m_flowOutIndex;
	/**
    *	@brief Routing layers according to the flow direction
    *
    *	There are not flow relationships within each layer.
    *	The first element in each layer is the number of cells in the layer
    */
    float **m_routingLayers;
	/// number of routing layers
    int m_nRoutingLayers;
    /// amount of organic nitrogen in surface runoff
    float *m_sedorgn;
    /// average air temperature
    float *m_tmean;
	///percent organic carbon in soil layer (%)
	float **m_sol_cbn;
	/// soil thick of each layer (mm)
	float **m_sol_thick;

    /// output data
    /// amount of nitrate transported with lateral flow, kg/ha
    float *m_latno3;
    /// amount of nitrate percolating past bottom of soil profile
    float *m_perco_n;
	/// amount of solute P percolating past bottom of soil profile
	float *m_perco_p;
    /// amount of nitrate transported with surface runoff, kg/ha
    float *m_surqno3;
	/// amount of ammonian transported with surface runoff, kg/ha
	float *m_surqnh4;
    /// amount of soluble phosphorus in surface runoff
    float *m_surqsolp;
    /// carbonaceous oxygen demand of surface runoff
    float *m_surcod;
    /// chlorophyll-a concentration in water yield
    float *m_surchl_a;
    /// dissolved oxygen concentration in the surface runoff
    //float* m_doxq;
    /// dissolved oxygen saturation concentration
    //float* m_soxy;

	// N and P to channel
	float *m_latno3ToCh;  // amount of nitrate transported with lateral flow to channel, kg
	float *m_sur_no3ToCh; // amount of nitrate transported with surface runoff to channel, kg
	float *m_sur_nh4ToCh; // amount of ammonian transported with surface runoff to channel, kg
	float *m_sur_solpToCh;// amount of soluble phosphorus in surface runoff to channel, kg
	float *m_perco_n_gw;  // amount of nitrate percolating past bottom of soil profile sum by sub-basin, kg
	float *m_perco_p_gw;  // amount of solute P percolating past bottom of soil profile sum by sub-basin, kg

	// amount of COD to reach in surface runoff (kg)
	float *m_sur_codToCh; 

	/// subbasin related
	/// the total number of subbasins
	int m_nSubbasins;
	//! subbasin IDs
	vector<int> m_subbasinIDs;
	/// subbasin grid (subbasins ID)
	float *m_subbasin;
	/// subbasins information
	clsSubbasins *m_subbasinsInfo;

    /// input & output
    /// average annual amount of phosphorus leached into second soil layer
    float m_wshd_plch;

    /// amount of nitrogen stored in the nitrate pool in soil layer
    float **m_sol_no3;
    /// amount of phosphorus stored in solution
    float **m_sol_solp;

	/* CENTURY C/N cycling model related */
	/// amount of C lost with sediment, kg/ha, input from NUTRSED module
	float *m_sedc_d;
private:
    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData(void);

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);

    /*!
     * \brief Calculate the loss of nitrate via surface runoff, lateral flow, tile flow, and percolation out of the profile.
     *        mainly rewrited from nlch.f of SWAT
	 * 1. nitrate loss with surface flow
	 * 2. nitrate loss with subsurface flow (routing considered)
	 * 3. nitrate loss with percolation
     */
    void NitrateLoss();

    /*!
     * \brief Calculates the amount of phosphorus lost from the soil
     *        profile in runoff and the movement of soluble phosphorus from the first
     *        to the second layer via percolation.
     *		 rewrite from solp.f of SWAT
     */
    void PhosphorusLoss();
	/*
	 * \brief compute loadings of chlorophyll-a, BOD, and dissolved oxygen to the main channel
	 *        rewrite from subwq.f of SWAT
	 */
	void SubbasinWaterQuality();
    ///*!
    // * \brief Calculate enrichment ratio.
    // * enrsb.f of SWAT
    // * \return void
    // */
    //float *CalculateEnrRatio();

    void initialOutputs();

	void SumBySubbasin();
};




