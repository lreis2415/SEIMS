#pragma once

#include <string>
#include "api.h"
#include "util.h"
#include "SimulationModule.h"

using namespace std;

class NPS_Management : public SimulationModule
{
private:
    /// valid cells number
    int m_nCells;
	/// cell width (m)
	float m_cellWidth;
	/// area of cell (m^2)
	float m_cellArea;
	/// time step (second)
	float m_timestep;
	/// management fields raster
	float *m_mgtFields;
	/* areal source operations
	 * key: unique index, BMPID * 100000 + subScenarioID
	 * value: areal source management factory instance
	 */
	map<int, BMPArealSrcFactory*> m_arealSrcFactory;

	/// variables to be updated (optionals)

	/// water storage of soil layers
	float **m_soilStorage;
	/// nitrate kg/ha
	float **m_sol_no3;
	/// ammonium kg/ha
	float **m_sol_nh4;
	/// soluble phosphorus kg/ha
	float **m_sol_solp;
	float **m_sol_orgn;
	float **m_sol_orgp;
public:
    //! Constructor
    NPS_Management(void);

    //! Destructor
    ~NPS_Management(void);

    virtual int Execute();

	virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Set2DData(const char *key, int n, int col, float **data);

    //virtual void Get1DData(const char *key, int *n, float **data);

    //virtual void Get2DData(const char *key, int *n, int *col, float ***data);

	virtual void SetScenario(Scenario *sce);

private:
    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData(void);

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);
};
