#include "SettingsOutput.h"

#include "utils_time.h"
#include "text.h"
#include "Logging.h"

using namespace utils_time;

SettingsOutput::SettingsOutput(const int subbasinNum, const int outletID, const int subbasinID,
                               vector<OrgOutItem>& outputItems,
                               int scenarioID /* = 0 */, int calibrationID /* = -1 */,
                               const int mpi_rank /* = 0 */, const int mpi_size /* = -1 */) :
    m_nSubbasins(subbasinNum), m_outletID(outletID), m_subbasinID(subbasinID),
    m_scenarioID(scenarioID), m_calibrationID(calibrationID),
    m_mpi_rank(mpi_rank), m_mpi_size(mpi_size) {
    for (auto iter = outputItems.begin(); iter != outputItems.end(); ++iter) {
        string coreFileName = GetCoreFileName((*iter).outFileName);
        string suffix = GetSuffix((*iter).outFileName);
        /// First, if OutputID does not existed in m_printInfos, then create a new one.
        if (m_printInfosMap.find((*iter).outputID) == m_printInfosMap.end()) {
            m_printInfosMap[(*iter).outputID] = new PrintInfo(m_scenarioID, m_calibrationID);
            m_printInfosMap[(*iter).outputID]->setOutputID((*iter).outputID); /// set the OUTPUTID for the new PrintInfo
        }
        PrintInfo* pi = m_printInfosMap[(*iter).outputID];

        bool isRaster = false;
        if (StringMatch(suffix, string(GTiffExtension))) {
            if (m_subbasinID == 9999) {
                /// For field-version model, all spatial outputs should be text!
                (*iter).outFileName = coreFileName + "." + TextExtension;
                suffix = TextExtension;
            } else {
                isRaster = true;
            }
        }
        /// Check Tag_OutputSubbsn first
        if (StringMatch((*iter).subBsn, Tag_Outlet)) {
            /// Output of outlet, such as Qoutlet, SEDoutlet, etc.
            if (m_subbasinID == 0 || m_subbasinID == m_outletID) {
                /// Only added as print item when running omp version or the current subbasin is outlet for mpi version
                pi->setInterval((*iter).interval);
                pi->setIntervalUnits((*iter).intervalUnit);
                pi->AddPrintItem((*iter).sTimet, (*iter).eTimet, coreFileName,
                                 ValueToString(m_outletID), suffix, true);
            }
        } else if (StringMatch((*iter).subBsn, Tag_AllSubbsn) && (isRaster || m_subbasinID == 9999)) {
            vector<string> aggTypes = SplitString((*iter).aggType, '-');
            /// Output of all subbasins of DT_Raster1D and DT_Raster2D or DT_Array1D and DT_Array2D (field-version)
            for (auto it = aggTypes.begin(); it != aggTypes.end(); ++it) {
                pi->AddPrintItem(*it, (*iter).sTimet, (*iter).eTimet, coreFileName, suffix, m_subbasinID);
            }
        } else {
            // subbasin IDs is provided
            pi->setInterval((*iter).interval);
            pi->setIntervalUnits((*iter).intervalUnit);
            vector<string> subBsns;
            if (StringMatch((*iter).subBsn, Tag_AllSubbsn)) {
                for (int i = 0; i <= m_nSubbasins; i++) {
                    subBsns.emplace_back(ValueToString(i));
                }
                vector<string>(subBsns).swap(subBsns); // deprecated
                // subBsns.shrink_to_fit();
            } else {
                subBsns = SplitString((*iter).subBsn, '-');
            }
            for (auto it = subBsns.begin(); it != subBsns.end(); ++it) {
                string newCoreFileName = coreFileName;
                if (m_subbasinID > 0 && m_subbasinID != 9999) {
                    newCoreFileName += "_" + ValueToString(m_subbasinID);
                }
                if (m_subbasinID == 0 || StringMatch(*it, ValueToString(m_subbasinID))) {
                    pi->AddPrintItem((*iter).sTimet, (*iter).eTimet, newCoreFileName, *it, suffix, true);
                }
            }
        }
    }
    for (auto it = m_printInfosMap.begin(); it != m_printInfosMap.end(); ++it) {
        m_printInfos.emplace_back(it->second);
    }
    vector<PrintInfo *>(m_printInfos).swap(m_printInfos);
    // m_printInfos.shrink_to_fit();
}

SettingsOutput* SettingsOutput::Init(const int subbasinNum, const int outletID, const int subbasinID,
                                     vector<OrgOutItem>& outputItems,
                                     int scenarioID /* = 0 */, int calibrationID /* = -1 */,
                                     const int mpi_rank /* = 0 */, const int mpi_size /* = -1 */) {
    if (outputItems.empty()) {
        LOG(ERROR) << "To run SEIMS-based model, at least one output item should be set!";
        return nullptr;
    }
    return new SettingsOutput(subbasinNum, outletID, subbasinID, outputItems, scenarioID, calibrationID,
                              mpi_rank, mpi_size);
}

SettingsOutput::~SettingsOutput() {
    CLOG(TRACE, LOG_RELEASE) << "Start to release SettingsOutput ...";
    for (auto it = m_printInfosMap.begin(); it != m_printInfosMap.end();) {
        if (it->second != nullptr) {
            delete it->second;
            it->second = nullptr;
        }
        m_printInfosMap.erase(it++);
    }
    m_printInfosMap.clear();
    /// All the PrintInfo instance have been released in the above code, so just set m_pringInfos to empty.
    for (auto it = m_printInfos.begin(); it != m_printInfos.end();) {
        *it = nullptr;
        it = m_printInfos.erase(it);
    }
    m_printInfos.clear();
    CLOG(TRACE, LOG_RELEASE) << "End to release SettingsOutput.";
}

/* This function has been deprecated and replaced by DataCenter::UpdateOutputDate(). The code should be removed later. -LJ.
void SettingsOutput::checkDate(time_t startTime, time_t endTime) {
    // if (m_mpi_rank > 0) return; // Only log warning messages to master rank for MPI. The subbasin 0 may not assigned to rank 0!
    if (m_subbasinID > 1 && m_subbasinID != 9999) return; // Only log once (subbasin 1) for one rank (the rank ID is casual)

    for (auto it = m_printInfos.begin(); it < m_printInfos.end(); ++it) {
        for (auto itemIt = (*it)->m_PrintItems.begin(); itemIt < (*it)->m_PrintItems.end(); ++itemIt) {
            if ((*itemIt)->getStartTime() < startTime || (*itemIt)->getStartTime() >= endTime) {
                (*itemIt)->setStartTime(startTime);
                LOG(WARNING) << "The start time of output " << (*it)->getOutputID() << " to " << (*itemIt)->Filename
                << " is " << (*itemIt)->StartTime << ". It's earlier than start time of time series data "
                << ConvertToString(startTime) << ", and will be updated.";
            }
            if ((*itemIt)->getEndTime() > endTime || (*itemIt)->getEndTime() <= startTime) {
                (*itemIt)->setEndTime(endTime);
                LOG(WARNING) << "The end time of output " << (*it)->getOutputID() << " to " << (*itemIt)->Filename
                << " is " << (*itemIt)->EndTime << ". It's later than end time of time series data "
                << ConvertToString(endTime) << ", and will be updated.";
            }
            break; // To avoid print several logs with the same output ID for different aggregated type
        }
    }
}
*/

void SettingsOutput::Dump(const string& fileName) {
    std::ofstream fs;
    fs.open(fileName.c_str(), std::ios::out);
    if (fs.is_open()) {
        for (size_t idx = 0; idx < m_printInfos.size(); idx++) {
            PrintInfo* info = m_printInfos.at(idx);

            fs << "Output ID: " << info->m_OutputID << endl;

            fs << "---------- All the print info item----------" << endl;
            for (size_t idx2 = 0; idx2 < info->m_PrintItems.size(); idx2++) {
                PrintInfoItem* item = info->m_PrintItems.at(idx2);
                fs << "Type: " << item->getAggregationType() << endl;
                fs << "Start Time:" << ConvertToString2(item->m_startTime) << endl;
                fs << "End Time:" << ConvertToString2(item->m_endTime) << endl;
                fs << "File Name:" << item->Filename << endl;
            }
            fs << "-------------------------------------------" << endl;
        }
        fs.close();
    }
}
