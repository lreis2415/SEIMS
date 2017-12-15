#include "clsSubbasin.h"

using namespace std;

Subbasin::Subbasin(int id) : m_id(id), m_nCells(-1), m_cells(nullptr), m_isRevapChanged(true),
                             m_cellArea(-1.f), m_Area(-1.f),
                             m_slope(-1.f), m_slopeCoefficient(1.f),
                             m_GWMAX(-1.f), m_kg(-1.f), m_dp_co(-1.f), m_base_ex(-1.f), m_QGConvert(-1.f),
                             m_Revap(-1.f), m_GW(-1.f), m_PERCO(-1.f), m_PERDE(-1.f),
                             m_PET(-1.f), m_QG(-1.f), m_RG(-1.f),
                             m_DepressionET(-1.f), m_Infiltration(-1.f), m_Interception(-1.f), m_NetPercolation(-1.f),
                             m_Pcp(-1.f), m_pNet(-1.f), m_R(-1.f), m_RI(-1.f), m_RS(-1.f), m_S_MOI(-1.f),
                             m_TotalET(-1.f),
                             m_soilET(-1.f), m_TMean(-1.f), m_SoilT(-1.f), m_interceptionET(-1.f) {
}

Subbasin::~Subbasin() {
    Release1DArray(m_cells);
}

bool Subbasin::CheckInputSize(int n) {
    if (n <= 0) {
        throw ModelException("Subbasin Class", "CheckInputSize",
                             "Input data for Subbasin is invalid. The size could not be less than zero.");
    }

    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException("Subbasin Class", "CheckInputSize", "All input data should have same size.");
        }
    }
    return true;
}

void Subbasin::setCellList(int nCells, int *cells) {
    CheckInputSize(nCells);
    m_cells = cells;
}

// Note: Since slope is calculated by drop divided by distance in TauDEM,
//		 the average should be calculated after doing atan().
//		 By LJ, 2016-7-27
void Subbasin::setSlope(float *slope) {
    m_slope = 0.f;
    int index = 0;
    for (int i = 0; i < m_nCells; i++) {
        index = m_cells[i];
        //m_slope += slope[index];  // percent
        m_slope += atan(slope[index]); // radian
    }
    m_slope /= m_nCells;
    m_slope = tan(m_slope); // to keep consistent with the slope unit in the whole model
}

//////////////////////////////////////////////////////////////////////////
//////////  clsSubbasins                           ///////////////////////
////////////////////////////////////////////////////////////////////////// 
clsSubbasins::clsSubbasins(MongoGridFS* spatialData, map<string, clsRasterData<float> *> &rsMap,
                           int prefixID): m_nSubbasins(-1) {
    m_subbasinIDs.clear();
    m_subbasinsInfo.clear();

// read subbasin data
    int nCells = -1;
    float *subbasinData = nullptr;
    float cellWidth = NODATA_VALUE;
    ostringstream oss;
    oss << prefixID << "_" << VAR_SUBBSN;
    string subbasinFileName = GetUpper(oss.str());
    oss.str("");
    
    rsMap[subbasinFileName]->getRasterData(&nCells, &subbasinData);

    m_nSubbasins = 0;
// valid cell indexes of each subbasin, key is subbasin ID, value is vector of cell's index
    map<int, vector<int> *>cellListMap;
    for (int i = 0; i < nCells; i++) {
        int subID = (int) subbasinData[i];
        if (cellListMap.find(subID) == cellListMap.end())
            cellListMap[subID] = new vector<int>;
        cellListMap[subID]->push_back(i);
    }
    m_nSubbasins = (int) cellListMap.size();
    for (auto it = cellListMap.begin(); it != cellListMap.end(); it++) {
        // swap for saving memory, using shrink_to_fit() instead.
        vector<int>(*it->second).swap(*it->second);
        // (*it->second).shrink_to_fit();
        int subID = it->first;
        m_subbasinIDs.push_back(subID);
        Subbasin *newSub = new Subbasin(subID);
        int nCellsTmp = (int) it->second->size();
        int *tmp = new int[nCellsTmp];
        for (size_t j = 0; j < nCellsTmp; j++)
            tmp[j] = it->second->at(j);
        newSub->setCellList(nCellsTmp, tmp);
        newSub->setArea(cellWidth * cellWidth * nCellsTmp);
        m_subbasinsInfo[subID] = newSub;
    }
    vector<int>(m_subbasinIDs).swap(m_subbasinIDs);
    // m_subbasinIDs.shrink_to_fit();
    // release cellListMap to save memory
    for (auto it = cellListMap.begin(); it != cellListMap.end();) {
        if (it->second != nullptr) {
            delete it->second;
        }
        cellListMap.erase(it++);
    }
    cellListMap.clear();
}

clsSubbasins *clsSubbasins::Init(MongoGridFS* spatialData, map<string,clsRasterData<float> *> &rsMap, 
                                 int prefixID) {
    ostringstream oss;
    oss << prefixID << "_" << Tag_Mask;
    string maskFileName = GetUpper(oss.str());
    oss.str("");
    oss << prefixID << "_" << VAR_SUBBSN;
    string subbasinFileName = GetUpper(oss.str());
    oss.str("");

    if (rsMap.find(maskFileName) == rsMap.end()) { // if mask not loaded yet
        cout << "MASK data has not been loaded yet!" << endl;
        return nullptr;
    }
    int nCells = -1;
    float *subbasinData = nullptr;
    if (rsMap.find(subbasinFileName) == rsMap.end()) { // if subbasin not loaded yet
        clsRasterData<float> *subbasinRaster = nullptr;
        subbasinRaster = clsRasterData<float>::Init(spatialData, subbasinFileName.c_str(),
                                                    true, rsMap[maskFileName]);
        assert(nullptr != subbasinRaster);
        if (!subbasinRaster->getRasterData(&nCells, &subbasinData)) {
            cout << "Subbasin data loaded failed!" << endl;
            return nullptr;
        }
        rsMap[subbasinFileName] = subbasinRaster;
    }
    else {
        if (!rsMap[subbasinFileName]->getRasterData(&nCells, &subbasinData)) {
            cout << "Subbasin data preloaded is unable to access!" << endl;
            return nullptr;
        }
    }

    return new clsSubbasins(spatialData, rsMap, prefixID);
}

clsSubbasins::~clsSubbasins() {
    StatusMessage("Release subbasin class ...");
    if (!m_subbasinsInfo.empty()) {
        for (auto iter = m_subbasinsInfo.begin(); iter != m_subbasinsInfo.end();) {
            if (iter->second != nullptr) {
                delete iter->second;
                iter->second = nullptr;
            }
            m_subbasinsInfo.erase(iter++);
        }
        m_subbasinsInfo.clear();
    }
}

float clsSubbasins::subbasin2basin(string key) {
    float temp = 0.f;
    int totalCount = 0;
    int nCount;
    Subbasin *sub = nullptr;
    for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); it++) {
        sub = m_subbasinsInfo[*it];
        nCount = sub->getCellCount();
        if (StringMatch(key, VAR_SLOPE)) {
            temp += atan(sub->getSlope()) * nCount;
        } else if (StringMatch(key, VAR_PET)) {
            temp += sub->getPET() * nCount;
        } else if (StringMatch(key, VAR_PERCO)) {
            temp += sub->getPerco() * nCount;
        } else if (StringMatch(key, VAR_REVAP)) {
            temp += sub->getEG() * nCount;
        } else if (StringMatch(key, VAR_PERDE)) {
            temp += sub->getPerde() * nCount;
        } else if (StringMatch(key, VAR_RG)) {
            temp += sub->getRG() * nCount;
        } else if (StringMatch(key, VAR_QG)) {
            temp += sub->getQG();
        } else if (StringMatch(key, VAR_GW_Q)) {
            temp += sub->getGW() * nCount;
        }

        totalCount += nCount;
    }
    if (StringMatch(key, VAR_QG)) {
        return temp;
    }    // basin sum
    if (StringMatch(key, VAR_SLOPE))
        return tan(temp / totalCount);
    return temp / totalCount;  // basin average
}

void clsSubbasins::SetSlopeCoefficient() {
    float basinSlope = subbasin2basin(VAR_SLOPE);
    for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); it++) {
        Subbasin *curSub = m_subbasinsInfo[*it];
        if (basinSlope <= 0.f) {
            curSub->setSlopeCoefofBasin(1.f);
        } else {
            float slpCoef = curSub->getSlope() / basinSlope;
            curSub->setSlopeCoefofBasin(slpCoef);
        }
    }
}
