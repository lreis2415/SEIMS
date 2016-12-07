/*!
 * \brief Nutrient removed and lost in surface runoff.
 *        Support three carbon model, static method (orgn.f), C-FARM one carbon model (orgncswat.f),
 *                                    and CENTURY C/N cycling model (NCsed_leach.f90) from SWAT
 *        As for phosphorus, psed.f of SWAT calculates the attached to sediment in surface runoff.
 * \author Huiran Gao
 * \date April 2016
 * 
 * \revised Liang-Jun Zhu
 * \date 2016-9-28
 * \description: 1. Code revision.
 *               2. Add CENTURY model of calculating organic nitrogen removed in surface runoff
 * \TODO         1. Ammonian adsorbed to soil should be considered.
 */

#pragma once

#include <string>
#include "api.h"
#include "SimulationModule.h"
#include "NutrientCommon.h"
using namespace std;
/** \defgroup NUTRSED
 * \ingroup Nutrient
 * \brief Nutrient removed and lost with the eroded sediment in surface runoff
 */

/*!
 * \class NutrientTransportSediment
 * \ingroup NUTRSED
 *
 * \brief Nutrient removed and lost with the eroded sediment in surface runoff
 *
 */

class NutrientTransportSediment : public SimulationModule
{
public:
    NutrientTransportSediment(void);

    ~NutrientTransportSediment(void);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Set2DData(const char *key, int nRows, int nCols, float **data);

    virtual void SetValue(const char *key, float value);

	virtual void SetSubbasins(clsSubbasins *);

    virtual int Execute();

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

private:
    /// cell width of grid map (m)
    float m_cellWidth;
	/// cell area of grid map (ha)
	float m_cellArea;
    /// number of cells
    int m_nCells;
    /// soil layers
    float *m_nSoilLayers;
    /// maximum soil layers
    int m_soiLayers;
	/// soil rock content, %
	float **m_sol_rock;
	/// sol_ul, soil saturated water amount, mm
	float **m_sol_wsatur;
	/* carbon modeling method
     *   = 0 Static soil carbon (old mineralization routines)
     *   = 1 C-FARM one carbon pool model
     *   = 2 Century model
	 */
    int m_CbnModel;
	/// enrichment ratio
	float *m_enratio;

    ///inputs

    // soil loss caused by water erosion
    float *m_sedEroded;
    // surface runoff generated
    float *m_surfaceRunoff;
    //bulk density of the soil
    float **m_sol_bd;
    // thickness of soil layer
	float **m_soilThick;


	/// subbasin related
	/// the total number of subbasins
	int m_nSubbasins;
	//! subbasin IDs
	vector<int> m_subbasinIDs;
	/// subbasin grid (subbasins ID)
	float *m_subbasin;
	/// subbasins information
	clsSubbasins *m_subbasinsInfo;

    ///output data
    //amount of organic nitrogen in surface runoff
    float *m_sedorgn;
    //amount of organic phosphorus in surface runoff
    float *m_sedorgp;
    //amount of active mineral phosphorus sorbed to sediment in surface runoff
    float *m_sedminpa;
    //amount of stable mineral phosphorus sorbed to sediment in surface runoff
    float *m_sedminps;

	/// output to channel

	float *m_sedorgnToCh;  // amount of organic N in surface runoff to channel, kg
	float *m_sedorgpToCh;  // amount of organic P in surface runoff to channel, kg
	float *m_sedminpaToCh; // amount of active mineral P in surface runoff to channel, kg
	float *m_sedminpsToCh; // amount of stable mineral P in surface runoff to channel, kg

    ///input & output
    //amount of nitrogen stored in the active organic (humic) nitrogen pool, kg N/ha
    float **m_sol_aorgn;
    //amount of nitrogen stored in the fresh organic (residue) pool, kg N/ha
    float **m_sol_fon;
    //amount of nitrogen stored in the stable organic N pool, kg N/ha
    float **m_sol_orgn;
    //amount of phosphorus stored in the organic P pool, kg P/ha
    float **m_sol_orgp;
    //amount of phosphorus stored in the fresh organic (residue) pool, kg P/ha
    float **m_sol_fop;
    //amount of phosphorus in the soil layer stored in the stable mineral phosphorus pool, kg P/ha
    float **m_sol_stap;
    //amount of phosphorus stored in the active mineral phosphorus pool, kg P/ha
    float **m_sol_actp;
	/// for C-FARM one carbon model
	float **m_sol_mp;
	/// for CENTURY C/Y cycling model
	/// inputs from other modules
	float **m_sol_LSN;
	float **m_sol_LMN;
	float **m_sol_HPN;
	float **m_sol_HSN;
	float **m_sol_HPC;
	float **m_sol_HSC;
	float **m_sol_LMC;
	float **m_sol_LSC;
	float **m_sol_LS;
	float **m_sol_LM;
	float **m_sol_LSL;
	float **m_sol_LSLC;
	float **m_sol_LSLNC;
	float **m_sol_BMC;
	float **m_sol_WOC;
	float **m_sol_perco;
	float **m_sol_laterq;
	/// outputs
	float **m_sol_latC; /// lateral flow Carbon loss in each soil layer
	float **m_sol_percoC; /// percolation Carbon loss in each soil layer
	float *m_laterC; /// lateral flow Carbon loss in soil profile
	float *m_percoC; /// percolation Carbon loss in soil profile
	float *m_sedCLoss; /// amount of C lost with sediment pools 

private:

    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData(void);
	/*!
     * \brief check the input data for running CENTURY model. Make sure all the inputs data is available.
     * \return bool The validity of the inputs data.
     */
	bool CheckInputData_CENTURY(void);
	/*!
     * \brief check the input data for running C-FARM one carbon model. Make sure all the inputs data is available.
     * \return bool The validity of the inputs data.
     */
	bool CheckInputData_CFARM(void);
    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);

    /*!
     * \brief calculates the amount of organic nitrogen removed in surface runoff.
     *        orgn.f of SWAT, when CSWAT = 0
     * \return void
     */
    void OrgNRemovedInRunoff_StaticMethod(int i);

	/*!
     * \brief calculates the amount of organic nitrogen removed in surface runoff.
     *        orgnswat.f of SWAT, when CSWAT = 1
	 * \TODO THIS IS ON TODO LIST
     * \return void
     */
    void OrgNRemovedInRunoff_CFARMOneCarbonModel(int i);

	/*!
     * \brief calculates the amount of organic nitrogen removed in surface runoff.
     *        NCsed_leach.f90 of SWAT, when CSWAT = 2
     * \return void
     */
    void OrgNRemovedInRunoff_CENTURY(int i);

    /*!
     * \brief Calculates the amount of organic and mineral phosphorus attached to sediment in surface runoff.
     * psed.f of SWAT
     * \return void
     */
    void OrgPAttachedtoSed(int i);
	/// initial outputs
    void initialOutputs();
};
