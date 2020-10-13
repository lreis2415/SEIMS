#include "SNO_WB.h"
#include "text.h"

SNO_WB::SNO_WB(void) {
    
    // set default values for member variables
    this->m_nCells = -1;
    this->m_t0 = NODATA_VALUE;
    this->m_tsnow = NODATA_VALUE;
    this->m_kblow = NODATA_VALUE;
    this->m_swe0 = NODATA_VALUE;
    this->m_SA = NULL;
    this->m_Pnet = NULL;
    this->m_tMax = NULL;
    this->m_SR = NULL;
    this->m_SE = NULL;
    this->m_SM = NULL;
    this->m_WindSpeed = NULL;
    this->m_P = NULL;
}

SNO_WB::~SNO_WB(void) {
    if (nullptr != m_SA) Release1DArray(m_SA);
}

bool SNO_WB::CheckInputData(void) {
    if (this->m_date <= 0) throw ModelException(MID_SNO_WB, "CheckInputData", "You have not set the time.");
    if (this->m_nCells <= 0) {
        throw ModelException(MID_SNO_WB, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    }
    if (this->m_P == NULL) {
        throw ModelException(MID_SNO_WB, "CheckInputData", "The precipitation data can not be NULL.");
    }
    if (this->m_tMean == NULL) {
        throw ModelException(MID_SNO_WB, "CheckInputData", "The mean temperature data can not be NULL.");
    }
    if (this->m_tMax == NULL) {
        throw ModelException(MID_SNO_WB, "CheckInputData", "The max temperature data can not be NULL.");
    }
    if (this->m_WindSpeed == NULL) {
        throw ModelException(MID_SNO_WB, "CheckInputData", "The wind speed data can not be NULL.");
    }
    if (this->m_kblow == NODATA_VALUE) {
        throw ModelException(MID_SNO_WB, "CheckInputData",
                             "The fraction coefficient of snow blowing into or out of the watershed can not be NULL.");
    }
    if (this->m_t0 == NODATA_VALUE) {
        throw ModelException(MID_SNO_WB, "CheckInputData", "The snowmelt threshold temperature can not be NULL.");
    }
    if (this->m_tsnow == NODATA_VALUE) {
        throw ModelException(MID_SNO_WB, "CheckInputData", "The snowfall threshold temperature can not be NULL.");
    }
    if (this->m_swe0 == NODATA_VALUE) {
        throw ModelException(MID_SNO_WB, "CheckInputData", "The Initial snow water equivalent can not be NULL.");
    }
    //if(this->m_subbasinSelected == NULL)	throw ModelException(MID_SNO_WB,"CheckInputData","The subbasin selected can not be NULL.");
    //if(this->m_subbasinSelectedCount < 0)	throw ModelException(MID_SNO_WB,"CheckInputData","The number of subbasin selected can not be lower than 0.");
    //if(this->m_subbasin == NULL)	throw ModelException(MID_SNO_WB,"CheckInputData","The subbasin data can not be NULL.");
    if (this->m_Pnet == NULL) {
        throw ModelException(MID_SNO_WB, "CheckInputData", "The net precipitation data can not be NULL.");
    }
    if (this->m_SM == NULL) throw ModelException(MID_SNO_WB, "CheckInputData", "The snow melt data can not be NULL.");
    //if(this->m_SR == NULL)			throw ModelException(MID_SNO_WB,"CheckInputData","The snow redistribution data can not be NULL.");
    if (this->m_SE == NULL) {
        throw ModelException(MID_SNO_WB, "CheckInputData", "The snow sublimation data can not be NULL.");
    }
    return true;
}

void SNO_WB:: InitialOutputs() {
    if (m_nCells <= 0) {
        throw ModelException(MID_SNO_WB, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    }
    /// m_isInitial should be removed and replaced by initialOutputs.By LJ.
    if (this->m_SA == NULL) {
        this->m_SA = new float[this->m_nCells];
        for (int i = 0; i < this->m_nCells; i++) {
            m_SA[i] = 0.0f;
            //if(this->m_tMean[i] < this->m_tsnow)	this->m_SA[i] = this->m_swe0;	//winter
            //else
            //	this->m_SA[i] = 0.0f;			// other seasons
        }
    }
    if (m_SWE == NODATA_VALUE) {
        m_SWE = 0.f;
    }
}

int SNO_WB::Execute() {
    this->CheckInputData();

    this-> InitialOutputs();

    /*if(m_subbasinList == NULL && this->m_subbasinSelected != NULL && this->m_subbasinSelectedCount > 0)
    {
        getSubbasinList(this->m_nCells,this->m_subbasin,this->m_subbasinSelectedCount ,this->m_subbasinSelected);
    }*/

    if (m_SR == NULL)  /// the initialization should be removed when snow redistribution module is accomplished. LJ
    {
        m_SR = new float[m_nCells];
        for (int i = 0; i < this->m_nCells; i++) {
            m_SR[i] = 0.f;
        }
    }

    ///this->m_SWE = 0.0f;/// move to  InitialOutputs()
    for (int rw = 0; rw < this->m_nCells; rw++) {
        float dT = this->m_tMean[rw];
        float dPnet = this->m_Pnet[rw];
        float dSE = this->m_SE[rw];
        float dSR = this->m_SR[rw];
        float dSA = this->m_SA[rw];
        float dSM = this->m_SM[rw];

        float dtmp2 = dSA + dSR - dSE - dSM;
        if (dT <= this->m_tsnow)        //snowfall
        {
            dtmp2 += (1 + this->m_kblow) * dPnet;
        } else if (dT > this->m_tsnow && dT < this->m_t0 && dSA >
            dPnet) //rain on snow, if the snow accumulation is larger than net precipitation, assume all the precipitation is accumulated in snow pack.
        {
            dtmp2 += dPnet;
        }

        this->m_SA[rw] = Max(dtmp2, 0.0f);

        this->m_SWE += this->m_SA[rw];
    }

    this->m_SWE /= this->m_nCells;
    return 0;
}

bool SNO_WB::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_SNO_WB, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n) {
        if (this->m_nCells <= 0) { this->m_nCells = n; }
        else {
            throw ModelException(MID_SNO_WB, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
            return false;
        }
    }
    return true;
}

void SNO_WB::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, VAR_K_BLOW)) { this->m_kblow = data; }
    else if (StringMatch(s, VAR_T0)) { this->m_t0 = data; }
    else if (StringMatch(s, VAR_T_SNOW)) { this->m_tsnow = data; }
    else if (StringMatch(s, VAR_SWE0)) { this->m_swe0 = data; }
    else if (StringMatch(s, Tag_CellSize)) { this->m_nCells = (int) data; }
    else {
        throw ModelException(MID_SNO_WB, "SetValue", "Parameter " + s
            +
                " does not exist in current module. Please contact the module developer.");
    }
}

void SNO_WB::Set1DData(const char *key, int n, float *data) {
    string s(key);
    /*if(StringMatch(s,Tag_SubbasinSelected))
    {
        this->m_subbasinSelected = data;
        this->m_subbasinSelectedCount = n;
        return;
    }*/

    //if(StringMatch(s, VAR_WS))
    //{
    //	this->m_WindSpeed = data;
    //	this->m_nCells = n;
    //	return;
    //}
    this->CheckInputSize(key, n);
    if (StringMatch(s, VAR_NEPR)) { this->m_Pnet = data; }
    else if (StringMatch(s, VAR_SNRD)) { this->m_SR = data; }
    else if (StringMatch(s, VAR_SNSB)) { this->m_SE = data; }
    else if (StringMatch(s, VAR_SNME)) { this->m_SM = data; }
    else if (StringMatch(s, VAR_WS)) { this->m_WindSpeed = data; }
    else if (StringMatch(s, VAR_TMAX)) { this->m_tMax = data; }
    else if (StringMatch(s, VAR_TMEAN)) { this->m_tMean = data; }
    else if (StringMatch(s, VAR_PCP)) {
        this->m_P = data;
        //else if(StringMatch(s, VAR_SUBBSN))		this->m_subbasin = data;
    } else {
        throw ModelException(MID_SNO_WB, "Set1DData", "Parameter " + s
            +
                " does not exist in current module. Please contact the module developer.");
    }

}

void SNO_WB::Get1DData(const char *key, int *n, float **data) {
    InitialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SNAC)) {
        *data = this->m_SA;
        *n = this->m_nCells;
    } else {
        throw ModelException(MID_SNO_WB, "Get1DData", "Result " + s +
                             " does not exist in current module. Please contact the module developer.");
    }
}

void SNO_WB::GetValue(const char *key, float *data) {
    InitialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SWE)) { *data = this->m_SWE; }
    else {
        throw ModelException(MID_SNO_WB, "GetValue", "Result " + s +
            " does not exist in current module. Please contact the module developer.");
    }
}
