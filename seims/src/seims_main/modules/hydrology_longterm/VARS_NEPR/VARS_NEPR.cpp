#include "VARS_NEPR.h"
#include "text.h"


VARS_NEPR::VARS_NEPR():
m_nCells(-1),
m_NEPR(nullptr),
m_DPST(nullptr),
m_SNME(nullptr),
m_GREL(nullptr)
{
    SetModuleName("VARS_NEPR");
}
bool VARS_NEPR::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}
void VARS_NEPR::Set1DData(const char* key, int n, FLTPT* data) {
    if (StringMatch(key, VAR_NEPR[0])) { m_NEPR=data;} 
    else if (StringMatch(key, VAR_DPST[0])) { m_DPST=data;} 
    else if (StringMatch(key, VAR_SNME[0])){ m_SNME=data;} 
    else if (StringMatch(key, VAR_GLAC_REL[0])) { m_GREL=data;}
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + string(key)
                             + " does not exist in current module. Please contact the module developer.");
    }
}
//Execute module
int VARS_NEPR::Execute() {
#ifdef PRINT_DEBUG
    FLTPT s1 = 0;
    FLTPT s2 = 0;
    FLTPT s3 = 0;
    FLTPT s4 = 0;
    for (int i = 0; i < m_nCells; i++) {
        if (m_DPST != nullptr) { s1 += m_DPST[i]; }
        if (m_SNME != nullptr) { s2 += m_SNME[i]; }
        if (m_GREL != nullptr) { s3 += m_GREL[i]; }
        if (m_NEPR != nullptr) { s4 += m_NEPR[i]; }
    }
    printf("[VARS_NEPR] DPST(%f) + SNME(%f) + GREL(%f) -> Original NEPR(%f)\n",
        s1,s2,s3,s4);
    fflush(stdout);
#endif

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if(m_DPST!=nullptr){
            Flush(m_DPST[i],m_NEPR[i]);
        }
        if(m_SNME!=nullptr){
            Flush(m_SNME[i],m_NEPR[i]);
        }
        if(m_GREL!=nullptr){
            Flush(m_GREL[i],m_NEPR[i]);
        }
    }
    return 0;
}
