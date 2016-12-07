#include "Cell.h"

Cell::Cell(void):m_FieldID(-1)
{
}

Cell::~Cell(void)
{
}

bool Cell::IsMyNeighbor(Cell* iCell, int nCols)
{
	int id  = iCell->m_id;
	map<int, bool>::iterator it = m_IsNeighbCellmap.find(id);
	if (it != m_IsNeighbCellmap.end())  // found
	{
		return it->second;
	}
	else
	{
		int i1, i2, j1, j2;
		i1 = id/nCols;
		j1 = id%nCols;
		i2 = m_id/nCols;
		j2 = m_id%nCols;
		int i1i2 = i1-i2;
		int j1j2 = j1-j2;
		if (i1 == i2 && (abs(j1j2)==1))
		{
			setNeighborCellId(id, true);
			iCell->setNeighborCellId(m_id, true);
			return true;
		}
		else if (j1 == j2 && (abs(i1i2)==1))
		{
			setNeighborCellId(id, true);
			iCell->setNeighborCellId(m_id, true);
			return true;
		}
		else if ((abs(i1i2)==1) && (abs(j1j2)==1))
		{
			setNeighborCellId(id, true);
			iCell->setNeighborCellId(m_id, true);
			return true;
		}
		else
		{
			setNeighborCellId(id, false);
			iCell->setNeighborCellId(m_id, false);
			return false;
		}
	}
}
