#include "SettingsOutput.h"

SettingsOutput::SettingsOutput(int subbasinNum, int outletID, vector<OrgOutItem>& outputItems) :
                               m_nSubbasins(subbasinNum), m_outletID(outletID) 
{
    for (vector<OrgOutItem>::iterator iter = outputItems.begin(); iter != outputItems.end(); ++iter) {
        string coreFileName = GetCoreFileName((*iter).outFileName);
        string suffix = GetSuffix((*iter).outFileName);
        /// First, if OutputID does not existed in m_printInfos, then create a new one.
        if (m_printInfosMap.find((*iter).outputID) == m_printInfosMap.end()) {
            m_printInfosMap[(*iter).outputID] = new PrintInfo();
            m_printInfosMap[(*iter).outputID]->setOutputID((*iter).outputID);/// set the OUTPUTID for the new PrintInfo
        }
        PrintInfo *pi = NULL; /// reset the pointer
        pi = m_printInfosMap[(*iter).outputID];

        bool isRaster = false;
        if (StringMatch(suffix, string(GTiffExtension))) {
            isRaster = true;
        }
        /// Check Tag_OutputSubbsn first
        if (StringMatch((*iter).subBsn, Tag_Outlet)) /// Output of outlet, such as Qoutlet, SEDoutlet, etc.
        {
            pi->setInterval((*iter).interval);
            pi->setIntervalUnits((*iter).intervalUnit);
            pi->AddPrintItem((*iter).sTimeStr, (*iter).eTimeStr, coreFileName, ValueToString(m_outletID), suffix, true);
        }
        else if (StringMatch((*iter).subBsn, Tag_AllSubbsn) && isRaster) {
            /// Output of all subbasins of DT_Raster1D or DT_Raster2D
            vector<string> aggTypes = SplitString((*iter).aggType, ',');
            for (vector<string>::iterator it = aggTypes.begin(); it != aggTypes.end(); it++) {
                pi->AddPrintItem(*it, (*iter).sTimeStr, (*iter).eTimeStr, coreFileName, suffix);
            }
        }
        else // subbasin IDs is provided
        {
            pi->setInterval((*iter).interval);
            pi->setIntervalUnits((*iter).intervalUnit);
            vector<string> subBsns;
            if (StringMatch((*iter).subBsn, Tag_AllSubbsn)) {
                for (int i = 0; i <= m_nSubbasins; i++) {
                    subBsns.push_back(ValueToString(i));
                }
                vector<string>(subBsns).swap(subBsns);
            }
            else {
                subBsns = SplitString((*iter).subBsn, ',');
            }
            for (vector<string>::iterator it = subBsns.begin(); it != subBsns.end(); it++) {
                pi->AddPrintItem((*iter).sTimeStr, (*iter).eTimeStr, coreFileName, *it, suffix, true);
            }
        }
    }
    for (map<string, PrintInfo *>::iterator it = m_printInfosMap.begin(); it != m_printInfosMap.end(); it++) {
        m_printInfos.push_back(it->second);
    }
    vector<PrintInfo *>(m_printInfos).swap(m_printInfos);
}

SettingsOutput* SettingsOutput::Init(int subbasinNum, int outletID, vector<OrgOutItem>& outputItems) {
    if (outputItems.empty()) {
        return NULL;
    }
    return new SettingsOutput(subbasinNum, outletID, outputItems);
}

SettingsOutput::~SettingsOutput(void) {
    StatusMessage("Start to release SettingsOutput ...");
    for (map<string, PrintInfo *>::iterator it = m_printInfosMap.begin(); it != m_printInfosMap.end(); ) {
        if (it->second != NULL) {
            delete it->second;
            it->second = NULL;
        }
        m_printInfosMap.erase(it++);
    }
    m_printInfosMap.clear();
    /// All the PrintInfo instance have been released in the above code, so just set m_pringInfos to empty.
    for (vector<PrintInfo *>::iterator it = m_printInfos.begin(); it != m_printInfos.end();) {
        *it = NULL;
        it = m_printInfos.erase(it);
    }
    m_printInfos.clear();
    vector<PrintInfo*>().swap(m_printInfos);
    StatusMessage("End to release SettingsOutput ...");
}

void SettingsOutput::checkDate(time_t startTime, time_t endTime) {
    vector<PrintInfo *>::iterator it;
    for (it = m_printInfos.begin(); it < m_printInfos.end(); it++) {
        vector<PrintInfoItem *>::iterator itemIt;
        for (itemIt = (*it)->m_PrintItems.begin(); itemIt < (*it)->m_PrintItems.end(); itemIt++) {
            if ((*itemIt)->getStartTime() < startTime) {
                // cout<<(*itemIt)->getStartTime()<<", "<<startTime<<endl;
                (*itemIt)->setStartTime(startTime);
                cout << "WARNING: The start time of output " << (*it)->getOutputID() << " to " << (*itemIt)->Filename
                     << " is " << (*itemIt)->StartTime <<
                     ". It's earlier than start time of time series data " << ConvertToString(&startTime)
                     << ", and will be updated." << endl;
            }
            if ((*itemIt)->getEndTime() > endTime) {
                (*itemIt)->setEndTime(endTime);
                cout << "WARNING: The end time of output " << (*it)->getOutputID() << " to " << (*itemIt)->Filename
                     << " is " << (*itemIt)->EndTime <<
                     ". It's later than end time of time series data " << ConvertToString(&endTime)
                     << ", and will be updated." << endl;
            }
        }
    }
}

void SettingsOutput::Dump(string fileName) {
    ofstream fs;
    utils util;
    fs.open(fileName.c_str(), ios::out);
    if (fs.is_open()) {
        for (size_t idx = 0; idx < m_printInfos.size(); idx++) {
            PrintInfo *info = m_printInfos.at(idx);

            fs << "Output ID: " << info->m_OutputID << endl;

            fs << "---------- All the print info item----------" << endl;
            for (size_t idx2 = 0; idx2 < info->m_PrintItems.size(); idx2++) {
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
