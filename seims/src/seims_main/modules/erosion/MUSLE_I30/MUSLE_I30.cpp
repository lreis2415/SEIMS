#include "MUSLE_I30.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"

MUSLE_I30::MUSLE_I30(void)
{
	// set default values for member variables	
	this->m_Date = -1;
	this->m_cellSize = -1;
	this->m_cellWidth = -1;

	this->m_usle_c = NULL;
	this->m_usle_p = NULL;
	this->m_usle_k = NULL;
	this->m_flowacc = NULL;
	this->m_slope = NULL;

	this->m_usle_ls = NULL;
	this->m_slopeForPq = NULL;

	this->m_snowAccumulation = NULL;
	this->m_surfaceRunoff = NULL;

	this->m_sedimentYield = NULL;

	m_alpha_month = NULL;
	m_p_stat = NULL;
	m_adj_pkr = -99.0f;
	m_rain_yrs = -1;
	m_rndseed = -1;
	m_p = NULL;
	m_snowMelt = NULL;
	m_t_concentration = NULL;
}

MUSLE_I30::~MUSLE_I30(void)
{
	//// cleanup
	if(this->m_sedimentYield != NULL) delete [] this->m_sedimentYield;
	if(this->m_usle_ls != NULL) delete [] this->m_usle_ls;
	if(this->m_slopeForPq != NULL) delete [] this->m_slopeForPq;
	if(this->m_alpha_month != NULL) delete [] this->m_alpha_month;
}

bool MUSLE_I30::CheckInputData(void)
{
	if(this->m_Date <=0)			throw ModelException("MUSLE_I30","CheckInputData","You have not set the time.");
	if(m_cellSize <= 0)				throw ModelException("MUSLE_I30","CheckInputData","The dimension of the input data can not be less than zero.");
	if(m_cellWidth <= 0)			throw ModelException("MUSLE_I30","CheckInputData","The cell width can not be less than zero.");
	if(this->m_usle_c == NULL)		throw ModelException("MUSLE_I30","CheckInputData","The factor C can not be NULL.");
	if(this->m_usle_k == NULL)		throw ModelException("MUSLE_I30","CheckInputData","The factor K can not be NULL.");
	if(this->m_usle_p == NULL)		throw ModelException("MUSLE_I30","CheckInputData","The factor P can not be NULL.");
	if(this->m_flowacc == NULL)		throw ModelException("MUSLE_I30","CheckInputData","The flow accumulation can not be NULL.");
	if(this->m_slope == NULL)		throw ModelException("MUSLE_I30","CheckInputData","The slope can not be NULL.");
	if(this->m_snowAccumulation == NULL)	throw ModelException("MUSLE_I30","CheckInputData","The snow accumulation can not be NULL.");
	if(this->m_surfaceRunoff == NULL)		throw ModelException("MUSLE_I30","CheckInputData","The surface runoff can not be NULL.");

	if(this->m_p_stat == NULL)		throw ModelException("MUSLE_I30","CheckInputData","The p_stat can not be NULL.");
	if(this->m_p == NULL)			throw ModelException("MUSLE_I30","CheckInputData","The precipitation can not be NULL.");
	if(this->m_snowMelt == NULL)	throw ModelException("MUSLE_I30","CheckInputData","The snow melt can not be NULL.");
	if(this->m_t_concentration == NULL)		throw ModelException("MUSLE_I30","CheckInputData","The time of concentration can not be NULL.");
	if(this->m_adj_pkr <=0)			throw ModelException("MUSLE_I30","CheckInputData","You have not set adj_pkr.");
	if(this->m_rain_yrs <=0)		throw ModelException("MUSLE_I30","CheckInputData","You have not set ran_yrs.");

	return true;
}

void MUSLE_I30::initalOutputs()
{
	if(m_sedimentYield == NULL) m_sedimentYield = new float[this->m_cellSize];
	if(m_usle_ls == NULL) 
	{
		float constant = pow(22.13f,0.4f);
		m_usle_ls = new float[this->m_cellSize];
		for(int i=0;i<this->m_cellSize;i++)
		{
			float lambda_i1 = this->m_flowacc[i] * this->m_cellWidth;
			float lambda_i  = lambda_i1 + this->m_cellWidth;
			float L = pow(lambda_i,1.4f) - pow(lambda_i1,1.4f);
			L /= this->m_cellWidth * constant;

			float S = pow(this->m_slope[i] / 100.0f / 0.0896f,1.3f);  

			this->m_usle_ls[i] = L * S;//equation 3 in memo, LS factor
		}
	}
	if(m_slopeForPq == NULL)
	{
		m_slopeForPq = new float[this->m_cellSize];
		for(int i=0;i<this->m_cellSize;i++)
		{
			m_slopeForPq[i] = pow(this->m_slope[i] / 100.0f * 1000.0f,0.16f);
		}
	}
	m_cellAreaKM = pow(this->m_cellWidth/1000.0f,2.0f);


	//initial average half-hour rainfall fraction for each month
	if(m_alpha_month == NULL)
	{
		m_alpha_month = new float[12];

		//first smooth the extreme half-hour rainfall values, eq.1:3.2.1 p65
		for(int i=0;i<12;i++)
		{
			if(i == 0)			m_alpha_month[i] = this->m_p_stat[11][0] + this->m_p_stat[0][0] + this->m_p_stat[1][0];
			else if(i == 11)	m_alpha_month[i] = this->m_p_stat[10][0] + this->m_p_stat[11][0] + this->m_p_stat[0][0];
			else				m_alpha_month[i] = this->m_p_stat[i-1][0] + this->m_p_stat[i][0] + this->m_p_stat[i+1][0];

			m_alpha_month[i] /= 3.0f;
		}

		//get alpha, eq.1:3.2.2 p66
		for(int i=0;i<12;i++)
		{
			float alpha = 0.0f;
			float mu = this->m_p_stat[i][1] / this->m_p_stat[i][2];
			alpha = log(0.5f / this->m_rain_yrs / this->m_p_stat[i][2]);
			alpha *= mu;
			alpha = exp(m_alpha_month[i] / alpha);
			alpha = this->m_adj_pkr * (1 - alpha);

			m_alpha_month[i] = alpha;
		}
	}

	//rndseed(6,j) = 1094585182  gcycl.f Line73
	m_rndseed = 1094585182;
}

float MUSLE_I30::getPeakRunoffRate(int cell)
{
	float p = this->m_p[cell];
	float snomlt = this->m_snowMelt[cell];
	if(p>snomlt)	p -= snomlt;	//minus snow melt
	else			p = 0.0f;

	struct tm datestruture;
	LocalTime(m_Date, &datestruture);
	//localtime_s(&datestruture,&m_Date);			//get month
	float max = 1.0f - exp(-125.0f / (p + 5));	//eq.1:3.2.3 p66
	float a15 = triangularDistribution(0.02083f,m_alpha_month[datestruture.tm_mon],max,&m_rndseed); //eq.1:3.2.4-5 p67

	float t = this->m_t_concentration[cell];
	if(t<0.1f) t = 0.1f;								//make sure t >= 0.1
	float altc = 1.0f - exp(2.0f * t * log(1.0f-a15));	//eq.2:1.3.19 p111
	float peakr = altc * this->m_surfaceRunoff[cell] * m_cellAreaKM / 3.6f / t;//eq.2:1.3.20 p111

	return peakr;
}

float MUSLE_I30::aunif(int* seed)
{
	int x2 = 0;

	x2 = *seed / 127773;
	*seed = 16807 * (*seed-x2*127773) - x2 * 2836;
	if(*seed < 0) *seed = *seed + 2147483647;

	return float((*seed) * 4.656612875e-10);
}

float MUSLE_I30::triangularDistribution(float min, float mean, float max, int* seed)
{
	float at1 = min;
	float at2 = mean;
	float at3 = max;
	int* at4i = seed;
	float u3, rn, y, b1, b2, x1, xx, yy, amn;
	float atri;

	u3 = 0.0f;
	rn = 0.0f;
	y = 0.0f;
	b1 = 0.0f;
	b2 = 0.0f;
	x1 = 0.0f;
	xx = 0.0f;
	yy = 0.0f;
	amn = 0.0f;

	u3 = at2 - at1;
	rn = aunif(at4i);
	y = 2.0f / (at3 - at1);
	b2 = at3 - at2;
	b1 = rn / y;
	x1 = y * u3 / 2.0f;

	if (rn <= x1) 
	{
		xx = 2.0f * b1 * u3;
		if (xx <= 0.0f)
			yy = 0.0f;
		else
			yy = sqrt(xx);

		atri = yy + at1;
	}
	else
	{
		xx = b2 * b2 - 2.0f * b2 * (b1 - 0.5f * u3);
		if (xx <= 0.0f)
			yy = 0.0f;
		else
			yy = sqrt(xx);

		atri = at3 - yy;
	}

	amn = (at3 + at2 + at1) / 3.0f;
	atri = atri * at2 / amn;

	if (atri >= 1.0) return 0.99f;
	if (atri <= 0.0) return 0.001f;
	return atri;
}

int MUSLE_I30::Execute()
{
	this->CheckInputData();

	this->initalOutputs();

	for(int i=0;i<this->m_cellSize;i++)
	{
		if(this->m_surfaceRunoff[i] < 0.0001f) this->m_sedimentYield[i] = 0.0f;
		else
		{
			float q = getPeakRunoffRate(i); //equation 2 in memo, peak flow
			float Y = 11.8f * pow(this->m_surfaceRunoff[i] * m_cellAreaKM * 1000.0f * q, 0.56f) 
				* this->m_usle_k[i] * this->m_usle_ls[i] * this->m_usle_c[i] * this->m_usle_p[i];    //equation 1 in memo, sediment yield

			if(this->m_snowAccumulation[i] > 0.0001f) Y /= exp(3.0f * this->m_snowAccumulation[i] / 25.4f);  //equation 4 in memo, the snow pack effect
			this->m_sedimentYield[i] = Y;
		}
	}
	return 0;
}

bool MUSLE_I30::CheckInputSize(const char* key, int n)
{
	if(n<=0)
	{
		throw ModelException("MUSLE_I30","CheckInputSize","Input data for "+string(key) +" is invalid. The size could not be less than zero.");
		return false;
	}
	if(this->m_cellSize != n)
	{
		if(this->m_cellSize <=0) this->m_cellSize = n;
		else
		{
			throw ModelException("MUSLE_I30","CheckInputSize","Input data for "+string(key) +" is invalid. All the input data should have same size.");
			return false;
		}
	}

	return true;
}

void MUSLE_I30::SetValue(const char* key, float data)
{
	string sk(key);
	if(StringMatch(sk,"cellwidth"))			this->m_cellWidth = int(data);
	else if(StringMatch(sk,"adj_pkr"))		this->m_adj_pkr = data;
	else if(StringMatch(sk,"rain_yrs"))		this->m_rain_yrs = data;
	else									throw ModelException("MUSLE_I30","SetValue","Parameter " + sk 
		+ " does not exist in MUSLE_I30 method. Please contact the module developer.");
	
}

void MUSLE_I30::Set1DData(const char* key, int n, float* data)
{

	this->CheckInputSize(key,n);

	//set the value
	string s(key);

	if(StringMatch(s,"USLE_C"))				this->m_usle_c = data;
	else if(StringMatch(s,"USLE_P"))		this->m_usle_p = data;
	else if(StringMatch(s,"USLE_K"))		this->m_usle_k = data;
	else if(StringMatch(s,"flow_acc"))		this->m_flowacc = data;
	else if(StringMatch(s,"slope"))			this->m_slope = data;
	else if(StringMatch(s,"D_SURU"))		this->m_surfaceRunoff = data;
	else if(StringMatch(s,"D_SNAC"))		this->m_snowAccumulation = data;
	else if(StringMatch(s,"T0_s"))			this->m_t_concentration = data;
	else if(StringMatch(s,"D_P"))			this->m_p = data;
	else if(StringMatch(s,"D_SNME"))		this->m_snowMelt = data;
	else									throw ModelException("MUSLE_I30","SetValue","Parameter " + s + 
		" does not exist in MUSLE_I30 method. Please contact the module developer.");

}

void MUSLE_I30::Set2DData(const char* key, int nRows, int nCols, float** data)
{
	string s(key);
	if(StringMatch(s,"p_stat"))			
	{
		if(nRows != 12) throw ModelException("MUSLE_I30","SetValue","p_stat must have 12 rows.");
		if(nCols != 3) throw ModelException("MUSLE_I30","SetValue","p_stat must have 3 columns");

		this->m_p_stat = data;
	}
	else									throw ModelException("MUSLE_I30","SetValue","Parameter " + s + " does not exist in MUSLE_I30 method. Please contact the module developer.");
}

void MUSLE_I30::Get1DData(const char* key, int* n, float** data)
{
	string sk(key);
	if(StringMatch(sk,"SOER"))				
	{
		*data = this->m_sedimentYield;
	}
	else									throw ModelException("MUSLE_I30","getResult","Result " + sk + " does not exist in MUSLE_I30 method. Please contact the module developer.");

	*n = this->m_cellSize;
}


