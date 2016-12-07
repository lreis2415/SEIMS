#pragma once
// Build by Wu Hui, 2012.4.28
// objective: to build the relationships of the each field, and to aggregate very small upstream fields
//  into their downstream fields. This is controlled by the threshold given by user. 
//
#include<algorithm>
#include <vector>
#include <map>
#include "Cell.h"
#include "Field.h"

using namespace std;

class fieldTnode
{
public:
	fieldTnode(void);
	~fieldTnode(void);
	// build tree struct
	fieldTnode* m_Tparent;
	vector<fieldTnode*> m_Tchildren;
	int m_degree;
	vector<Field*> m_childFieldsVec;
	Field* m_field;

};