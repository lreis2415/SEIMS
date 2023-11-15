#include "SoilErosion_MUSLE.h"

#include "text.h"

SERO_MUSLE::SERO_MUSLE() :
    m_nCells(-1), m_cellWth(-1.), m_maxSoilLyrs(-1), m_soilRock(nullptr),
    m_usleK(nullptr), m_usleP(nullptr),
    m_flowAccm(nullptr), m_slope(nullptr), m_slpLen(nullptr),
    m_rchID(nullptr), m_detSand(nullptr), m_detSilt(nullptr), m_detClay(nullptr),
    m_detSmAgg(nullptr), m_detLgAgg(nullptr), m_iCfac(1),
    m_aveAnnUsleC(nullptr), m_landCover(nullptr), m_rsdCovSoil(nullptr),
    m_rsdCovCoef(NODATA_VALUE), m_canHgt(nullptr), m_lai(nullptr), m_surfRf(nullptr),
    m_snowAccum(nullptr), m_usleMult(nullptr), m_cellAreaKM(NODATA_VALUE),
    m_cellAreaKM1(NODATA_VALUE), m_cellAreaKM2(NODATA_VALUE), m_slopeForPq(nullptr),
    m_usleL(nullptr), m_usleS(nullptr), m_usleC(nullptr),
    m_eroSed(nullptr), m_eroSand(nullptr), m_eroSilt(nullptr), m_eroClay(nullptr),
    m_eroSmAgg(nullptr), m_eroLgAgg(nullptr) {
}

SERO_MUSLE::~SERO_MUSLE() {
    if (m_usleMult != nullptr) Release1DArray(m_usleMult);
    if (m_slopeForPq != nullptr) Release1DArray(m_slopeForPq);
    if (m_usleL != nullptr) Release1DArray(m_usleL);
    if (m_usleS != nullptr) Release1DArray(m_usleS);
    if (m_usleC != nullptr) Release1DArray(m_usleC);
    if (m_eroSed != nullptr) Release1DArray(m_eroSed);
    if (m_eroSand != nullptr) Release1DArray(m_eroSand);
    if (m_eroSilt != nullptr) Release1DArray(m_eroSilt);
    if (m_eroClay != nullptr) Release1DArray(m_eroClay);
    if (m_eroSmAgg != nullptr) Release1DArray(m_eroSmAgg);
    if (m_eroLgAgg != nullptr) Release1DArray(m_eroLgAgg);
}

bool SERO_MUSLE::CheckInputData() {
    CHECK_POSITIVE(M_SERO_MUSLE[0], m_nCells);
    CHECK_POSITIVE(M_SERO_MUSLE[0], m_cellWth);
    CHECK_POINTER(M_SERO_MUSLE[0], m_soilRock);
    CHECK_POINTER(M_SERO_MUSLE[0], m_usleK);
    CHECK_POINTER(M_SERO_MUSLE[0], m_usleP);
    CHECK_POINTER(M_SERO_MUSLE[0], m_flowAccm);
    CHECK_POINTER(M_SERO_MUSLE[0], m_slope);
    CHECK_POINTER(M_SERO_MUSLE[0], m_rchID);
    CHECK_POINTER(M_SERO_MUSLE[0], m_detSand);
    CHECK_POINTER(M_SERO_MUSLE[0], m_detSilt);
    CHECK_POINTER(M_SERO_MUSLE[0], m_detClay);
    CHECK_POINTER(M_SERO_MUSLE[0], m_detSmAgg);
    CHECK_POINTER(M_SERO_MUSLE[0], m_detLgAgg);

    CHECK_NONNEGATIVE(M_SERO_MUSLE[0], m_iCfac);
    CHECK_POINTER(M_SERO_MUSLE[0], m_aveAnnUsleC);
    CHECK_POINTER(M_SERO_MUSLE[0], m_landCover);
    if (m_iCfac == 1) {
        CHECK_NODATA(M_SERO_MUSLE[0], m_rsdCovCoef);
        CHECK_POINTER(M_SERO_MUSLE[0], m_canHgt);
        CHECK_POINTER(M_SERO_MUSLE[0], m_lai);
    }

    CHECK_POINTER(M_SERO_MUSLE[0], m_surfRf);
    CHECK_POINTER(M_SERO_MUSLE[0], m_snowAccum);
    return true;
}

void SERO_MUSLE::InitialOutputs() {
    CHECK_POSITIVE(M_SERO_MUSLE[0], m_nCells);
    if (nullptr == m_eroSed) Initialize1DArray(m_nCells, m_eroSed, 0.);
    if (nullptr == m_eroSand) Initialize1DArray(m_nCells, m_eroSand, 0.);
    if (nullptr == m_eroSilt) Initialize1DArray(m_nCells, m_eroSilt, 0.);
    if (nullptr == m_eroClay) Initialize1DArray(m_nCells, m_eroClay, 0.);
    if (nullptr == m_eroSmAgg) Initialize1DArray(m_nCells, m_eroSmAgg, 0.);
    if (nullptr == m_eroLgAgg) Initialize1DArray(m_nCells, m_eroLgAgg, 0.);
    if (nullptr == m_usleC) {
        Initialize1DArray(m_nCells, m_usleC, 0.);

#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            if (m_rchID[i] > 0) {
                m_usleC[i] = 0.;
                continue;
            }

            if (m_aveAnnUsleC[i] < 1.e-4 || FloatEqual(m_aveAnnUsleC[i], NODATA_VALUE)) {
                m_aveAnnUsleC[i] = 0.001; // line 289 of readplant.f of SWAT source
            }
            m_usleC[i] = m_aveAnnUsleC[i]; // By default, the m_usleC equals to the annual USLE_C value.
            if (nullptr != m_rsdCovSoil && nullptr != m_landCover) {
                // Which means dynamic USLE_C will be updated, so, m_aveAnnUsleC store the natural log of
                //  the minimum value of the USLE_C for the land cover
                m_aveAnnUsleC[i] = CalLn(m_aveAnnUsleC[i]); // line 290 of readplant.f of SWAT source
            }
        }
    }
}

void SERO_MUSLE::InitialIntermediates() {
    if (!m_reCalIntermediates) return;

    // m_cellAreaKM put here is for future improvement of cells -> arbitrary polygons
    if (FloatEqual(m_cellAreaKM, NODATA_VALUE)) {
        m_cellAreaKM = m_cellWth * m_cellWth * 0.000001;
        m_cellAreaKM1 = 3.79 * CalPow(m_cellAreaKM, 0.7);
        m_cellAreaKM2 = 0.903 * CalPow(m_cellAreaKM, 0.017);
    }

    if (nullptr == m_usleMult) {
        Initialize1DArray(m_nCells, m_usleL, 0.);
        Initialize1DArray(m_nCells, m_usleS, 0.);
        Initialize1DArray(m_nCells, m_slopeForPq, 0.);
        Initialize1DArray(m_nCells, m_usleMult, 0.);
    }
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        // Calculate USLE slope length factor, line 286-292 of readhru.f of SWAT source.
        // Section 4:1.1.4 and 4:1.1.5 of SWAT Theory 2009.
        // In SWAT, the calculation of LS factor needs `slsubbsn` (average slope length for subbasin)
        //   I found the calcuation of `slsubbsn` in QSWAT (function `getSlsubbsn` in `hrus.py`)
        //def getSlsubbsn(meanSlope) :
        //    """Estimate the average slope length in metres from the mean slope."""
        //    if meanSlope < 0.01 : return 120
        //        elif meanSlope < 0.02 : return 100
        //        elif meanSlope < 0.03 : return 90
        //        elif meanSlope < 0.05 : return 60
        //    else: return 30
        //
        // Currently, I decided to take slope length as input parameter.
        // The calculation of LS factor follows the equation of McCool et al.(1989) used in RUSLE.
        // Also refers to Zhang et al.(2013), C&G, 52, 177-188.
        //                Liu et al.(2015), C&G, 78, 110-122.
        FLTPT slp_rad = atan(m_slope[i]);
        FLTPT slp_deg = slp_rad * rad2deg; // rad2deg = 180. / PI;
        FLTPT sin_slp = sin(slp_rad);
        FLTPT beta = sin_slp / 0.0896 / (3. * CalPow(sin_slp, 0.8) + 0.56);
        FLTPT m = beta / (1. + beta);
        FLTPT S = 0.;
        if (slp_deg < 5.) S = 10.8 * sin_slp + 0.03;
        else if (slp_deg >= 5. && slp_deg <= 14.) S = 16.8 * sin_slp - 0.5;
        else S = 21.91 * sin_slp - 0.96;
        // float xm = 0.6f * (1.f - CalExp(-35.835f * m_slope[i])); // eq. of SWAT
        // float s = 65.41f * sin_slp * sin_slp + 4.56f * sin_slp + 0.065f; // eq. of SWAT
        // If m_slpLen is not provided, use the equation developed by Renard et al.(1997) and the extended
        //    variation by Desmet and Govers (1996).
        FLTPT L = 0.;
        if (nullptr == m_slpLen) {
            FLTPT up_lambda = m_flowAccm[i] * m_cellWth;
            FLTPT slope_lambda = up_lambda + m_cellWth;
            L = CalPow(slope_lambda, m + 1.) - CalPow(up_lambda, m + 1.);
            L /= m_cellWth * CalPow(22.13, m);
        } else {
            L = CalPow(m_slpLen[i] / 22.13, m);
        }

        if (m_usleP[i] < 0.) m_usleP[i] = 0.;
        if (m_usleP[i] > 1.) m_usleP[i] = 1.;
        m_slopeForPq[i] = CalPow(m_slope[i] * 1000., 0.16);
        m_usleL[i] = L;
        m_usleS[i] = S;
        // line 111-113 of soil_phys.f of SWAT source.
        m_usleMult[i] = 11.8 * CalExp(-0.053 * m_soilRock[i][0]) * m_usleK[i][0] * m_usleP[i] * L * S;

        // for debug only
        //if (i == 18181 || i == 18182 || i == 18183 ||
        //    i == 24796 || i == 24797 || i == 24798 ||
        //    i == 25139 || i == 25140 || i == 25141) // NODATA cell id of USLE_C
        //{
        //    cout << " i: " << i << "c: " << m_usleC[i] << " p: " << m_usleP[i] << " k: " << m_usleK[i][0]
        //        << " L: " << L << " S: " << S << " mult: " << m_usleMult[i] << endl;
        //}
    }

    m_reCalIntermediates = false;
}

int SERO_MUSLE::Execute() {
    CheckInputData();
    InitialIntermediates();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_surfRf[i] < 0.0001 || m_rchID[i] > 0) {
            m_eroSed[i] = 0.;
            m_eroSand[i] = 0.;
            m_eroSilt[i] = 0.;
            m_eroClay[i] = 0.;
            m_eroSmAgg[i] = 0.;
            m_eroLgAgg[i] = 0.;
            continue;
        }
        // Update C factor
        if (m_iCfac == 0 && nullptr != m_rsdCovSoil) {
            // Original method as described in section 4:1.1.2 in SWAT Theory 2009
            if (m_landCover[i] > 0. && !FloatEqual(m_landCover[i], LANDUSE_ID_WATR)) {
                // exclude WATER
                // ln(0.8) = -0.2231435513142097
                m_usleC[i] = CalExp((-0.223144 - m_aveAnnUsleC[i]) *
                                 CalExp(-0.00115 * m_rsdCovSoil[i]) + m_aveAnnUsleC[i]);
            } else {
                if (m_rsdCovSoil[i] > 1.e-4) {
                    m_usleC[i] = CalExp(-0.223144 * CalExp(-0.00115 * m_rsdCovSoil[i]));
                } else {
                    m_usleC[i] = 0.001; // In SWAT, this is 0.8. But I think it should be 0.
                }
            }
        } else {
            if (m_landCover[i] > 0. && !FloatEqual(m_landCover[i], LANDUSE_ID_WATR)) {
                // exclude WATER
                // new calculation method from RUSLE with the minimum C factor value
                //! fraction of cover by residue
                FLTPT rsd_frcov = CalExp(-m_rsdCovCoef * m_rsdCovSoil[i]);
                //! fraction of cover by biomass as function of lai
                FLTPT grcov_fr = m_lai[i] / (m_lai[i] + CalExp(1.748 - 1.748 * m_lai[i]));
                //! fraction of cover by biomass - adjusted for canopy height
                FLTPT bio_frcov = 1. - grcov_fr * CalExp(-0.01 * m_canHgt[i]);
                m_usleC[i] = Max(1.e-10, rsd_frcov * bio_frcov);
            } else {
                m_usleC[i] = 0.001;
            }
        }
        if (m_usleC[i] > 1.) m_usleC[i] = 1.;
        if (m_usleC[i] < 0.) m_usleC[i] = 0.;
        // TODO, use pkq.f of SWAT to calculate peak runoff rate? LJ.
        // peak flow, 1. / 25.4 = 0.03937007874015748
        FLTPT q = m_cellAreaKM1 * m_slopeForPq[i] * CalPow(m_surfRf[i] * 0.03937007874015748, m_cellAreaKM2);
        // sediment yield, unit: tons, eq. 4:1.1.1 in SWAT theory 2009.
        FLTPT sed_yld = m_usleMult[i] * m_usleC[i] * CalPow(m_surfRf[i] * m_cellAreaKM * 1000.0 * q, 0.56);
        // the snow pack effect
        if (m_snowAccum[i] > 0.0001) {
            sed_yld /= CalExp(3. * m_snowAccum[i] * 0.03937007874015748);
        }
        m_eroSed[i] = sed_yld * 1000.; /// kg

        // for debug only
        //if (i == 18181 || i == 18182 || i == 18183 ||
        //    i == 24796 || i == 24797 || i == 24798 ||
        //    i == 25139 || i == 25140 || i == 25141)
        //{
        //    cout << " i: " << i << "c: " << m_usleC[i] << " surfRf: " << m_surfRf[i] << " mult: " << m_usleMult[i]
        //        << " slopeForPq: " << m_slopeForPq[i] << " sed_yld: " << sed_yld << endl;
        //}

        /// particle size distribution of sediment yield
        m_eroSand[i] = m_eroSed[i] * m_detSand[i];
        m_eroSilt[i] = m_eroSed[i] * m_detSilt[i];
        m_eroClay[i] = m_eroSed[i] * m_detClay[i];
        m_eroSmAgg[i] = m_eroSed[i] * m_detSmAgg[i];
        m_eroLgAgg[i] = m_eroSed[i] * m_detLgAgg[i];
    }

    //debug
    //cout << ConvertToString2(m_date) << ": " << m_usleK[50608][0] << ", " << m_eroSed[50608] << endl;
    return 0;
}

void SERO_MUSLE::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, Tag_CellWidth[0])) {
        m_cellWth = value;
    } else if (StringMatch(sk, VAR_RSDCOV_COEF[0])) {
        m_rsdCovCoef = value;
    } else {
        throw ModelException(M_SERO_MUSLE[0], "SetValue",
                             "Parameter " + sk + " does not exist in current module.");
    }
}

void SERO_MUSLE::SetValue(const char* key, const int value) {
    string sk(key);
    if (StringMatch(sk, VAR_ICFAC[0])) {
        m_iCfac = value;
    } else {
        throw ModelException(M_SERO_MUSLE[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist in current module.");
    }
}

void SERO_MUSLE::Set1DData(const char* key, const int n, FLTPT* data) {
    CheckInputSize(key, n, m_nCells);
    string s(key);
    if (StringMatch(s, VAR_USLE_C[0])) m_aveAnnUsleC = data;
    else if (StringMatch(s, VAR_SOL_COV[0])) m_rsdCovSoil = data;
    else if (StringMatch(s, VAR_CHT[0])) m_canHgt = data;
    else if (StringMatch(s, VAR_LAIDAY[0])) m_lai = data;
    else if (StringMatch(s, VAR_USLE_P[0])) m_usleP = data;
    else if (StringMatch(s, VAR_ACC[0])) m_flowAccm = data;
    else if (StringMatch(s, VAR_SLOPE[0])) m_slope = data;
    else if (StringMatch(s, VAR_SLPLEN[0])) m_slpLen = data;
    else if (StringMatch(s, VAR_SURU[0])) m_surfRf = data;
    else if (StringMatch(s, VAR_SNAC[0])) m_snowAccum = data;
    else if (StringMatch(s, VAR_DETACH_SAND[0])) m_detSand = data;
    else if (StringMatch(s, VAR_DETACH_SILT[0])) m_detSilt = data;
    else if (StringMatch(s, VAR_DETACH_CLAY[0])) m_detClay = data;
    else if (StringMatch(s, VAR_DETACH_SAG[0])) m_detSmAgg = data;
    else if (StringMatch(s, VAR_DETACH_LAG[0])) m_detLgAgg = data;
    else {
        throw ModelException(M_SERO_MUSLE[0], "Set1DData",
                             "Parameter " + s + " does not exist.");
    }
}

void SERO_MUSLE::Set1DData(const char* key, const int n, int* data) {
    CheckInputSize(key, n, m_nCells);
    string s(key);
    if (StringMatch(s, VAR_LANDCOVER[0])) m_landCover = data;
    else if (StringMatch(s, VAR_STREAM_LINK[0])) m_rchID = data;
    else {
        throw ModelException(M_SERO_MUSLE[0], "Set1DData",
                             "Integer Parameter " + s + " does not exist.");
    }
}

void SERO_MUSLE::Set2DData(const char* key, const int nrows, const int ncols, FLTPT** data) {
    CheckInputSize2D(key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    string s(key);
    m_maxSoilLyrs = ncols;
    if (StringMatch(s, VAR_USLE_K[0])) m_usleK = data;
    else if (StringMatch(s, VAR_ROCK[0])) m_soilRock = data;
    else {
        throw ModelException(M_SERO_MUSLE[0], "Set2DData",
                             "Parameter " + s + " does not exist.");
    }
}

void SERO_MUSLE::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_USLE_L[0])) *data = m_usleL;
    else if (StringMatch(sk, VAR_USLE_S[0])) *data = m_usleS;
    else if (StringMatch(sk, VAR_USLE_C[0])) *data = m_usleC;
    else if (StringMatch(sk, VAR_SOER[0])) *data = m_eroSed;
    else if (StringMatch(sk, VAR_SANDYLD[0])) *data = m_eroSand;
    else if (StringMatch(sk, VAR_SILTYLD[0])) *data = m_eroSilt;
    else if (StringMatch(sk, VAR_CLAYYLD[0])) *data = m_eroClay;
    else if (StringMatch(sk, VAR_SAGYLD[0])) *data = m_eroSmAgg;
    else if (StringMatch(sk, VAR_LAGYLD[0])) *data = m_eroLgAgg;
    else {
        throw ModelException(M_SERO_MUSLE[0], "Get1DData",
                             "Result " + sk + " does not exist.");
    }
    *n = m_nCells;
}

void SERO_MUSLE::Get2DData(const char* key, int* nrows, int* ncols, FLTPT*** data) {
    InitialOutputs();
    string sk(key);
    *nrows = m_nCells;
    *ncols = m_maxSoilLyrs;
    if (StringMatch(sk, VAR_USLE_K[0])) {
        *data = m_usleK;
    }
    else {
        throw ModelException(M_SERO_MUSLE[0], "Get2DData",
                             "Parameter " + sk + " does not exist.");
    }
}
