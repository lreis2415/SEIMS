#include "SNO_HBV.h"
#include "text.h"

SNO_HBV::SNO_HBV():
    m_nCells(-1),
    m_potentialMelt(nullptr),
    m_snowLiq(nullptr),
    m_tMean(nullptr),
    m_snowRefreezeFactor(nullptr),
    m_tMelt(0),
    m_swi(0),
    m_snowAcc(nullptr),
    m_snowMelt(nullptr)
    {
        SetModuleName(M_SNO_HBV[0]);
    }

void SNO_HBV::InitialOutputs() {
    Initialize1DArray(m_nCells, m_snowAcc, 0.);
    Initialize1DArray(m_nCells, m_snowMelt, 0.);
}

SNO_HBV::~SNO_HBV() {
    Release1DArray(m_snowAcc);
    Release1DArray(m_snowMelt);
}

void SNO_HBV::SetValue(const char* key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_T0[0])) m_tMelt = value;
    else if (StringMatch(sk, VAR_SWI[0])) m_swi = value;
    else {
        throw ModelException(GetModuleName(), "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void SNO_HBV::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_POTENTIAL_MELT[0])) { m_potentialMelt = data; }
    else if (StringMatch(sk, VAR_TMEAN[0])) { m_tMean = data; }
    else if (StringMatch(sk, VAR_SNOW_REFREEZE_FACTOR[0])) { m_snowRefreezeFactor = data; }
    else if (StringMatch(sk, VAR_SNOW_LIQUID[0])) { m_snowLiq = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool SNO_HBV::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

bool SNO_HBV::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(GetModuleName(), "CheckInputData", "Input data is invalid. The size could not be less than zero.");
        return false;
    }
    CHECK_POINTER(GetModuleName(), m_potentialMelt);
    CHECK_POINTER(GetModuleName(), m_snowLiq);
    CHECK_POINTER(GetModuleName(), m_tMean);
    return true;
}

void SNO_HBV::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_SNME[0])) { *data = m_snowMelt; }
    else if (StringMatch(sk, VAR_SNAC[0])) { *data = m_snowAcc; }
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

int SNO_HBV::Execute() {
    CheckInputData();
    InitialOutputs();
    // freezing temperature of water [C]. It's hardly affected by pressure/altitude.
    const FLTPT FREEZING_TEMP = 0.0;
    #pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT Ka = m_snowRefreezeFactor[i]; // refreeze_factor [mm/d/K]
        FLTPT Ta = m_tMean[i]; // temp_daily_ave
        FLTPT SWE = m_snowAcc[i]; // snow, SWE mm
        FLTPT SLiq = m_snowLiq[i]; // liquid part of snowpack, mm

        FLTPT melt = Min(NonNeg(m_potentialMelt[i]), SWE / m_dt_day); // positive, constrained by available snow
        SWE -= melt * m_dt_day;

        FLTPT potRefreeze = Ka * NonNeg(FREEZING_TEMP - Ta); // positively valued
        FLTPT refreeze = NonNeg(Min(SLiq / m_dt_day, potRefreeze));
        Convey(SLiq, m_snowAcc[i], refreeze);

        FLTPT SLiqCap = m_swi*SWE; // Calculate snow liquid capacity

        /* Following is equivalent to the pseudo code:
        if liqCap > liq:                                // if snow is not saturated
            if justMelted > liqCap - liq:                 // if melt > remaining capacity
                liq = liqCap                                // fill the capacity
            else:                                         // if melt < remaining capacity
                liq += justMelted                           // all melt to liquid
            overflow+=0                                   // no overflow
        else:                                           // if snow is saturated, cannot hold more liquid
            liq += 0                                      // no melt to liquid
            overflow += justMelted + (liq - liqCap)       // all melt and exceeded liq overflow 
        */
        if (SLiqCap > SLiq) {
            Supply(m_snowLiq[i], Min(melt,SLiqCap - SLiq));
            m_snowMelt[i]+=0;
        } else{
            m_snowLiq[i] += 0;
            Supply(m_snowMelt[i], SLiq - SLiqCap);
        }
        
        ////// Above is equivalent to the following more like from Raven:
        // FLTPT melt = Min(NonNeg(m_potentialMelt[i]), SWE / m_dt_day); // positive, constrained by available snow
        // SWE -= melt * m_dt_day;

        // FLTPT potRefreeze = Ka * NonNeg(FREEZING_TEMP - Ta); // positively valued
        // FLTPT refreeze = NonNeg(Min(SLiq / m_dt_day, potRefreeze));
        // SLiq -= refreeze * m_dt_day;

        // FLTPT SLiqCap = m_swi*SWE; // Calculate snow liquid capacity
        // FLTPT toLiq = Min(melt, NonNeg(SLiqCap - SLiq) / m_dt_day);
        // SLiq += toLiq * m_dt_day;
        
        // // Phase change. snow <-> liquid snow
        // Convey(m_snowAcc[i], m_snowLiq[i], toLiq-refreeze, m_dt_day);
        // // Direct melt. snow <-> snow melt (to EXCP, SURU, or soil[0])
        // Convey(m_snowAcc[i], m_snowMelt[i], NonNeg(melt - toLiq), m_dt_day);
        // // Overflow. liquid snow <-> snow melt (to EXCP, SURU, or soil[0])
        // Convey(m_snowLiq[i], m_snowMelt[i], NonNeg(SLiq - SLiqCap), m_dt_day);


    }
#ifdef PRINT_DEBUG
    FLTPT s1 = 0;
    FLTPT s2 = 0;
    for (int i = 0; i < m_nCells; i++) {
        s1 += m_snowLiq[i];
        s2 += m_snowMelt[i];
    }
    printf("[SNO_HBV] m_snowLiq=%f, m_snowMelt=%f\n", s1,s2);
    fflush(stdout);
#endif // PRINT_DEBUG
    return 0;
}
