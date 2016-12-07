#pragma once
#include <string>
#include <ctime>
#include "api.h"

using namespace std;
#include "SimulationModule.h"

class MUSLE_I30:public SimulationModule
{
public:
	MUSLE_I30(void);
	~MUSLE_I30(void);
	virtual int Execute();
	virtual void SetValue(const char* key, float data);
	virtual void Set1DData(const char* key, int n, float* data);
	virtual void Get1DData(const char* key, int* n, float** data);
	virtual void Set2DData(const char* key, int nRows, int nCols, float** data);

	bool CheckInputSize(const char* key, int n);
	bool CheckInputData(void);

private:

	time_t m_Date;//

	int m_cellSize;
	int m_cellWidth;

	//grid from parameter
	float* m_usle_p;
	float* m_usle_k;
	float* m_usle_c;
	float* m_slope;
	float* m_flowacc;

	float* m_usle_ls;   //LS factor
	float m_cellAreaKM;	//A in qp, km^2
	float* m_slopeForPq;//S^0.16 in qp

	//the following four variables are for calculation of 
	//peak runoff rate using modified rational method
	float* m_alpha_month;	//average half-hour rainfall fraction for each month, 1:3.2.2 P66
	float** m_p_stat;		//p_stat table, 0 - RAINHHMX, 1 - PCPMM, 2 - PCPD
	float  m_adj_pkr;
	float  m_rain_yrs;
	int    m_rndseed;
	float* m_p;
	float* m_snowMelt;
	float* m_t_concentration;

	//grid from module
	float* m_snowAccumulation;
	float* m_surfaceRunoff;

	//result
	float* m_sedimentYield;

	void initalOutputs();

	float getPeakRunoffRate(int);

	/**
	*	@brief this function generates a random number from a triangular distribution
	*			given X axis points at start, end, and peak Y value
	*	
	*	@param min lower limit for distribution
	*	@param mean monthly mean for distribution
	*	@param max upper limit for distribution
	*	@param seed random number seed
	*	@return float daily value generated for distribution
	*/
	float triangularDistribution(float min, float mean, float max, int* seed);

//!!    ~ ~ ~ PURPOSE ~ ~ ~
//!!    This function generates random numbers ranging from 0.0 to 1.0.
//!!    In the process of calculating the random number, the seed (x1) is 
//!!    set to a new value.
//!!    This function implements the prime-modulus generator
//!!    xi = 16807 xi Mod(2**(31) - 1)
//!!    using code which ensures that no intermediate result uses more than
//!!    31 bits
//!!    the theory behind the code is summarized in
//!!    Bratley, P., B.L. Fox and L.E. Schrage. 1983. A Guide to Simulation.
//!!        Springer-Verlag, New York. (pages 199-202)
	/**
	*	@brief generate random numbers ranging from 0.0 to 1.0.
	*	
	*	@param seed random number seed
	*	@return float random numbers ranging from 0.0 to 1.0
	*/
	float aunif(int* seed);

};

