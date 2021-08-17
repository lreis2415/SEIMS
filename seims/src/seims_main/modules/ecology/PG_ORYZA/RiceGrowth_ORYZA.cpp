#include "RiceGrowth_ORYZA.h"

#include <cmath>

#include "text.h"

ORYZA::ORYZA() :
    m_nCells(-1), m_co2(NODATA_VALUE), m_meanTemp(nullptr), m_tMax(nullptr), m_tMin(nullptr), m_SR(nullptr),
    //rice related parameters,they are all read from DB, single
    m_nSoilLyrs(nullptr), m_maxSoilLyrs(-1), m_soilZMX(nullptr), m_soilALB(nullptr),
    m_soilDepth(nullptr), m_soilThick(nullptr), m_soilAWC(nullptr), m_sol_sat(nullptr), m_soilWP(nullptr),
    m_totSoilAWC(nullptr), m_totSoilSat(nullptr), m_soilStorage(nullptr), m_soilWtrStoPrfl(nullptr),
    m_stoSoilRootD(nullptr),
    m_soilRsd(nullptr), m_rsdCovSoil(nullptr), m_sol_rsdin(nullptr), m_alb(nullptr),
    m_snowAcc(nullptr), m_frStrsWtr(nullptr), m_frStrsN(nullptr), m_tbd(NODATA_VALUE),
    m_tod(NODATA_VALUE), m_tmd(NODATA_VALUE), m_dvrj(NODATA_VALUE), m_dvri(NODATA_VALUE),
    m_dvrp(NODATA_VALUE), m_dvrr(NODATA_VALUE), m_mopp(NODATA_VALUE), m_ppse(NODATA_VALUE),
    m_shckd(NODATA_VALUE), m_knf(NODATA_VALUE), m_rgrlMX(NODATA_VALUE), m_rgrlMN(NODATA_VALUE),
    m_nh(NODATA_VALUE), m_nplh(NODATA_VALUE), m_nplsb(NODATA_VALUE), m_lape(NODATA_VALUE),
    m_zrttr(NODATA_VALUE), m_tmpsb(NODATA_VALUE), m_aFsh(NODATA_VALUE), m_bFsh(NODATA_VALUE),
    m_aFlv(NODATA_VALUE), m_bFlv(NODATA_VALUE), m_aFso(NODATA_VALUE), m_bFso(NODATA_VALUE),
    m_aDrlv(NODATA_VALUE), m_bDrlv(NODATA_VALUE), m_tclstr(NODATA_VALUE), m_q10(NODATA_VALUE),
    m_tref(NODATA_VALUE), m_mainLV(NODATA_VALUE), m_mainST(NODATA_VALUE), m_mainSO(NODATA_VALUE),
    m_mainRT(NODATA_VALUE), m_crgLV(NODATA_VALUE), m_crgST(NODATA_VALUE), m_crgSTR(NODATA_VALUE),
    m_crgSO(NODATA_VALUE), m_crgRT(NODATA_VALUE), m_fstr(NODATA_VALUE), m_lrstr(NODATA_VALUE),
    m_aSLA(NODATA_VALUE), m_bSLA(NODATA_VALUE), m_cSLA(NODATA_VALUE), m_dSLA(NODATA_VALUE),
    m_slaMX(NODATA_VALUE), m_fcRT(NODATA_VALUE), m_fcLV(NODATA_VALUE), m_fcST(NODATA_VALUE),
    m_fcSTR(NODATA_VALUE), m_fcSO(NODATA_VALUE), m_wgrMX(NODATA_VALUE), m_gzrt(NODATA_VALUE),
    m_zrtMCD(NODATA_VALUE),
    //soil related parameters
    m_frpar(NODATA_VALUE), m_spgf(NODATA_VALUE), m_nMaxL(NODATA_VALUE), m_nMinL(NODATA_VALUE), m_rfnlv(NODATA_VALUE),
    m_rfnst(NODATA_VALUE), m_fntrt(NODATA_VALUE), m_tcntrf(NODATA_VALUE), m_nMaxSO(NODATA_VALUE),
    m_anMinSO(NODATA_VALUE),
    m_bnMinSO(NODATA_VALUE), m_shckl(NODATA_VALUE), m_sbdur(-1), m_llls(NODATA_VALUE), m_ulls(NODATA_VALUE),
    m_llle(NODATA_VALUE), m_ulle(NODATA_VALUE), m_lldl(NODATA_VALUE), m_uldl(NODATA_VALUE),
    m_ppt(nullptr),
    m_actPltET(nullptr), m_epco(nullptr), m_cropsta(nullptr),
    m_ts(nullptr), m_celllat(nullptr),
    // rice
    m_cellLat(NODATA_VALUE), m_dayL(nullptr), m_sinLD(nullptr), m_cosLD(nullptr),
    // parameters related to the current day and latitude
    m_dsinbe(nullptr), m_sinb(nullptr), m_solcon(nullptr), m_rdpdf(nullptr), m_rdpdr(nullptr),
    m_gaid(nullptr),
    m_gai(nullptr), m_rapshl(nullptr), m_rapppl(nullptr),
    // parameters related to the growth of rice
    m_fslla(nullptr), m_nflv(NODATA_VALUE), m_redf(NODATA_VALUE), m_eff(NODATA_VALUE), m_gpl(nullptr),
    m_rapl(nullptr), m_gpc(nullptr),
    m_rapc(nullptr), m_hour(NODATA_VALUE), m_gpcdt(nullptr), m_rapcdt(nullptr), m_dtga(nullptr),
    gnsp(NODATA_VALUE), m_gcr(nullptr),
    m_coldTT(nullptr), m_tfert(nullptr), m_ntfert(NODATA_VALUE), m_nsp(nullptr), m_gngr(nullptr),
    m_gLai(NODATA_VALUE),
    m_rwlvg(nullptr), m_sla(nullptr), m_zrt(nullptr), m_sai(NODATA_VALUE), m_aLAI(NODATA_VALUE),
    m_fsh(NODATA_VALUE),
    m_frt(NODATA_VALUE), m_flv(NODATA_VALUE), m_fst(NODATA_VALUE), m_fso(NODATA_VALUE),
    m_drlv(NODATA_VALUE), m_nsllv(NODATA_VALUE),
    m_lstr(NODATA_VALUE), m_teff(NODATA_VALUE), m_wlvd(nullptr), m_wsts(nullptr),
    m_wstr(nullptr), m_ngr(nullptr),
    m_tnass(nullptr), m_wlv(nullptr), m_wagt(nullptr), m_biomass(nullptr), m_frRoot(nullptr), glv(NODATA_VALUE),
    gst(NODATA_VALUE), gso(NODATA_VALUE), m_wst(nullptr),
    m_wso(nullptr), m_wlvg(nullptr), m_wrt(nullptr), m_soilNO3(nullptr), m_pltN(nullptr), m_anst(nullptr),
    // parameter related to the N of plant and soil
    m_plantUpTkN(nullptr), m_ancrf(nullptr), m_anlv(nullptr),
    // rice related parameters, output
    sowDay(-1), m_dvs(nullptr), m_lai(nullptr), m_wrr(nullptr) {
}


ORYZA::~ORYZA() {
    if (m_rsdCovSoil != nullptr) Release1DArray(m_rsdCovSoil);
    if (m_soilRsd != nullptr) Release2DArray(m_nCells, m_soilRsd);
    if (m_stoSoilRootD != nullptr) Release1DArray(m_stoSoilRootD);
    if (m_actPltET != nullptr) Release1DArray(m_actPltET);
    if (m_plantUpTkN != nullptr) Release1DArray(m_plantUpTkN);
    if (m_pltN != nullptr) Release1DArray(m_pltN);
    if (m_frStrsN != nullptr) Release1DArray(m_frStrsN);
    if (m_frStrsWtr != nullptr) Release1DArray(m_frStrsWtr);
    if (m_biomass != nullptr) Release1DArray(m_biomass);
    if (m_wagt != nullptr) Release1DArray(m_wagt);
    if (m_dvs != nullptr) Release1DArray(m_dvs);
    if (m_wrr != nullptr) Release1DArray(m_wrr);
}

void ORYZA::SetValue(const char* key, float value) {
    string sk(key);
    if (StringMatch(sk, VAR_CO2[0])) m_co2 = value;
    else if (StringMatch(sk, VAR_TBD[0])) m_tbd = value;
    else if (StringMatch(sk, VAR_TOD[0])) m_tod = value;
    else if (StringMatch(sk, VAR_TMD[0])) m_tmd = value;
    else if (StringMatch(sk, VAR_DVRJ[0])) m_dvrj = value;
    else if (StringMatch(sk, VAR_DVRI[0])) m_dvri = value;
    else if (StringMatch(sk, VAR_DVRP[0])) m_dvrp = value;
    else if (StringMatch(sk, VAR_DVRR[0])) m_dvrr = value;
    else if (StringMatch(sk, VAR_MOPP[0])) m_mopp = value;
    else if (StringMatch(sk, VAR_PPSE[0])) m_ppse = value;
    else if (StringMatch(sk, VAR_SHCKD[0])) m_shckd = value;
    else if (StringMatch(sk, VAR_KNF[0])) m_knf = value;
    else if (StringMatch(sk, VAR_RGRLMX[0])) m_rgrlMX = value;
    else if (StringMatch(sk, VAR_RGRLMN[0])) m_rgrlMN = value;
    else if (StringMatch(sk, VAR_NH[0])) m_nh = value;
    else if (StringMatch(sk, VAR_NPLH[0])) m_nplh = value;
    else if (StringMatch(sk, VAR_NPLSB[0])) m_nplsb = value;
    else if (StringMatch(sk, VAR_LAPE[0])) m_lape = value;
    else if (StringMatch(sk, VAR_ZRTTR[0])) m_zrttr = value;
    else if (StringMatch(sk, VAR_TMPSB[0])) m_tmpsb = value;
    else if (StringMatch(sk, VAR_AFSH[0])) m_aFsh = value;
    else if (StringMatch(sk, VAR_BFSH[0])) m_bFsh = value;
    else if (StringMatch(sk, VAR_AFLV[0])) m_aFlv = value;
    else if (StringMatch(sk, VAR_BFLV[0])) m_bFlv = value;
    else if (StringMatch(sk, VAR_AFSO[0])) m_aFso = value;
    else if (StringMatch(sk, VAR_BFSO[0])) m_bFso = value;
    else if (StringMatch(sk, VAR_ADRLV[0])) m_aDrlv = value;
    else if (StringMatch(sk, VAR_BDRLV[0])) m_bDrlv = value;
    else if (StringMatch(sk, VAR_TCLSTR[0])) m_tclstr = value;
    else if (StringMatch(sk, VAR_Q10[0])) m_q10 = value;
    else if (StringMatch(sk, VAR_TREF[0])) m_tref = value;
    else if (StringMatch(sk, VAR_MAINLV[0])) m_mainLV = value;
    else if (StringMatch(sk, VAR_MAINST[0])) m_mainST = value;
    else if (StringMatch(sk, VAR_MAINSO[0])) m_mainSO = value;
    else if (StringMatch(sk, VAR_MAINRT[0]))m_mainRT = value;
    else if (StringMatch(sk, VAR_CRGLV[0])) m_crgLV = value;
    else if (StringMatch(sk, VAR_CRGST[0])) m_crgST = value;
    else if (StringMatch(sk, VAR_CRGSTR[0])) m_crgSTR = value;
    else if (StringMatch(sk, VAR_CRGSO[0])) m_crgSO = value;
    else if (StringMatch(sk, VAR_CRGRT[0])) m_crgRT = value;
    else if (StringMatch(sk, VAR_FSTR[0])) m_fstr = value;
    else if (StringMatch(sk, VAR_LRSTR[0])) m_lrstr = value;
    else if (StringMatch(sk, VAR_ASLA[0])) m_aSLA = value;
    else if (StringMatch(sk, VAR_BSLA[0])) m_bSLA = value;
    else if (StringMatch(sk, VAR_CSLA[0])) m_cSLA = value;
    else if (StringMatch(sk, VAR_DSLA[0])) m_dSLA = value;
    else if (StringMatch(sk, VAR_SLAMX[0])) m_slaMX = value;
    else if (StringMatch(sk, VAR_FCRT[0])) m_fcRT = value;
    else if (StringMatch(sk, VAR_FCST[0])) m_fcLV = value;
    else if (StringMatch(sk, VAR_FCLV[0])) m_fcST = value;
    else if (StringMatch(sk, VAR_FCSTR[0])) m_fcSTR = value;
    else if (StringMatch(sk, VAR_FCSO[0])) m_fcSO = value;
    else if (StringMatch(sk, VAR_WGRMX[0])) m_wgrMX = value;
    else if (StringMatch(sk, VAR_GZRT[0])) m_gzrt = value;
    else if (StringMatch(sk, VAR_ZRTMCD[0])) m_zrtMCD = value;
    else if (StringMatch(sk, VAR_FRPAR[0])) m_frpar = value;
    else if (StringMatch(sk, VAR_SPGF[0])) m_spgf = value;
    else if (StringMatch(sk, VAR_NMAXL[0])) m_nMaxL = value;
    else if (StringMatch(sk, VAR_NMINL[0])) m_nMinL = value;
    else if (StringMatch(sk, VAR_RFNLV[0])) m_rfnlv = value;
    else if (StringMatch(sk, VAR_RFNST[0])) m_rfnst = value;
    else if (StringMatch(sk, VAR_RFNRT[0])) m_fntrt = value;
    else if (StringMatch(sk, VAR_TCNTRF[0])) m_tcntrf = value;
    else if (StringMatch(sk, VAR_NMAXSO[0])) m_nMaxSO = value;
    else if (StringMatch(sk, VAR_ANMINSO[0])) m_anMinSO = value;
    else if (StringMatch(sk, VAR_BNMINSO[0])) m_bnMinSO = value;
    else if (StringMatch(sk, VAR_SHCKL[0])) m_shckl = value;
    else if (StringMatch(sk, VAR_SBDUR[0])) m_sbdur = CVT_INT(value);
    else if (StringMatch(sk, VAR_LLLS[0])) m_llls = value;
    else if (StringMatch(sk, VAR_ULLS[0])) m_ulls = value;
    else if (StringMatch(sk, VAR_LLLE[0])) m_llle = value;
    else if (StringMatch(sk, VAR_ULLE[0])) m_ulle = value;
    else if (StringMatch(sk, VAR_LLDL[0])) m_lldl = value;
    else if (StringMatch(sk, VAR_ULDL[0])) m_uldl = value;
    else
        throw ModelException(M_PG_ORYZA[0], "SetValue", "Parameter " + sk + " does not exist.");
}

void ORYZA::Set1DData(const char* key, int n, float* data) {
    string sk(key);
    CheckInputSize(M_PG_ORYZA[0], key, n, m_nCells);
    //// climate
    if (StringMatch(sk, DataType_MeanTemperature)) m_meanTemp = data;
    else if (StringMatch(sk, DataType_MinimumTemperature)) m_tMin = data;
    else if (StringMatch(sk, DataType_MaximumTemperature)) m_tMax = data;
    else if (StringMatch(sk, DataType_SolarRadiation)) m_SR = data;
        //// soil properties and water related
    else if (StringMatch(sk, VAR_SOILLAYERS[0])) m_nSoilLyrs = data;
    else if (StringMatch(sk, VAR_SOL_ZMX[0])) m_soilZMX = data;
    else if (StringMatch(sk, VAR_SOL_ALB[0])) m_soilALB = data;
    else if (StringMatch(sk, VAR_SOL_SW[0])) m_soilWtrStoPrfl = data;
    else if (StringMatch(sk, VAR_SOL_SUMAWC[0])) m_totSoilAWC = data;
    else if (StringMatch(sk, VAR_SOL_SUMSAT[0])) m_totSoilSat = data;
    else if (StringMatch(sk, VAR_SOL_COV[0])) m_rsdCovSoil = data;
    else if (StringMatch(sk, VAR_SNAC[0])) m_snowAcc = data;
        //// management
    else if (StringMatch(sk, VAR_EPCO[0])) m_epco = data;
        // rice
    else if (StringMatch(sk, VAR_CROPSTA[0])) m_cropsta = data;
    else if (StringMatch(sk, VAR_SOL_RSDIN[0])) m_sol_rsdin = data;
    else if (StringMatch(sk, VAR_PPT[0])) m_ppt = data;
    else if (StringMatch(sk, VAR_CELL_LAT[0])) m_celllat = data;
    else if (StringMatch(sk, VAR_LAIDAY[0])) m_lai = data;
    else if (StringMatch(sk, VAR_ANCRF[0])) m_ancrf = data;
    else
        throw ModelException(M_PG_ORYZA[0], "Set1DData", "Parameter " + sk + " does not exist.");
}

void ORYZA::Set2DData(const char* key, int nrows, int ncols, float** data) {
    string sk(key);
    CheckInputSize2D(M_PG_ORYZA[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    if (StringMatch(sk, VAR_SOILDEPTH[0])) m_soilDepth = data;
    else if (StringMatch(sk, VAR_SOILTHICK[0])) m_soilThick = data;
    else if (StringMatch(sk, VAR_SOL_RSD[0])) m_soilRsd = data;
    else if (StringMatch(sk, VAR_SOL_AWC[0])) m_soilAWC = data;
    else if (StringMatch(sk, VAR_SOL_ST[0])) m_soilStorage = data;
    else if (StringMatch(sk, VAR_SOL_NO3[0])) m_soilNO3 = data;
    else if (StringMatch(sk, VAR_SOL_UL[0])) m_sol_sat = data;
    else if (StringMatch(sk, VAR_SOL_WPMM[0])) m_soilWP = data;
    else {
        throw ModelException(M_PG_ORYZA[0], "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

float ORYZA::CalHeatUnitDaily(int i) {
    float tt = 0.f;
    float hu = 0.f;
    for (int k = 1; k <= 24; k++) {
        float td = m_meanTemp[i] + 0.5f * std::abs(m_tMax[i] - m_tMin[i]) * cos(0.2618f * (k - 14.f));
        if (td > m_tbd && td < m_tmd) {
            if (td > m_tod) {
                td = m_tod - (td - m_tod) * (m_tod - m_tbd) / (m_tmd - m_tod);
            }
            tt = tt + (td - m_tbd) / 24;
        }
    }
    hu = tt;
    return hu;
}

float ORYZA::CalDevelopmentRate(int i) {
    float dl = 0.f, tstr = 0.f, ppfac = 0.f;
    float dvr = 0.f;
    float hu = CalHeatUnitDaily(i);

    if (m_dvs[i] >= 0 && m_dvs[i] < 0.4f) {
        dvr = m_dvrj * hu;}
    else if (m_dvs[i] < 0.65f) {
        dl = m_dayL[i] + 0.9f;
        if (dl < m_mopp)
            ppfac = 1.f;
        else
            ppfac = 1.f - (dl - m_mopp) * m_ppse;
        ppfac = Min(1.f, Max(0, ppfac));
        dvr = m_dvri * hu * ppfac;
    } else if (m_dvs[i] < 1.f) dvr = m_dvrp * hu;
    else if (m_dvs[i] > 1.f) dvr = m_dvrr * hu;

    if (m_cropsta[i] == 3.f) tstr = m_ts[i];
    float tshckd = m_shckd * tstr;
    if (m_cropsta[i] > 3.f && m_ts[i] < tstr + tshckd) dvr = 0.f;

    m_dvs[i] = m_dvs[i] + dvr;
    return dvr;
}

void ORYZA::CalDayLengthAndSINB(int i) {
    float DEGTRAD = 0.017453292f, zzcos, zzsin;
    // float dayLenP = 0.f; // not used?
    /// compute the params according to lat

    float dec = -asin(sin(23.45f * DEGTRAD) * cos(2.f * PI * (m_dayOfYear + 10.f) / 365.f));
    /*m_sinLD[i] = sin (DEGTRAD * 31.2f) * sin (dec);
    m_cosLD[i] = cos (DEGTRAD * 31.2f) * cos (dec);*/

    m_sinLD[i] = sin(DEGTRAD * m_cellLat) * sin(dec);
    m_cosLD[i] = cos(DEGTRAD * m_cellLat) * cos(dec);
    float aob = m_sinLD[i] / m_cosLD[i];

    if (aob < -1) {
        m_dayL[i] = 0;
        zzcos = 0;
        zzsin = 1;
    } else if (aob > 1) {
        m_dayL[i] = 24;
        zzcos = 0;
        zzsin = -1;
    } else {
        m_dayL[i] = 12.f * (1.f + 2.f * asin(aob) / PI);
        //dayLenP = 12.f * (1.f + 2.0f * asin(aob) / PI);
        float zza = PI * (12.f + m_dayL[i]) / 24.f;
        zzcos = cos(zza);
        zzsin = sin(zza);
    }

    float dsinb = 2.f * 3600.f * (m_dayL[i] * 0.5f * m_sinLD[i] - 12.f * m_cosLD[i] * zzcos / PI);
    m_dsinbe[i] = 2.f * 3600.f * (m_dayL[i] * (0.5f * m_sinLD[i] + 0.2f * pow(m_sinLD[i], 2.f) + 0.1f *
        pow(m_cosLD[i], 2.f)) - (12.f * m_cosLD[i] * zzcos + 9.6f * m_sinLD[i] * m_cosLD[i] *
        zzcos + 2.4f * pow(m_cosLD[i], 2.f) * zzcos * zzsin) / PI);

    m_solcon[i] = 1370.f * (1.f + 0.033f * cos(2.f * PI * m_dayOfYear / 365.f));
}

void ORYZA::CalDirectRadiation(int i) {
    float atmtr = 0.f, frdif = 0.f;
    m_sinb[i] = m_sinLD[i] + m_cosLD[i] * cos((m_hour - 12.f) * 0.2617993f);
    if (m_sinb[i] > 0) {
        /// sun is above the horizon
        //  SR : convert from MJ/m2/day to J/m2/day
        float tmpr1 = m_SR[i] * 1000000.f * m_sinb[i] * (1.f + 0.4f * m_sinb[i]) / m_dsinbe[i];
        atmtr = tmpr1 / (m_solcon[i] * m_sinb[i]);
        if (atmtr < 0.22) frdif = 1.f;
        else if (atmtr > 0.22 && atmtr < 0.35) frdif = 1.f - 6.4f * pow(atmtr - 0.22f, 2.f);
        else frdif = 1.47f - 1.66f * atmtr;

        float xx = 0.15f + 0.85f * (1.f - exp(-0.1f / m_sinb[i]));
        frdif = Max(frdif, xx);

        m_rdpdf[i] = tmpr1 * m_frpar * frdif;
        m_rdpdr[i] = tmpr1 * m_frpar * (1 - frdif);
    }
}

void ORYZA::CalLeafAbsorbRadiation(int i) {
    /// Scattering coefficient of leaves for PAR
    float cslv = 0.2f;
    float tmpr1 = sqrt(1.f - cslv);
    float rflh = (1.f - tmpr1) / (1.f + tmpr1);
    float rfls = rflh * 2.f / (1.f + 2.f * m_sinb[i]);
    //compute Extinction coefficient
    float kdf = 0.f;
    if (m_dvs[i] > 0.f && m_dvs[i] < 0.65f) kdf = 0.4f;
    else if (m_dvs[i] < 1.f) kdf = 0.4f + 0.2f / 0.35f * (m_dvs[i] - 0.65f);
    else kdf = 0.6f;
    float ecpdf = kdf;

    float clustf = ecpdf / (0.8f * tmpr1);
    float ecpbl = 0.5f / m_sinb[i] * clustf;
    float ecptd = ecpbl * tmpr1;

    float rapdfl = (1.f - rflh) * m_rdpdf[i] * ecpdf * exp(-ecpdf * m_gaid[i]);
    float raptdl = (1.f - rfls) * m_rdpdr[i] * ecptd * exp(-ecptd * m_gaid[i]);
    float rapddl = (1.f - cslv) * m_rdpdr[i] * ecpbl * exp(-ecpbl * m_gaid[i]);

    m_rapshl[i] = rapdfl + raptdl - rapddl;
    m_rapppl[i] = (1.f - cslv) * m_rdpdr[i] / m_sinb[i];
    m_fslla[i] = clustf * exp(-ecpbl * m_gaid[i]);
}

float ORYZA::CalLeafMaxAssimilationRate(float gai, float gaid, float nflv, float redf) {
    /// AmaxIn shows in the oryza.for,but it is not in use everywhere,so delete it
    float slni = 0.f, Amax = 0.f;
    float AmaxCO2 = 49.57f / 34.26f * (1.f - exp(-0.208f * (m_co2 - 60.f) / 49.57f));
    AmaxCO2 = Max(0.f, AmaxCO2);

    if (gai > 0.01f && m_knf > 0.f) {
        slni = nflv * gai * m_knf * exp(m_knf * gaid) / (1.f - exp(-m_knf * gai));
    } else {
        slni = nflv;
    }

    if (slni > 0.5f) {
        Amax = 9.5f + 22.f * slni * redf * AmaxCO2;
    } else {
        Amax = Max(0.f, 68.33f * (slni - 0.2f) * redf *AmaxCO2);
    }

    return Amax;
}

void ORYZA::Sgpl(int i) {
    // Gauss weights for three point Gauss
    float gsx[3] = {0.112702, 0.500000, 0.887298};
    float gsw[3] = {0.277778, 0.444444, 0.277778};
    float gpshl = 0.f, gpsll = 0.f, rapsll = 0.f;
    // m_gai[i] = m_aLAI;

    CalLeafAbsorbRadiation(i);
    // calculate redf and nflv
    if (m_meanTemp[i] <= 10.f) m_redf = 0.f;
    else if (m_meanTemp[i] <= 20.f) m_redf = 0.1f * (m_meanTemp[i] - 10.f);
    else if (m_meanTemp[i] <= 37.f) m_redf = 1.f;
    else m_redf = Max(0.f, (1.f - (m_meanTemp[i] - 37.f) * 0.2f));
    if (m_dvs[i] <= 0.16f) m_nflv = 0.54f;
    else if (m_dvs[i] <= 0.33f) m_nflv = 0.54f + (m_dvs[i] - 0.16f) * 6.25f;
    else if (m_dvs[i] <= 0.65f) m_nflv = 1.53f - (m_dvs[i] - 0.33f);
    else if (m_dvs[i] <= 0.8f) m_nflv = 1.222f + (m_dvs[i] - 0.65f) * 2.3f;
    else if (m_dvs[i] <= 1.f) m_nflv = 1.56f - (m_dvs[i] - 0.8f) * 1.25f;
    else if (m_dvs[i] <= 1.45f) m_nflv = 1.29f + (m_dvs[i] - 1.f) * 0.16f;
    else m_nflv = 1.36f - (m_dvs[i] - 1.45f);

    float Amax2 = CalLeafMaxAssimilationRate(m_gai[i], m_gaid[i], m_nflv, m_redf);
    if (Amax2 > 0) {
        gpshl = Amax2 * (1.f - exp(-m_rapshl[i] * m_eff / Amax2));
    }

    for (int k = 0; k < 3; k++) {
        float tmpr1 = m_rapshl[i] + rapsll * gsx[k];
        if (Amax2 > 0) {
            gpsll = gpsll + Amax2 * (1.f - exp(-tmpr1 * m_eff / Amax2)) * gsw[k];
        }
        rapsll = rapsll + tmpr1 * gsw[k];
    }
    m_gpl[i] = m_fslla[i] * gpsll + (1.f - m_fslla[i]) * gpshl;
    m_rapl[i] = m_fslla[i] * rapsll + (1.f - m_fslla[i]) * m_rapshl[i];
}

void ORYZA::CalCanopyAssimilationRate(int i) {
    // Gauss weights for three point Gauss
    float gsx[3] = {0.112702, 0.500000, 0.887298};
    float gsw[3] = {0.277778, 0.444444, 0.277778};
    m_gai[i] = m_aLAI;

    for (int k = 0; k < 3; k++) {
        m_gaid[i] = m_gai[i] * gsx[k];
        Sgpl(i);
        // Integration of local assimilation rate to canopy assimilation (GPC) and absorption of PAR by canopy (RAPC)
        m_gpc[i] = m_gpc[i] + m_gpl[i] * gsw[k];
        m_rapc[i] = m_rapc[i] + m_rapl[i] * gsw[k];
    }
    m_gpc[i] = m_gpc[i] * m_gai[i];
    m_rapc[i] = m_rapc[i] * m_gai[i];
}

void ORYZA::CalDailyCanopyPhotosynthesisRate(int i) {
    // Gauss weights for three point Gauss
    float gsx[3] = {0.112702, 0.500000, 0.887298};
    float gsw[3] = {0.277778, 0.444444, 0.277778};

    //CalDayLengthAndSINB(i);

    for (int k = 0; k < 3; k++) {
        m_hour = 12.f + m_dayL[i] * 0.5f * gsx[k];

        CalDirectRadiation(i);
        CalCanopyAssimilationRate(i);

        m_gpcdt[i] = m_gpcdt[i] + m_gpc[i] * gsw[k];
        m_rapcdt[i] = m_rapcdt[i] + m_rapc[i] * gsw[k];
    }
    // Integration of instantaneous assimilation/absorption rate to a daily total (GPCDT/RAPCDT)
    m_gpcdt[i] = m_gpcdt[i] * m_dayL[i];
    m_rapcdt[i] = m_rapcdt[i] * m_dayL[i] * 3600.f;
    m_dtga[i] = m_gpcdt[i];
}

void ORYZA::CalSpikeletAndGrainRate(int i) {
    float tincr = 5.f * (1.f - m_frStrsWtr[i]) * 1.6f;

    // Spikelet formation between PI and Flowering
    if (m_dvs[i] > 0.65f && m_dvs[i] < 1.f) gnsp = m_gcr[i] * m_spgf;
    else gnsp = 0;

    // Grain formation from spikelets,compute the reduction factors
    if (m_dvs[i] > 0.75f && m_dvs[i] < 1.2f) {
        float ctt = Max(0.f, 22.f - (m_meanTemp[i] - tincr));
        // Accumulated cold degree
        m_coldTT[i] = m_coldTT[i] + ctt;
    }
    if (m_dvs[i] > 0.96f && m_dvs[i] < 1.2f) {
        // Average daily maximum temperature during flowering
        m_tfert[i] += m_tMax[i] + tincr;
        m_ntfert += 1.f;
    }

    // there is a second requirement that GRAINS (Fortran logical function whether grains are formed) is true
    if (m_dvs[i] > 1.2f) {
        float sf1 = 1.f - (4.6f + 0.054f * pow(m_coldTT[i], 1.56f)) / 100.f;
        sf1 = Min(1.f, Max(0.f, sf1));
        float xx = m_tfert[i] / m_ntfert;
        float sf2 = 1.f / (1.f + exp(0.853f * (xx - 36.6f)));
        sf2 = Min(1.f, Max(0.f,sf2));
        float spfert = Min(sf1, sf2);
        m_gngr[i] = m_nsp[i] * spfert;
    } else {
        m_gngr[i] = 0;
    }
}

void ORYZA::LAI(int i) {
    // Actual relative growth rate of leaves ((oCd)-1)
    float rgrl = m_rgrlMX - (1.f - m_frStrsN[i]) * (m_rgrlMX - m_rgrlMN);
    float x = 1.f, testSet = 0.0001f;
    bool flag = false;
    float wlvgExs = 0.f, laiExs = 0.f;
    float hu = CalHeatUnitDaily(i);
    // Temperature sum for leaf area development at transplanting
    float tslvtr = 0.f, tshckl = 0.f;

    // for transplanted rice
    if (m_cropsta[i] < 3.f) {
        // seedbed: no drought stress effects in seedbed
        if (m_lai[i] < 1.f) {
            m_gLai = m_lai[i] * rgrl * hu;
            wlvgExs = m_wlvg[i];
            laiExs = m_lai[i];
        } else {
            float test = std::abs(m_lai[i] / m_wlvg[i] - m_sla[i]) / m_sla[i];
            if (test < testSet) flag = true;
            if (flag) {
                m_gLai = (m_wlvg[i] + m_rwlvg[i]) * m_sla[i] - m_lai[i];
            } else {
                float gLai1 = (m_wlvg[i] + m_rwlvg[i] - wlvgExs) * m_sla[i] + laiExs - m_lai[i];
                float gLai2 = (m_wlvg[i] + m_rwlvg[i]) * m_sla[i] - m_lai[i];
                m_gLai = (gLai1 + gLai2) / (x + 1.f);
                x += 1.f;
            }
        }
    } else if (m_cropsta[i] == 3) {
        // Transplanting effects: dilution and shock-setting
        tslvtr = m_ts[i];
        tshckl = m_shckl * tslvtr;
        m_gLai = m_lai[i] * m_nh * m_nplh / m_nplsb - m_lai[i];
    } else if (m_cropsta[i] == 4) {
        // After transplanting: main crop growth
        if (m_ts[i] < tslvtr + tshckl) m_gLai = 0;
        else {
            if (m_lai[i] < 0.f && m_dvs[i] < 1.f) {
                m_gLai = m_frStrsWtr[i] * m_lai[i] * rgrl * hu;
                wlvgExs = m_wlvg[i];
                laiExs = m_lai[i];
            } else {
                float test = std::abs(m_lai[i] / m_wlvg[i] - m_sla[i]) / m_sla[i];
                if (test < testSet) flag = true;
                if (flag) m_gLai = (m_wlvg[i] + m_rwlvg[i]) * m_sla[i] - m_lai[i];
                else {
                    float gLai1 = (m_wlvg[i] + m_rwlvg[i] - wlvgExs) * m_sla[i] + laiExs - m_lai[i];
                    float gLai2 = (m_wlvg[i] + m_rwlvg[i]) * m_sla[i] - m_lai[i];
                    m_gLai = (gLai1 + gLai2) / (x + 1.f);
                    x += 1.f;
                }
            }
        }
    }
}

void ORYZA::CalRiceGrowth(int i) {
    // 1.compute the heat unit and development rate of current day
    float hu = CalHeatUnitDaily(i);
    float dvr = CalDevelopmentRate(i);
    // 2.compute the development stage of current day
    //m_dvs[i] =m_dvs[i] + dvr;
    if (m_dvs[i] < 2.f) {
        // rice growing
        // if(m_cropsta[i] == 1) m_lai[i] = m_lape * m_nplsb; //re-initialize LAI at day of emergence
        if (m_cropsta[i] == 3) m_zrt[i] = m_zrttr; // re-initialize rooting depth at day of transplanting

        //if (m_frStrsWa[i] > 0.f) CalPlantNUptake(i);
        // Computation of weather variables
        float m_tmpCov = m_tmpsb;
        float m_tav = (m_tMax[i] + m_tmpCov + m_tMin[i]) *0.5f;
        float m_tavD = (m_tav + m_tMax[i]) *0.5f;
        float m_co2EFF = (1.f - exp(-0.00305f * m_co2 - 0.222f)) / (1.f - exp(-0.00305f * 340.f - 0.222f));
        if (0.f < m_tavD && m_tavD <= 10.f) {
            m_eff = 0.54f * m_co2EFF; // compute eff use linear_interp
        } else {
            m_eff = (0.54f - 0.06f * (m_tavD - 10.f)) * m_co2EFF;
        }

        // Leaf rolling under drought stress (only for photosynthesis)
        float m_laiRol = m_lai[i] * (0.5f * m_frStrsWtr[i] + 0.5f);
        float ssga = 0.f;
        if (m_dvs[i] >= 0.f && m_dvs[i] < 0.9f) ssga = 0.0003f;
        else ssga = 0.0003f - 0.00025f * (m_dvs[i] - 0.9f);
        m_sai = ssga * m_wst[i];
        m_aLAI = m_laiRol + 0.5f * m_sai;

        CalDailyCanopyPhotosynthesisRate(i);

        float pari1 = m_rapcdt[i] / 1.e6f;
        float dpari = m_rapcdt[i] / 1.e6f;
        // compute the daily incoming photosynthetically active radiation
        float dpar = m_frpar * m_SR[i];
        // Unrolling of ALAI again
        m_aLAI = m_lai[i] + 0.5f * m_sai;
        // drought stress will decreases the rate
        m_dtga[i] = m_dtga[i] * m_frStrsWtr[i];
        //compute the fraction of dry matter to the shoot(FSH), leave(FLV), stems(FST), panicle(FSO), root(FRT), leaf death(DRLV)
        if (m_dvs[i] > 0.f && m_dvs[i] <= 1.f) {
            m_fsh = m_aFsh + m_bFsh * m_dvs[i];
            if (m_dvs[i] < 0.5f) {
                m_flv = m_aFlv;
            } else {
                m_flv = m_aFlv - m_bFlv * (m_dvs[i] - 0.5f);
            }
            if (m_dvs[i] < 0.75f) {
                m_fst = 1.f - m_flv;
                m_fso = 0.f;
            } else {
                m_fst = 0.7f - (m_dvs[i] - 0.75f);
                m_fso = 1.f - m_flv - m_fst;
            }
            m_drlv = 0.f;
        } else if (m_dvs[i] < 1.2f) {
            m_fsh = 1.f;
            m_flv = 0.f;
            m_fst = 1.f - m_aFso - m_bFso * (m_dvs[i] - 1.f);
            m_fso = 1.f - m_fst;
            m_drlv = m_aDrlv + m_bDrlv * (m_dvs[i] - 1.f);
        } else {
            m_fsh = 1.f;
            m_flv = 0.f;
            m_fst = 0.f;
            m_fso = 1.f;
            m_drlv = m_aDrlv + m_bDrlv * (m_dvs[i] - 1.f);
        }
        m_frt = 1.f - m_fsh;
        // compute the loss of leaves and stems
        if (m_pltN[i] >= 0.f && m_pltN[i] <= 1.f) {
            m_nsllv = 1.f;
        } else if (m_pltN[i] < 2.f) {
            m_nsllv = 1.f + 0.2f * (m_pltN[i] - 1.f);
        } else {
            m_nsllv = 1.5f;
        }
        // the death or loss rate of leaves
        float llv = m_nsllv * m_wlvg[i] * m_drlv;
        if (m_dvs[i] > 1.f) {
            m_lstr = m_wst[i] / m_tclstr;
        } else {
            m_lstr = 0.f;
        }
        // Maintenance requirements
        m_teff = pow(m_q10, (m_tav - m_tref) *0.1f);
        float mnDVS = m_wlvg[i] / (m_wlvg[i] + m_wlvd[i]);
        float rmcr = (m_wlvg[i] * m_mainLV + m_wst[i] * m_mainST + m_wso[i] *
            m_mainSO + m_wrt[i] * m_mainRT) * m_teff * mnDVS;
        // Carbohydrate requirement for dry matter production
        float crGCR = m_fsh * (m_crgLV * m_flv + m_crgST * m_fst * (1.f - m_fstr) +
            m_crgSTR * m_fstr * m_fst + m_crgSO * m_fso) + m_crgRT * m_frt;
        // Gross and net growth rate of crop (GCR, NGCR)
        m_gcr[i] = (m_dtga[i] * 30.f / 44.f - rmcr + m_lstr * m_lrstr * m_fcSTR * 30.f / 12.f) / crGCR;
        float xx = m_gcr[i] - m_lstr * m_lrstr * m_fcSTR * 30.f / 12.f;
        float ngcr = Max(0, xx);
        // Intermediate variable for planting density, used to calculate the reduction in net weight
        float pltr = 1.f;
        if (m_cropsta[i] == 3.f) {
            pltr = m_nplh * m_nh / m_nplsb;
        }
        // Growth rates of crop organs at transplanting
        float rwlvg1 = m_wlvg[i] * (1.f - pltr);
        float gst1 = m_wsts[i] * (1.f - pltr);
        float rwstr1 = m_wstr[i] * (1.f - pltr);
        float grt1 = m_wrt[i] * (1.f - pltr);
        // Growth rates of crop organs
        float grt = m_gcr[i] * m_frt - grt1;
        glv = m_gcr[i] * m_fsh * m_flv - rwlvg1;
        float rwlvg = glv - llv;
        gst = m_gcr[i] * m_fsh * m_fst * (1.f - m_fstr) - gst1;
        float gstr = m_gcr[i] * m_fsh * m_fstr - rwstr1;
        float rwstr = gstr - m_lstr;
        gso = m_gcr[i] * m_fsh * m_fso;
        float ggr = 0.f;
        if (m_dvs[i] > 0.95) ggr = gso;

        CalSpikeletAndGrainRate(i);

        m_sla[i] = m_aSLA + m_bSLA * exp(m_cSLA * (m_dvs[i] - m_dSLA));
        m_sla[i] = Min(m_slaMX, m_sla[i]);
        LAI(i);

        // Growth respiration of the crop (RGCR)
        float co2RT = 44.f / 12.f * (m_crgRT * 12.f / 30.f - m_fcRT);
        float co2LV = 44.f / 12.f * (m_crgLV * 12.f / 30.f - m_fcLV);
        float co2ST = 44.f / 12.f * (m_crgST * 12.f / 30.f - m_fcST);
        float co2STR = 44.f / 12.f * (m_crgSTR * 12.f / 30.f - m_fcSTR);
        float co2SO = 44.f / 12.f * (m_crgSO * 12.f / 30.f - m_fcSO);
        float m_rgcr = (grt + grt1) * co2RT + (glv + rwlvg1) * co2LV + (gst + gst1) * co2ST + gso * co2SO +
                (gstr + rwstr1) * co2STR + (1.f - m_lrstr) * m_lstr * m_fcSTR * 44.f / 12.f;
        float ctrans = rwlvg1 * m_fcLV + gst1 * m_fcST + rwstr1 * m_fcSTR + grt1 * m_fcRT;
        float rtnass = (m_dtga[i] * 30.f / 44.f - rmcr) * 44.f / 30.f - m_rgcr - ctrans * 44.f / 12.f;

        // update the state variables like dvs, lai, wso, wrr
        // Integrate rate variables
        m_ts[i] += hu;
        m_wlvg[i] += rwlvg;
        m_wlvd[i] += llv;
        m_wsts[i] += gst;
        m_wstr[i] += rwstr;
        m_wso[i] += gso; // storage organs
        m_wrt[i] += grt; // root
        m_wrr[i] += ggr; // final yield
        m_ngr[i] += m_gngr[i];
        m_nsp[i] += gnsp;
        m_tnass[i] += rtnass;
        // Calculate sums of states
        m_wst[i] = m_wstr[i] + m_wsts[i];           // stem
        m_wlv[i] = m_wlvg[i] + m_wlvd[i];           // leaf
        m_wagt[i] = m_wlv[i] + m_wst[i] + m_wso[i]; // dry weight above ground
        m_biomass[i] = m_wagt[i] + m_wrt[i];
        m_frRoot[i] = m_wrt[i] / m_biomass[i];
        // Leaf area index and total area index (leaves + stems)
        m_lai[i] += m_gLai;
        m_aLAI = m_lai[i] + 0.5f * m_sai;
        // Root length
        m_zrt[i] += m_gzrt;
        m_zrt[i] = Min(m_zrt[i], m_zrtMCD);
    }
}


void ORYZA::CalPlantETAndWStress(int i) {
    // Only stress in main field after day of transplanting
    if (m_cropsta[i] == 4) {
        // compute the potential transpiration rate per unit of root length
        float trrm = m_ppt[i] / (m_zrt[i] + 1.0e-10f);
        float trw = 0.f, lrav = 0.f, zll = 0.f, leav = 0.f, ldav = 0.f;
        float *fact(nullptr), *musc(nullptr), *mskpa(nullptr);
        Initialize1DArray((int)m_nSoilLyrs[i], fact, 0.f);
        Initialize1DArray((int)m_nSoilLyrs[i], musc, 0.f);
        Initialize1DArray((int)m_nSoilLyrs[i], mskpa, 0.f);
        float trr;

        for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
            // Root length in each soil layer, the unit should be m
            float zrtl = Min(m_soilThick[i][j] / 1000.f, Max(m_zrt[i] - zll, 0.f));


            /// update total soil water in profile
            m_soilWtrStoPrfl[i] = 0.f;
            for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
                m_soilWtrStoPrfl[i] += m_soilStorage[i][ly];
            }
            if (m_soilStorage[i][j] >= m_soilAWC[i][j]) {
                fact[j] = Max(0.f, Min(1.f, (m_sol_sat[i][j] - m_soilStorage[i][j]) / (m_sol_sat[i][j] - m_soilAWC[i][j]
                )));
                musc[j] = pow(10.f, fact[j] * 2.f);
            } else if (m_soilStorage[i][j] >= m_soilWP[i][j] && m_soilStorage[i][j] < m_soilAWC[i][j]) {
                fact[j] = Max(0.f, Min(1.f, (m_soilStorage[i][j] - m_soilWP[i][j]) / (m_soilAWC[i][j] - m_soilWP[i][
                    j])));
                musc[j] = pow(10.f, 4.2f - fact[j] * 2.2f);
            } else if (m_soilStorage[i][j] < m_soilWP[i][j]) {
                fact[j] = Max(0.f, Min(1.f, (m_soilStorage[i][j] - 0.01f) / (m_soilWP[i][j] - 0.01f)));
                musc[j] = pow(10.f, 7.f - fact[j] * 2.8f);
            }

            mskpa[j] = musc[j] / 10.f;
            // Leaf-rolling factor
            float lr = (log10(mskpa[j]) - log10(m_llls)) / (log10(m_ulls) - log10(m_llls));
            lr = Min(0.f, Max(1.f, lr));
            lrav = lrav + zrtl / m_zrt[i] * lr;
            // Relative leaf expansion rate factor
            float le = (log10(mskpa[j]) - log10(m_llle)) / (log10(m_ulle) - log10(m_llle));
            le = Min(0.f, Max(1.f, le));
            leav = leav + zrtl / m_zrt[i] * le;
            // Relative death rate factor
            float ld = (log10(mskpa[j]) - log10(m_lldl)) / (log10(m_uldl) - log10(m_lldl));
            ld = Min(0.f, Max(1.f, ld));
            ldav = ldav + zrtl / m_zrt[i] * ld;

            // Relative transpiration ratio (actual/potential)
            if (mskpa[j] >= 10000.f) {
                trr = 0.f;
            } else {
                trr = 2.f / (1.f + exp(0.003297f * mskpa[j]));
            }
            trr = Min(0.f, Max(1.f, trr));

            float wla = Max(0.f, (m_soilStorage[i][j] - m_soilWP[i][j]) * zrtl * 1000.f);
            float trwl = Min(wla, trr * zrtl * trrm);
            trw = trw + trwl;

            // update soil water content
            m_soilStorage[i][j] = Max(UTIL_ZERO, m_soilStorage[i][j] - trwl);
            // accumlate the root length of each layer
            zll += m_soilThick[i][j] / 1000.f;
        }
        /// update total soil water in profile
        m_soilWtrStoPrfl[i] = 0.f;
        for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
            m_soilWtrStoPrfl[i] += m_soilStorage[i][ly];
        }
        m_frStrsWtr[i] = trw / m_ppt[i];
        m_actPltET[i] = trw;

        Release1DArray(fact);
        Release1DArray(musc);
        Release1DArray(mskpa);
    } else {
        // If crop is not in the main field, set stress factors at 1
        m_frStrsWtr[i] = 1.f;
        m_actPltET[i] = 0.f;
    }

}

void ORYZA::CalPlantNUptake(int i) {
    // compute N demand of crop
    float nMinSO;
    float nMaxL = m_dvs[i] < 0.4f ? 0.053f : 0.053f - m_nMaxL * (m_dvs[i] - 0.4f);
    float nMinL = 0.025f - m_nMinL * m_dvs[i];
    if (m_ancrf[i] < 50.f) {
        nMinSO = 0.006f - m_anMinSO * m_ancrf[i];
    } else if (m_ancrf[i] < 400.f) {
        nMinSO = 0.0008f + m_bnMinSO * (m_ancrf[i] - 50.f);
    } else {
        nMinSO = 0.017f;
    }
    float LeafDemandN = nMaxL * (m_wlvg[i] + glv) - m_anlv[i];
    float StemDemandN = nMaxL * 0.5f * (m_wst[i] + gst) - m_anst[i];
    float SODemandNMax = m_nMaxSO * gso;
    float SODemandNMin = nMinSO * gso;
    // Calculate translocation of N from organs, No translocation before DVS = 0.95
    float aTNlv, aTNst, aTN, ntso, aTnrt;
    if (m_dvs[i] < 0.95f) {
        aTNlv = 0.f;
        aTNst = 0.f;
        aTN = 1.f;
        ntso = 0.f;
        aTnrt = 0.f;
    } else {
        aTNlv = Max(0.f, m_anlv[i] - m_wlvg[i] * m_rfnlv);
        aTNst = Max(0.f, m_anst[i] - m_wst[i] * m_rfnst);
        aTnrt = (aTNlv + aTNst) * m_fntrt;
        aTN = aTNlv + aTNst + aTnrt;
        ntso = aTN / m_tcntrf;
        ntso = Max(ntso, SODemandNMin);
        ntso = Min(ntso, SODemandNMax);
    }
    // Actual N translocation rates from plant organs, ATN should not be 0
    float ntlv = ntso * aTNlv / aTN;
    float ntst = ntso * aTNst / aTN;
    float ntrt = ntso * aTnrt / aTN;
    // Calculate nitrogen uptake
    // float n_reduc = 300.f; /// nitrogen uptake reduction factor (not currently used; defaulted 300.)
    float tnsoil = 0.f;
    for (int k = 0; k < m_nSoilLyrs[i]; k++) {
        tnsoil += m_soilNO3[i][k];
    }
    // Available N uptake is mimimum of soil supply and maximum crop uptake
    float nupp = Min(8.f, tnsoil);
    float totNDemand = LeafDemandN + ntlv + (StemDemandN + ntst) + (SODemandNMax - ntso);
    float naLV = Max(0.f, Min(LeafDemandN + ntlv, nupp * (LeafDemandN + ntlv) / totNDemand));
    float naST = Max(0.f, Min(StemDemandN + ntst, nupp * (StemDemandN + ntst) / totNDemand));
    float naSO = Max(0.f, Min(SODemandNMax - ntso, nupp* (SODemandNMax - ntso) / totNDemand));
    m_anst[i] += naST - ntst;
    m_plantUpTkN[i] = naLV + naST + naSO;
    if (m_plantUpTkN[i] < 0.f) m_plantUpTkN[i] = 0.f;
    m_pltN[i] += m_plantUpTkN[i];
    for (int l = 0; l < CVT_INT(m_nSoilLyrs[i]); l++) {
        float uno3l = Min(m_plantUpTkN[i], m_soilNO3[i][l]);
        m_soilNO3[i][l] -= uno3l;
    }
    float nstan = 0.f, nlvan = 0.f;
    if (m_dvs[i] < 1.f) {
        nstan = naST - ntst;
        nlvan = naLV - ntlv;
    }
    m_ancrf[i] += nstan + nlvan;
    m_anlv[i] += naLV;
    float fnlv = m_anlv[i] / m_wlvg[i];
    m_frStrsN[i] = (fnlv - 0.9f * nMaxL) / (nMaxL - 0.9f * nMaxL);
    m_frStrsN[i] = Max(m_frStrsN[i], 0);
    m_frStrsN[i] = Min(m_frStrsN[i], 1.f);
}

int ORYZA::Execute() {
    CheckInputData();
    InitialOutputs();

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        /// calculate albedo in current day
        float cej = -5.e-5f, eaj = 0.f;
        eaj = exp(cej * (m_rsdCovSoil[i] + 0.1f));
        if (m_snowAcc[i] < 0.5f) {
            m_alb[i] = m_soilALB[i];
            if (m_lai[i] > 0.f)
                m_alb[i] = 0.23f * (1.f - eaj) + m_soilALB[i] * eaj;
        } else
            m_alb[i] = 0.8f;
        /// calculate residue on soil surface for current day
        m_rsdCovSoil[i] = Max((m_wagt[i] + m_soilRsd[i][0]), 0.f);
        /// calculate the parameters related to the lat and date
        m_cellLat = m_celllat[i];
        CalDayLengthAndSINB(i);

        if (m_cropsta[i] > 0.f && m_dvs[i] < 2.f) {
            /// rice growing
            CalPlantETAndWStress(i);
            CalRiceGrowth(i);
            if (m_frStrsWtr[i] > 0.f) CalPlantNUptake(i);

            if (m_cropsta[i] == 1.f) {
                sowDay = m_dayOfYear;
            }
            if (m_dayOfYear <= sowDay + m_sbdur) {
                m_cropsta[i] = 2.f;
            } else if (m_dayOfYear == sowDay + m_sbdur) {
                m_cropsta[i] = 3.f;
            } else {
                m_cropsta[i] = 4.f;
            }
        }
    }
    return true;
}

bool ORYZA::CheckInputData() {
    /// DT_Single
    CHECK_POSITIVE(M_PG_ORYZA[0], m_nCells);
    if (m_nCells <= 0)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    if (m_maxSoilLyrs <= 0)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData",
                             "The layer number of the input 2D raster data can not be less than zero.");
    if (FloatEqual(m_co2, NODATA_VALUE))
        throw ModelException(M_PG_ORYZA[0], "CheckInputData",
                             "The ambient atmospheric CO2 concentration must be provided.");
    /// DT_Raster1D
    if (m_tMin == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData", "The min temperature data can not be NULL.");
    if (m_tMax == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData", "The max temperature data can not be NULL.");
    if (m_meanTemp == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData", "The mean temperature data can not be NULL.");
    if (m_SR == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData",
                             "The solar radiation data can not be NULL.");
    if (m_nSoilLyrs == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData", "The soil layers data can not be NULL.");
    if (m_soilZMX == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData",
                             "The maximum rooting depth in soil profile can not be NULL.");
    if (m_soilALB == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData", "The albedo when soil is moist can not be NULL.");
    if (m_soilWtrStoPrfl == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData",
                             "The amount of water stored in the soil profile can not be NULL.");
    if (m_totSoilAWC == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData",
                             "The amount of water held in soil profile at field capacity can not be NULL.");
    if (m_totSoilSat == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData",
                             "The amount of water held in soil profile at saturation can not be NULL.");
    /// DT_Raster2D
    if (m_soilDepth == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData", "The soil depth data can not be NULL.");
    if (m_soilThick == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData", "The soil thickness data can not be NULL.");
    if (m_soilAWC == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData",
                             "The water available to plants in soil layer at field capacity can not be NULL.");
    if (m_soilStorage == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData", "The soil moisture in soil layers can not be NULL.");
    if (m_soilNO3 == nullptr)
        throw ModelException(M_PG_ORYZA[0], "CheckInputData",
                             "The nitrogen stored in the nitrate pool in soil layer can not be NULL.");
    return true;
}

void ORYZA::InitialOutputs() {
    if (m_alb == nullptr) Initialize1DArray(m_nCells, m_alb, 0.f);
    if (m_rsdCovSoil == nullptr || m_soilRsd == nullptr) {
        Initialize1DArray(m_nCells, m_rsdCovSoil, m_sol_rsdin);
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilRsd, 0.f);
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            m_soilRsd[i][0] = m_rsdCovSoil[i];
        }
    }
    if (m_frStrsWtr == nullptr)
        Initialize1DArray(m_nCells, m_frStrsWtr, 1.f);
    if (m_frStrsN == nullptr)
        Initialize1DArray(m_nCells, m_frStrsN, 1.f);
    if (m_actPltET == nullptr)
        Initialize1DArray(m_nCells, m_actPltET, 0.f);
    if (m_ts == nullptr)
        Initialize1DArray(m_nCells, m_ts, 0.f);
    if (m_dayL == nullptr)
        Initialize1DArray(m_nCells, m_dayL, 0.f);
    if (m_sinLD == nullptr)
        Initialize1DArray(m_nCells, m_sinLD, 0.f);
    if (m_cosLD == nullptr)
        Initialize1DArray(m_nCells, m_cosLD, 0.f);
    if (m_dsinbe == nullptr)
        Initialize1DArray(m_nCells, m_dsinbe, 0.f);
    if (m_sinb == nullptr)
        Initialize1DArray(m_nCells, m_sinb, 0.f);
    if (m_solcon == nullptr)
        Initialize1DArray(m_nCells, m_solcon, 0.f);
    if (m_rdpdf == nullptr)
        Initialize1DArray(m_nCells, m_rdpdf, 0.f);
    if (m_rdpdr == nullptr)
        Initialize1DArray(m_nCells, m_rdpdr, 0.f);
    if (m_wlvg == nullptr)
        Initialize1DArray(m_nCells, m_wlvg, 0.01f);
    if (m_wlvd == nullptr)
        Initialize1DArray(m_nCells, m_wlvd, 0.f);
    if (m_wsts == nullptr)
        Initialize1DArray(m_nCells, m_wsts, 0.f);
    if (m_wstr == nullptr)
        Initialize1DArray(m_nCells, m_wstr, 0.f);
    if (m_wso == nullptr)
        Initialize1DArray(m_nCells, m_wso, 0.f);
    if (m_wrt == nullptr)
        Initialize1DArray(m_nCells, m_wrt, 0.f);
    if (m_wrr == nullptr)
        Initialize1DArray(m_nCells, m_wrr, 0.f);
    if (m_ngr == nullptr)
        Initialize1DArray(m_nCells, m_ngr, 0.f);
    if (m_nsp == nullptr)
        Initialize1DArray(m_nCells, m_nsp, 0.f);
    if (m_tnass == nullptr)
        Initialize1DArray(m_nCells, m_tnass, 0.f);
    if (m_wst == nullptr)
        Initialize1DArray(m_nCells, m_wst, 0.f);
    if (m_wlv == nullptr)
        Initialize1DArray(m_nCells, m_wlv, 0.f);
    if (m_wagt == nullptr)
        Initialize1DArray(m_nCells, m_wagt, 0.f);
    if (m_zrt == nullptr)
        Initialize1DArray(m_nCells, m_zrt, 0.f);
    if (m_dvs == nullptr)
        Initialize1DArray(m_nCells, m_dvs, 0.f);
    if (m_ancrf == nullptr)
        Initialize1DArray(m_nCells, m_ancrf, 0.f);
    /*if(m_cellLat == NULL)
    Initialize1DArray(m_nCells, m_cellLat, 0.f);*/
    if (m_gai == nullptr)
        Initialize1DArray(m_nCells, m_gai, 0.f);
    if (m_gaid == nullptr)
        Initialize1DArray(m_nCells, m_gaid, 0.f);
    if (m_rapshl == nullptr)
        Initialize1DArray(m_nCells, m_rapshl, 0.f);
    if (m_rapppl == nullptr)
        Initialize1DArray(m_nCells, m_rapppl, 0.f);
    if (m_fslla == nullptr)
        Initialize1DArray(m_nCells, m_fslla, 0.f);
    if (m_gpl == nullptr)
        Initialize1DArray(m_nCells, m_gpl, 0.f);
    if (m_rapl == nullptr)
        Initialize1DArray(m_nCells, m_rapl, 0.f);
    if (m_gpc == nullptr)
        Initialize1DArray(m_nCells, m_gpc, 0.f);
    if (m_rapc == nullptr)
        Initialize1DArray(m_nCells, m_rapc, 0.f);
    if (m_gpcdt == nullptr)
        Initialize1DArray(m_nCells, m_gpcdt, 0.f);
    if (m_rapcdt == nullptr)
        Initialize1DArray(m_nCells, m_rapcdt, 0.f);
    if (m_dtga == nullptr)
        Initialize1DArray(m_nCells, m_dtga, 0.f);
    if (m_pltN == nullptr)
        Initialize1DArray(m_nCells, m_pltN, 0.f);
    if (m_gcr == nullptr)
        Initialize1DArray(m_nCells, m_gcr, 0.f);
    if (m_wsts == nullptr)
        Initialize1DArray(m_nCells, m_wsts, 0.01f);
    if (m_anlv == nullptr)
        Initialize1DArray(m_nCells, m_anlv, 0.f);
    if (m_anst == nullptr)
        Initialize1DArray(m_nCells, m_anst, 0.f);
    if (m_coldTT == nullptr)
        Initialize1DArray(m_nCells, m_coldTT, 0.f);
    if (m_tfert == nullptr)
        Initialize1DArray(m_nCells, m_tfert, 0.f);
    if (m_gngr == nullptr)
        Initialize1DArray(m_nCells, m_gngr, 0.f);
    if (m_sla == nullptr)
        Initialize1DArray(m_nCells, m_sla, 0.f);
    if (m_biomass == nullptr)
        Initialize1DArray(m_nCells, m_biomass, 0.f);
    if (m_frRoot == nullptr)
        Initialize1DArray(m_nCells, m_frRoot, 0.f);
    if (m_plantUpTkN == nullptr)
        Initialize1DArray(m_nCells, m_plantUpTkN, 0.f);
    if (m_rwlvg == nullptr)
        Initialize1DArray(m_nCells, m_rwlvg, 0.f);

}

void ORYZA::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_LAST_SOILRD[0])) *data = m_stoSoilRootD;
    else if (StringMatch(sk, VAR_PLANT_N[0])) *data = m_pltN;
    else if (StringMatch(sk, VAR_FR_STRSWTR[0])) *data = m_frStrsWtr;
    else if (StringMatch(sk, VAR_SOL_COV[0])) *data = m_rsdCovSoil;
    else if (StringMatch(sk, VAR_SOL_SW[0])) *data = m_soilWtrStoPrfl;
    else if (StringMatch(sk, VAR_CROPSTA[0])) *data = m_cropsta;
    else if (StringMatch(sk, VAR_ALBDAY[0])) *data = m_alb;
    else if (StringMatch(sk, VAR_AET_PLT[0])) *data = m_actPltET;
    else if (StringMatch(sk, VAR_TS[0])) *data = m_ts;
    else if (StringMatch(sk, VAR_WLVG[0])) *data = m_wlvg;
    else if (StringMatch(sk, VAR_WLVD[0])) *data = m_wlvd;
    else if (StringMatch(sk, VAR_WSTS[0])) *data = m_wsts;
    else if (StringMatch(sk, VAR_WSTR[0])) *data = m_wstr;
    else if (StringMatch(sk, VAR_WSO[0])) *data = m_wso;
    else if (StringMatch(sk, VAR_WRT[0])) *data = m_wrt;
    else if (StringMatch(sk, VAR_WRR[0])) *data = m_wrr;
    else if (StringMatch(sk, VAR_NGR[0])) *data = m_ngr;
    else if (StringMatch(sk, VAR_NSP[0])) *data = m_nsp;
    else if (StringMatch(sk, VAR_TNASS[0])) *data = m_tnass;
    else if (StringMatch(sk, VAR_WST[0])) *data = m_wst;
    else if (StringMatch(sk, VAR_WLV[0])) *data = m_wlv;
    else if (StringMatch(sk, VAR_WAGT[0])) *data = m_wagt;
    else if (StringMatch(sk, VAR_ZRT[0])) *data = m_zrt;
    else if (StringMatch(sk, VAR_BIOMASS[0])) *data = m_biomass;
    else if (StringMatch(sk, VAR_DVS[0])) *data = m_dvs;
    else if (StringMatch(sk, VAR_ANCRF[0])) *data = m_ancrf;
    else {
        throw ModelException(M_PG_ORYZA[0], "Get1DData", "Result " + sk + " does not exist.");
    }
}

void ORYZA::Get2DData(const char* key, int* n, int* col, float*** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    *col = m_maxSoilLyrs;
    if (StringMatch(sk, VAR_SOL_RSD[0])) *data = m_soilRsd;
    else {
        throw ModelException(M_PG_ORYZA[0], "Get2DData", "Result " + sk + " does not exist.");
    }
}
