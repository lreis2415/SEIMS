#include "SettingsOutput.h"

#include <vector>
#include <algorithm>

#include "utils_time.h"
#include "text.h"
#include "Logging.h"

using namespace utils_time;
using std::vector;

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
            m_printInfosMap[(*iter).outputID]->setInterval((*iter).interval);
            m_printInfosMap[(*iter).outputID]->setIntervalUnits((*iter).intervalUnit);
        }
        PrintInfo* pi = m_printInfosMap[(*iter).outputID];

        bool isRaster = false;
        if (StringMatch(suffix.c_str(), data_raster::GTiffExtension)) {
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

vector<OrgOutItem> SettingsOutput::ReadFileOutFile(const InputArgs *input_args,
                                                   map<string, OrgOutItem>& org_items) {
    vector<OrgOutItem> outputItems;
    string model_cfgpath = input_args->model_path;
    if (!input_args->model_cfgname.empty() && !StringMatch(input_args->model_cfgname, "_BASE_")) {
        model_cfgpath += SEP + input_args->model_cfgname;
    }
    string file_out = model_cfgpath + SEP + File_Output;
    if (!FileExists(file_out)) {
        LOG(ERROR) << file_out << " does not exist!";
        return outputItems;
    }
    vector<string> stringvector;

    if (!LoadPlainTextFile(file_out, stringvector) || stringvector.empty()) {
        LOG(ERROR) << file_out << " is not loaded!";
        return outputItems;
    }
    vector<string> output_flds = SplitString(stringvector[0], ',');
    // find index of Tag_OutputID
    int id_idx = -1;
    auto it = std::find(output_flds.begin(), output_flds.end(), Tag_OutputID);
    if (it != output_flds.end()) {
        id_idx = std::distance(output_flds.begin(), it);
    } else {
        LOG(ERROR) << Tag_OutputID << " CANNOT be found in file.out!";
        return outputItems;
    }
    if (stringvector.size() < 2) {
        LOG(ERROR) << "file.out only have one title line!";
        return outputItems;
    }
    for (auto it = stringvector.begin(); it != stringvector.end(); ++it) {
        if (it - stringvector.begin() == 0) { continue; }
        vector<string> output_items = SplitString(*it, ',');
        string cid = output_items[id_idx];
        if (org_items.find(cid) == org_items.end()) {
            LOG(WARNING) << cid << " in file.out is not supported! We will ignore it!";
            continue;
        }
        OrgOutItem tmp_output_item = org_items.at(cid);
        for (auto fld_it = output_flds.begin(); fld_it != output_flds.end(); ++fld_it) {
            int idx = fld_it - output_flds.begin();
            if (StringMatch(*fld_it, Tag_MODCLS)) {
                tmp_output_item.modCls = output_items[idx];
            } else if (StringMatch(*fld_it, Tag_OutputID)) {
                tmp_output_item.outputID = GetUpper(output_items[idx]);
            } else if (StringMatch(*fld_it, Tag_OutputDESC)) {
                tmp_output_item.descprition = output_items[idx];
            } else if (StringMatch(*fld_it, Tag_OutputUNIT)) {
                tmp_output_item.unit = output_items[idx];
            } else if (StringMatch(*fld_it, Tag_AggType)) {
                tmp_output_item.aggType = output_items[idx];
            } else if (StringMatch(*fld_it, Tag_StartTime)) {
                tmp_output_item.sTimet = ConvertToTime(output_items[idx],
                                                       "%d-%d-%d %d:%d:%d", true);
            } else if (StringMatch(*fld_it, Tag_EndTime)) {
                tmp_output_item.eTimet = ConvertToTime(output_items[idx],
                                                       "%d-%d-%d %d:%d:%d", true);
            } else if (StringMatch(*fld_it, Tag_Interval)) {
                tmp_output_item.interval = ToInt(output_items[idx]);
            } else if (StringMatch(*fld_it, Tag_IntervalUnit)) {
                tmp_output_item.intervalUnit = output_items[idx];
            } else if (StringMatch(*fld_it, Tag_FileName)) {
                tmp_output_item.outFileName = output_items[idx];
            } else if (StringMatch(*fld_it, Tag_OutputSubbsn)) {
                tmp_output_item.subBsn = output_items[idx];
            }
        }
        tmp_output_item.use = 1;
        outputItems.push_back(tmp_output_item);
    }
    vector<OrgOutItem>(outputItems).swap(outputItems);

    return outputItems;
}


SettingsOutput::~SettingsOutput() {
    CLOG(TRACE, LOG_RELEASE) << "Start to release SettingsOutput ...";
    for (auto it = m_printInfosMap.begin(); it != m_printInfosMap.end(); ++it) {
        if (it->second != nullptr) {
            delete it->second;
            it->second = nullptr;
        }
        // m_printInfosMap.erase(it++);
    }
    m_printInfosMap.clear();
    /// All the PrintInfo instance have been released in the above code, so just set m_pringInfos to empty.
    for (auto it = m_printInfos.begin(); it != m_printInfos.end(); ++it) {
        *it = nullptr;
        // it = m_printInfos.erase(it);
    }
    m_printInfos.clear();
    CLOG(TRACE, LOG_RELEASE) << "End to release SettingsOutput.";
}

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
