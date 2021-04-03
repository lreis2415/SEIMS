#include "NonPointSource_Management.h"

#include "text.h"

NPS_Management::NPS_Management() :
    m_nCells(-1), m_cellWth(-1.f), m_cellArea(-1.f), m_timestep(-1.f),
    m_mgtFields(nullptr), m_soilWtrSto(nullptr),
    m_soilNO3(nullptr), m_soilNH4(nullptr), m_soilSolP(nullptr),
    m_soilStabOrgN(nullptr), m_soilHumOrgP(nullptr) {
    
    m_arealSrcFactory.clear();
}

NPS_Management::~NPS_Management() {
    if (!m_arealSrcFactory.empty()) {
        for (auto it = m_arealSrcFactory.begin(); it != m_arealSrcFactory.end();) {
            delete it->second;
            m_arealSrcFactory.erase(it++);
        }
        m_arealSrcFactory.clear();
    }
}

void NPS_Management::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_TimeStep[0])) m_timestep = value;
    else if (StringMatch(sk, Tag_CellWidth[0])) m_cellWth = value;
    else {
        throw ModelException(M_NPSMGT[0], "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void NPS_Management::Set2DData(const char* key, const int n, const int col, float** data) {
    CheckInputSize(M_NPSMGT[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SOL_ST[0])) m_soilWtrSto = data;
    else if (StringMatch(sk, VAR_SOL_NO3[0])) m_soilNO3 = data;
    else if (StringMatch(sk, VAR_SOL_NH4[0])) m_soilNH4 = data;
    else if (StringMatch(sk, VAR_SOL_SOLP[0])) m_soilSolP = data;
    else if (StringMatch(sk, VAR_SOL_SORGN[0])) m_soilStabOrgN = data;
    else if (StringMatch(sk, VAR_SOL_HORGP[0])) m_soilHumOrgP = data;
    else {
        throw ModelException(M_NPSMGT[0], "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void NPS_Management::SetScenario(Scenario* sce) {
    if (nullptr == sce) {
        throw ModelException(M_MUSK_CH[0], "SetScenario", "The scenario can not to be nullptr.");
    }
    map<int, BMPFactory *>& tmpBMPFactories = sce->GetBMPFactories();
    for (auto it = tmpBMPFactories.begin(); it != tmpBMPFactories.end(); ++it) {
        /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
        if (it->first / 100000 == BMP_TYPE_AREALSOURCE) {
#ifdef HAS_VARIADIC_TEMPLATES
            m_arealSrcFactory.emplace(it->first, static_cast<BMPArealSrcFactory *>(it->second));
#else
            m_arealSrcFactory.insert(make_pair(it->first,
                                               static_cast<BMPArealSrcFactory *>(it->second)));
#endif
        }
        /// Set areal source locations data
        if (nullptr == m_mgtFields) {
            m_mgtFields = static_cast<BMPArealSrcFactory *>(it->second)->GetRasterData();
        }
    }
}

bool NPS_Management::CheckInputData() {
    /// TOTO to be implemented.
    return true;
}

int NPS_Management::Execute() {
    CheckInputData();
    if (m_cellArea < 0.f) m_cellArea = m_cellWth * m_cellWth;
    for (auto it = m_arealSrcFactory.begin(); it != m_arealSrcFactory.end(); ++it) {
        /// 1 Set valid cells index of areal source regions
        if (!it->second->GetLocationLoadStatus()) {
            it->second->SetArealSrcLocsMap(m_nCells, m_mgtFields);
        }
        /// 2 Loop the management operations by sequence
        vector<int>& tmpArealFieldIDs = it->second->GetArealSrcIDs();
        vector<int>& tmpMgtSeqs = it->second->GetArealSrcMgtSeqs();
        for (auto mgtSeqIter = tmpMgtSeqs.begin(); mgtSeqIter != tmpMgtSeqs.end(); ++mgtSeqIter) {
            ArealSourceMgtParams* tmpMgtParams = it->second->GetArealSrcMgtMap().at(*mgtSeqIter);
            if (tmpMgtParams->GetStartDate() != 0 && tmpMgtParams->GetEndDate() != 0) {
                if (m_date < tmpMgtParams->GetStartDate() || m_date > tmpMgtParams->GetEndDate()) {
                    continue;
                }
            }
            for (auto fIDIter = tmpArealFieldIDs.begin(); fIDIter != tmpArealFieldIDs.end(); ++fIDIter) {
                float deltaWtrMM = 0.f, deltaNH4 = 0.f, deltaNO3 = 0.f, deltaOrgN = 0.f;
                float deltaMinP = 0.f, deltaOrgP = 0.f;
                map<int, ArealSourceLocations *>& tmpLocMap = it->second->GetArealSrcLocsMap();
                if (tmpLocMap.find(*fIDIter) == tmpLocMap.end()) {
                    continue;
                }
                ArealSourceLocations* tmpLoc = tmpLocMap.at(*fIDIter);
                float tmpSize = tmpLoc->GetSize();
                int tmpNCells = tmpLoc->GetValidCells();
                if (tmpMgtParams->GetWaterVolume() > 0.f) {
                    /// m3/'size'/day ==> mm
                    deltaWtrMM = tmpMgtParams->GetWaterVolume() * tmpSize * m_timestep *
                            1.1574074074074073e-05f / tmpNCells / m_cellArea * 1000.f;
                }
                /// kg/'size'/day ==> kg/ha
                float cvt = tmpSize * m_timestep * 1.1574074074074073e-05f / tmpNCells / m_cellArea * 10000.f;
                if (tmpMgtParams->GetNH4() > 0.f) {
                    deltaNH4 = cvt * tmpMgtParams->GetNH4();
                }
                if (tmpMgtParams->GetNO3() > 0.f) {
                    deltaNO3 = cvt * tmpMgtParams->GetNO3();
                }
                if (tmpMgtParams->GetOrgN() > 0.f) {
                    deltaOrgN = cvt * tmpMgtParams->GetOrgN();
                }
                if (tmpMgtParams->GetMinP() > 0.f) {
                    deltaMinP = cvt * tmpMgtParams->GetMinP();
                }
                if (tmpMgtParams->GetOrgP() > 0.f) {
                    deltaOrgP = cvt * tmpMgtParams->GetOrgP();
                }
                vector<int>& tmpCellIdx = tmpLoc->GetCellsIndex();
                for (auto idxIter = tmpCellIdx.begin(); idxIter != tmpCellIdx.end(); ++idxIter) {
                    /// add to the top first soil layer
                    if (deltaWtrMM > 0.f && nullptr != m_soilWtrSto) {
                        m_soilWtrSto[*idxIter][0] += deltaWtrMM;
                    }
                    if (deltaNO3 > 0.f && nullptr != m_soilNO3) {
                        m_soilNO3[*idxIter][0] += deltaNO3;
                    }
                    if (deltaNH4 > 0.f && nullptr != m_soilNH4) {
                        m_soilNH4[*idxIter][0] += deltaNH4;
                    }
                    if (deltaMinP > 0.f && nullptr != m_soilSolP) {
                        m_soilSolP[*idxIter][0] += deltaMinP;
                    }
                    if (deltaOrgN > 0.f && nullptr != m_soilStabOrgN) {
                        m_soilStabOrgN[*idxIter][0] += deltaOrgN;
                    }
                    if (deltaOrgP > 0.f && nullptr != m_soilHumOrgP) {
                        m_soilHumOrgP[*idxIter][0] += deltaOrgP;
                    }
                }
            }
        }
    }
    return true;
}
