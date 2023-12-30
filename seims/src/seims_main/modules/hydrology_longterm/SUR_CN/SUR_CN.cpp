#include "SUR_CN.h"
#include "text.h"

SUR_CN::SUR_CN(void) : m_nCells(-1), m_Tsnow(NODATA_VALUE), m_Tsoil(NODATA_VALUE), m_T0(NODATA_VALUE),
                       m_Sfrozen(NODATA_VALUE), m_upSoilDepth(nullptr),
                       m_cn2(nullptr), m_initSoilMoisture(nullptr), m_rootDepth(nullptr),
                       m_soilDepth(nullptr), m_porosity(nullptr), m_fieldCap(nullptr), m_wiltingPoint(nullptr),
                       m_P_NET(nullptr), m_SD(nullptr), m_tMean(nullptr), m_TS(nullptr), m_SM(nullptr), m_SA(nullptr),
                       m_PE(nullptr), m_INFIL(nullptr), m_soilMoisture(nullptr), m_soilWtrStoPrfl(nullptr),
                       m_w1(nullptr), m_w2(nullptr), m_sMax(nullptr) {
    // replaced by m_upSoilDepth. LJ
    //m_depth[0] = 10.f;
    //m_depth[1] = 90.f;
    SetModuleName(M_SUR_CN[0]);
}

SUR_CN::~SUR_CN(void) {
    //// cleanup output variables
    Release1DArray(m_PE);
    Release1DArray(m_INFIL);
    Release2DArray(m_soilMoisture);
    Release1DArray(m_soilWtrStoPrfl);
    /// clean up temporary variables
    Release1DArray(m_w1);
    Release1DArray(m_w2);
    Release1DArray(m_sMax);
}

bool SUR_CN::CheckInputData(void) {
    CHECK_POSITIVE(GetModuleName(), m_date);
    CHECK_POSITIVE(GetModuleName(), m_nCells);

    CHECK_NODATA(GetModuleName(), m_Sfrozen);
    CHECK_NODATA(GetModuleName(), m_Tsnow);
    CHECK_NODATA(GetModuleName(), m_Tsoil);
    CHECK_NODATA(GetModuleName(), m_T0);

    CHECK_POINTER(GetModuleName(), m_cn2);
    CHECK_POINTER(GetModuleName(), m_initSoilMoisture);
    CHECK_POINTER(GetModuleName(), m_rootDepth);
    CHECK_POINTER(GetModuleName(), m_soilDepth);
    CHECK_POINTER(GetModuleName(), m_porosity);
    CHECK_POINTER(GetModuleName(), m_fieldCap);
    CHECK_POINTER(GetModuleName(), m_wiltingPoint);
    CHECK_POINTER(GetModuleName(), m_P_NET);
    CHECK_POINTER(GetModuleName(), m_tMean);
    CHECK_POINTER(GetModuleName(), m_TS);
    CHECK_POINTER(GetModuleName(), m_SD);
    CHECK_POINTER(GetModuleName(), m_SM);
    CHECK_POINTER(GetModuleName(), m_SA);
    return true;
}

void SUR_CN:: InitialOutputs() {
    // allocate the output variables
    if (m_PE != nullptr) {
        return;
    }
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    Initialize1DArray(m_nCells, m_PE, 0);
    Initialize1DArray(m_nCells, m_INFIL, 0);
    Initialize1DArray(m_nCells, m_soilWtrStoPrfl, 0);
    Initialize2DArray(m_nCells, m_nSoilLayers, m_soilMoisture, 0);
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        for (int j = 0; j < m_nSoilLayers; j++) {
            m_soilMoisture[i][j] = m_initSoilMoisture[i] * m_fieldCap[i][j];
            m_soilWtrStoPrfl[i] += m_soilMoisture[i][j];
        }
    }
    initalW1W2();
}

int SUR_CN::Execute() {
    CheckInputData();
    InitialOutputs();


#pragma omp parallel for
    for (int iCell = 0; iCell < m_nCells; iCell++) {
        //initialize the variables
        FLTPT cnday = 0;
        FLTPT pNet = 0;
        FLTPT surfq = 0;
        FLTPT infil = 0;
        FLTPT pcp = m_P_NET[iCell];  //rainfall - interception
        FLTPT dep = 0.0f;  //depression storage
        FLTPT snm = 0.0f;  //snowmelt
        FLTPT t = 0.0f;  // air temperature

        //set values to variables
        if (m_SD == nullptr)        // the depression storage module is not available, set the initial depression storage
        {
            //dep = m_Depre_in * m_Depression[iCell];
            dep = 0.f;
        } else {
            dep = m_SD[iCell];
        }

        if (m_SM == nullptr) {
            snm = 0.f;
        } else {
            snm = m_SM[iCell];
        }

        ///t = (m_tMin[iCell] + m_tMax[iCell]) / 2; /// replaced by m_tMean directly. LJ
        t = m_tMean[iCell];
        //account for the effects of snowmelt and soil temperature
        // snow, without snow melt
        if (t <= m_Tsnow) {
            pNet = 0.0f;
            //m_SA[iCell] += pcp;
        }
            // rain on snow, no snow melt
        else if (t > m_Tsnow && t <= m_T0 && m_SA != nullptr && m_SA[iCell] > pcp) {
            pNet = 0.0f;
            //m_SA[iCell] += pcp;
        } else {
            pNet = pcp + dep + snm;    // default
        }

        if (pNet > 0.0f) {
            FLTPT sm = 0.f;
            FLTPT por = 0.f;
            //FLTPT aboveDepth = 0.f;

            int curSoilLayers = -1, j;
            m_upSoilDepth[0] = m_soilDepth[iCell][0];
            for (j = 1; j < m_nSoilLayers; j++) {
                if (!FloatEqual(m_soilDepth[iCell][j], NODATA_VALUE)) {
                    m_upSoilDepth[j] = m_soilDepth[iCell][j] - m_soilDepth[iCell][j - 1];
                } else {
                    break;
                }
            }
            curSoilLayers = j;

            for (j = 0; j < curSoilLayers; j++) {
                sm += m_soilMoisture[iCell][j] * m_upSoilDepth[j];
                por += m_porosity[iCell][j] * m_upSoilDepth[j];
                //aboveDepth += m_depth[j];
            }
            //sm += m_soilMoisture[i][m_nSoilLayers - 1] * (m_rootDepth[i] - aboveDepth);
            //por += m_porosity[i][m_nSoilLayers - 1] * (m_rootDepth[i] - aboveDepth);

            sm /= por;
            sm = Min(sm, 1.0f);

            //for frozen soil
            if (m_TS[iCell] <= m_Tsoil && sm >= m_Sfrozen * por) {
                m_PE[iCell] = pcp + snm;
                m_INFIL[iCell] = 0.0f;
            }
                //for saturation overland flow
            else if (sm > por) {
                m_PE[iCell] = pcp + snm;
                m_INFIL[iCell] = 0.0f;
            }
                //for CN method
            else {
                cnday = Calculate_CN(sm, iCell);
                FLTPT bb, pb;
                FLTPT s = 0.0f;
                bb = 0.0f;
                pb = 0.0f;
                s = 25400.0f / cnday - 254.0f;
                bb = 0.2f * s;
                pb = pNet - bb;
                // calculate infiltration and excess rainfall
                if (pb > 0.0f) {
                    surfq = pb * pb / (pNet + 0.8f * s);
                }

                if (surfq < 0.0f) {
                    surfq = 0.0f;
                }

                infil = pNet - surfq;

                // set output value
                m_INFIL[iCell] = infil;
                m_PE[iCell] = pcp + snm - infil;    //excess precipitation could be negative
            }

            if (m_INFIL[iCell] < 0) {
                m_INFIL[iCell] = 0.f;
                m_PE[iCell] = pcp + snm - m_INFIL[iCell];
            }

            // check the output data
            if (m_INFIL[iCell] != m_INFIL[iCell] || m_INFIL[iCell] < 0.0f) {
                cout << m_INFIL[iCell] << endl;
                throw ModelException(GetModuleName(),
                                     "Execute",
                                     "Output data error: infiltration is less than zero. :\n Please contact the module developer. ");
            }

        } else {
            m_PE[iCell] = 0.0f;
            m_INFIL[iCell] = 0.0f;
        }
    }
    return 0;
}

bool SUR_CN::CheckInputSize(const char *key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

void SUR_CN::SetValue(const char *key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_T_SNOW[0])) { m_Tsnow = value; }
    else if (StringMatch(sk, VAR_T_SOIL[0])) { m_Tsoil = value; }
    else if (StringMatch(sk, VAR_T0[0])) { m_T0 = value; }
    else if (StringMatch(sk, VAR_S_FROZEN[0])) { m_Sfrozen = value; }
    else {
        throw ModelException(GetModuleName(), "SetValue", "Parameter " + sk
            +
                " does not exist in current module. Please contact the module developer.");
    }
}

void SUR_CN::Set1DData(const char *key, int n, FLTPT *data) {
    string sk(key);

    if (StringMatch(sk, VAR_CN2[0])) { m_cn2 = data; }
    else if (StringMatch(sk, VAR_MOIST_IN[0])) { m_initSoilMoisture = data; }
    else if (StringMatch(sk, VAR_ROOTDEPTH[0])) { m_rootDepth = data; }
    else if (StringMatch(sk, VAR_NEPR[0])) { m_P_NET = data; }
    else if (StringMatch(sk, VAR_DPST[0])) {
        m_SD = data; //depression storage
    } else if (StringMatch(sk, VAR_TMEAN[0])) { m_tMean = data; }
    else if (StringMatch(sk, VAR_SNME[0])) { m_SM = data; }
    else if (StringMatch(sk, VAR_SNAC[0])) { m_SA = data; }
    else if (StringMatch(sk, VAR_SOTE[0])) { m_TS = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk +
            " does not exist in current module. Please contact the module developer.");
    }
}

void SUR_CN::Set2DData(const char *key, int nrows, int ncols, FLTPT **data) {
    string sk(key);
    m_nSoilLayers = ncols;
    if (StringMatch(sk, VAR_SOILDEPTH[0])) { m_soilDepth = data; }
    else if (StringMatch(sk, VAR_POROST[0])) { m_porosity = data; }
    else if (StringMatch(sk, VAR_FIELDCAP[0])) { m_fieldCap = data; }
    else if (StringMatch(sk, VAR_WILTPOINT[0])) { m_wiltingPoint = data; }
    else {
        throw ModelException(GetModuleName(), "Set2DData", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
    }
}

void SUR_CN::Get1DData(const char *key, int *n, FLTPT **data) {
    string sk(key);

    if (StringMatch(sk, VAR_INFIL[0])) { *data = m_INFIL; }
    else if (StringMatch(sk, VAR_EXCP[0])) { *data = m_PE; }
    else if (StringMatch(sk, VAR_SOL_SW[0])) { *data = m_soilWtrStoPrfl; }
    else {
        throw ModelException(GetModuleName(), "Get1DData",
                             "Result " + sk +
                                 " does not exist in current module. Please contact the module developer.");
    }
    *n = m_nCells;
}

void SUR_CN::Get2DData(const char *key, int *nRows, int *nCols, FLTPT ***data) {
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_nSoilLayers;
    if (StringMatch(sk, VAR_SOL_ST[0])) { *data = m_soilMoisture; }
    else {
        throw ModelException(GetModuleName(), "Get2DData", "Output " + sk
            + " does not exist. Please contact the module developer.");
    }
}

FLTPT SUR_CN::Calculate_CN(FLTPT sm, int cell) {
    FLTPT sw, s, CNday, xx;

    s = 0.;
    sw = sm * m_rootDepth[cell];
    xx = m_w1[cell] - m_w2[cell] * sw;
    if (xx < -20.f) {
        xx = -20.f;
    }
    if (xx > 20.f) {
        xx = 20.f;
    }
    // traditional CN method (function of soil water)
    if ((sw + CalExp(xx)) > 0.001f) {
        s = m_sMax[cell] * (1.f - sw / (sw + CalExp(xx)));  //2:1.1.6
    }
    CNday = 25400.f / (s + 254.f);  //2:1.1.11
    return CNday;
}

void SUR_CN::initalW1W2() {
    m_w1 = new FLTPT[m_nCells];
    m_w2 = new FLTPT[m_nCells];
    m_sMax = new FLTPT[m_nCells];
    if (m_upSoilDepth == nullptr) {
        m_upSoilDepth = new FLTPT[m_nSoilLayers];
    }

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT fieldcap = 0.f;
        FLTPT wsat = 0.f;
        //FLTPT aboveDepth = 0.f;
        ///// add by LJ.
        int curSoilLayers = -1, j;
        m_upSoilDepth[0] = m_soilDepth[i][0];
        for (j = 1; j < m_nSoilLayers; j++) {
            if (!FloatEqual(m_soilDepth[i][j], NODATA_VALUE)) {
                m_upSoilDepth[j] = m_soilDepth[i][j] - m_soilDepth[i][j - 1];
            } else {
                break;
            }
        }
        curSoilLayers = j;
        /// end assign the curSoilLayers
        for (j = 0; j < curSoilLayers; j++) {
            fieldcap += m_fieldCap[i][j] * m_upSoilDepth[j];
            wsat += m_porosity[i][j] * m_upSoilDepth[j];
            //aboveDepth += m_upSoilDepth[j];
        }
        /* fieldcap += m_fieldCap[i][m_nSoilLayers - 1] * (m_rootDepth[i] - aboveDepth);
         wsat += m_porosity[i][m_nSoilLayers - 1] * (m_rootDepth[i] - aboveDepth);*/

        FLTPT cnn = m_cn2[i];
        //FLTPT fieldcap = m_fieldCap[i] * m_rootDepth[i];
        //FLTPT wsat = m_porosity[i] * m_rootDepth[i];
        FLTPT c1, c3, c2, smx, s3, rto3, rtos, xx, wrt1, wrt2;
        c2 = 100.0f - cnn;
        c1 = cnn - 20.f * c2 / (c2 + CalExp(2.533f - 0.0636f * c2));    //CN1  2:1.1.4
        c1 = Max(c1, 0.4f * cnn);
        c3 = cnn * CalExp(0.006729f * c2);                                //CN3  2:1.1.5

        //calculate maximum retention parameter value
        smx = 254.f * (100.f / c1 - 1.f);                            //2:1.1.2

        //calculate retention parameter value for CN3
        s3 = 254.f * (100.f / c3 - 1.f);

        rto3 = 1.f - s3 / smx;
        rtos = 1.f - 2.54f / smx;

        xx = CalLn(fieldcap / rto3 - fieldcap);
        wrt2 = (xx - CalLn(wsat / rtos - wsat)) /
            (wsat - fieldcap);    //soilWater :completely saturated  (= soil_por - sol_wp)  w1  2:1.1.8
        wrt1 = xx + (fieldcap * wrt2); //w2 2:1.1.7

        m_w1[i] = wrt1;
        m_w2[i] = wrt2;
        m_sMax[i] = smx;
    }
}

/// TODO: These code should be coupled to SUR_CN module. By LJ.
/// curno.f in SWAT
//FLTPT smxOld;
//if(m_CN1[i] > UTIL_ZERO)
//	smxOld = 254. * (100. / m_CN1[i] - 1.);
//FLTPT c2 = 0.f, c3 = 0.f;
//m_CN3[i] = 0.f;
//m_CN1[i] = 0.f;
///// calculate moisture condition I and III curve numbers
//c2 = 100. - cnn;
//m_CN1[i] = cnn - 20. * c2 / (c2 + CalExp(2.533 - 0.0636 * c2));
//m_CN1[i] = max(m_CN1[i], 0.4 * cnn);
//m_CN3[i] = cnn * CalExp(0.006729 * c2);

///// calculate maximum retention parameter value
//m_reCoefSoilMois[i] = 254. * (100. / m_CN1[i] - 1.);

//// calculate retention parameter value for CN3
//FLTPT s3 = 254. * (100. / m_CN3[i] - 1.);

//// calculate fraction difference in retention parameters
//FLTPT rto3 = 0.f, rtos = 0.f;
//rto3 = 1. - s3 / m_reCoefSoilMois[i];
//rtos = 1. - 2.54 / m_reCoefSoilMois[i];
///// calculate shape parameters
//FLTPT *w1, *w2;
//getScurveShapeParameter(rto3, rtos, m_soilSumFC[i], m_soilSumUl[i], w1, w2);
//if(m_yearIdx < 0) /// in SWAT, curyr is from 1 to nbyr. in SEIMS, m_yearIdx is start from 0
//	m_reCoefCN[i] = 0.9 * m_reCoefSoilMois[i];
//else /// plant ET
//	m_reCoefCN[i] = (1. - ((smxOld - m_reCoefCN[i]) / smxOld)) * m_reCoefSoilMois[i];
