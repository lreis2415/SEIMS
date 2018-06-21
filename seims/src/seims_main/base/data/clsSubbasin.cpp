#include "clsSubbasin.h"

#include "text.h"

Subbasin::Subbasin(const int id) :
    subbsn_id_(id), n_cells_(-1), cells_(nullptr), cell_area_(-1.f),
    area_(-1.f), pet_(-1.f),
    perco_(-1.f), pcp_(-1.f),
    intercep_(-1.f), intercep_et_(-1.f), depression_et_(-1.f), infil_(-1.f),
    soil_et_(-1.f),
    total_et_(-1.f), net_perco_(-1.f), runoff_(-1.f), interflow_(-1.f),
    soil_wtr_(-1.f), net_pcp_(-1.f),
    mean_tmp_(-1.f), soil_tmp_(-1.f), gwmax_(-1.f), kg_(-1.f),
    revap_coef_(-1.f), base_ex_(-1.f), qg_cvt_(-1.f), slope_coef_(1.f), slope_(-1.f),
    revap_(-1.f),
    gw_(-1.f),
    deep_perco_(-1.f), qg_(-1.f), rg_(-1.f), output_(false), revap_changed_(true) {
}

Subbasin::~Subbasin() {
    if (cells_ != nullptr) Release1DArray(cells_);
}

bool Subbasin::CheckInputSize(const int n) {
    if (n <= 0) {
        throw ModelException("Subbasin Class", "CheckInputSize",
                             "Input data for Subbasin is invalid. The size could not be less than zero.");
    }

    if (n_cells_ != n) {
        if (n_cells_ <= 0) {
            n_cells_ = n;
        } else {
            throw ModelException("Subbasin Class", "CheckInputSize", "All input data should have same size.");
        }
    }
    return true;
}

void Subbasin::SetCellList(const int n_cells, int* cells) {
    CheckInputSize(n_cells);
    cells_ = cells;
}

// Note: Since slope is calculated by drop divided by distance in TauDEM,
//		 the average should be calculated after doing atan().
//		 By LJ, 2016-7-27
void Subbasin::SetSlope(float* slope) {
    slope_ = 0.f;
    int index = 0;
    for (int i = 0; i < n_cells_; i++) {
        index = cells_[i];
        //m_slope += slope[index];  // percent
        slope_ += atan(slope[index]); // radian
    }
    slope_ /= n_cells_;
    slope_ = tan(slope_); // to keep consistent with the slope unit in the whole model
}

//////////////////////////////////////////////////////////////////////////
//////////  clsSubbasins                           ///////////////////////
//////////////////////////////////////////////////////////////////////////
clsSubbasins::clsSubbasins(map<string, FloatRaster *>& rs_map, const int prefix_id): n_subbasins_(-1) {
    subbasin_ids_.clear();
    subbasin_objs_.clear();

    // read subbasin data
    int n_cells = -1;
    float* subbasin_data = nullptr;
    float cell_width = NODATA_VALUE;
    std::ostringstream oss;
    oss << prefix_id << "_" << VAR_SUBBSN;
    string subbasin_file_name = GetUpper(oss.str());
    oss.str("");

    rs_map[subbasin_file_name]->GetRasterData(&n_cells, &subbasin_data);

    // valid cell indexes of each subbasin, key is subbasin ID, value is vector of cell's index
    map<int, vector<int> > cell_list_map;
    for (int i = 0; i < n_cells; i++) {
        int sub_id = CVT_INT(subbasin_data[i]);
        if (cell_list_map.find(sub_id) == cell_list_map.end()) {
#ifdef HAS_VARIADIC_TEMPLATES
            cell_list_map.emplace(sub_id, vector<int>());
#else
            cell_list_map.insert(make_pair(sub_id, vector<int>()));
#endif
        }
        cell_list_map[sub_id].emplace_back(i);
    }
    n_subbasins_ = CVT_INT(cell_list_map.size());
    for (auto it = cell_list_map.begin(); it != cell_list_map.end(); ++it) {
        // swap for saving memory, using shrink_to_fit() instead.
        vector<int>(it->second).swap(it->second);
        // (*it->second).shrink_to_fit();
        int sub_id = it->first;
        subbasin_ids_.emplace_back(sub_id);
        Subbasin* new_sub = new Subbasin(sub_id);
        int n_cells_tmp = CVT_INT(it->second.size());
        int* tmp = new int[n_cells_tmp];
        for (int j = 0; j < n_cells_tmp; j++)
            tmp[j] = it->second.at(j);
        new_sub->SetCellList(n_cells_tmp, tmp);
        new_sub->SetArea(cell_width * cell_width * n_cells_tmp);
        subbasin_objs_[sub_id] = new_sub;
    }
    vector<int>(subbasin_ids_).swap(subbasin_ids_);
    // m_subbasinIDs.shrink_to_fit();
}

clsSubbasins* clsSubbasins::Init(MongoGridFs* spatialData, map<string,FloatRaster *>& rsMap,
                                 int prefixID) {
    std::ostringstream oss;
    oss << prefixID << "_" << Tag_Mask;
    string mask_file_name = GetUpper(oss.str());
    oss.str("");
    oss << prefixID << "_" << VAR_SUBBSN;
    string subbasin_file_name = GetUpper(oss.str());
    oss.str("");

    if (rsMap.find(mask_file_name) == rsMap.end()) {
        // if mask not loaded yet
        cout << "MASK data has not been loaded yet!" << endl;
        return nullptr;
    }
    int n_cells = -1;
    float* subbasin_data = nullptr;
    if (rsMap.find(subbasin_file_name) == rsMap.end()) {
        // if subbasin not loaded yet
        FloatRaster* subbasinRaster = nullptr;
        subbasinRaster = FloatRaster::Init(spatialData, subbasin_file_name.c_str(),
                                           true, rsMap[mask_file_name]);
        assert(nullptr != subbasinRaster);
        if (!subbasinRaster->GetRasterData(&n_cells, &subbasin_data)) {
            cout << "Subbasin data loaded failed!" << endl;
            return nullptr;
        }
#ifdef HAS_VARIADIC_TEMPLATES
        rsMap.emplace(subbasin_file_name, subbasinRaster);
#else
        rsMap.insert(make_pair(subbasin_file_name, subbasinRaster));
#endif
    } else {
        if (!rsMap[subbasin_file_name]->GetRasterData(&n_cells, &subbasin_data)) {
            cout << "Subbasin data preloaded is unable to access!" << endl;
            return nullptr;
        }
    }

    return new clsSubbasins(rsMap, prefixID);
}

clsSubbasins::~clsSubbasins() {
    StatusMessage("Release subbasin class ...");
    if (!subbasin_objs_.empty()) {
        for (auto iter = subbasin_objs_.begin(); iter != subbasin_objs_.end();) {
            if (iter->second != nullptr) {
                delete iter->second;
                iter->second = nullptr;
            }
            subbasin_objs_.erase(iter++);
        }
        subbasin_objs_.clear();
    }
}

float clsSubbasins::Subbasin2Basin(const string& key) {
    float temp = 0.f;
    int total_count = 0;
    Subbasin* sub = nullptr;
    for (auto it = subbasin_ids_.begin(); it != subbasin_ids_.end(); ++it) {
        sub = subbasin_objs_[*it];
        int n_count = sub->GetCellCount();
        if (StringMatch(key, VAR_SLOPE)) {
            temp += atan(sub->GetSlope()) * n_count;
        } else if (StringMatch(key, VAR_PET)) {
            temp += sub->GetPet() * n_count;
        } else if (StringMatch(key, VAR_PERCO)) {
            temp += sub->GetPerco() * n_count;
        } else if (StringMatch(key, VAR_REVAP)) {
            temp += sub->GetEg() * n_count;
        } else if (StringMatch(key, VAR_PERDE)) {
            temp += sub->GetPerde() * n_count;
        } else if (StringMatch(key, VAR_RG)) {
            temp += sub->GetRg() * n_count;
        } else if (StringMatch(key, VAR_QG)) {
            temp += sub->GetQg();
        } else if (StringMatch(key, VAR_GW_Q)) {
            temp += sub->GetGw() * n_count;
        }

        total_count += n_count;
    }
    if (StringMatch(key, VAR_QG)) {
        return temp;
    } // basin sum
    if (StringMatch(key, VAR_SLOPE))
        return tan(temp / total_count);
    return temp / total_count; // basin average
}

void clsSubbasins::SetSlopeCoefficient() {
    float basin_slope = Subbasin2Basin(VAR_SLOPE);
    for (auto it = subbasin_ids_.begin(); it != subbasin_ids_.end(); ++it) {
        Subbasin* cur_sub = subbasin_objs_[*it];
        if (basin_slope <= 0.f) {
            cur_sub->SetSlopeCoefofBasin(1.f);
        } else {
            float slp_coef = cur_sub->GetSlope() / basin_slope;
            cur_sub->SetSlopeCoefofBasin(slp_coef);
        }
    }
}
