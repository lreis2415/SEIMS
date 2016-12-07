/*!
 * \brief use MUSLE method to calculate sediment yield of each cell
 * \author Zhiqiang Yu
 * \date Feb. 2012
 * \revised LiangJun Zhu
 * \revised date May. 2016
 */
#pragma once

#include <string>
#include <ctime>
#include "api.h"

using namespace std;

#include "SimulationModule.h"
/** \defgroup MUSLE_AS
 * \ingroup Erosion
 * \brief use MUSLE method to calculate sediment yield of each cell
 */
/*!
 * \class MUSLE_AS
 * \ingroup MUSLE_AS
 *
 * \brief use MUSLE method to calculate sediment yield of each cell
 *
 */
class MUSLE_AS : public SimulationModule
{
public:
	//! Constructor
    MUSLE_AS(void);
	//! Destructor
    ~MUSLE_AS(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void GetValue(const char *key, float *value);

    virtual void Set1DData(const char *key, int n, float *data);

	virtual void Set2DData(const char *key, int nRows, int nCols, float **data);

	virtual void SetSubbasins(clsSubbasins *subbasins);

    virtual void Get1DData(const char *key, int *n, float **data);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputData(void);

private:
	//! valid cell number
    int m_nCells;
	//! cell width (m)
    float m_cellWidth;
	//! subbasin number
    int m_nsub;
	//! soil layer number
	int m_nSoilLayers;
	//! deposition ratio
	float m_depRatio;  // added by Wu Hui, 2013.11.16
    //grid from parameter
	//! sand fraction
	float *m_detachSand;
	//! silt fraction
	float *m_detachSilt;
	//! clay fraction
	float *m_detachClay;
	//! small aggregate fraction
	float *m_detachSmAggre;
	//! large aggregate fraction
	float *m_detachLgAggre;

	//! USLE P factor (Practice)
    float *m_usle_p;
	//! USLE K factor (erodibility), multi-layer paramters. By LJ
    float **m_usle_k;
	//! USLE C factor (land cover)
    float *m_usle_c;
	//! Slope gradient (drop/distance)
    float *m_slope;
	//! flow accumulation (number of accumulated cells)
    float *m_flowacc;
    //! stream link
    float *m_streamLink;
	////! Subbasin map
 //   float *m_subbasin;

	//! USLE LS factor
    float *m_usle_ls;
	//! cell area (A, km^2)
    float m_cellAreaKM;
	//! cell area factor (3.79 * A^0.7)
    float m_cellAreaKM1;
    //! cell area factor (0.903 * A^0.017)
    float m_cellAreaKM2;
    //! Slope^0.16
    float *m_slopeForPq;

    //grid from other modules

	//! snow accumulation
    float *m_snowAccumulation;
	//! surface runoff (mm)
    float *m_surfaceRunoff;

    //result

	//! sediment yield on each cell
    float *m_sedimentYield;
	//! sand yield
	float *m_sandYield;
	//! silt yield
	float *m_siltYield;
	//! clay yield
	float *m_clayYield;
	//! small aggregate yield
	float *m_smaggreYield;
	//! large aggregate yield
	float *m_lgaggreYield;
	//! initialize outputs
    void initialOutputs();
	//! 
    float getPeakRunoffRate(int);
};

