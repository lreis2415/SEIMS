#pragma once

#include <vector>
#include <map>
#include <stdlib.h>

using namespace std;

class Cell
{
public:
	Cell(void);
	~Cell(void);

	void AddInCellID(int idIn)
	{
		m_inCellIDs.push_back(idIn);
	}

	std::vector<int>& GetInCellIDs()
	{
		return m_inCellIDs;
	}

	void SetOutCellID(int idOut)
	{
		m_outCellID = idOut;
	}

	int GetOutCellID()
	{
		return m_outCellID;
	}

	void SetID(int id)
	{
		m_id = id;
	}

	int GetID()
	{
		return m_id;
	}

	void SetLanduseCode(int code)
	{
		m_LanduseCode = code;
	}

	int GetLanduseCode()
	{
		return m_LanduseCode;
	}

	void SetFieldID(int id)
	{
		m_FieldID = id;
	}

	int GetFieldID()
	{
		return m_FieldID;
	}

	void SetDegree(int de)
	{
		m_degree = de;
	}

	int GetDegree()
	{
		return m_degree;
	}

	bool IsMyNeighbor(Cell* iCell, int ncols);

	void setNeighborCellId(int id, bool bl)
	{
		m_IsNeighbCellmap[id] = bl;
	}

private:
	vector<int> m_inCellIDs;
	int m_outCellID;
	int m_id;
	int m_LanduseCode;
	int m_FieldID;
	int m_degree;

	map<int, bool> m_IsNeighbCellmap;    // int: cell id, bool: if neighbour cell id, true, else false

};
