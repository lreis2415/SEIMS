/*!
 * \file clsPond.h
 * \brief Class to store reach related parameters from Pond table
 *
 * \author Shen fang
 * \version 1.0
 * \date 2017.12
 *
 */
#include "clsPond.h"

#include "utils_array.h"
#include "utils_string.h"
#include "utils_math.h"

#include "text.h"

using namespace utils_array;
using namespace utils_string;
using namespace utils_math;


clsPond::clsPond(const bson_t *&bsonTable)
{
	bson_iter_t iterator;
	if (bson_iter_init_find(&iterator, bsonTable, POND_PADDYID))
		GetNumericFromBsonIterator(&iterator, this->PaddyID);
		
	if (bson_iter_init_find(&iterator, bsonTable, POND_REACHID))
		GetNumericFromBsonIterator(&iterator, this->ReachID);
		
	if (bson_iter_init_find(&iterator, bsonTable, POND_PONDID1))
		GetNumericFromBsonIterator(&iterator, this->PondID1);
		
	if (bson_iter_init_find(&iterator, bsonTable, POND_PONDID2))
		GetNumericFromBsonIterator(&iterator, this->PondID2);
		
	if (bson_iter_init_find(&iterator, bsonTable, POND_PONDID3))
		GetNumericFromBsonIterator(&iterator, this->PondID3);
		
}

clsPond::~clsPond(void)
{
}

clsPonds::clsPonds(mongoc_client_t *conn, string &dbName, string collectionName){
	bson_t *b = bson_new();
	bson_t *child1 = bson_new();
	bson_t *child2 = bson_new();
	BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);  /// query all records
	bson_append_document_end(b, child1);
	BSON_APPEND_DOCUMENT_BEGIN(b, "$orderby", child2); /// and order by pond ID
	BSON_APPEND_INT32(child2, POND_PADDYID, 1);
	bson_append_document_end(b, child2);
	bson_destroy(child1);
	bson_destroy(child2);

	mongoc_cursor_t *cursor;
	const bson_t *bsonTable;
	mongoc_collection_t *collection = NULL;
	bson_error_t *err = NULL;

	collection = mongoc_client_get_collection(conn, dbName.c_str(), collectionName.c_str());
	if (collection == NULL)
		throw ModelException("clsPonds", "ReadAllPondsInfo", "Failed to get collection: " + collectionName + ".\n");
	const bson_t *qCount = bson_new();
	this->m_paddyNum = (int) mongoc_collection_count(collection, MONGOC_QUERY_NONE, qCount, 0, 0, NULL, err);
	if (err != NULL || this->m_paddyNum < 0)
		throw ModelException("clsPonds", "ReadAllPondsInfo",
		"Failed to get document number of collection: " + collectionName + ".\n");
	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);


	while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable))
	{
		clsPond *curPond = new clsPond(bsonTable);
		m_pondsInfo[curPond->GetPaddyID()] = curPond;
		this->m_paddyIDs.push_back(curPond->GetPaddyID());
	}
	vector<int>(m_paddyIDs).swap(m_paddyIDs);

	bson_destroy(b);
	mongoc_collection_destroy(collection);
	mongoc_cursor_destroy(cursor);
}
clsPonds::~clsPonds(void)
{
	if (!m_pondsInfo.empty())
	{
		for (map<int, clsPond *>::iterator iter = m_pondsInfo.begin(); iter != m_pondsInfo.end();)
		{
			if (iter->second != NULL)
				delete iter->second;
			iter = m_pondsInfo.erase(iter);
		}
		m_pondsInfo.clear();
	}
}
