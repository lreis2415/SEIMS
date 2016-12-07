/*!
 * \brief Plant management operations factory
 * \author Liang-Jun Zhu
 * \date June 2016
 */
#include "BMPPlantMgtFactory.h"
#include "utils.h"

using namespace MainBMP;

BMPPlantMgtFactory::BMPPlantMgtFactory(int scenarioId, int bmpId, int subScenario, int bmpType, int bmpPriority,
                                       string distribution, string collection, string location)
        : BMPFactory(scenarioId, bmpId, subScenario, bmpType, bmpPriority, distribution, collection, location)
{
        if (StringMatch(location, "ALL"))
        {
                m_location.clear();
        }
        else
                m_location = utils::SplitStringForInt(location, ',');
}

BMPPlantMgtFactory::~BMPPlantMgtFactory()
{
        map<int, PlantManagementOperation *>::iterator it;
        for (it = m_bmpPlantOps.begin(); it != m_bmpPlantOps.end(); )
        {
                if (it->second != NULL)
                        delete it->second;
                it = m_bmpPlantOps.erase(it);
        }
        m_bmpPlantOps.clear();
}

void BMPPlantMgtFactory::loadBMP(mongoc_client_t *conn, string &bmpDBName)
{
        bson_t *b = bson_new();
        bson_t *child1 = bson_new(), *child2 = bson_new();
        BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
        BSON_APPEND_INT32(child1, FLD_SCENARIO_SUB, m_subScenarioId);
        bson_append_document_end(b, child1);
        BSON_APPEND_DOCUMENT_BEGIN(b, "$orderby", child2);
        BSON_APPEND_INT32(child2, BMP_FLD_SEQUENCE, 1);
        bson_append_document_end(b, child2);
        bson_destroy(child1);
        bson_destroy(child2);

        mongoc_cursor_t *cursor;
        const bson_t *bsonTable;
        mongoc_collection_t *collection = NULL;

        collection = mongoc_client_get_collection(conn, bmpDBName.c_str(), m_bmpCollection.c_str());
        if (collection == NULL)
                throw ModelException("BMPs Scenario", "Read Plant Management Operations",
                                     "Failed to get collection: " + m_bmpCollection + ".\n");
        cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);
        bson_iter_t itertor;
        int paramNum = 10;
        m_parameters = new float[paramNum];
        int count = 1; /// Use count to counting sequence number, in case of discontinuous of SEQUENCE in database.
        while (mongoc_cursor_next(cursor, &bsonTable))
        {
                //int seqNo;
                int mgtCode = -1, year = -9999, month = -9999, day = -9999;
                float husc = 0.f;
                bool usebaseHU = true;
                if (bson_iter_init_find(&itertor, bsonTable, BMP_FLD_NAME))
                        m_name = GetStringFromBSONITER(&itertor);
                if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_LUCC))
                        m_luccID = GetIntFromBSONITER(&itertor);
                if(m_luccID < 0) m_luccID = 1; // AGRL	Agricultural Land-Generic
                if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_MGTOP))
                        mgtCode = GetIntFromBSONITER(&itertor);
                //if (bson_iter_init_find(&itertor,bsonTable,BMP_FLD_SEQUENCE))
                //	seqNo = GetIntFromBSONITER(&itertor);
                if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_YEAR))
                        year = GetIntFromBSONITER(&itertor);
                if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_MONTH))
                        month = GetIntFromBSONITER(&itertor);
                if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_DAY))
                        day = GetIntFromBSONITER(&itertor);
                if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_BASEHU))
                        usebaseHU = GetBoolFromBSONITER(&itertor);
                if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_HUSC))
                        husc = GetFloatFromBSONITER(&itertor);
                for (int i = 0; i < paramNum; i++)
                {
                        ostringstream oss;
                        oss << BMP_PLTOP_FLD_MGT_PRE << (i + 1);
                        if (bson_iter_init_find(&itertor, bsonTable, oss.str().c_str()))
                                m_parameters[i] = GetFloatFromBSONITER(&itertor);
                }
                int uniqueMgtCode = count * 1000 + mgtCode;
                m_bmpSequence.push_back(uniqueMgtCode);
                switch (mgtCode)
                {
                case BMP_PLTOP_Plant:
                        m_bmpPlantOps[uniqueMgtCode] = new PlantOperation(mgtCode, usebaseHU, husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_Irrigation:
                        m_bmpPlantOps[uniqueMgtCode] = new IrrigationOperation(mgtCode,usebaseHU, husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_Fertilizer:
                        m_bmpPlantOps[uniqueMgtCode] = new FertilizerOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_Pesticide:
                        m_bmpPlantOps[uniqueMgtCode] = new PesticideOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_HarvestKill:
                        m_bmpPlantOps[uniqueMgtCode] = new HarvestKillOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_Tillage:
                        m_bmpPlantOps[uniqueMgtCode] = new TillageOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_Harvest:
                        m_bmpPlantOps[uniqueMgtCode] = new HarvestOnlyOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_Kill:
                        m_bmpPlantOps[uniqueMgtCode] = new KillOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_Grazing:
                        m_bmpPlantOps[uniqueMgtCode] = new GrazingOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_AutoIrrigation:
                        m_bmpPlantOps[uniqueMgtCode] = new AutoIrrigationOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_AutoFertilizer:
                        m_bmpPlantOps[uniqueMgtCode] = new AutoFertilizerOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_ReleaseImpound:
                        m_bmpPlantOps[uniqueMgtCode] = new ReleaseImpoundOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_ContinuousFertilizer:
                        m_bmpPlantOps[uniqueMgtCode] = new ContinuousFertilizerOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_ContinuousPesticide:
                        m_bmpPlantOps[uniqueMgtCode] = new ContinuousPesticideOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                case BMP_PLTOP_Burning:
                        m_bmpPlantOps[uniqueMgtCode] = new BurningOperation(mgtCode, usebaseHU,husc, year, month, day, m_parameters);
                        break;
                default:
                        break;
                }
                count++;
        }
        bson_destroy(b);
        mongoc_collection_destroy(collection);
        mongoc_cursor_destroy(cursor);
}

void BMPPlantMgtFactory::Dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "Plant Management Factory: " << endl <<
                "    SubScenario ID: " << m_subScenarioId << " Name = " << m_name << endl;
        for (vector<int>::iterator it = m_bmpSequence.begin(); it != m_bmpSequence.end(); it++)
        {
                map<int, PlantManagementOperation *>::iterator findIdx = m_bmpPlantOps.find(*it);
                if (findIdx != m_bmpPlantOps.end())
                        m_bmpPlantOps[*it]->dump(fs);
        }
}
