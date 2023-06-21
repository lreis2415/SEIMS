#include "Percolation.h"
#include "text.h"

Percolation_DARCY::Percolation_DARCY(void) {

    // set default values for member variables
    this->m_timestep = -1;
    m_nCells = -1;

    m_Conductivity = NULL;
    m_Porosity = NULL;
    //m_Residual= NULL;
    m_Poreindex = NULL;
    m_Moisture = NULL;
    //m_ES = NULL;
    m_FieldCapacity = NULL;
    //m_SoilT = NULL;
    //m_ForzenT = -99.0f;

    m_recharge = NULL;
    m_rootDepth = NULL;
    m_cellArea = nullptr;
}

Percolation_DARCY::~Percolation_DARCY(void) {
    if (m_recharge == NULL) Release1DArray(m_recharge);
}


//Execute module
int Percolation_DARCY::Execute() {

    if (m_recharge == NULL) {
        CheckInputData();
        m_recharge = new float[m_nCells];
    }

#pragma omp parallel for
	for (int i = 0; i < m_nCells; i++) {
		for (int j = 0; j < m_nSoilLyrs; j++) {
			//if(this->m_SoilT[i] <= this->m_ForzenT)	//if the soil temperature is lower than tFrozen, then PERC = 0.
			//{
			//	m_recharge[i] = 0.0f;
			//	continue;
			//}

			float moisture = m_Moisture[i][j];

			//float temp = m_Porosity[i] - m_Residual[i];
			//if(temp <= 0.0f)
			//	temp = 0.001f;

			m_recharge[i] = 0.f;
			if (moisture > m_FieldCapacity[i][j]) {
				// the water exceeds the porosity is added to percolation directly
				if (moisture > m_Porosity[i][j]) {
					m_recharge[i] += (moisture - m_Porosity[i][j]) * m_rootDepth[i][j];
					m_Moisture[i][j] = m_Porosity[i][j];
				}

				// recharge capacity (mm)
				float dcIndex = 3.f + 2.f / m_Poreindex[i][j]; // pore disconnectedness index
				//float rechargeCap = m_Conductivity[i] / 3600.f * m_timestep * CalPow((moisture - m_Residual[i])/temp, dcIndex);
				float rechargeCap =
					m_Conductivity[i][j] / 3600.f * m_timestep * CalPow(moisture / m_Porosity[i][j], dcIndex); //Campbell, 1974
				float availableWater = (m_Moisture[i][j] - m_FieldCapacity[i][j]) * m_rootDepth[i][j];
				if (rechargeCap >= availableWater) {
					rechargeCap = availableWater;
				}

				m_recharge[i] += rechargeCap;
				m_Moisture[i][j] -= m_recharge[i] / m_rootDepth[i][j];
			}
		}
	}

    return true;

}

void Percolation_DARCY::Get1DData(const char *key, int *nRows, float **data) {
    string s(key);
    if (StringMatch(s, VAR_PERCO[0])) {
        *data = m_recharge;
    } else {
        throw ModelException(M_PERCO_DARCY[0], "Get1DData", "Result " + s + " does not exist.");
    }
    *nRows = m_nCells;
}

//void Percolation_DARCY::Set1DData(const char *key, int nRows, float *data) {
    //string s(key);

    //this->CheckInputSize(key, nRows);

//     if (StringMatch(s, VAR_CONDUCT[0])) { this->m_Conductivity = data; }
//     else if (StringMatch(s, VAR_POROST[0])) { this->m_Porosity = data; }
//     else if (StringMatch(s, VAR_POREIDX[0])) { this->m_Poreindex = data; }
//     else if (StringMatch(s, VAR_SOL_ST[0])) { this->m_Moisture = data; }
//     else if (StringMatch(s, VAR_FIELDCAP[0])) { this->m_FieldCapacity = data; }
//     else if (StringMatch(s, VAR_SOILDEPTH[0])) {
//         this->m_rootDepth = data;
//         //else if(StringMatch(s,"D_ES"))			this->m_ES = data;
//         //else if(StringMatch(s,"D_SOTE"))		this->m_SoilT = data;
//     } else {
//         throw ModelException(M_PERCO_DARCY[0], "SetValue", "Parameter " + s +
//             " does not exist in current module. Please contact the module developer.");
//     }

// }

void Percolation_DARCY::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, Tag_HillSlopeTimeStep[0])) {
        this->m_timestep = int(data);
    } else {
        throw ModelException(M_PERCO_DARCY[0], "SetValue", "Parameter " + s +
            " does not exist in current module. Please contact the module developer.");
    }
}

bool Percolation_DARCY::CheckInputData() {
    if (this->m_date <= 0) {
        throw ModelException(M_PERCO_DARCY[0], "CheckInputData", "You have not set the time.");
    }
    if (m_nCells <= 0) {
        throw ModelException(M_PERCO_DARCY[0], "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    }
    if (this->m_timestep <= 0) {
        throw ModelException(M_PERCO_DARCY[0], "CheckInputData", "The time step can not be less than zero.");
    }
    if (this->m_cellArea == NULL) {
        throw ModelException(M_PERCO_DARCY[0], "CheckInputData", "The m_cellArea can not be NULL.");
    }
    if (this->m_Conductivity == NULL) {
        throw ModelException(M_PERCO_DARCY[0], "CheckInputData", "The Conductivity can not be NULL.");
    }
    if (this->m_Porosity == NULL) {
        throw ModelException(M_PERCO_DARCY[0], "CheckInputData", "The Porosity can not be NULL.");
    }

    if (this->m_Poreindex == NULL) {
        throw ModelException(M_PERCO_DARCY[0], "CheckInputData", "The Poreindex can not be NULL.");
    }
    if (this->m_Moisture == NULL) {
        throw ModelException(M_PERCO_DARCY[0], "CheckInputData", "The Moisture can not be NULL.");
    }
    //if(this->m_SoilT == NULL)			throw ModelException(M_PERCO_DARCY[0],"CheckInputData","The soil temerature can not be NULL.");
    //if(this->m_ForzenT == -99.0f)		throw ModelException(M_PERCO_DARCY[0],"CheckInputData","The threshold soil freezing temerature can not be NULL.");

    return true;
}

bool Percolation_DARCY::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(M_PERCO_DARCY[0], "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n) {
        if (this->m_nCells <= 0) { this->m_nCells = n; }
        else {
            throw ModelException(M_PERCO_DARCY[0], "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
            return false;
        }
    }

    return true;
}
