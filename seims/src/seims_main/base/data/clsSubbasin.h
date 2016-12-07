#pragma once

#include <vector>
#include <map>
#include <string>
#include <cmath>
#include "mongoc.h"
#include "ModelException.h"
#include "clsRasterData.cpp"
using namespace std;

/*!
 * \class Subbasin
 * \ingroup base
 * \brief Subbasin related parameters and methods.
 * 
 * \Revision:   1. Remove isOutput, since the output is handled in printInfo class
 *              2. Add soil water balance related. 2016-7-28
 */
class Subbasin
{
public:
	/*
	 * \brief Constructor
	 * \param [in] id Subbasin ID
	 */
    Subbasin(int id);
	//! Destructor
    ~Subbasin(void);

private:
    //! Subbasin ID
    int m_id;
    //! valid cells number
    int m_nCells;
	//! index of valid cells
    int *m_cells;
	//! area of one cell
    float m_cellArea;    
	//! area of current Subbasin
    float m_Area; 

	//! PET
    float m_PET;
	//! average percolation (mm) of each valid cells 
    float m_PERCO;

    // Subbasin scale parameters' mean value
    
	// 1. Soil water balance related parameters
	
	//! precipitation
	float m_Pcp;
	//! interception loss
	float m_Interception;
	//! ET from interception storage
	float m_interceptionET;
	//! depression evaporation
	float m_DepressionET;
	//! infiltration loss
	float m_Infiltration;
	//! soil et
	float m_soilET;
	//! total ET
	float m_TotalET;
	//! net percolation
	float m_NetPercolation;
	//! surface runoff generated (mm)
	float m_RS;
	//! subsurface (interflow) runoff 
	float m_RI;
	//! runoff?
	float m_R;
	//! soil moisture (mm)
	float m_S_MOI;
	////! useless?
	//float m_moistureDepth;
	//! net precipitation
	float m_pNet;
	//! mean temperature
	float m_TMean;
	//! soil temperature
	float m_SoilT;
	// 2. Groundwater related parameters
    //float m_GW0;

	//! maximum groundwater storage
    float m_GWMAX;
	//! baseflow recession coefficient
    float m_kg;
	//! groundwater revaporization coefficient
    float m_dp_co;
	//! baseflow recession exponent
    float m_base_ex;
	//! convert coefficient from mm to m3/s
    float m_QGConvert;
	//! slope correction factor of current subbasin
    float m_slopeCoefficient;
	//! average slope of the subbasin
    float m_slope;
	//! revaporization from groundwater
    float m_Revap;
	//! initial groundwater or time (t-1)
    float m_GW;
	//! deep percolation
    float m_PERDE;
	//! groundwater discharge (m3/s)
    float m_QG;
	//! groundwater runoff (mm)
    float m_RG;
	//! Is output defined by file.out or not
    bool m_isOutput; 
	//! Is the revap (m_EG) is different from last time step
    bool m_isRevapChanged;
public:
	//! Check input size
	bool CheckInputSize(int n);
	//! Set cell index list, as well as subbasin area
    void setCellList(int nCells, int *cells);
	////! Set soil layers number of valid cells
	//void setSoilLayers(int nCells, int *soilLayers);
	////! Set parameters of current subbasin
 //   void setParameter4Groundwater(float rv_co, float GW0, float GWMAX, float kg, float base_ex, float cellWidth, int timeStep);
	
	
	////! set inputs for groundwater calculation
 //   void setInputs4Groundwater(float *PET, float *EI, float *ED, float *ES, float **PERCO, float groundwaterFromBankStorage);
	////! set inputs for soil water balance statistics
	//void setInputs4SoilWaterBalance();

	// Set functions

	//! area of subbasin
	void setArea(float area){m_Area = area;}
	//! average slope (%)
	void setSlope(float slp){m_slope = slp;}
	//! Set slope of current subbasin as the average of all cells
	void setSlope(float *slope);
	//! Set slope correction factor of current subbasin
	void setSlopeCoefofBasin(float slopeBasin){m_slopeCoefficient = slopeBasin;}

	//! pet
	void setPET(float pet){m_PET = pet;}
	//! Set average percolation (mm)
	void setPerco(float perco){m_PERCO = perco;};
	//! Set average deep percolation (mm)
	void setPerde(float perde){m_PERDE = perde;};
	//! groundwater revaporization
	void setEG(float eg){m_Revap = eg;}
	//! Set groundwater storage
	void setGW(float gw){m_GW = gw;}
	//! Set groundwater discharge
	void setQG(float qg){m_QG = qg;}
	//! Set groundwater runoff
	void setRG(float rg){m_RG = rg;};
	//!
	void setIsRevapChanged(bool isrevap){m_isRevapChanged = isrevap;};
	// Get functions
	
	//! Get subbasin ID
	int getId(){return m_id;};
	//! Get valid cells number
	int getCellCount(){return m_nCells;};
	//! Get index of valid cells
	int *getCells(){return m_cells;};
	////! Get soil layers number of valid cells
	//int *getSoilLayers(){return m_nSoilLayers;};
	//! Get the output flag (true mean output), the function will be deprecated. By LJ
	bool getIsOutput(){return m_isOutput;};
	//! area of subbasin
	float getArea(){return m_Area;}
	//! Get the Revap change flat (true mean changed from last time step)
	bool getIsRevapChanged(){return m_isRevapChanged;};


	//! Get average PET
	float getPET(){return m_PET;};
	//! Get average percolation (mm)
	float getPerco(){return m_PERCO;};
	//! Get average deep percolation (mm)
	float getPerde(){return m_PERDE;};
	//! Get average slope (%)
	float getSlope(){return m_slope;};
	//! Get slope coefficient of basin
	float getSlopeCoef(){return m_slopeCoefficient;};
	//! groundwater revaporization
	float getEG(){return m_Revap;};
	//! Get groundwater storage
	float getGW(){return m_GW;};
	//! Get groundwater discharge
	float getQG(){return m_QG;};
	//! Get groundwater runoff
	float getRG(){return m_RG;};

	/*
	 * \brief Get basin (watershed) scale variable (key) value
	 * \param [in] key Variable name which is defined in text.h
	 * \param [in] *subbasins Vector of all Subbasins
	 */
    //static float subbasin2basin(string key, vector<Subbasin *> *subbasins);
};

/*!
 *\class clsSubbasins
 * \ingroup base
 * \brief Manager all Subbasin related parameters and methods.
 */
class clsSubbasins
{
public:
    /*!
     * \brief Constructor
     *
     * Query and constructor basic subbasin's information from MongoDB
     *
     * \param[in] spatialData Spatial data database
     * \param[in] rsMap Map of rasters that have been loaded
     * \param[in] prefixID subbasin ID as prefix in MongoDB
     */
    clsSubbasins(mongoc_gridfs_t *spatialData, map<string, clsRasterData<float> *> &rsMap, int prefixID);

    /// Destructor
    ~clsSubbasins();

    /// Get single reach information by subbasin ID
    Subbasin *GetSubbasinByID(int id) { return m_subbasinsInfo.at(id); }

    /// Get subbasin number
    int GetSubbasinNumber() { return this->m_nSubbasins; }

    /// Get subbasin IDs
    vector<int>& GetSubbasinIDs() { return this->m_subbasinIDs; }

	/// Set slope coefficient of each subbasin
	void SetSlopeCoefficient();
	/*
	 * \brief Get basin (watershed) scale variable (key) value
	 * \param [in] key Variable name which is defined in text.h
	 */
    float subbasin2basin(string key);

private:
    /// Subbasins number
    int m_nSubbasins;
    /// Subbasin IDs
    vector<int> m_subbasinIDs;
    /* Map container to store all Subbasins information
     * key: Subbasin ID
     * value: Subbasin instance (pointer)
     */
    map<int, Subbasin *> m_subbasinsInfo;
};