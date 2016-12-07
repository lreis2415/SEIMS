#pragma once
// Build by Wu Hui, 2012.4.28
// objective: to build the relationships of the each field, and to aggregate very small upstream fields
//  into their downstream fields. This is controlled by the threshold given by user. 
//
#include<algorithm>
#include <iterator>
#include <vector>
#include <map>
#include "Cell.h"

using namespace std;

class Field
{
public:
	Field(void);
	~Field(void);

	void AddInFieldID(int idIn)
	{
		m_inFieldIDs.push_back(idIn);
	}

	vector<int>& GetInFieldIDs()
	{
		return m_inFieldIDs;
	}

	vector<int>& GetCellsIDs()
	{
		return m_cellsIds;
	}

	vector<Cell*>& GetCells()
	{
		return m_cells;
	}

	int GetCellNum()
	{
		return (int) m_cells.size();
	}

	void AddCellIDintoField(int ID)
	{
		m_cellsIds.push_back(ID);
	}

	void AddCellintoField(Cell* icell)
	{
		m_cells.push_back(icell);
	}

	void SetOutFieldID(int idOut)
	{
		m_outFieldID = idOut;
	}

	int GetOutFieldID()
	{
		return m_outFieldID;
	}

	void SetID(int id)
	{
		m_id = id;
	}

	int GetID()
	{
		return m_id;
	}

	void SetDegree(int de)
	{
		m_degree = de;
	}

	int GetDegree()
	{
		return m_degree;
	}

	void SetLanduseCode(int code)
	{
		m_landCode = code;
	}

	int GetLanduseCode()
	{
		return m_landCode;
	}

	//get field's rectangle: the first point in up-left (x1,y1), the second point in down-right (x2,y2)
	void GetRetangleField(int ncols);

	bool IsFieldsNeighbor(Field* f1, int ncols);

	void mergeCells(vector<int> cellid1)
	{
		vector<int> cellid2 = m_cellsIds;
		int sizecell = m_cellsIds.size();
		m_cellsIds.clear();
		m_cellsIds.resize(cellid1.size() + sizecell);
		merge(cellid1.begin(),cellid1.end(),cellid2.begin(),cellid2.end(),m_cellsIds.begin());
	};

	void mergeFieldtoMe(Field* pfield, int ncols)
	{
		vector<Cell*>& pcells = pfield->GetCells();
		copy(pcells.begin(), pcells.end(), back_inserter(m_cells));
		//reset_xyminmax();  
		// revised by Wu Hui, 2013.10.17
		if (m_xmin < 0 || m_ymin < 0 || m_xmax <0 || m_ymax < 0)
		{
			GetRetangleField(ncols);
		}
		if (pfield->m_xmin < 0 || pfield->m_ymin < 0 || pfield->m_xmax <0 || pfield->m_ymax < 0)
		{
			pfield->GetRetangleField(ncols);
		}

		if (m_xmin > pfield->m_xmin)   // bigger and smaller are selected
		{
			m_xmin = pfield->m_xmin;
		}
		if (m_ymin > pfield->m_ymin)
		{
			m_ymin = pfield->m_ymin;
		}
		if (m_xmax < pfield->m_xmax)
		{
			m_xmax = pfield->m_xmax;
		}
		if (m_ymax < pfield->m_ymax)
		{
			m_ymax = pfield->m_ymax;
		}
	}

	void reset_xyminmax()
	{
		m_xmin = -99;
		m_ymin = -99;
		m_xmax = -99;
		m_ymax = -99;
	}
	/*void mergeCells(std::vector<int> cellid1)   // original version: merge cells into the downstream field. This might be problematic...
	{
	std::vector<int> cellid2 = m_cellsIds;
	int sizecell = m_cellsIds.size();
	m_cellsIds.clear();
	m_cellsIds.resize(cellid1.size() + sizecell);
	merge(cellid1.begin(),cellid1.end(),cellid2.begin(),cellid2.end(),m_cellsIds.begin());
	}*/

	int m_xmin, m_ymin, m_xmax, m_ymax;   //field's rectangle
private:
	vector<int> m_inFieldIDs;
	int m_outFieldID;
	int m_id;
	int m_degree;
	int m_landCode;

	vector<int> m_cellsIds;

	//-------
	vector<Cell*> m_cells;

	//map<int, bool> m_IsNeighbFieldmap;    // int: field id, bool: if neighbor field id, true, else false
};