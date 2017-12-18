#include "CellOrdering.h"
#include "utilities.h"

#include <iostream>
#include <set>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;

const int CellOrdering::m_d1[8] = {0, 1, 1, 1, 0, -1, -1, -1};
const int CellOrdering::m_d2[8] = {1, 1, 0, -1, -1, -1, 0, 1};

int CellOrdering::cfid = 1;
int CellOrdering::FID = 1;


CellOrdering::CellOrdering(IntRaster *rsDir, IntRaster *rsLandu, IntRaster *rsMask,
                           FlowDirectionMethod flowDirMtd, int threshold)
    : m_dir(rsDir), m_mask(rsMask), m_landu(rsLandu), m_threshold(threshold), m_flowDirMtd(flowDirMtd),
      m_FieldNum(0), m_maxDegree(0) {
    m_nRows = m_dir->getRows();
    m_nCols = m_dir->getCols();
    m_size = m_nRows * m_nCols;
    m_cells.resize(m_size, nullptr);
    m_cellwidth = m_dir->getCellWidth();
    //m_layers.resize(10);

    m_validCellsCount = 0;
    for (int i = 0; i < m_nRows; ++i) {
        for (int j = 0; j < m_nCols; ++j) {
            if (!m_mask->isNoData(i, j)) {
                int id = i * m_nCols + j;
                m_cells[id] = new Cell();
                m_validCellsCount += 1;
            }
        }
    }
    if (m_flowDirMtd) {
        /// ArcGIS
        m_dirToIndexMap[1] = 0;
        m_dirToIndexMap[2] = 1;
        m_dirToIndexMap[4] = 2;
        m_dirToIndexMap[8] = 3;
        m_dirToIndexMap[16] = 4;
        m_dirToIndexMap[32] = 5;
        m_dirToIndexMap[64] = 6;
        m_dirToIndexMap[128] = 7;
    } else {
        /// TauDEM
        m_dirToIndexMap[1] = 0;
        m_dirToIndexMap[8] = 1;
        m_dirToIndexMap[7] = 2;
        m_dirToIndexMap[6] = 3;
        m_dirToIndexMap[5] = 4;
        m_dirToIndexMap[4] = 5;
        m_dirToIndexMap[3] = 6;
        m_dirToIndexMap[2] = 7;
    }
}


CellOrdering::~CellOrdering(void) {
    if (!m_cells.empty()) {
        for (auto it = m_cells.begin(); it != m_cells.end();) {
            if (nullptr != *it) {
                delete *it;
                *it = nullptr;
            }
            it = m_cells.erase(it);
        }
        m_cells.clear();
    }
    if (!m_fields.empty()) {
        for (auto it = m_fields.begin(); it != m_fields.end();) {
            if (nullptr != *it) {
                delete *it;
                *it = nullptr;
            }
            it = m_fields.erase(it);
        }
        m_fields.clear();
    }
    if (!m_mapfields.empty()) {
        for (auto it = m_mapfields.begin(); it != m_mapfields.end();) {
            if (nullptr != it->second) {
                delete it->second;
                it->second = nullptr;
            }
            m_mapfields.erase(it++);
        }
    }
    m_mapfields.clear();
    if (!m_mapSameDegreefields.empty()) {
        for (auto it = m_mapSameDegreefields.begin(); it != m_mapSameDegreefields.end();) {
            for (auto it2 = it->second.begin(); it2 != it->second.end();) {
                *it2 = nullptr;
                it2 = it->second.erase(it2);
            }
            it->second.clear();
            m_mapSameDegreefields.erase(it++);
        }
    }
    m_mapSameDegreefields.clear();

    FID = 1;
    cfid = 1;
}

//from down to up
bool CellOrdering::Execute(int iOutlet, int jOutlet) {
    if (iOutlet < 0 || iOutlet >= m_nRows || jOutlet < 0 || jOutlet >= m_nCols) {
        cerr << "Failed to execute the cell ordering.\n"
             << "The outlet location(" << iOutlet << ", " << jOutlet
             << ") is out of the extent of the study area." << endl;
        return false;
    }
    BuildTree();

    int id = iOutlet * m_nCols + jOutlet;
    if (nullptr == m_cells[id]) {
        cerr << "Failed to execute the cell ordering." << endl
             << "The outlet location(" << iOutlet << ", " << jOutlet << ") is null." << endl;
        return false;
    }
    BuildRoutingLayer(id, 0);
    return true;
}


bool CellOrdering::ExcuteFieldsDis(int iOutlet, int jOutlet) {
    cout << "\t\tBuilding tree of landuse with flow in and out information  ..." << endl;
    BuildTree();
    /*--------------------------------------------------------------------------
    Add the landuse (crop) information to the Tree, and set a thread to aggregate
    the small fields (number of cells < thread) into their downstream fields.
    ---------------------------------------------------------------------------*/
    BuildFieldsTree(iOutlet, jOutlet);
    return true;
}

void CellOrdering::BuildTree(void)   // tree of the land use 
{
    int *dir = m_dir->getRasterDataPointer();
    int *LanduCode = m_landu->getRasterDataPointer();
    for (int i = 0; i < m_nRows; ++i) {
        for (int j = 0; j < m_nCols; ++j) {
            if (m_mask->isNoData(i, j)) {
                continue;
            }
            int id = i * m_nCols + j;
            // flow out
            int k = dir[id];
            int outIndex = m_dirToIndexMap[dir[id]];
            int iOut = i + m_d1[outIndex];
            int jOut = j + m_d2[outIndex];
            int idOut = iOut * m_nCols + jOut;
            m_cells[id]->SetID(id);
            // set landuse information to cells
            int landu = LanduCode[id];
            m_cells[id]->SetLanduseCode(landu);

            if (iOut < 0 || iOut >= m_nRows || jOut < 0 || jOut >= m_nCols || nullptr == m_cells[idOut]) {
                m_cells[id]->SetOutCellID(-1);
            } else {
                m_cells[id]->SetOutCellID(idOut);
                m_cells[idOut]->AddInCellID(id);
            }
        }
    }
    cout << "\t\t\tTotally " << m_cells.size() << " cells has been build!" << endl;
}

bool CellOrdering::IsCellsNeighbor(int id1, int id2) {
    int i1, i2, j1, j2;
    i1 = id1 / m_nCols;
    j1 = id1 % m_nCols;
    i2 = id2 / m_nCols;
    j2 = id2 % m_nCols;
    int i1i2 = i1 - i2;
    int j1j2 = j1 - j2;
    if (i1 == i2 && (abs(j1j2) == 1)) {
        return true;
    } else if (j1 == j2 && (abs(i1i2) == 1)) {
        return true;
    } else if ((abs(i1i2) == 1) && (abs(j1j2) == 1)) {
        return true;
    } else {
        return false;
    }
}

bool CellOrdering::IsFieldsNeighbor(int fid1, int fid2) {
    vector<int> &cellsF1 = m_fields[fid1]->GetCellsIDs();
    vector<int> &cellsF2 = m_fields[fid2]->GetCellsIDs();
    size_t i, j;
    for (i = 0; i < cellsF1.size(); i++) {
        int cellID1 = cellsF1[i];
        for (j = 0; j < cellsF2.size(); j++) {
            int cellID2 = cellsF2[j];
            if (IsCellsNeighbor(cellID1, cellID2)) {
                return true;
            }
        }
    }
    if (j == cellsF2.size()) {
        return false;
    } else {
        return true;
    }
}

//from the children of the outlet, start to recursive
void CellOrdering::BuildField(int id, Field *pfield) {
    vector<int> &inCells = m_cells[id]->GetInCellIDs();
    if (inCells.empty()) {
        return;
    }

    for (unsigned int i = 0; i < inCells.size(); ++i) {
        int child = inCells[i];
        int LC = m_cells[child]->GetLanduseCode();
        int OutLC = pfield->GetLanduseCode();
        if (LC == OutLC) {
            pfield->AddCellintoField(m_cells[child]);
            BuildField(child, pfield);
        } else {
            Field *qfield = new Field();
            FID++;
            qfield->SetID(FID);
            qfield->SetOutFieldID(pfield->GetID());     // set relationship of the fields
            pfield->AddInFieldID(FID);                  // set relationship of the fields
            qfield->AddCellintoField(m_cells[child]);
            qfield->SetLanduseCode(LC);
            m_mapfields.insert(make_pair(FID, qfield));
            BuildField(child, qfield);
        }
    }
}

void CellOrdering::mergefieldsofsamefather(Field *f1, Field *f2) {
    //merge f1 to f2 and update children,
    vector<int> &f1infds = f1->GetInFieldIDs();
    if (!f1infds.empty()) {
        for (size_t k = 0; k < f1infds.size(); k++) {
            int chid = f1infds[k];
            m_mapfields[chid]->SetOutFieldID(f2->GetID());    // set f2 to the outfield of f1's child
            f2->AddInFieldID(chid);  //set f1's child to f2's child
        }
    }
    f2->mergeFieldtoMe(f1, m_nCols);
    // delete f1 in its outfield's(father's) infieldids;
    Field *pfield = m_mapfields[f1->GetOutFieldID()];
    vector<int> &inF = pfield->GetInFieldIDs();
    if (!inF.empty()) {
        vector<int>::iterator iter = find(inF.begin(), inF.end(), f1->GetID());
        if (iter != inF.end()) { // if find, delete it's field ID
            inF.erase(iter);
        }
    }
    map<int, Field *>::iterator it = m_mapfields.find(f1->GetID());
    if (it != m_mapfields.end()) {
        delete it->second;
        it->second = nullptr;
        m_mapfields.erase(it);
    }
}

void CellOrdering::mergefieldschild2father(Field *child, Field *father) {
    //merge child to father and update children,
    vector<int> &childinfds = child->GetInFieldIDs();
    if (!childinfds.empty()) {
        for (size_t k = 0; k < childinfds.size(); k++) {
            int chid = childinfds[k];
            m_mapfields[chid]->SetOutFieldID(father->GetID());    // set father to the outfield of child's child
            father->AddInFieldID(chid);  //set child's child to father's child
        }
    }
    // set the landuse code of larger field to smaller field  //added by Wu Hui, 2013.10.17
    if (father->GetCellNum() < child->GetCellNum()) {
        int landu = child->GetLanduseCode();
        father->SetLanduseCode(landu);
    }

    father->mergeFieldtoMe(child, m_nCols);
    // delete child in its outfield's(father's) infieldids;
    vector<int> &inF = father->GetInFieldIDs();
    if (!inF.empty()) {
        vector<int>::iterator iter = find(inF.begin(), inF.end(), child->GetID());
        if (iter != inF.end()) { // if find, delete it's field ID
            inF.erase(iter);
        }
    }
    map<int, Field *>::iterator it = m_mapfields.find(child->GetID());
    if (it != m_mapfields.end()) {
        delete it->second;
        it->second = nullptr;
        m_mapfields.erase(it);
    }
}

void CellOrdering::MergeSameLanduseChildFieldsFromUpDown() {
    vector<int> upperFieldIDs;
    for (map<int, Field *>::iterator it = m_mapfields.begin(); it != m_mapfields.end(); it++) {
        Field *curFld = it->second;
        if (!curFld->GetInFieldIDs().empty()) {
            upperFieldIDs.push_back(it->first);
        }
    }
    vector<int>(upperFieldIDs).swap(upperFieldIDs);
    cout << "\t\t\tThere are " << upperFieldIDs.size() << " uppermost fields." << endl;
    set<int> downFieldIDs;
    set<int> downFieldIDs2;
    for (auto it = upperFieldIDs.begin(); it != upperFieldIDs.end(); it++) {
        /// get the downstream field of the current upper field
        Field *curField = m_mapfields.at(*it);
        int downFieldID = curField->GetOutFieldID();
        downFieldIDs.insert(downFieldID);
    }
    set<int>(downFieldIDs).swap(downFieldIDs);
    set<int>::iterator downID = downFieldIDs.begin();
    //int count = 0;
    while (downFieldIDs.size() > 0 && !(downFieldIDs.size() == 1 && *downID == 1)) {
        for (downID = downFieldIDs.begin(); downID != downFieldIDs.end(); downID++) {
            if (*downID != 1) /// downstream field is not the root field (i.e., outlet)
            {
                /// If FieldID (*it) existed in m_mapfields
                map<int, Field *>::iterator findIDIter = m_mapfields.find(*downID);
                if (findIDIter == m_mapfields.end()) {
                    continue;
                }
                Field *downField = m_mapfields[*downID];
                MergeSameLanduseChildFieldsOneLayer(downField);
                if (downField->GetOutFieldID()) {
                    downFieldIDs2.insert(downField->GetOutFieldID());
                }
            }
        }
        downFieldIDs.clear();
        for (auto curID = downFieldIDs2.begin(); curID != downFieldIDs2.end(); curID++) {
            downFieldIDs.insert(*curID);
        }
        downFieldIDs2.clear();
        downID = downFieldIDs.begin();
        //cout<<"Iterator num: "<<count++<<", downFieldIDs num: "<<downFieldIDs.size()<<", downID: "<<*downID<<endl;
    }
    cout << "\t\t\tThere are " << m_mapfields.size() << " fields left before merge fields from root field" << endl;
    Field *rootfd = m_mapfields[1];
    MergeSameLanduseChildFieldsOneLayer(rootfd);
}

void CellOrdering::MergeSameLanduseChildFieldsOneLayer(Field *pfield) {
    vector<int> &infieldIds = pfield->GetInFieldIDs();
    if (infieldIds.empty()) {
        return;
    }
    int infdSize = infieldIds.size();
    //cout<<"curFieldID: "<<pfield->GetID()<<", InFieldNum: "<<infdSize<<endl;
    if (infdSize < 2) {/// pfield has only one field flow in.
        return;
    }
    /// merge adjacent father fields with the same landuse
    for (int i = 0; i < infdSize - 1; i++) {
        int fieldi = infieldIds[i];
        Field *f1 = m_mapfields[fieldi];
        for (int j = i + 1; j < infdSize; j++) {
            int fieldj = infieldIds[j];
            Field *f2 = m_mapfields[fieldj];
            if (f1->GetLanduseCode() != f2->GetLanduseCode()) {
                continue;
            }
            if (f1->IsFieldsNeighbor(f2, m_nCols)) {
                mergefieldsofsamefather(f1, f2);
                //once merged, it may change the infieldIds vector, will bring some problems.
                // to solve the problems
                infdSize--;
                i--;
                break;
            }
        }
    }
}

void CellOrdering::MergeSameLanduseChildFields(Field *pfield) {
    vector<int> &infieldIds = pfield->GetInFieldIDs();
    if (infieldIds.empty()) {
        return;
    }
    int infdSize = infieldIds.size();
    //cout<<"curFieldID: "<<pfield->GetID()<<", InFieldNum: "<<infdSize<<endl;
    if (infdSize < 2) {
        Field *pdf = m_mapfields[infieldIds[0]]; /// pfield has only one field flow in.
        MergeSameLanduseChildFields(pdf); /// Continue to trace upstream
    }
    /// merge adjacent father fields with the same landuse
    for (int i = 0; i < infdSize - 1; i++) {
        int fieldi = infieldIds[i];
        Field *f1 = m_mapfields[fieldi];
        for (int j = i + 1; j < infdSize; j++) {
            int fieldj = infieldIds[j];
            Field *f2 = m_mapfields[fieldj];
            if (f1->GetLanduseCode() != f2->GetLanduseCode()) {
                continue;
            }
            if (f1->IsFieldsNeighbor(f2, m_nCols)) {
                //once merged, it may change the infieldIds vector, will bring some problems.
                mergefieldsofsamefather(f1, f2);    
                // to solve the problems
                infdSize--;
                i--;
                break;
            }
        }
    }
    vector<int> &updatedInfieldIds = pfield->GetInFieldIDs();
    infdSize = updatedInfieldIds.size();
    for (int i = 0; i < infdSize; i++) {
        Field *pfd = m_mapfields[updatedInfieldIds[i]];
        MergeSameLanduseChildFields(pfd);
    }
}


void CellOrdering::AggregateSmallField(Field *pfield) {
    cout << "\t\t\tAggregate small fields ..." << endl;
    getpostorderfield(pfield);
    cout << "\t\t\t\tTotally " << m_posterorderfieldId.size() << " postorder fields of outlet field" << endl;
    if (m_posterorderfieldId.empty()) {
        cout << "Err happened in get postorder field, at AggregateSmallField, Check it first!\n";
        return;
    }

    for (size_t i = 0; i < m_posterorderfieldId.size(); i++) {
        int id = m_posterorderfieldId[i];
        Field *pfd = m_mapfields[id];
        vector<Cell *> &cells = pfd->GetCells();
        int ncells = cells.size();
        if (ncells < m_threshold) {
            if (id == 1)   // root
            {
                vector<int> &infid = pfd->GetInFieldIDs();
                int numinfid = infid.size();
                int child = numinfid / 2;
                Field *chfd = m_mapfields[infid[child]];
                mergefieldschild2father(chfd, pfd);
                return;
            }

            int outid = pfd->GetOutFieldID();
            Field *outfd = m_mapfields[outid];
            if (nullptr == outfd) {
                cout << "Err happened in AggregateSmallField, please check it!\n";
                return;
            }
            mergefieldschild2father(pfd, outfd);
        }
    }
    cout << "\t\t\t\tTotally " << m_mapfields.size() << " remain" << endl;
}

void CellOrdering::getpostorderfield(Field *pfield) {
    vector<int> &infids = pfield->GetInFieldIDs();
    if (!infids.empty()) {
        for (size_t i = 0; i < infids.size(); i++) {
            Field *qfield = m_mapfields[infids[i]];
            getpostorderfield(qfield);

            int id = qfield->GetID();
            m_posterorderfieldId.push_back(id);
        }
        if (pfield->GetID() == 1) {
            m_posterorderfieldId.push_back(1);
        }
    }
}

void CellOrdering::remergesamelandusefield(Field *pfield)  // merge same landuse fields between father and children
{
    cout << "\t\t\tMerge fields between father and child with same landuse ..." << endl;
    m_posterorderfieldId.clear();
    getpostorderfield(pfield);
    cout << "\t\t\t\tTotally " << m_posterorderfieldId.size() << " postorder fields of outlet field" << endl;
    if (m_posterorderfieldId.size() != m_mapfields.size()) {
        cout << "Err happened at remergesamelandusefield, please check it!\n";
        return;
    }
    for (size_t i = 0; i < m_posterorderfieldId.size(); i++) {
        int id = m_posterorderfieldId[i];
        if (id == 1)  // root
        {
            cout << "\t\t\t\tTotally " << m_mapfields.size() << " remain" << endl;
            return;
        }

        Field *pfd = m_mapfields[id];
        int outid = pfd->GetOutFieldID();
        Field *outfd = m_mapfields[outid];
        if (nullptr == outfd) {
            cout << "Err happened in remergesamelandusefield, please check it!\n";
            return;
        }
        int fatherlanduse = outfd->GetLanduseCode();
        int childlanduse = pfd->GetLanduseCode();
        if (fatherlanduse == childlanduse) {
            mergefieldschild2father(pfd, outfd);
        }
    }
}

void CellOrdering::BuildFieldsTree(int iOutlet, int jOutlet) {
    cout << "\t\tBegin from outlet to trace upstream fields ..." << endl;
    int id = iOutlet * m_nCols + jOutlet;
    m_rootID = 1;

    if (nullptr == m_cells[id]) {
        cerr << "Failed to execute the cell ordering.\n" <<
             "The outlet location(" << iOutlet << ", " << jOutlet << ") is null.\n";
        return;
    }
    Field *pfield = new Field();
    pfield->SetID(1);                /// start from 1
    pfield->SetOutFieldID(0);        /// outfieldID of outlet field is 0
    pfield->AddCellintoField(m_cells[id]);
    pfield->SetLanduseCode(m_cells[id]->GetLanduseCode());
    if (!m_mapfields.insert(make_pair(1, pfield)).second) exit(-1);
    cout << "\t\tFrom the children of the outlet to recursively trace upstream to build fields tree" << endl;
    BuildField(id, pfield);  // rootcellid, rootfield
    cout << "\t\t\tTotally " << m_mapfields.size() << " fields has been generated" << endl;
    cout << "\t\tMerge same field (landuse) with the same flow in ..." << endl;
    Field *rootfd = m_mapfields.at(1);
    MergeSameLanduseChildFieldsFromUpDown();
    cout << "\t\t\tTotally " << m_mapfields.size() << " fields remained after merging the same flow in adjacent fields"
         << endl;
    // aggregate the small field into its downstream field according to a given threshold
    cout << "\t\tAggregate small fields into its downstream (flow out) field according to the threshold: "
         << m_threshold << " ..." << endl;
    if (m_threshold > 0) {
        AggregateSmallField(rootfd);
        remergesamelandusefield(rootfd);
        cout << "\t\t\tMerge child fields again according to same landuse ..." << endl;
        MergeSameLanduseChildFieldsFromUpDown();
        cout << "\t\t\t\tTotally " << m_mapfields.size() << " fields remain" << endl;
    }

}

void CellOrdering::MergeSameFatherSameLanduseField(int id) {
    if (nullptr == m_fields[id]) {
        return;
    }
    vector<int> &inFields = m_fields[id]->GetInFieldIDs();
    int nsize = (int) inFields.size();

    for (int i = 0; i < nsize; i++) {
        int iid = inFields[i];
        /*if(m_fields[iid] == NULL)
            continue;*/
        MergeSameFatherSameLanduseField(iid);
    }
    for (int i = 0; i < nsize - 1; i++) {
        int f1, f2;  //merge f1 to f2
        f1 = inFields[i];
        if (nullptr == m_fields[f1]) {
            continue;
        }
        for (int j = i + 1; j < nsize; j++) {
            f2 = inFields[j];
            if (nullptr == m_fields[f2]) {
                continue;
            }
            if (f1 == f2) {
                cerr << "The ID of the two fields merged can not be same!\n";
                return;
            }

            if (m_fields[f1]->GetLanduseCode() != m_fields[f2]->GetLanduseCode()) {
                continue;
            }
            bool IsNeighbor = IsFieldsNeighbor(f1, f2);
            if (IsNeighbor) {
                vector<int> &inFieldID = m_fields[f1]->GetInFieldIDs();
                if (!inFieldID.empty()) {
                    for (size_t j = 0; j < inFieldID.size(); j++) {
                        int inID = inFieldID[j];
                        if (nullptr != m_fields[inID]) {
                            m_fields[inID]->SetOutFieldID(f2);
                            m_fields[f2]->AddInFieldID(inID);
                        }
                    }
                }
                vector<int> &cellid = m_fields[f1]->GetCellsIDs();
                m_fields[f2]->mergeCells(cellid);
                m_fields[f1] = nullptr;
                // finish f1
                break;
            }
        }
    }

}

bool greatermark(Field *f1, Field *f2) {
    return (f1->GetDegree() > f2->GetDegree());
}

void CellOrdering::ReMergeSameLanduseField(int id, int degree)  // merge father and children fields with same land use
{
    if (nullptr == m_fields[id]) {
        cerr << " ReMergeSameLanduseField(int id, int degree) function: The field cannot be NULL!\n";
        return;
    }

    int landu = m_fields[id]->GetLanduseCode();
    int outid = m_fields[id]->GetOutFieldID();
    int outlandu = m_fields[outid]->GetLanduseCode();

    if (landu == outlandu) {
        vector<int> &inFieldID = m_fields[id]->GetInFieldIDs();
        if (!inFieldID.empty()) {
            for (size_t j = 0; j < inFieldID.size(); j++) {
                int inID = inFieldID[j];
                m_fields[inID]->SetOutFieldID(outid);
                m_fields[outid]->AddInFieldID(inID);
                ReMergeSameLanduseField(inID, degree);
            }
        }
        vector<int> &cellid = m_fields[id]->GetCellsIDs();
        m_fields[outid]->mergeCells(cellid);

        m_fields[id] = nullptr;
    } else {
        degree++;
        m_fields[id]->SetDegree(degree);
        m_maxDegree < degree ? m_maxDegree = degree : m_maxDegree = m_maxDegree;    // maximum degree

        vector<int> &inFieldID = m_fields[id]->GetInFieldIDs();
        if (!inFieldID.empty()) {
            for (size_t j = 0; j < inFieldID.size(); j++) {
                int inID = inFieldID[j];
                if (nullptr == m_fields[inID]) {
                    continue;
                }
                ReMergeSameLanduseField(inID, degree);
            }
        }
    }
}

void CellOrdering::reclassfieldid(Field *pfield, int degree) {
    int id = pfield->GetID();
    m_relassFID[id] = cfid;
    pfield->SetDegree(degree);
    degree++;
    vector<int> &infds = pfield->GetInFieldIDs();
    if (infds.empty()) {
        return;
    }
    for (size_t i = 0; i < infds.size(); i++) {
        Field *qfield = m_mapfields[infds[i]];
        cfid++;
        reclassfieldid(qfield, degree);
    }
}

void CellOrdering::sortReclassedfieldid() {
    for (auto it = m_relassFID.begin(); it != m_relassFID.end(); it++) {
        int oldid = it->first;
        int newid = it->second;
        m_newoldfidmap[newid] = oldid;
    }
}

void CellOrdering::OutputFieldMap(const char *filename) {
    IntRaster output;
    output.Copy(m_mask);
    output.replaceNoData(output.getNoDataValue());
    Field *rootfd = m_mapfields[1];
    int degreeRoot = 1;
    reclassfieldid(rootfd, degreeRoot);
    cout << "\t\tFinally, " << m_mapfields.size() << " fields has been build!" << endl;
    for (auto iter = m_mapfields.begin(); iter != m_mapfields.end(); iter++) {
        if (nullptr == iter->second) {
            cout << "Err happened in OutputFieldMap, check it first!\n";
            return;
        }
        Field *fd = iter->second;
        int FID = fd->GetID();
        int ReFID = m_relassFID[FID];
        vector<Cell *> &cells = fd->GetCells();
        for (size_t j = 0; j < cells.size(); j++) {
            int ID = cells[j]->GetID();
            int ik = ID / m_nCols;
            int jk = ID % m_nCols;
            output.setValue(ik, jk, ReFID);
        }
    }
    output.outputToFile(string(filename));
    //if(StringMatch(GetSuffix(string(filename)), "ASC"))
    //	output.OutputArcAscii(filename);
    //else
    //	output.OutputGeoTiff(filename);
}

void CellOrdering::OutputFieldRelationship(const char *filename) {
    ofstream rasterFile(filename);
    size_t m_FieldNum = m_mapfields.size();
    //write header
    rasterFile << " Relationship of the fields ---- field number:\n " << m_FieldNum << "\n";
    rasterFile << " FID  " << "downstreamFID  " << "Area(ha)  " << "LanduseID  " << "Degree" << "\n";
    int FID, ReFID, outFID, ReoutFID, LANDU, degree;//, outLANDU;
    float Area;

    sortReclassedfieldid();

    map<int, int>::iterator it;
    for (it = m_newoldfidmap.begin(); it != m_newoldfidmap.end(); it++) {
        ReFID = it->first;
        FID = it->second;
        outFID = m_mapfields[FID]->GetOutFieldID();
        ReoutFID = m_relassFID[outFID];
        LANDU = m_mapfields[FID]->GetLanduseCode();
        degree = m_mapfields[FID]->GetDegree();
        vector<Cell *> &cells = m_mapfields[FID]->GetCells();
        int n = (int) cells.size();
        Area = n * m_cellwidth * m_cellwidth / 10000;         // ha, 0.01km2

        rasterFile << " " << ReFID << "\t" << ReoutFID << "\t" << Area << "\t" << LANDU << "\t" << degree << endl;
    }
    rasterFile.close();
}

void CellOrdering::BuildRoutingLayer(int idOutlet, int layerNum) {
    if ((int) m_layers.size() <= layerNum) {
        m_layers.resize(2 * m_layers.size());
    }

    // add current cell to the layerNum layer
    m_layers[layerNum].push_back(idOutlet);

    // recursive to build children nodes
    layerNum++;
    vector<int> &inCells = m_cells[idOutlet]->GetInCellIDs();
    for (unsigned int i = 0; i < inCells.size(); ++i) {
        int row = inCells[i] / m_nCols;
        int col = inCells[i] % m_nCols;
        if (!m_mask->isNoData(row, col)) {
            BuildRoutingLayer(inCells[i], layerNum);
        }
    }
}

