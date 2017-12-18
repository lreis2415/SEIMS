#ifndef FIELD_PARTITION_FIELD_H
#define FIELD_PARTITION_FIELD_H
// Build by Wu Hui, 2012.4.28
// objective: to build the relationships of the each field, and to aggregate very small upstream fields
//  into their downstream fields. This is controlled by the threshold given by user. 
//
#include "Cell.h"
#include <algorithm>
#include <iterator>
#include <vector>
#include <map>

using namespace std;

class Field {
public:
    Field();

    ~Field();

    void AddInFieldID(int idIn) { m_inFieldIDs.push_back(idIn); }

    vector<int> &GetInFieldIDs() { return m_inFieldIDs; }

    vector<int> &GetCellsIDs() { return m_cellsIds; }

    vector<Cell *> &GetCells() { return m_cells; }

    int GetCellNum() { return (int) m_cells.size(); }

    void AddCellIDintoField(int ID) { m_cellsIds.push_back(ID); }

    void AddCellintoField(Cell *icell) {
        // Cell* should be copied into m_cells, by lj. 12/17/2017
        m_cells.push_back(new Cell(*icell));
        //m_cells.push_back(icell);
    }

    void SetOutFieldID(int idOut) { m_outFieldID = idOut; }

    int GetOutFieldID() { return m_outFieldID; }

    void SetID(int id) { m_id = id; }

    int GetID() { return m_id; }

    void SetDegree(int de) { m_degree = de; }

    int GetDegree() { return m_degree; }

    void SetLanduseCode(int code) { m_landCode = code; }

    int GetLanduseCode() { return m_landCode; }

    //get field's rectangle: the first point in up-left (x1,y1), the second point in down-right (x2,y2)
    void GetRetangleField(int ncols);

    bool IsFieldsNeighbor(Field *f1, int ncols);

    void mergeCells(vector<int> cellid1);

    void mergeFieldtoMe(Field *pfield, int ncols);

public:
    int m_xmin, m_ymin, m_xmax, m_ymax;   //field's rectangle
private:
    int m_outFieldID;
    int m_id;
    int m_degree;
    int m_landCode;
    vector<int> m_inFieldIDs;
    vector<int> m_cellsIds;
    vector<Cell *> m_cells;
};

#endif /* FIELD_PARTITION_FIELD_H */