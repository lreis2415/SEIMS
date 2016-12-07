/*!
 * \brief Setting Outputs for SEIMS
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date June 2010
 */
#include "SettingsOutput.h"
#include "util.h"
#include "utils.h"
#include "ModelException.h"
#include "clsRasterData.cpp"

SettingsOutput::SettingsOutput(int subBasinID, string fileName, mongoc_client_t *conn, string dbName,
                               mongoc_gridfs_t *gfs)
        : m_conn(conn), m_dbName(dbName), m_outputGfs(gfs)
{
	SetSubbasinIDs();
    LoadSettingsFromFile(subBasinID, fileName);
}

SettingsOutput::SettingsOutput(int subBasinID, mongoc_client_t *conn, string dbName, mongoc_gridfs_t *gfs)
        : m_conn(conn), m_dbName(dbName), m_outputGfs(gfs)
{
	SetSubbasinIDs();
    LoadSettingsOutputFromMongoDB(subBasinID);
}

SettingsOutput::~SettingsOutput(void)
{
    StatusMessage("Start to release SettingsOutput ...");
    for (vector<PrintInfo *>::iterator it = m_printInfos.begin(); it != m_printInfos.end();)
    {
        if (*it != NULL)
        {
            delete *it;
            *it = NULL;
        }
        it = m_printInfos.erase(it);
    }
    m_printInfos.clear();
    StatusMessage("End to release SettingsOutput ...");
}

bool SettingsOutput::LoadSettingsOutputFromMongoDB(int subBasinID)
{
    bson_t *b = bson_new();
    bson_t *child1 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
    bson_append_document_end(b, child1);
    bson_destroy(child1);

    mongoc_cursor_t *cursor;
    const bson_t *bsonTable;
    mongoc_collection_t *collection;

    collection = mongoc_client_get_collection(m_conn, m_dbName.c_str(), DB_TAB_FILEOUT);
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);

    bson_iter_t itertor;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable))
    {
        int use = -1;
		string modCls = "", outputID = "", descprition = "";
		string outFileName = "", aggType = "", unit = "", subBsn = "";
		string dataType = "", intervalUnit = "";
		int interval = -1;
		string sTimeStr = "", eTimeStr = "";
        if (bson_iter_init_find(&itertor, bsonTable, Tag_OutputUSE))
            use = GetIntFromBSONITER(&itertor);
        if (bson_iter_init_find(&itertor, bsonTable, Tag_MODCLS))
			modCls = GetStringFromBSONITER(&itertor);
		if (bson_iter_init_find(&itertor, bsonTable, Tag_OutputID))
			outputID = GetStringFromBSONITER(&itertor);
		if (bson_iter_init_find(&itertor, bsonTable, Tag_OutputDESC))
			descprition = GetStringFromBSONITER(&itertor);
		if (bson_iter_init_find(&itertor, bsonTable, Tag_FileName))
			outFileName = GetStringFromBSONITER(&itertor);
		string coreFileName = GetCoreFileName(outFileName);
		string suffix = GetSuffix(outFileName);
		if (bson_iter_init_find(&itertor, bsonTable, Tag_AggType))
			aggType = GetStringFromBSONITER(&itertor);
		if (bson_iter_init_find(&itertor, bsonTable, Tag_OutputUNIT))
			unit = GetStringFromBSONITER(&itertor);
		if (bson_iter_init_find(&itertor, bsonTable, Tag_OutputSubbsn))
			subBsn = GetStringFromBSONITER(&itertor);
		if (bson_iter_init_find(&itertor, bsonTable, Tag_StartTime))
			sTimeStr = GetStringFromBSONITER(&itertor);
		if (bson_iter_init_find(&itertor, bsonTable, Tag_EndTime))
			eTimeStr = GetStringFromBSONITER(&itertor);
		if (bson_iter_init_find(&itertor, bsonTable, Tag_Interval))
			interval = GetIntFromBSONITER(&itertor);
		if (bson_iter_init_find(&itertor, bsonTable, Tag_IntervalUnit))
			intervalUnit = GetStringFromBSONITER(&itertor);
		if(use <= 0)
			continue;
		/// First, if OutputID does not existed in m_printInfos, then create a new one.
		if(m_printInfosMap.find(outputID) == m_printInfosMap.end())
		{
			m_printInfosMap[outputID] = new PrintInfo();
			m_printInfosMap[outputID]->setOutputID(outputID);/// set the OUTPUTID for the new PrintInfo
		}
		PrintInfo *pi = NULL; /// reset the pointer
		pi = m_printInfosMap[outputID];

		ostringstream oss;
		oss << subBasinID << "_";
		string strSubbasinID = oss.str();
		bool isRaster = false;
		string gtiff(GTiffExtension);
		if (gtiff.find(suffix) != gtiff.npos)
			isRaster = true;
		/// Check Tag_OutputSubbsn first
		if (StringMatch(subBsn, Tag_Outlet)) /// Output of outlet, such as Qoutlet, SEDoutlet, etc.
		{
			pi->setInterval(interval);
			pi->setIntervalUnits(intervalUnit);
			pi->AddPrintItem(sTimeStr, eTimeStr, strSubbasinID + coreFileName, ValueToString(m_outletID), suffix, m_conn, m_outputGfs, true);
			//pi->AddPrintItem(sTimeStr, eTimeStr, strSubbasinID + coreFileName, suffix);
		}
		else if (StringMatch(subBsn, Tag_AllSubbsn) && isRaster) 
		{
			/// Output of all subbasins of DT_Raster1D or DT_Raster2D
			vector<string> aggTypes = utils::SplitString(aggType, ',');
			for(vector<string>::iterator it = aggTypes.begin(); it != aggTypes.end(); it++)
				pi->AddPrintItem(*it, sTimeStr, eTimeStr, strSubbasinID + coreFileName, suffix, m_conn, m_outputGfs);
		}
		else // subbasin IDs is provided
		{
			pi->setInterval(interval);
			pi->setIntervalUnits(intervalUnit);
			vector<string> subBsns;
			if (StringMatch(subBsn, Tag_AllSubbsn))
			{
				for(int i = 0; i <= m_nSubbasins; i++)
					subBsns.push_back(ValueToString(i));
				vector<string>(subBsns).swap(subBsns);
			}
			else
				subBsns = utils::SplitString(subBsn, ',');
			for(vector<string>::iterator it = subBsns.begin(); it != subBsns.end(); it++)
				pi->AddPrintItem(sTimeStr, eTimeStr, strSubbasinID + coreFileName, *it, suffix, m_conn, m_outputGfs, true);
		}
    }
	for (map<string, PrintInfo*>::iterator it = m_printInfosMap.begin(); it != m_printInfosMap.end(); it++)
	{
		m_printInfos.push_back(it->second);
	}
	vector<PrintInfo*>(m_printInfos).swap(m_printInfos);
    bson_destroy(b);
    mongoc_collection_destroy(collection);
    mongoc_cursor_destroy(cursor);
    return true;
}

void SettingsOutput::SetSubbasinIDs()
{
	bson_t *b = bson_new();
	bson_t *child = bson_new();
	bson_t *child2 = bson_new();
	bson_t *child3 = bson_new();
	BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child);
	BSON_APPEND_DOCUMENT_BEGIN(child, PARAM_FLD_NAME, child2);
	BSON_APPEND_ARRAY_BEGIN(child2, "$in", child3);
	BSON_APPEND_UTF8(child3,PARAM_FLD_NAME, VAR_OUTLETID);
	BSON_APPEND_UTF8(child3,PARAM_FLD_NAME, VAR_SUBBSNID_NUM);
	bson_append_array_end(child2, child3);
	bson_append_document_end(child, child2);
	bson_append_document_end(b, child);
	//printf("%s\n",bson_as_json(b,NULL));

	mongoc_cursor_t *cursor;
	const bson_t *bsonTable;
	mongoc_collection_t *collection;

	collection = mongoc_client_get_collection(m_conn, m_dbName.c_str(), DB_TAB_PARAMETERS);
	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);

	bson_iter_t iter;
	while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable))
	{
		string nameTmp = "";
		int numTmp = -1;
		if (bson_iter_init_find(&iter, bsonTable, PARAM_FLD_NAME))
			nameTmp = GetStringFromBSONITER(&iter);
		if (bson_iter_init_find(&iter, bsonTable, PARAM_FLD_VALUE))
			numTmp = GetIntFromBSONITER(&iter);
		if(!StringMatch(nameTmp, "") && numTmp != -1)
		{
			if(StringMatch(nameTmp, VAR_OUTLETID))
				m_outletID = GetIntFromBSONITER(&iter);
			else if (StringMatch(nameTmp, VAR_SUBBSNID_NUM))
				m_nSubbasins = GetIntFromBSONITER(&iter);
		}
		else
			throw ModelException("SettingOutput","SetSubbasinIDs","No valid values found in MongoDB!");
	}
	bson_destroy(child);
	bson_destroy(child2);
	bson_destroy(child3);
	bson_destroy(b);
	mongoc_collection_destroy(collection);
	mongoc_cursor_destroy(cursor);
	return;
}
bool SettingsOutput::LoadSettingsFromFile(int subBasinID, string fileName)
{
    if (!Settings::LoadSettingsFromFile(fileName)) return false;
    if (!ParseOutputSettings(subBasinID)) return false;
    return true;
}

bool SettingsOutput::ParseOutputSettings(int subBasinID)
{
    PrintInfo *pi = NULL;
    ostringstream oss;
    oss << subBasinID << "_";
    string strSubbasinID = oss.str();

    for (size_t i = 0; i < m_Settings.size(); i++)
    {
        // Sample output entries from the FILE.OUT file
        // placed here for reference
        //OUTPUTID  | PET_TS
        //INTERVAL  | 24 | HOURS
        //SITECOUNT | 2
        //SITENAME  | 720_pet
        //STARTTIME | 2000/01/01/00
        //ENDTIME   | 2000/12/31/00
        //FILENAME  | PET_720.txt
        //SITENAME  | 736_pet
        //STARTTIME | 2001/01/01/00
        //ENDTIME   | 2001/12/31/00
        //FILENAME  | 736_PET.txt

        //OUTPUTID  | D_PREC
        //TYPE      | SUM
        //COUNT     | 2
        //STARTTIME | 2000/01/01/00
        //ENDTIME   | 2000/01/31/00
        //FILENAME  | D_PREC_1.asc
        //STARTTIME | 2000/02/01/00
        //ENDTIME   | 2000/02/28/00
        //FILENAME  | D_PREC_2.asc

        // OUTPUTID starts a new PrintInfo
        if (StringMatch(m_Settings[i][0], Tag_OutputID))
        {
            // is this the first PrintInfo or not
            if (pi != NULL)
            {
                // not the first PrintInfo so add this one to the list before starting a new one
                m_printInfos.push_back(pi);
            }
            // reset the pointer
            pi = NULL;
            // start a new PrintInfo
            pi = new PrintInfo();
            // set the OUTPUTID for the new PrintInfo
            pi->setOutputID(m_Settings[i][1]);

            // for QOUTLET or QTotal or SEDOUTLET or DissovePOutlet or AmmoniumOutlet or NitrateOutlet
            if (StringMatch(m_Settings[i][1], TAG_OUT_QOUTLET) || StringMatch(m_Settings[i][1], TAG_OUT_QTOTAL) ||
                StringMatch(m_Settings[i][1], TAG_OUT_SEDOUTLET)
                || StringMatch(m_Settings[i][1], Tag_DisPOutlet) || StringMatch(m_Settings[i][1], Tag_AmmoOutlet)
                || StringMatch(m_Settings[i][1], Tag_NitrOutlet))
            {
                string starttm = "";
                string endtm = "";
                string fname = "";
                string suffix = "";
                // check to see if we have all 4 values we need
                for (int flag = 0; flag < 4; flag++)
                {
                    i++;
                    if (StringMatch(m_Settings[i][0], Tag_Interval))
                    {
                        // set the interval length
                        pi->setInterval(atoi(m_Settings[i][1].c_str()));
                        // set the interval units
                        pi->setIntervalUnits(m_Settings[i][2].c_str());
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_StartTime))
                    {
                        // get the start time
                        starttm = m_Settings[i][1];
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_EndTime))
                    {
                        // get the end time
                        endtm = m_Settings[i][1];
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_FileName))
                    {
                        // get the filename, but not include the suffix.
                        // modified by ZhuLJ, 2015/6/16
                        fname = strSubbasinID + GetCoreFileName(m_Settings[i][1]);
                        suffix = GetSuffix(m_Settings[i][1]);
                    }
                }
                pi->AddPrintItem(starttm, endtm, fname, suffix);
            }
        }
        // INTERVAL is used only for the PET_TS output
        if (StringMatch(m_Settings[i][0], Tag_Interval))
        {
            // check that an object exists
            if (pi != NULL)
            {
                // set the interval length
                pi->setInterval(atoi(m_Settings[i][1].c_str()));
                // set the interval units
                pi->setIntervalUnits(m_Settings[i][2].c_str());
            }
        }
        // SITECOUNT is used only for the time series output for sites
        if (StringMatch(m_Settings[i][0], Tag_SiteCount))
        {
            string sitename = "";
            string starttm = "";
            string endtm = "";
            string fname = "";
            string suffix = "";
            // get the number of sites in the list
            int cnt = atoi(m_Settings[i][1].c_str());
            // for each site in the list
            for (int idx = 0; idx < cnt; idx++)
            {
                // reset values
                sitename = "";
                endtm = "";
                starttm = "";
                fname = "";

                // check to see if we have all 4 values we need
                for (int flag = 0; flag < 4; flag++)
                {
                    i++;
                    if (StringMatch(m_Settings[i][0], Tag_SiteID))
                    {
                        // get the sitename
                        sitename = m_Settings[i][1];
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_StartTime))
                    {
                        // get the start time
                        starttm = m_Settings[i][1];
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_EndTime))
                    {
                        // get the end time
                        endtm = m_Settings[i][1];
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_FileName))
                    {
                        // get the filename
                        fname = strSubbasinID + GetCoreFileName(m_Settings[i][1]);
                        //get suffix
                        suffix = GetSuffix(m_Settings[i][1]);
                    }
                }

                // check to see if there is a site to save
                if (sitename.size() > 0)
                {
                    // add the print item
                    pi->AddPrintItem(starttm, endtm, fname, sitename, suffix, m_conn, m_outputGfs, false);
                }
            }
        }
        // SUBBASINCOUNT is used only for the time series output for subbasins
        if (StringMatch(m_Settings[i][0], Tag_SubbasinCount) ||
            StringMatch(m_Settings[i][0], Tag_ReservoirCount))
        {
            string subbasinname = "";
            string starttm = "";
            string endtm = "";
            string fname = "";
            string suffix = "";
            // get the number of sites in the list
            int cnt = atoi(m_Settings[i][1].c_str());
            // for each site int he list
            for (int idx = 0; idx < cnt; idx++)
            {
                // reset values
                subbasinname = "";
                endtm = "";
                starttm = "";
                fname = "";
                suffix = "";
                // check to see if we have all 4 values we need
                for (int flag = 0; flag < 4; flag++)
                {
                    i++;
                    if (StringMatch(m_Settings[i][0], Tag_SubbasinId) ||
                        StringMatch(m_Settings[i][0], Tag_ReservoirId))
                    {
                        // get the sitename
                        subbasinname = m_Settings[i][1];
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_StartTime))
                    {
                        // get the start time
                        starttm = m_Settings[i][1];
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_EndTime))
                    {
                        // get the end time
                        endtm = m_Settings[i][1];
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_FileName))
                    {
                        // get the filename
                        fname = strSubbasinID + GetCoreFileName(m_Settings[i][1]);
                        // get the suffix
                        suffix = GetSuffix(m_Settings[i][1]);
                    }
                }

                // check to see if there is a site to save
                if (subbasinname.size() > 0)
                {
                    // add the print item
                    pi->AddPrintItem(starttm, endtm, fname, subbasinname, suffix, m_conn, m_outputGfs, true);
                }
            }
        }
        if (StringMatch(m_Settings[i][0], Tag_Count))
        {
            string type = "";
            string starttm = "";
            string endtm = "";
            string fname = "";
            string suffix = "";
            // get the number of sites in the list
            int cnt = atoi(m_Settings[i][1].c_str());
            // for each site int he list
            for (int idx = 0; idx < cnt; idx++)
            {
                endtm = "";
                starttm = "";
                fname = "";
                suffix = "";
                // check to see if we have all 3 values we need
                for (int flag = 0; flag < 4; flag++)
                {
                    i++;
                    if (StringMatch(m_Settings[i][0], Tag_AggType))
                    {
                        // get the type
                        type = m_Settings[i][1];
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_StartTime))
                    {
                        // get the start time
                        starttm = m_Settings[i][1];
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_EndTime))
                    {
                        // get the end time
                        endtm = m_Settings[i][1];
                    }
                    else if (StringMatch(m_Settings[i][0], Tag_FileName))
                    {
                        // get the filename
                        fname = strSubbasinID + GetCoreFileName(m_Settings[i][1]);
                        // get the suffix
                        suffix = GetSuffix(m_Settings[i][1]);
                    }
                }

                // check to see if there is a site to save
                if (starttm.size() > 0)
                {
                    // add the print item
                    pi->AddPrintItem(type, starttm, endtm, fname, suffix, m_conn, m_outputGfs);
                }
            }
        }
    }

    if (pi != NULL)
    {
        // add the last one.
        m_printInfos.push_back(pi);
        pi = NULL;
    }
    return true;
}

void SettingsOutput::checkDate(time_t startTime, time_t endTime)
{
    utils util;
    vector<PrintInfo *>::iterator it;
    for (it = m_printInfos.begin(); it < m_printInfos.end(); it++)
    {
        vector<PrintInfoItem *>::iterator itemIt;
        for (itemIt = (*it)->m_PrintItems.begin(); itemIt < (*it)->m_PrintItems.end(); itemIt++)
        {
            if ((*itemIt)->getStartTime() < startTime){
				// cout<<(*itemIt)->getStartTime()<<", "<<startTime<<endl;
				(*itemIt)->setStartTime(startTime);
				cout<<"WARNING: The start time of output "<<(*it)->getOutputID()<<" to "<<(*itemIt)->Filename<<" is "<<(*itemIt)->StartTime<<
					". It's earlier than start time of time series data "<<util.ConvertToString(&startTime)<<", and will be updated."<<endl;
                //throw ModelException("SettingsOutput", "CheckDate",
                //                     "The start time of output " + (*it)->getOutputID() + " to " + (*itemIt)->Filename +
                //                     " is " + (*itemIt)->StartTime +
                //                     ". It's earlier than start time of time series data " +
                //                     util.ConvertToString(&startTime) +
                //                     ". Please check time setting of file.in and file.out.");
			}
            if ((*itemIt)->getEndTime() > endTime){
				cout<<"WARNING: The end time of output "<<(*it)->getOutputID()<<" to "<<(*itemIt)->Filename<<" is "<<(*itemIt)->EndTime<<
					". It's later than end time of time series data "<<util.ConvertToString(&endTime)<<", and will be updated."<<endl;
                //throw ModelException("SettingsOutput", "CheckDate",
                //                     "The end time of output " + (*it)->getOutputID() + " to " + (*itemIt)->Filename +
                //                     " is " + (*itemIt)->EndTime + ". It's later than end time of time series data " +
                //                     util.ConvertToString(&endTime) +
                //                     ". Please check time setting of file.in and file.out.");
			}
        }
    }
}

//! output is ASCII file? The output should be dependent on Data type and suffix, so this function should be deprecated. LJ
//bool SettingsOutput::isOutputASCFile()
//{
//	vector<PrintInfo*>::iterator it;
//	for(it=m_printInfos.begin();it<m_printInfos.end();it++)
//	{
//		if((*it)->m_OutputID.find_first_of("D") != string::npos) return true;
//	}
//	return false;
//}
//! set specific cell raster output
//void SettingsOutput::setSpecificCellRasterOutput(string projectPath,string databasePath,clsRasterData* templateRasterData)
//{
//	vector<PrintInfo*>::iterator itInfo;
//	for(itInfo=m_printInfos.begin();itInfo<m_printInfos.end();itInfo++)
//	{
//		(*itInfo)->setSpecificCellRasterOutput(projectPath,databasePath,templateRasterData);
//	}
//}

void SettingsOutput::Dump(string fileName)
{
    ofstream fs;
    utils util;
    fs.open(fileName.c_str(), ios::out);
    if (fs.is_open())
    {
        for (size_t idx = 0; idx < m_printInfos.size(); idx++)
        {
            PrintInfo *info = m_printInfos.at(idx);

            fs << "Output ID: " << info->m_OutputID << endl;

            fs << "---------- All the print info item----------" << endl;
            for (size_t idx2 = 0; idx2 < info->m_PrintItems.size(); idx2++)
            {
                PrintInfoItem *item = info->m_PrintItems.at(idx2);
                fs << "Type: " << item->getAggregationType() << endl;
                fs << "Start Time:" << item->StartTime << endl;
                fs << "End Time:" << item->EndTime << endl;
                fs << "File Name:" << item->Filename << endl;
            }
            fs << "-------------------------------------------" << endl;
        }

        fs.close();
    }
}
