#include "Field.h"

using namespace std;

Field::Field() : m_xmin(-99), m_xmax(-99), m_ymin(-99), m_ymax(-99),
                 m_id(-1), m_outFieldID(-1), m_degree(-1), m_landCode(-1),
                 m_inFieldIDs(), m_cellsIds(), m_cells() {

}

Field::~Field() {
    if (!m_cells.empty()) {
        for (auto it = m_cells.begin(); it != m_cells.end();) {
            if (nullptr != *it) {
                delete *it;
                *it = nullptr;
            }
            *it = nullptr;
            it = m_cells.erase(it);
        }
        m_cells.clear();
    }
}

void Field::GetRetangleField(int ncols) {
    size_t mysize = m_cells.size();
    int id = m_cells[0]->GetID();
    int x = id % ncols;      // column id
    int y = id / ncols;      // line id
    m_xmin = x;
    m_xmax = x;
    m_ymin = y;
    m_ymax = y;
    for (size_t i = 1; i < mysize; i++) {
        id = m_cells[i]->GetID();
        x = id % ncols;      // column id
        y = id / ncols;      // line id
        if (x < m_xmin) {
            m_xmin = x;
        }
        if (y < m_ymin) {
            m_ymin = y;
        }
        if (x > m_xmax) {
            m_xmax = x;
        }
        if (y > m_ymax) {
            m_ymax = y;
        }
    }
}

bool Field::IsFieldsNeighbor(Field *fd, int ncols) {
    if (m_xmin < 0 || m_ymin < 0 || m_xmax < 0 || m_ymax < 0) {
        GetRetangleField(ncols);
    }
    if (fd->m_xmin < 0 || fd->m_ymin < 0 || fd->m_xmax < 0 || fd->m_ymax < 0) {
        fd->GetRetangleField(ncols);
    }
    int x1 = m_xmin;
    int y1 = m_ymin;
    int x2 = m_xmax;
    int y2 = m_ymax;
    int x3 = fd->m_xmin;
    int y3 = fd->m_ymin;
    int x4 = fd->m_xmax;
    int y4 = fd->m_ymax;
    if (y1 > y4 || x1 > x4 || x2 < x3 || y2 < y3) {
        return false;
    }
    vector<int> xvec, yvec;
    xvec.push_back(x1);
    xvec.push_back(x2);
    xvec.push_back(x3);
    xvec.push_back(x4);
    yvec.push_back(y1);
    yvec.push_back(y2);
    yvec.push_back(y3);
    yvec.push_back(y4);

    //ascending sort
    sort(xvec.begin(), xvec.end());
    sort(yvec.begin(), yvec.end());

    int rangx1 = xvec[1];
    int rangy1 = yvec[1];
    int rangx2 = xvec[2];
    int rangy2 = yvec[2];

    vector<Cell *> &fdCells = fd->GetCells();
    for (size_t i = 0; i < m_cells.size(); i++) {
        int mcid = m_cells[i]->GetID();
        int x = mcid % ncols;      // column id
        int y = mcid / ncols;      // line id
        if (x < rangx1 || x > rangx2 || y < rangy1 || y > rangy2) {
            continue;
        }
        for (size_t j = 0; j < fdCells.size(); j++) {
            int fdcid = fdCells[j]->GetID();
            x = fdcid % ncols;
            y = fdcid / ncols;
            if (x < rangx1 || x > rangx2 || y < rangy1 || y > rangy2) {
                continue;
            }
            if (!m_cells[i]->IsMyNeighbor(fdCells[j], ncols)) {      // revised by Wu Hui, 2013.10.17
                continue;
            } else {
                return true;
            }
        }
    }
    return false;
}

void Field::mergeCells(vector<int> cellid1) {
    vector<int> cellid2 = m_cellsIds;
    size_t sizecell = m_cellsIds.size();
    m_cellsIds.clear();
    m_cellsIds.resize(cellid1.size() + sizecell);
    merge(cellid1.begin(), cellid1.end(), cellid2.begin(), cellid2.end(), m_cellsIds.begin());
}

void Field::mergeFieldtoMe(Field *pfield, int ncols) {
    vector<Cell *> &pcells = pfield->GetCells();
    // Use deep copy instead, revised by lj. 12/18/2017
    //copy(pcells.begin(), pcells.end(), back_inserter(m_cells));
    for (auto it = pcells.begin(); it != pcells.end(); it++) {
        m_cells.push_back(new Cell(**it));
    }
    //reset_xyminmax();
    // revised by Wu Hui, 2013.10.17
    if (m_xmin < 0 || m_ymin < 0 || m_xmax < 0 || m_ymax < 0) {
        GetRetangleField(ncols);
    }
    if (pfield->m_xmin < 0 || pfield->m_ymin < 0 || pfield->m_xmax < 0 || pfield->m_ymax < 0) {
        pfield->GetRetangleField(ncols);
    }

    if (m_xmin > pfield->m_xmin)   // bigger and smaller are selected
    {
        m_xmin = pfield->m_xmin;
    }
    if (m_ymin > pfield->m_ymin) {
        m_ymin = pfield->m_ymin;
    }
    if (m_xmax < pfield->m_xmax) {
        m_xmax = pfield->m_xmax;
    }
    if (m_ymax < pfield->m_ymax) {
        m_ymax = pfield->m_ymax;
    }
}