#pragma once
#include "CellOrdering.h"
#include "util.h"
#include <iostream>
#include <set>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;


const int CellOrdering::m_d1[8] = {0,1,1, 1, 0,-1,-1,-1};
const int CellOrdering::m_d2[8] = {1,1,0,-1,-1,-1, 0, 1};

int CellOrdering:: cfid = 1; 
int CellOrdering:: FID = 1;

CellOrdering::CellOrdering(IntRaster *rsDir, IntRaster *rsMask, FlowDirectionMethod flowDirMtd)
	:m_dir(rsDir), m_mask(rsMask), m_flowDirMtd(flowDirMtd)
{
	m_nRows = m_dir->GetNumberOfRows();
	m_nCols = m_dir->GetNumberofColumns();
	m_size = m_nRows * m_nCols;
	m_cells.resize(m_size, NULL);
	m_layers.resize(10);

	m_validCellsCount = 0;
	for (int i = 0; i < m_nRows; ++i)
	{
		for (int j = 0; j < m_nCols; ++j)
		{
			if ( !m_mask->IsNull(i, j) )
			{
				int id = i*m_nCols + j;
				m_cells[id] = new Cell();
				m_validCellsCount += 1;
			}
		}
	}
	if (m_flowDirMtd)
	{
		/// ArcGIS
		m_dirToIndexMap[1] = 0;
		m_dirToIndexMap[2] = 1;
		m_dirToIndexMap[4] = 2;
		m_dirToIndexMap[8] = 3;
		m_dirToIndexMap[16] = 4;
		m_dirToIndexMap[32] = 5;
		m_dirToIndexMap[64] = 6;
		m_dirToIndexMap[128] = 7;
	} 
	else
	{
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

CellOrdering::CellOrdering(IntRaster *rsDir, IntRaster* rsLandu, IntRaster *rsMask, FlowDirectionMethod flowDirMtd, int threshold)
	:m_dir(rsDir), m_mask(rsMask), m_landu(rsLandu), m_threshold(threshold), m_flowDirMtd(flowDirMtd), m_FieldNum(0),m_maxDegree(0)
{
	m_nRows = m_dir->GetNumberOfRows();
	m_nCols = m_dir->GetNumberofColumns();
	m_size = m_nRows * m_nCols;
	m_cells.resize(m_size, NULL);
	m_cellwidth = m_dir->GetXCellSize();
	//m_layers.resize(10);
	
	m_validCellsCount = 0;
	for (int i = 0; i < m_nRows; ++i)
	{
		for (int j = 0; j < m_nCols; ++j)
		{
			if ( !m_mask->IsNull(i, j) )
			{
				int id = i*m_nCols + j;
				m_cells[id] = new Cell();
				m_validCellsCount += 1;
			}
		}
	}
	if (m_flowDirMtd)
	{
		/// ArcGIS
		m_dirToIndexMap[1] = 0;
		m_dirToIndexMap[2] = 1;
		m_dirToIndexMap[4] = 2;
		m_dirToIndexMap[8] = 3;
		m_dirToIndexMap[16] = 4;
		m_dirToIndexMap[32] = 5;
		m_dirToIndexMap[64] = 6;
		m_dirToIndexMap[128] = 7;
	} 
	else
	{
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
	//m_root = new fieldTnode();
}

CellOrdering::CellOrdering(IntRaster *rsDir, IntRaster* rsLandu, Raster<int> *rsStreamLink, IntRaster *rsMask, FlowDirectionMethod flowDirMtd, int threshold)
	:m_dir(rsDir), m_mask(rsMask), m_landu(rsLandu), m_streamlink(rsStreamLink), m_flowDirMtd(flowDirMtd), m_threshold(threshold), m_FieldNum(0),m_maxDegree(0)
{
	m_nRows = m_dir->GetNumberOfRows();
	m_nCols = m_dir->GetNumberofColumns();
	m_size = m_nRows * m_nCols;
	m_cells.resize(m_size, NULL);
	m_cellwidth = m_dir->GetXCellSize();
	//m_layers.resize(10);

	m_validCellsCount = 0;
	for (int i = 0; i < m_nRows; ++i)
	{
		for (int j = 0; j < m_nCols; ++j)
		{
			if ( !m_mask->IsNull(i, j) )
			{
				int id = i*m_nCols + j;
				m_cells[id] = new Cell();
				m_validCellsCount += 1;
			}
		}
	}
	if (m_flowDirMtd)
	{
	/// ArcGIS
		m_dirToIndexMap[1] = 0;
		m_dirToIndexMap[2] = 1;
		m_dirToIndexMap[4] = 2;
		m_dirToIndexMap[8] = 3;
		m_dirToIndexMap[16] = 4;
		m_dirToIndexMap[32] = 5;
		m_dirToIndexMap[64] = 6;
		m_dirToIndexMap[128] = 7;
	} 
	else
	{
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
	//m_root = new fieldTnode();
}

CellOrdering::~CellOrdering(void)
{
	for (int i = 0; i < m_size; ++i)
		delete m_cells[i];
	for(int i = 0; i < m_FieldNum; ++i)
		delete m_fields[i];
	
	//DestoryFieldsTree(m_root);
	FID = 1;
	cfid = 1;
}

//void CellOrdering::DestoryFieldsTree(fieldTnode * proot)
//{
//	if (proot != NULL)
//	{
//		vector<fieldTnode*> children = proot->m_Tchildren;
//		for (size_t i=0; i<children.size(); i++)
//		{
//			DestoryFieldsTree(children[i]);
//		}
//		delete proot; 
//	} 
//	else
//	{
//		return;
//	}
//}

//from down to up
bool CellOrdering::Execute(int iOutlet, int jOutlet)
{
	if (iOutlet < 0 || iOutlet >= m_nRows || jOutlet < 0 || jOutlet >= m_nCols)
	{
		cerr << "Failed to execute the cell ordering.\n" <<
			"The outlet location(" << iOutlet << ", " << jOutlet << ") is out of the extent of the study area.\n";
		return false;
	}
	BuildTree();
	
	int id = iOutlet*m_nCols + jOutlet;
	if(m_cells[id] == NULL)
	{
	cerr << "Failed to execute the cell ordering.\n" <<
	"The outlet location(" << iOutlet << ", " << jOutlet << ") is null.\n";
	return false;
	}
	BuildRoutingLayer(id, 0);
	//BuildRoutingLayer2(id);

	return true;
}

//from up to down
bool CellOrdering::Execute2(int iOutlet, int jOutlet)
{
	if (iOutlet < 0 || iOutlet >= m_nRows || jOutlet < 0 || jOutlet >= m_nCols)
	{
		cerr << "Failed to execute the cell ordering.\n" <<
			"The outlet location(" << iOutlet << ", " << jOutlet << ") is out of the extent of the study area.\n";
		return false;
	}
	BuildTree();
	
	int id = iOutlet*m_nCols + jOutlet;
	if(m_cells[id] == NULL)
	{
		cerr << "Failed to execute the cell ordering.\n" <<
			"The outlet location(" << iOutlet << ", " << jOutlet << ") is null.\n";
		return false;
	}
	// only the following statement is different from the Execute function
	BuildRoutingLayer2(id);

	return true;
}

//bool CellOrdering::ExcuteFieldsDis(int iOutlet, int jOutlet)
//{
//	BuildTree();
//	/*--------------------------------------------------------------------------
//	Add the landuse (crop) information to the Tree, and set a thread to aggregate
//	the small fields (number of cells < thread) into their downstream fields.
//	---------------------------------------------------------------------------*/
//	int id = iOutlet*m_nCols + jOutlet;
//	if(m_cells[id] == NULL)
//	{
//		cerr << "Failed to execute the cell ordering.\n" <<
//			"The outlet location(" << iOutlet << ", " << jOutlet << ") is null.\n";
//		return false;
//	}
//
//	m_cells[id]->SetFieldID(1);
//	vector<int>& inCells = m_cells[id]->GetInCellIDs();
//	for (unsigned int i = 0; i < inCells.size(); ++i)
//	{
//		BuildField(inCells[i], 1);
//	}
//	
//	return true;
//}

bool CellOrdering::ExcuteFieldsDis(int iOutlet, int jOutlet)
{
	cout<<"\t\tBuilding tree of landuse with flow in and out information  ..."<<endl;
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
	int** dir = m_dir->GetData();
	int** LanduCode = m_landu->GetData();
	for (int i = 0; i < m_nRows; ++i)
	{
		for (int j = 0; j < m_nCols; ++j)
		{
			if ( m_mask->IsNull(i, j) )
				continue;
			
			// flow out 
			int k = dir[i][j];
			int outIndex = m_dirToIndexMap[dir[i][j]];
			int iOut = i + m_d1[outIndex];
			int jOut = j + m_d2[outIndex];
			int idOut = iOut*m_nCols + jOut;
			int id = i*m_nCols + j;
			m_cells[id]->SetID(id);
			// set landuse information to cells
			int landu = LanduCode[i][j];
			m_cells[id]->SetLanduseCode(landu);

			if (iOut < 0 || iOut >= m_nRows || jOut < 0 || jOut >= m_nCols || m_cells[idOut] == NULL)
			{
				m_cells[id]->SetOutCellID(-1);
			}
			else
			{
				m_cells[id]->SetOutCellID(idOut);
				m_cells[idOut]->AddInCellID(id);
			}
		}
	}
	cout<<"\t\t\tTotally "<<m_cells.size()<<" cells has been build!"<<endl;
}

bool CellOrdering::IsCellsNeighbor(int id1, int id2)
{
	int i1, i2, j1, j2;
	i1 = id1/m_nCols;
	j1 = id1%m_nCols;
	i2 = id2/m_nCols;
	j2 = id2%m_nCols;
	int i1i2 = i1-i2;
	int j1j2 = j1-j2;
	if (i1 == i2 && (abs(j1j2)==1))
	{
		return true;
	}
	else if (j1 == j2 && (abs(i1i2)==1))
	{
		return true;
	}
	else if ((abs(i1i2)==1) && (abs(j1j2)==1))
	{
		return true;
	}
	else
		return false;
}

bool CellOrdering::IsFieldsNeighbor(int fid1, int fid2)
{
	vector<int>& cellsF1 = m_fields[fid1]->GetCellsIDs();
	vector<int>& cellsF2 = m_fields[fid2]->GetCellsIDs();
	size_t i, j;
	for (i=0; i<cellsF1.size(); i++)
	{
		int cellID1 = cellsF1[i];
		for (j=0; j<cellsF2.size(); j++)
		{
			int cellID2 = cellsF2[j];
			if(IsCellsNeighbor(cellID1, cellID2))
				return true;
		}
	}
	if(j==cellsF2.size())
		return false;
	else
		return true;
}

//from the children of the outlet, start to recursive
void CellOrdering::BuildField(int id, Field* pfield)
{
	vector<int>& inCells = m_cells[id]->GetInCellIDs();
	if (inCells.empty())
		return;
	
	for (unsigned int i = 0; i < inCells.size(); ++i)
	{
		int child = inCells[i];
		int LC = m_cells[child]->GetLanduseCode();
		int OutLC = pfield->GetLanduseCode();   
		if (LC == OutLC)
		{
			pfield->AddCellintoField(m_cells[child]);
			BuildField(child, pfield);
		}
		else
		{
			Field* qfield = new Field();
			FID++; 
			qfield->SetID(FID);
			qfield->SetOutFieldID(pfield->GetID());     // set relationship of the fields
			pfield->AddInFieldID(FID);                  // set relationship of the fields
			qfield->AddCellintoField(m_cells[child]);
			qfield->SetLanduseCode(LC);
			m_mapfields[FID] = qfield;
			BuildField(child, qfield);
		}
	}
}

void CellOrdering::mergefieldsofsamefather(Field* f1, Field* f2)
{
	//merge f1 to f2 and update children, 
	vector<int>& f1infds = f1->GetInFieldIDs();
	if (!f1infds.empty())
	{
		for (size_t k=0; k<f1infds.size(); k++)
		{
			int chid = f1infds[k];
			m_mapfields[chid]->SetOutFieldID(f2->GetID());    // set f2 to the outfield of f1's child
			f2->AddInFieldID(chid);  //set f1's child to f2's child
		}
	}
	f2->mergeFieldtoMe(f1, m_nCols);
	// delete f1 in its outfield's(father's) infieldids;   
    Field* pfield = m_mapfields[f1->GetOutFieldID()];
	vector<int>& inF = pfield->GetInFieldIDs();
	if(!inF.empty())
	{
		vector<int>::iterator iter = find(inF.begin(), inF.end(), f1->GetID());     
		if (iter != inF.end()) // if find, delete it's field ID
			inF.erase(iter);
	}
	map<int, Field*>::iterator it = m_mapfields.find(f1->GetID());
	if (it != m_mapfields.end())
	{
		m_mapfields.erase(it);
	}
	delete f1;
}

void CellOrdering::mergefieldschild2father(Field* child, Field* father)
{
	//merge child to father and update children, 
	vector<int>& childinfds = child->GetInFieldIDs();
	if (!childinfds.empty())
	{
		for (size_t k=0; k<childinfds.size(); k++)
		{
			int chid = childinfds[k];
			m_mapfields[chid]->SetOutFieldID(father->GetID());    // set father to the outfield of child's child
			father->AddInFieldID(chid);  //set child's child to father's child
		}
	}
	// set the landuse code of larger field to smaller field  //added by Wu Hui, 2013.10.17
	if (father->GetCellNum() < child->GetCellNum())
	{
		int landu = child->GetLanduseCode();
		father->SetLanduseCode(landu);
	}
	
	father->mergeFieldtoMe(child, m_nCols);
	// delete child in its outfield's(father's) infieldids;   
	vector<int>& inF = father->GetInFieldIDs();
	if(!inF.empty())
	{
		vector<int>::iterator iter = find(inF.begin(), inF.end(), child->GetID());     
		if (iter != inF.end()) // if find, delete it's field ID
			inF.erase(iter);
	}
	map<int, Field*>::iterator it = m_mapfields.find(child->GetID());
	if (it != m_mapfields.end())
	{
		m_mapfields.erase(it);
	}
	//m_mapfields[child->GetID()] = NULL;
	delete child;
}
void CellOrdering::MergeSameLanduseChildFieldsFromUpDown()
{
	vector<int> upperFieldIDs;
	for(map<int, Field*>::iterator it = m_mapfields.begin(); it != m_mapfields.end(); it++)
	{
		Field* curFld = it->second;
		if(!curFld->GetInFieldIDs().size())
			upperFieldIDs.push_back(it->first);
	}
	vector<int>(upperFieldIDs).swap(upperFieldIDs);
	cout<<"\t\t\tThere are "<<upperFieldIDs.size()<<" uppermost fields."<<endl;
	set<int> downFieldIDs;
	set<int> downFieldIDs2;
	for (vector<int>::iterator it = upperFieldIDs.begin(); it != upperFieldIDs.end(); it++)
	{
		/// get the downstream field of the current upper field
		Field* curField = m_mapfields[*it];
		int downFieldID = curField->GetOutFieldID();
		downFieldIDs.insert(downFieldID);
	}
	set<int>(downFieldIDs).swap(downFieldIDs);
	set<int>::iterator downID = downFieldIDs.begin();
	//int count = 0;
	while(downFieldIDs.size() > 0 && !(downFieldIDs.size()==1 && *downID == 1))
	{
		for (downID = downFieldIDs.begin(); downID != downFieldIDs.end(); downID++)
		{
			//int count2=0;
			if (*downID != 1) /// downstream field is not the root field (i.e., outlet)
			{
				/// If FieldID (*it) existed in m_mapfields
				map<int, Field*>::iterator findIDIter = m_mapfields.find(*downID);
				if(findIDIter == m_mapfields.end())
					continue;
				Field* downField = m_mapfields[*downID];
				MergeSameLanduseChildFieldsOneLayer(downField);
				if(downField->GetOutFieldID())
					downFieldIDs2.insert(downField->GetOutFieldID());
				//cout<<count2++<<endl;
			}
		}
		downFieldIDs.clear();
		for(set<int>::iterator curID = downFieldIDs2.begin(); curID != downFieldIDs2.end(); curID++)
			downFieldIDs.insert(*curID);
		downFieldIDs2.clear();
		//cout<<count++<<", "<<downFieldIDs.size()<<endl;
	}
	cout<<"\t\t\tThere are "<<m_mapfields.size()<<" fields left before merge fields from root field"<<endl;
	Field* rootfd = m_mapfields[1];
	MergeSameLanduseChildFieldsOneLayer(rootfd);
}
void CellOrdering:: MergeSameLanduseChildFieldsOneLayer(Field* pfield)
{
	vector<int>& infieldIds = pfield->GetInFieldIDs();
	if(infieldIds.empty())
		return;
	int infdSize = infieldIds.size();
	//cout<<"curFieldID: "<<pfield->GetID()<<", InFieldNum: "<<infdSize<<endl;
	if (infdSize < 2)/// pfield has only one field flow in.
		return;
	/// merge adjacent father fields with the same landuse 
	for (int i = 0; i < infdSize-1; i++)
	{
		int fieldi = infieldIds[i];
		Field* f1 = m_mapfields[fieldi];
		for (int j=i+1; j < infdSize; j++)
		{
			int fieldj = infieldIds[j];		
			Field* f2 = m_mapfields[fieldj];
			if(f1->GetLanduseCode()!=f2->GetLanduseCode())
				continue;
			if (f1->IsFieldsNeighbor(f2, m_nCols))
			{
				mergefieldsofsamefather(f1, f2);    //once merged, it may change the infieldIds vector, will bring some problems.
				// to solve the problems
				infdSize--;
				i--;
				break;
			}	
		}
	}
}
void CellOrdering:: MergeSameLanduseChildFields(Field* pfield)
{
	vector<int>& infieldIds = pfield->GetInFieldIDs();
	if(infieldIds.empty())
		return;
	int infdSize = infieldIds.size();
	//cout<<"curFieldID: "<<pfield->GetID()<<", InFieldNum: "<<infdSize<<endl;
	if (infdSize < 2)
	{
		Field* pdf = m_mapfields[infieldIds[0]]; /// pfield has only one field flow in.
		MergeSameLanduseChildFields(pdf); /// Continue to trace upstream
	}
	/// merge adjacent father fields with the same landuse 
	for (int i = 0; i < infdSize-1; i++)
	{
		int fieldi = infieldIds[i];
		Field* f1 = m_mapfields[fieldi];
		for (int j=i+1; j < infdSize; j++)
		{
			int fieldj = infieldIds[j];		
			Field* f2 = m_mapfields[fieldj];
			if(f1->GetLanduseCode()!=f2->GetLanduseCode())
				continue;
			if (f1->IsFieldsNeighbor(f2, m_nCols))
			{
				mergefieldsofsamefather(f1, f2);    //once merged, it may change the infieldIds vector, will bring some problems.
				// to solve the problems
				infdSize--;
				i--;
				break;
			}	
		}
	}
	vector<int>& updatedInfieldIds = pfield->GetInFieldIDs();
	infdSize = updatedInfieldIds.size();
	for (int i=0; i<infdSize; i++)
	{
		Field* pfd = m_mapfields[updatedInfieldIds[i]];
		MergeSameLanduseChildFields(pfd);
	}
}

//void CellOrdering::AggregateSmallField(Field* pfield)
//{
//	vector<int>& chfids = pfield->GetInFieldIDs();
//	if (!chfids.empty())   
//	{
//		for (size_t i = 0; i<chfids.size(); i++)
//		{
//			Field* qfield = m_mapfields[chfids[i]];
//			if(qfield == NULL)
//			{
//				cout<<"Err happened in AggregateSmallField, please check it!\n";
//				return;
//			}
//			AggregateSmallField(qfield);
//		}
//		int id = pfield->GetID();
//		if (id == m_rootID)
//		{
//			cout<<"Aggregate Small Field is Ok!\n";
//			return;
//		}
//		vector<Cell*>& mycells = pfield->GetCells();
//		int celln = mycells.size();	
//		if (celln <= m_threshold)      //merge this field to its father field
//		{
//			int outid = pfield->GetOutFieldID();
//			Field* outfd = m_mapfields[outid];
//			if(outfd == NULL)
//			{
//				cout<<"Err happened in AggregateSmallField, please check it!\n";
//				return;
//			}
//			mergefieldschild2father(pfield, outfd);
//		}
//	}
//	else   //leaf node
//	{
//		vector<Cell*>& mycells = pfield->GetCells();
//		int celln = mycells.size();	
//		if (celln <= m_threshold)      //merge this field to its father field
//		{	
//			int outid = pfield->GetOutFieldID();
//			Field* outfd = m_mapfields[outid];
//			if(outfd == NULL)
//			{
//				cout<<"Err happened in AggregateSmallField, please check it!\n";
//				return;
//			}
//			mergefieldschild2father(pfield, outfd);
//		}
//	}
//}

void CellOrdering::AggregateSmallField(Field* pfield)
{
	cout<<"\t\t\tAggregate small fields ..."<<endl;
	getpostorderfield(pfield);
	cout<<"\t\t\t\tTotally "<<m_posterorderfieldId.size()<<" postorder fields of outlet field"<<endl;
	if (m_posterorderfieldId.empty())
	{
		cout<<"Err happened in get postorder field, at AggregateSmallField, Check it first!\n";
		return;
	}
	
	for (size_t i=0; i<m_posterorderfieldId.size(); i++)
	{
		int id = m_posterorderfieldId[i];
		Field* pfd = m_mapfields[id];
		vector<Cell*>& cells = pfd->GetCells();
		int ncells = cells.size();
		if (ncells < m_threshold)
		{
			if (id == 1)   // root
			{
				vector<int>& infid = pfd->GetInFieldIDs();
				int numinfid = infid.size();
				int child = numinfid / 2;
				Field* chfd = m_mapfields[infid[child]];
				mergefieldschild2father(chfd, pfd);
				return;
			}
			
			int outid = pfd->GetOutFieldID();
			Field* outfd = m_mapfields[outid];
			if(outfd == NULL)
			{
				cout<<"Err happened in AggregateSmallField, please check it!\n";
				return;
			}
			mergefieldschild2father(pfd, outfd);
		}
	}
	cout<<"\t\t\t\tTotally "<<m_mapfields.size()<<" remain"<<endl;
}

void CellOrdering::getpostorderfield(Field* pfield)
{
	vector<int>& infids = pfield->GetInFieldIDs();
	if (!infids.empty())
	{
		for (size_t i=0; i<infids.size(); i++)
		{
			Field* qfield = m_mapfields[infids[i]];
			getpostorderfield(qfield);

			int id = qfield->GetID();
			m_posterorderfieldId.push_back(id);
		}
		if (pfield->GetID() == 1)
		{
			m_posterorderfieldId.push_back(1);
		}
	}
}

void CellOrdering::remergesamelandusefield(Field* pfield)  // merge same landuse fields between father and children
{
	cout<<"\t\t\tMerge fields between father and child with same landuse ..."<<endl;
	m_posterorderfieldId.clear();
	getpostorderfield(pfield);
	cout<<"\t\t\t\tTotally "<<m_posterorderfieldId.size()<<" postorder fields of outlet field"<<endl;
	if (m_posterorderfieldId.size()!=m_mapfields.size())
	{
		cout<<"Err happened at remergesamelandusefield, please check it!\n";
		return;
	}
	for (size_t i=0; i<m_posterorderfieldId.size(); i++)
	{
		int id = m_posterorderfieldId[i];
		if (id == 1)  // root
		{
			cout<<"\t\t\t\tTotally "<<m_mapfields.size()<<" remain"<<endl;
			return;
		}
		
		Field* pfd = m_mapfields[id];
		int outid = pfd->GetOutFieldID();
		Field* outfd = m_mapfields[outid];
		if(outfd == NULL)
		{
			cout<<"Err happened in remergesamelandusefield, please check it!\n";
			return;
		}
		int fatherlanduse = outfd->GetLanduseCode();
		int childlanduse = pfd->GetLanduseCode();
		if (fatherlanduse == childlanduse)
		{
			mergefieldschild2father(pfd, outfd);
		}
	}
}

//void CellOrdering::remergesamelandusefield(Field* pfield)  // merge same landuse fields between father and children
//{
//	vector<int>& infieldIds = pfield->GetInFieldIDs();
//	if (infieldIds.empty())
//		return;
//
//	for (size_t i = 0; i < infieldIds.size(); i++)
//	{
//		int fatherlanduse = pfield->GetLanduseCode();
//		Field* child = m_mapfields[infieldIds[i]];
//		int childlanduse = child->GetLanduseCode();
//		if(fatherlanduse != childlanduse)
//			continue;
//		else  //merge child to father field
//			mergefieldschild2father(child, pfield);
//	}
//
//	vector<int>& updatedinfieldIds = pfield->GetInFieldIDs();
//	if (updatedinfieldIds.empty())
//		return;
//	for (size_t i=0; i<updatedinfieldIds.size(); i++)
//	{
//		Field* qfield = m_mapfields[updatedinfieldIds[i]];
//		remergesamelandusefield(qfield);
//	}
//	
//}

void CellOrdering::BuildFieldsTree(int iOutlet, int jOutlet)
{
	cout<<"\t\tBegin from outlet to trace upstream fields ..."<<endl;
	int id = iOutlet*m_nCols + jOutlet;
	m_rootID = 1;

	if(m_cells[id] == NULL)
	{
		cerr << "Failed to execute the cell ordering.\n" <<
			"The outlet location(" << iOutlet << ", " << jOutlet << ") is null.\n";
		return ;
	}
	Field* pfield = new Field();
	pfield->SetID(1);                /// start from 1
	pfield->SetOutFieldID(0);        /// outfieldID of outlet field is 0
	pfield->AddCellintoField(m_cells[id]);
	pfield->SetLanduseCode(m_cells[id]->GetLanduseCode());
	m_mapfields[1] = pfield;
	cout<<"\t\tFrom the children of the outlet to recursively trace upstream to build fields tree"<<endl;
	BuildField(id, pfield);  // rootcellid, rootfield
	cout<<"\t\t\tTotally "<<m_mapfields.size()<< " fields has been generated"<<endl;
	cout<<"\t\tMerge same field (landuse) with the same flow in ..."<<endl;
	Field* rootfd = m_mapfields[1];
	MergeSameLanduseChildFieldsFromUpDown();
	//MergeSameLanduseChildFields(rootfd);
	cout<<"\t\t\tTotally "<<m_mapfields.size()<< " fields remained after merging the same flow in adjacent fields"<<endl;
	// aggregate the small field into its downstream field according to a given threshold
	cout<<"\t\tAggregate small fields into its downstream (flow out) field according to the threshold: "<<m_threshold<<" ..."<<endl;
	if (m_threshold > 0)
	{
		AggregateSmallField(rootfd);
		remergesamelandusefield(rootfd);
		cout<<"\t\t\tMerge child fields again according to same landuse ..."<<endl;
		//MergeSameLanduseChildFields(rootfd);  Deprecated by LJ
		MergeSameLanduseChildFieldsFromUpDown();
		cout<<"\t\t\t\tTotally "<<m_mapfields.size()<<" fields remain"<<endl;
	}

}

void CellOrdering::MergeSameFatherSameLanduseField(int id)
{
	if(m_fields[id] == NULL)
		return;
	vector<int> &inFields = m_fields[id]->GetInFieldIDs();
	int nsize = (int) inFields.size();

	for (int i=0; i<nsize; i++)
	{
		int iid = inFields[i];
		/*if(m_fields[iid] == NULL)
			continue;*/
		MergeSameFatherSameLanduseField(iid);
	}                                               //ºóÐò±éÀú
	for (int i=0; i<nsize-1; i++)
	{
		int f1, f2;                   //merge f1 to f2
		f1 = inFields[i];
		if(m_fields[f1] == NULL)
			continue;
		for(int j=i+1; j<nsize; j++)
		{
			f2 = inFields[j];
			if(m_fields[f2] == NULL)
				continue;
			if(f1==f2)
			{
				cerr <<"The ID of the two fields merged can not be same!\n";
				return;
			}

			if (m_fields[f1]->GetLanduseCode()!=m_fields[f2]->GetLanduseCode())
				continue;
			bool IsNeighbor = IsFieldsNeighbor(f1, f2); 
			if (IsNeighbor)
			{
				vector<int>& inFieldID = m_fields[f1]->GetInFieldIDs();   
				if (!inFieldID.empty())
				{
					for (size_t j=0; j<inFieldID.size(); j++ )
					{
						int inID = inFieldID[j];
						if (m_fields[inID] != NULL)
						{
							m_fields[inID]->SetOutFieldID(f2);     
							m_fields[f2]->AddInFieldID(inID);    
						}
					}
				} 
				vector<int>& cellid = m_fields[f1]->GetCellsIDs();    
				m_fields[f2]->mergeCells(cellid);                    

				//// delete f1 in its outfield's infieldids;     
				//int outID = m_fields[f1]->GetOutFieldID();      
				//vector<int>& inF = m_fields[outID]->GetInFieldIDs();
				//if(!inF.empty())
				//{
				//	vector<int>::iterator iter = find(inF.begin(), inF.end(), f1);     
				//	if (iter != inF.end()) // if find, delete it's field ID
				//		inF.erase(iter);
				//}
				m_fields[f1] = NULL;      
				// finish f1
				break;
			}
		}
	}

}

bool greatermark(Field* f1, Field* f2)
{
	return (f1->GetDegree() > f2->GetDegree());
}

void CellOrdering::MergeSameDegreeLanduseFields()
{
	for (int i=m_maxDegree-1; i>=0; i--)   // from the top of upstream fields to the outlet field
	{
		int num = m_sameDegreeFID[i].size();
		if(num <= 1)
			continue;

		for (int j=0; j<num-1; j++)
		{
			int f1, f2;
			f1 = m_sameDegreeFID[i][j];
			/*if(f1==5)
				int oo = 0;*/
			if(m_fields[f1] == NULL)    
				continue;

			for (int k = j+1; k<num; k++)
			{
				f2 = m_sameDegreeFID[i][k];
				if(m_fields[f2] == NULL)
					continue;

				if(f1==f2)
				{
					cerr <<"The ID of the two fields merged can not be same!\n";
					return;
				}

				if (m_fields[f1]->GetLanduseCode()!=m_fields[f2]->GetLanduseCode())
					continue;
				/*if((f1==102 && f2 == 105)|| (f1==105 && f2 == 102))
					int ooo = 0;*/
				bool IsNeighbor = IsFieldsNeighbor(f1, f2);  // if two fields are neighbors, f2 is merged into f1  -- something wrong
				                                             // revision: if two fields are neighbors, f1 is merged into f2 -- to ensure
				                                             // every field (even added by other fields) being checked
				if (IsNeighbor)
				{
					vector<int>& inFieldID = m_fields[f1]->GetInFieldIDs();   //f2
					if (!inFieldID.empty())
					{
						for (size_t j=0; j<inFieldID.size(); j++ )
						{
							int inID = inFieldID[j];
							m_fields[inID]->SetOutFieldID(f2);     //f1
							m_fields[f2]->AddInFieldID(inID);     //f1
						}
					} 
					vector<int>& cellid = m_fields[f1]->GetCellsIDs();    //f2
					m_fields[f2]->mergeCells(cellid);                    //f1

					// delete f1 in its outfield's infieldids;     //f2
					int outID = m_fields[f1]->GetOutFieldID();      //f2
					vector<int>& inF = m_fields[outID]->GetInFieldIDs();
					if(!inF.empty())
					{
						vector<int>::iterator iter = find(inF.begin(), inF.end(), f1);      //f2
						if (iter != inF.end()) // if find, delete it's field ID
							inF.erase(iter);
					}
					m_fields[f1] = NULL;      //f2
					// finish f1
					break;
				}
			}
		}
	}
}

//void CellOrdering::AggregateSmallField()
//{
	//int smallFNumber = 0;
	//vector<Field*> smallFields;
	//smallFields.resize(20, NULL);
	//for (int i = 1; i < m_FieldNum+1; ++i)
	//{
	//	if(m_fields[i] == NULL)
	//		continue;

	//	vector<int>& cellsinField = m_fields[i]->GetCellsIDs();
	//	if ((int)cellsinField.size() <= m_threshold)
	//	{ 
	//		smallFields[smallFNumber] = m_fields[i];   //  == map<smallFNumber, Field*>, smallFNuber: 0,1,2,3...

	//		smallFNumber++;
	//		if(smallFields.size() == smallFNumber)
	//			smallFields.resize(2*smallFNumber, NULL);
	//	}
	//}

	//for(vector<Field*>::iterator it=smallFields.begin(); it!=smallFields.end(); )
	//{
	//	if(* it == NULL)
	//	{
	//		it = smallFields.erase(it);
	//	}
	//	else
	//	{
	//		++it;
	//	}
	//}

	//sort(smallFields.begin(), smallFields.end(), greatermark);  // descending by degree of the tree
	//// aggregating 
	////    method: each upstream field whose cells number is less than threshold is merged into its downstream field.
	//for (int i=0; i<smallFNumber; i++)
	//{
	//	int ID = smallFields[i]->GetID();
	//	int outID = m_fields[ID]->GetOutFieldID();

	//	vector<int>& inFieldID = m_fields[ID]->GetInFieldIDs();

	//	if (outID == 0) // if the field is outlet field
	//	{
	//		// merge one of the outlet field(root id) children to the outlet field
	//		int inf0 = inFieldID[0];
	//		for (size_t j=1;j<inFieldID.size(); j++)
	//		{
	//			int inID = inFieldID[j];
	//			m_fields[inID]->SetOutFieldID(inf0); 
	//			m_fields[inf0]->AddInFieldID(inID);
	//		}
	//		m_fields[inf0]->SetOutFieldID(0);
	//		vector<int>& cellid = m_fields[ID]->GetCellsIDs();
	//		m_fields[inf0]->mergeCells(cellid);        
	//		m_fields[ID] = NULL;
	//		m_rootID = inf0;
	//		 
	//	/*	//merge the outlet field(root id) to one of its children
	//		int oinf = inFieldID[0];
	//		vector<int>& oinFieldID = m_fields[oinf]->GetInFieldIDs();
	//		if (!inFieldID.empty())
	//		{
	//			for(size_t j=0; j<oinFieldID.size(); j++)
	//			{
	//				int inID = oinFieldID[j];
	//				m_fields[inID]->SetOutFieldID(ID); 
	//				m_fields[ID]->AddInFieldID(inID);
	//			}
	//		}
	//		vector<int>& cellid = m_fields[oinf]->GetCellsIDs();
	//		m_fields[ID]->mergeCells(cellid);        
	//		m_fields[oinf] = NULL;*/
	//	}
	//	else
	//	{
	//		if (!inFieldID.empty())
	//		{
	//			for (size_t j=0; j<inFieldID.size(); j++ )
	//			{
	//				int inID = inFieldID[j];
	//				m_fields[inID]->SetOutFieldID(outID); 
	//				m_fields[outID]->AddInFieldID(inID);
	//			}
	//		} 
	//		vector<int>& cellid = m_fields[ID]->GetCellsIDs();
	//		m_fields[outID]->mergeCells(cellid);                   

	//		// delete ith cell's FID in its outfield's infieldids;
	//		vector<int>& inF = m_fields[outID]->GetInFieldIDs();
	//		if(!inF.empty())
	//		{
	//			vector<int>::iterator iter = find(inF.begin(), inF.end(), ID);
	//			if (iter != inF.end()) // if find, delete it's field ID
	//				inF.erase(iter);
	//		}

	//		m_fields[ID] = NULL;
	//	}
	//}
//}

void CellOrdering::ReMergeSameLanduseField(int id, int degree)  // merge father and children fields with same land use
{
	if(m_fields[id] == NULL)
	{
		cerr << " ReMergeSameLanduseField(int id, int degree) function: The field cannot be NULL!\n";
		return;
	}

	int landu = m_fields[id]->GetLanduseCode();
	int outid = m_fields[id]->GetOutFieldID();
	int outlandu = m_fields[outid]->GetLanduseCode();

	if (landu == outlandu)
	{
		vector<int>& inFieldID = m_fields[id]->GetInFieldIDs();
		if (!inFieldID.empty())
		{
			for (size_t j=0; j<inFieldID.size(); j++ )
			{
				int inID = inFieldID[j];
				m_fields[inID]->SetOutFieldID(outid); 
				m_fields[outid]->AddInFieldID(inID);
				ReMergeSameLanduseField(inID, degree);
			}
		} 
		vector<int>& cellid = m_fields[id]->GetCellsIDs();
		m_fields[outid]->mergeCells(cellid);                   

		//// delete ith cell's FID in its outfield's infieldids;
		//vector<int>& inF = m_fields[outid]->GetInFieldIDs();
		//if(!inF.empty())
		//{
		//	vector<int>::iterator iter = find(inF.begin(), inF.end(), id);
		//	if (iter != inF.end()) // if find, delete it's field ID
		//		inF.erase(iter);
		//}

		m_fields[id] = NULL;
	}
	else
	{
		degree++;
		m_fields[id]->SetDegree(degree);
		m_maxDegree < degree? m_maxDegree=degree: m_maxDegree = m_maxDegree;    // maximum degree

		vector<int>& inFieldID = m_fields[id]->GetInFieldIDs();
		if (!inFieldID.empty())
		{
			for (size_t j=0; j<inFieldID.size(); j++ )
			{
				int inID = inFieldID[j];
				if(m_fields[inID] == NULL)
					continue;
				ReMergeSameLanduseField(inID, degree);
			}
		}
	}
}

//void CellOrdering::AggregateSmallField()
//{
//	int smallFNumber = 0;
//	vector<Field*> smallFields;
//	smallFields.resize(20, NULL);
//	for (int i = 1; i < m_FieldNum+1; ++i)
//	{
//		vector<int>& cellsinField = m_fields[i]->GetCellsIDs();
//		if ((int)cellsinField.size() <= m_threshold)
//		{ 
//			smallFields[smallFNumber] = m_fields[i];   //  == map<smallFNumber, Field*>, smallFNuber: 0,1,2,3...
//
//			smallFNumber++;
//			if(smallFields.size() == smallFNumber)
//				smallFields.resize(2*smallFNumber, NULL);
//		}
//	
//	}
//	for(vector<Field*>::iterator it=smallFields.begin(); it!=smallFields.end(); )
//	{
//		if(* it == NULL)
//		{
//			it = smallFields.erase(it);
//		}
//		else
//		{
//			++it;
//		}
//	}
//
//	sort(smallFields.begin(), smallFields.end(), greatermark);  // descending by degree of the tree
//	// aggregating 
//	//    method: each upstream field whose cells number is less than threshold is merged into its downstream field.
//	for (int i=0; i<smallFNumber; i++)
//	{
//		int ID = smallFields[i]->GetID();
//		int outID = m_fields[ID]->GetOutFieldID();
//		if (outID == 0) // if the field is outlet field
//			continue;
//		
//		vector<int>& inFieldID = m_fields[ID]->GetInFieldIDs();
//		if (!inFieldID.empty())
//		{
//			for (size_t j=0; j<inFieldID.size(); j++ )
//			{
//				int inID = inFieldID[j];
//				m_fields[inID]->SetOutFieldID(outID); 
//				m_fields[outID]->AddInFieldID(inID);
//			}
//		} 
//		vector<int>& cellid = m_fields[ID]->GetCellsIDs();
//		for (unsigned int j=0; j<cellid.size(); j++)
//		{
//			int idc = cellid[j];
//			m_cells[idc]->SetFieldID(outID);
//		}
//		// delete ith cell's FID in its outfield's infieldids;
//		vector<int>& inF = m_fields[outID]->GetInFieldIDs();
//		if(!inF.empty())
//		{
//			vector<int>::iterator iter = find(inF.begin(), inF.end(), ID);
//			if (iter != inF.end()) // if find, delete it's field ID
//				inF.erase(iter);
//		}
//		/*vector<Field*>::iterator fit = find(m_fields.begin(), m_fields.end(), m_fields[ID]);
//		if(fit != m_fields.end())
//			m_fields.erase(fit);*/
//		//// delete the this small field in m_fields 
//		//for(vector<Field*>::iterator fit=m_fields.begin(); fit!=m_fields.end(); )
//		//{
//		//	if(* fit == m_fields[ID])
//		//	{
//		//		fit = m_fields.erase(fit);
//		//	}
//		//	else
//		//	{
//		//		++fit;
//		//	}
//		//}
//		m_fields[ID] = NULL;
//	}
//	for (size_t i=1; i<m_FieldNum+1; i++)
//	{
//		if(m_fields[i]!=NULL)
//			cout<< "ID: "<<m_fields[i]->GetID()<<"    outID: "<<m_fields[i]->GetOutFieldID()<<endl;
//
//	}
//}

//void CellOrdering::BuildField()
//{
//bool done = false;
//vector<int> flag;
//flag.resize(m_fields.size(), 0);
//while (!done)
//{
//	for (size_t i = 0; i < m_fields.size(); ++i)
//	{
//		if (m_fields[i] != NULL)
//		{
//			vector<int>& inFields = m_fields[i]->GetInFieldIDs();
//			if (inFields.empty())
//			{
//				int outID = m_fields[i]->GetOutFieldID();
//				vector<int>& cellsinField = m_fields[i]->GetCellsIDs();
//				if ((int)cellsinField.size() <= m_threshold)
//				{ 
//					//m_fields[ID]->SetID(outID);
//					for (unsigned int j=0; j<cellsinField.size(); j++)
//					{
//						m_cells[cellsinField[j]]->SetFieldID(outID);
//					}
//					m_fields[i] = NULL;
//				}
//			}
//		}
//	}
//}
//	int cellsize = (int) m_cells.size();
//	if (cellsize <= 0)
//	{
//		cerr<< "The Number of cells cannot be less than 0.\n"; 
//		exit(-1);
//	}
//	int fieldcount = 0;
//	int fieldID = 1;
//	m_fields[fieldcount] = new Field();
//	m_fields[fieldcount]->SetID(fieldcount);
//	for(int i=0; i<m_validCellsCount; i++)
//	{
//		Cell &eachcell = m_cells[i];
//		eachcell.SetFieldID(fieldID);???
//			int Lu = eachcell.GetLanduseCode();
//		int outCellid = eachcell.GetOutCellID();
//		if (outCellid != -1)
//		{
//			Cell &outcell = m_cells[outCellid];
//			int outLu = outcell.GetLanduseCode();
//			if (Lu != outLu)
//			{
//				fieldID++;
//				outcell.SetFieldID(fieldID);
//
//				fieldcount++;
//				if ((int)m_fields.size() <= fieldcount)
//				{
//					m_fields.resize(2*m_fields.size());
//				}
//				m_fields[fieldcount] = new Field();
//				int infieldID = fieldcount-1;
//				m_fields[infieldID]->SetOutFieldID(fieldcount);
//				m_fields[fieldcount]->AddInFieldID(infieldID);
//			}
//			else
//			{
//
//				outcell.SetFieldID(fieldID);
//
//				m_fields[fieldcount]->AddCellIDintoField(i)
//			}
//
//		}
//
//
//	}


//}
//
//}
void CellOrdering::reclassfieldid(Field* pfield, int degree)
{
	int id = pfield->GetID();
	m_relassFID[id] = cfid;
	pfield->SetDegree(degree);
	degree++;
	vector<int>& infds = pfield->GetInFieldIDs();
	if(infds.empty())
		return;
	for (size_t i=0; i<infds.size(); i++)
	{
		Field* qfield = m_mapfields[infds[i]];
		cfid++;
		reclassfieldid(qfield, degree);
	}
}

void CellOrdering::sortReclassedfieldid()
{
	map<int, int>::iterator it;
	for (it=m_relassFID.begin(); it!=m_relassFID.end(); it++)
	{
		int oldid = it->first;
		int newid = it->second;
		m_newoldfidmap[newid] = oldid;
	}
}

void CellOrdering::OutputFieldMap(const char* filename)
{
	IntRaster output;
	output.Copy(*m_mask);
	output.SetDefaultValues(output.GetNoDataValue());
	Field* rootfd = m_mapfields[1];
	int degreeRoot = 1;
	reclassfieldid(rootfd, degreeRoot);
	cout<<"\t\tFinally, "<<m_mapfields.size()<<" fields has been build!"<<endl;
	map<int, Field*>::iterator iter;
	for (iter = m_mapfields.begin(); iter!=m_mapfields.end(); iter++)
	{
		if(iter->second == NULL)
		{
			cout<<"Err happened in OutputFieldMap, check it first!\n";
			return;
		}
		Field* fd = iter->second;
		int FID = fd->GetID();
		int ReFID = m_relassFID[FID];
		vector<Cell*>& cells = fd->GetCells();
		for(size_t j=0; j<cells.size(); j++)
		{
			int ID = cells[j]->GetID();
			int ik = ID / m_nCols;
			int jk = ID % m_nCols;
			output.SetValue(ik, jk, ReFID);
		}
	}
	//for (int i = 0; i < m_nRows; i++)
	//{
	//	for (int j = 0; j < m_nCols; j++)
	//	{
	//		cout<<output.At(i,j)<<",";
	//	}
	//	cout<<endl;
	//}
	if(StringMatch(GetSuffix(string(filename)), "ASC"))
		output.OutputArcAscii(filename);
	else
		output.OutputGeoTiff(filename);
}

void CellOrdering::reclassFieldID()
{
	for(vector<Field*>::iterator it=m_fields.begin(); it!=m_fields.end(); )
	{
		if(* it == NULL)
		{
			it = m_fields.erase(it);
		}
		else
		{
			++it;
		}
	}

	m_FieldNum = m_fields.size();
	for (size_t i=0; i<m_fields.size(); i++)
	{
		int Fid = m_fields[i]->GetID();
		m_relassFID[Fid] = i+1; //Fid;
	}
	
}

void CellOrdering::OutputFieldRelationship(const char* filename)
{
	ofstream rasterFile(filename);
	int m_FieldNum = m_mapfields.size();
	//write header
	rasterFile << " Relationship of the fields ---- field number:\n " << m_FieldNum << "\n";
	rasterFile << " FID  "<<"downstreamFID  "<<"Area(ha)  "<<"LanduseID  "<<"Degree"<<"\n";
	int FID, ReFID, outFID, ReoutFID, LANDU, degree;//, outLANDU;
	float Area;

	sortReclassedfieldid();

	map<int, int>::iterator it;
	for (it=m_newoldfidmap.begin(); it!=m_newoldfidmap.end(); it++)
	{
		ReFID = it->first;
		FID = it->second;
		outFID = m_mapfields[FID]->GetOutFieldID();
		ReoutFID = m_relassFID[outFID];
		LANDU = m_mapfields[FID]->GetLanduseCode();
		degree = m_mapfields[FID]->GetDegree();
		vector<Cell*>& cells = m_mapfields[FID]->GetCells();
		int n = cells.size();
		Area = n * m_cellwidth * m_cellwidth/10000;         // ha, 0.01km2

		rasterFile <<" "<<ReFID<<"\t"<<ReoutFID<<"\t"<<Area<<"\t"<<LANDU<<"\t"<<degree<<endl;
	}
	rasterFile.close();
}

void CellOrdering::BuildRoutingLayer(int idOutlet, int layerNum)
{
	if ((int)m_layers.size() <= layerNum)
	{
		m_layers.resize(2*m_layers.size());
	}

	// add current cell to the layerNum layer
	m_layers[layerNum].push_back(idOutlet);
	
	// recursive to build children nodes
	layerNum++;
	vector<int>& inCells = m_cells[idOutlet]->GetInCellIDs();
	for (unsigned int i = 0; i < inCells.size(); ++i)
	{
		int row = inCells[i] / m_nCols;
		int col = inCells[i] % m_nCols;
		if (!m_mask->IsNull(row, col))
			BuildRoutingLayer(inCells[i], layerNum);
	}
}

/// build routing layers from most up cells
void CellOrdering::BuildRoutingLayer2(int idOutlet)
{
	if (m_cells.empty())
	{
		cerr << "The tree structure must be built before the routing layers calculation using function BuildTree." << endl;
		return;
	}

	m_layers.clear();
	int counter = 0;
	bool done = false;
	vector<int> flag;
	flag.resize(m_cells.size(), 0);
	while (!done)
	{
		m_layers.push_back(vector<int>());
		for (size_t i = 0; i < m_cells.size(); ++i)
		{
			if (m_cells[i] != NULL)
			{
				vector<int>& inCells = m_cells[i]->GetInCellIDs();
				if (inCells.empty())
				{
					if (i == idOutlet || (flag[i] == 0 && m_cells[i]->GetOutCellID() > 0))
						m_layers[counter].push_back(i);
				}
			}
		}

		// delete this cell from the down stream cell inputs
		for (size_t j = 0; j < m_layers[counter].size(); ++j)
		{
			int id = m_layers[counter][j];
			int outId = m_cells[id]->GetOutCellID();
			if (id != idOutlet)
			{
				vector<int>& ins = m_cells[outId]->GetInCellIDs();
				vector<int>::iterator iter = find(ins.begin(), ins.end(), id);
				if (iter != ins.end()) 
					ins.erase(iter);
				flag[id] = 1;
				done = false;
			}
			else 
				done = true;
		}

		++counter;
	}
}

void CellOrdering::OutRoutingLayer(const char* filename)
{
	IntRaster output;
	output.Copy(*m_mask);
	output.SetDefaultValues(output.GetNoDataValue());
	
	int nn = 0;
	for (unsigned int i = 0; i < m_layers.size(); ++i)
	{
		int layerSize = m_layers[i].size();
		for (int j = 0; j < layerSize; ++j)
		{
			int ik = m_layers[i][j] / m_nCols;
			int jk = m_layers[i][j] % m_nCols;
			output.SetValue(ik, jk, i+1);
			nn++;
		}
	}
	//cout << "number of valid cells: " << nn << endl;
	output.OutputGeoTiff(filename);
	///output.OutputArcAscii(filename);
}

void CellOrdering::CalCompressedIndex(void)
{
	m_compressedIndex.clear();
	m_compressedIndex.resize(m_size, -1);
	int counter = 0;
	for (int i = 0; i < m_nRows; ++i)
	{
		for (int j = 0; j < m_nCols; ++j)
		{
			if ( m_mask->IsNull(i, j) )
				continue;
			int id = i*m_nCols + j;
			m_compressedIndex[id] = counter;
			counter++;
		}
	}
}

int CellOrdering::WriteStringToMongoDB(gridfs *gfs, int id, const char* type, int number, string s)
{
	bson *p = (bson*)malloc(sizeof(bson));
	bson_init(p);
	bson_append_int(p, "SUBBASIN", id );
	bson_append_string(p, "TYPE", type);

	ostringstream oss;
	oss << id << "_" << type;
	string remoteFilename = oss.str();

	bson_append_string(p, "ID", remoteFilename.c_str());
	bson_append_string(p, "DESCRIPTION", type);
	bson_append_double(p, "NUMBER", number);
	bson_finish(p);

	gridfile gfile[1];
	const char* pStr = s.c_str();
	int n = s.length() + 1;
	int index = 0;
	gridfile_writer_init(gfile, gfs, remoteFilename.c_str(), type);

	while(index < n)
	{
		int dataLen = 1024;
		if (n - index < dataLen)
			dataLen = n - index;
		gridfile_write_buffer(gfile, pStr + index, dataLen);
		index += dataLen;
	}
	
	gridfile_set_metadata(gfile, p);
	int flag = gridfile_writer_done(gfile);
	gridfile_destroy(gfile);

	bson_destroy(p);
	free(p);

	return flag;
}

void CellOrdering::OutputLayersToMongoDB(int id, gridfs* gfs)
{
	if (m_compressedIndex.empty())
		CalCompressedIndex();

	ostringstream oss;
	int layerCount = 0;
	for (int i = int(m_layers.size()-1); i >= 0; --i)
	{
		if (!m_layers[i].empty())
			layerCount++;
	}
	oss << "layers: " << layerCount << endl;

	for (int i = int(m_layers.size()-1); i >= 0; --i)
	{
		if (m_layers[i].empty())
			continue;

		int layerSize = m_layers[i].size();
		oss << layerSize << "\t";
		for (int j = 0; j < layerSize; ++j)
		{
			oss << m_compressedIndex[m_layers[i][j]] << "\t";
		}

		oss << "\n";
	}
	
	WriteStringToMongoDB(gfs, id, "ROUTING_LAYERS_DOWN_UP", layerCount, oss.str());
}

void CellOrdering::OutputLayersToMongoDB2(int id, gridfs* gfs)
{
	if (m_compressedIndex.empty())
		CalCompressedIndex();

	ostringstream oss;
	int layerCount = 0;
	for (int i = int(m_layers.size()-1); i >= 0; --i)
	{
		if (!m_layers[i].empty())
			layerCount++;
	}
	oss << "layers: " << layerCount << endl;

	for (size_t i = 0; i < m_layers.size(); ++i)
	{
		if (m_layers[i].empty())
			continue;

		int layerSize = m_layers[i].size();
		oss << layerSize << "\t";
		for (int j = 0; j < layerSize; ++j)
		{
			oss << m_compressedIndex[m_layers[i][j]] << "\t";
		}

		oss << "\n";
	}

	WriteStringToMongoDB(gfs, id, "ROUTING_LAYERS_UP_DOWN", layerCount, oss.str());
}

void CellOrdering::OutputCompressedLayer(const char* filename)
{
	if (m_compressedIndex.empty())
		CalCompressedIndex();
	
	ofstream ofs(filename);
	
	int layerCount = 0;
	for (int i = int(m_layers.size()-1); i >= 0; --i)
	{
		if (!m_layers[i].empty())
			layerCount++;
	}
	ofs << "layers: " << layerCount << endl;

	for (int i = int(m_layers.size()-1); i >= 0; --i)
	{
		if (m_layers[i].empty())
			continue;

		int layerSize = m_layers[i].size();
		ofs << layerSize << "\t";
		for (int j = 0; j < layerSize; ++j)
		{
			ofs << m_compressedIndex[m_layers[i][j]] << "\t";
		}

		ofs << "\n";
	}
	ofs.close();
}

void CellOrdering::OutputCompressedLayer2(const char* filename)
{
	if (m_compressedIndex.empty())
		CalCompressedIndex();

	ofstream ofs(filename);

	int layerCount = 0;
	for (int i = int(m_layers.size()-1); i >= 0; --i)
	{
		if (!m_layers[i].empty())
			layerCount++;
	}
	ofs << "layers: " << layerCount << endl;

	for (size_t i = 0; i < m_layers.size(); ++i)
	{
		if (m_layers[i].empty())
			continue;

		int layerSize = m_layers[i].size();
		ofs << layerSize << "\t";
		for (int j = 0; j < layerSize; ++j)
		{
			ofs << m_compressedIndex[m_layers[i][j]] << "\t";
		}

		ofs << "\n";
	}
	ofs.close();
}

void CellOrdering::OutputCompressedFlowIn(const char* filename)
{
	if (m_compressedIndex.empty())
		CalCompressedIndex();

	ofstream ofs(filename);
	ofs << "Cells: " << m_validCellsCount << endl;
	for (int i = 0; i < m_nRows; ++i)
	{
		for (int j = 0; j < m_nCols; ++j)
		{
			if ( m_mask->IsNull(i, j) )
				continue;
			int id = i*m_nCols + j;

			vector<int>& inCells = m_cells[id]->GetInCellIDs();
			ofs << inCells.size() << "\t";
			for (size_t k = 0; k < inCells.size(); ++k)
				ofs << m_compressedIndex[inCells[k]] << "\t";
			ofs << "\n";
		}
	}
	ofs.close();
}


void CellOrdering::OutputCompressedFlowOut(const char* filename)
{
	if (m_compressedIndex.empty())
		CalCompressedIndex();

	ofstream ofs(filename);
	ofs << "Cells: " << m_validCellsCount << endl;
	for (int i = 0; i < m_nRows; ++i)
	{
		for (int j = 0; j < m_nCols; ++j)
		{
			if ( m_mask->IsNull(i, j) )
				continue;
			int id = i*m_nCols + j;
			int outId = m_cells[id]->GetOutCellID();

			int dirValue = m_dir->At(i,j);
			if (outId == -1)
				ofs << -1 << "\n";
			else
				ofs << m_compressedIndex[outId] << "\n";
		}
	}
	ofs.close();
}

void CellOrdering::OutputFlowOutToMongoDB(int id, gridfs* gfs)
{
	if (m_compressedIndex.empty())
		CalCompressedIndex();

	ostringstream oss;
	oss << "Cells: " << m_validCellsCount << endl;
	for (int i = 0; i < m_nRows; ++i)
	{
		for (int j = 0; j < m_nCols; ++j)
		{
			if ( m_mask->IsNull(i, j) )
				continue;
			int id = i*m_nCols + j;
			int outId = m_cells[id]->GetOutCellID();

			int dirValue = m_dir->At(i,j);
			if (outId == -1)
				oss << -1 << "\n";
			else
				oss << m_compressedIndex[outId] << "\n";
		}
	}
	
	WriteStringToMongoDB(gfs, id, "FLOWOUT_INDEX", m_validCellsCount, oss.str());
}

void CellOrdering::OutputFlowInToMongoDB(int id, gridfs* gfs)
{
	if (m_compressedIndex.empty())
		CalCompressedIndex();

	ostringstream oss;
	oss << "Cells: " << m_validCellsCount << endl;
	for (int i = 0; i < m_nRows; ++i)
	{
		for (int j = 0; j < m_nCols; ++j)
		{
			if ( m_mask->IsNull(i, j) )
				continue;
			int id = i*m_nCols + j;

			vector<int>& inCells = m_cells[id]->GetInCellIDs();
			oss << inCells.size() << "\t";
			for (size_t k = 0; k < inCells.size(); ++k)
				oss << m_compressedIndex[inCells[k]] << "\t";
			oss << "\n";
		}
	}
	WriteStringToMongoDB(gfs, id, "FLOWIN_INDEX", m_validCellsCount, oss.str());
}