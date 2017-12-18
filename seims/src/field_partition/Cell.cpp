#include "Cell.h"

Cell::Cell() : m_id(-1), m_FieldID(-1), m_outCellID(-1), m_LanduseCode(-1),
               m_degree(-1), m_IsNeighbCellmap(), m_inCellIDs() {
}

Cell::Cell(const Cell &org) {
    m_id = org.m_id;
    m_FieldID = org.m_FieldID;
    m_outCellID = org.m_outCellID;
    m_LanduseCode = org.m_LanduseCode;
    m_degree = org.m_degree;
    map<int, bool> m_IsNeighbCellmap(org.m_IsNeighbCellmap);
    vector<int> m_inCellIDs(org.m_inCellIDs);
}

Cell::~Cell() {
}

bool Cell::IsMyNeighbor(Cell *iCell, int nCols) {
    int id = iCell->m_id;
    map<int, bool>::iterator it = m_IsNeighbCellmap.find(id);
    if (it != m_IsNeighbCellmap.end()) { // found
        return it->second;
    } else {
        int i1, i2, j1, j2;
        i1 = id / nCols;
        j1 = id % nCols;
        i2 = m_id / nCols;
        j2 = m_id % nCols;
        int i1i2 = i1 - i2;
        int j1j2 = j1 - j2;
        if (i1 == i2 && (abs(j1j2) == 1)) {
            setNeighborCellId(id, true);
            iCell->setNeighborCellId(m_id, true);
            return true;
        } else if (j1 == j2 && (abs(i1i2) == 1)) {
            setNeighborCellId(id, true);
            iCell->setNeighborCellId(m_id, true);
            return true;
        } else if ((abs(i1i2) == 1) && (abs(j1j2) == 1)) {
            setNeighborCellId(id, true);
            iCell->setNeighborCellId(m_id, true);
            return true;
        } else {
            setNeighborCellId(id, false);
            iCell->setNeighborCellId(m_id, false);
            return false;
        }
    }
}
