#include "GridLayering.h"

#ifdef USE_MONGODB
GridLayeringMFDmd::GridLayeringMFDmd(const int id, MongoGridFs* gfs, const char* stream_file, const char* out_dir) :
    GridLayering(id, gfs, out_dir), flow_fraction_(nullptr), flowfrac_matrix_(nullptr), flowin_fracs_(nullptr),
    flowout_fracs_(nullptr) {
    string prefix = ValueToString(subbasin_id_);
    // inputs
    flowdir_name_ = prefix + "_FLOW_DIR_MFDMD";
    flowfrac_corename_ = prefix + "_FLOW_FRACTION_MFDMD";
    mask_name_ = prefix + "_MASK";
    stream_file_ = stream_file;
    // outputs
    flowin_index_name_ = prefix + "_FLOWIN_INDEX_MFDMD";
    flowin_frac_name_ = prefix + "_FLOWIN_FRACTION_MFDMD";
    flowout_index_name_ = prefix + "_FLOWOUT_INDEX_MFDMD";
    flowout_frac_name_ = prefix + "_FLOWOUT_FRACTION_MFDMD";
    layering_updown_name_ = prefix + "_ROUTING_LAYERS_UP_DOWN_MFDMD";
    layering_downup_name_ = prefix + "_ROUTING_LAYERS_DOWN_UP_MFDMD";
}
#endif

GridLayeringMFDmd::GridLayeringMFDmd(const int id, const char* fd_file, const char* fraction_file,
                                     const char* mask_file, const char* stream_file, 
                                     const char* out_dir) :
    GridLayering(id, out_dir), flow_fraction_(nullptr), flowfrac_matrix_(nullptr), flowin_fracs_(nullptr),
    flowout_fracs_(nullptr) {
    string prefix = ValueToString(subbasin_id_);
    // inputs
    flowdir_name_ = fd_file;
    for (int i = 1; i <= 8; i++) {
        string abspath = GetPathFromFullName(fraction_file);
        string cname = GetCoreFileName(fraction_file);
        string suffix = GetSuffix(fraction_file);
        flowfrac_names_.emplace_back(abspath + cname + "_" + ValueToString(i) + "." + suffix);
    }
    mask_name_ = mask_file;
    stream_file_ = stream_file;
    // outputs
    flowin_index_name_ = prefix + "_FLOWIN_INDEX_MFDMD";
    flowin_frac_name_ = prefix + "_FLOWIN_FRACTION_MFDMD";
    flowout_index_name_ = prefix + "_FLOWOUT_INDEX_MFDMD";
    flowout_frac_name_ = prefix + "_FLOWOUT_FRACTION_MFDMD";
    layering_updown_name_ = prefix + "_ROUTING_LAYERS_UP_DOWN_MFDMD";
    layering_downup_name_ = prefix + "_ROUTING_LAYERS_DOWN_UP_MFDMD";
}

GridLayeringMFDmd::~GridLayeringMFDmd() {
    delete flow_fraction_;
    if (nullptr != flowin_fracs_) Release1DArray(flowin_fracs_);
    if (nullptr != flowout_fracs_) Release1DArray(flowout_fracs_);
}

bool GridLayeringMFDmd::LoadData() {
    if (use_mongo_) {
#ifdef USE_MONGODB
        has_mask_ = true;
        mask_ = FloatRaster::Init(gfs_, mask_name_.c_str(), true);
        flowdir_ = FltMaskFltRaster::Init(gfs_, flowdir_name_.c_str(), true, mask_, true);
        flow_fraction_ = FltMaskFltRaster::Init(gfs_, flowfrac_corename_.c_str(),
                                                true, mask_, true);
#else
        return false;
#endif
    } else {
        for (vector<string>::iterator it = flowfrac_names_.begin(); it != flowfrac_names_.end(); ++it) {
            if (!FileExists(*it)) {
                cout << *it << " not exist!\n";
                return false;
            }
        }
        if (StringMatch(flowdir_name_, mask_name_)) {
            flowdir_ = FloatRaster::Init(flowdir_name_, true);
            mask_ = flowdir_;
        } else {
            has_mask_ = true;
            mask_ = FloatRaster::Init(mask_name_, true);
            flowdir_ = FltMaskFltRaster::Init(flowdir_name_, true, mask_, true);
        }
        flow_fraction_ = FltMaskFltRaster::Init(flowfrac_names_, true, mask_, true);
    }
    if (nullptr == flowdir_ || nullptr == flow_fraction_ || nullptr == mask_) return false;

    n_rows_ = mask_->GetRows();
    n_cols_ = mask_->GetCols();
    mask_->GetRasterPositionData(&n_valid_cells_, &pos_rowcol_);

    flowdir_matrix_ = flowdir_->GetRasterDataPointer();
    if (FloatEqual(flowdir_->GetNoDataValue(), out_nodata_)) flowdir_->ReplaceNoData(out_nodata_);
    flowfrac_matrix_ = flow_fraction_->Get2DRasterDataPointer();
    if (flowdir_->GetValidNumber() != flow_fraction_->GetValidNumber()) {
        cout << "The valid cell number must be the same between "
                "MFD-md flow direction and flow fraction raster data!" << endl;
        return false;
    }

    // Force stream grid to flow into single downstream cell
    if (stream_file_.empty())
        return true;

    vector<vector<ROW_COL> > stream_rc;
    bool flag = read_stream_vertexes_as_rowcol(stream_file_, mask_, stream_rc);
    if (!flag) return false;

    for (vector<vector<ROW_COL> >::iterator it = stream_rc.begin(); it != stream_rc.end(); ++it) {
        if (it->size() < 2) continue; // A line should have at least two points!
        for (vector<ROW_COL>::reverse_iterator it2 = it->rbegin(); it2 != it->rend() - 1; ++it2) {
            // FOR TEST ONLY
            // cout << "(" << it2->first << ", " << it2->second << "), ";
            // XY_COOR xy = mask_->GetCoordinateByRowCol(it2->first, it2->second);
            // printf("(%.1f, %.1f), ", xy.first, xy.second);
            // cout << "(" << it2->first << ", " << it2->second << ") -> (" << (it2+1)->first << ", " << (it2+1)->second << "), ";
            int delta_row = (it2 + 1)->first - it2->first;
            int delta_col = (it2 + 1)->second - it2->second;
            int fd_index = find_flow_direction_index_ccw(delta_row, delta_col);
            //cout << flowdir_->GetValue(it2->first, it2->second) << ": ";
            //print_flow_fractions_mfdmd(flow_fraction_, it2->first, it2->second);
            if (fd_index < 0) {
                flowdir_->SetValue(it2->first, it2->second, -1.f);
                for (int i = 1; i <= 8; i++) {
                    flow_fraction_->SetValue(it2->first, it2->second,
                                             flow_fraction_->GetNoDataValue(), i);
                }
            }
            else {
                flowdir_->SetValue(it2->first, it2->second, CVT_FLT(fdccw[fd_index]));
                for (int i = 1; i <= 8; i++) {
                    flow_fraction_->SetValue(it2->first, it2->second, -1.f, i);
                }
                flow_fraction_->SetValue(it2->first, it2->second, 1.f, fd_index);
            }
            //cout << " --> " << flowdir_->GetValue(it2->first, it2->second) << ": ";
            //print_flow_fractions_mfdmd(flow_fraction_, it2->first, it2->second);
            //cout << endl;
        }
        // cout << endl;
    }

    return true;
}

bool GridLayeringMFDmd::OutputFlowIn() {
    GetReverseDirMatrix();
    CountFlowInCells();
    if (!BuildFlowInCellsArray()) return false;

    int datalength = n_valid_cells_ + flow_in_count_ + 1;
    if (nullptr == flowin_fracs_) Initialize1DArray(datalength, flowin_fracs_, 0.f);
    flowin_fracs_[0] = CVT_FLT(n_valid_cells_);
    int count = 1;
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        int i = pos_rowcol_[valid_idx][0]; // row
        int j = pos_rowcol_[valid_idx][1]; // col
        flowin_fracs_[count++] = CVT_FLT(flow_in_num_[valid_idx]);
        if (flow_in_num_[valid_idx] == 0) continue;

        int reversed_fdir = CVT_INT(reverse_dir_[valid_idx]);
        if (reversed_fdir < 0) continue; // This will not happen, just in case!

        vector<int> reversed_fdirs = uncompress_flow_directions(reversed_fdir);
        for (vector<int>::iterator it = reversed_fdirs.begin(); it != reversed_fdirs.end(); ++it) {
            int rfd_idx = find_flow_direction_index_ccw(*it);
            int source_row = i + drow[rfd_idx];
            int source_col = j + dcol[rfd_idx];
            if (!mask_->ValidateRowCol(source_row, source_col) ||
                mask_->IsNoData(source_row, source_col) ||
                flowdir_->IsNoData(source_row, source_col))
                continue;
            int source_index = pos_index_[source_row * n_cols_ + source_col];
            int source_fdir = CVT_INT(flowdir_matrix_[source_index]);
            vector<int> source_fdirs = uncompress_flow_directions(source_fdir);
            int srcfout_dir = get_reversed_fdir(*it);
            if (std::find(source_fdirs.begin(), source_fdirs.end(),
                          srcfout_dir) == source_fdirs.end()) {
                continue;
            }
            int srcfout_idx = find_flow_direction_index_ccw(srcfout_dir);
            if (flowfrac_matrix_[source_index][srcfout_idx - 1] < 0) continue;
            flowin_fracs_[count++] = flowfrac_matrix_[source_index][srcfout_idx - 1];
        }
    }

    if (count != datalength) {
        cout << "Build flow in fraction array failed!" << endl;
        return false;
    }

    string header = "ID\tUpstreamCount\tUpstreamID\tFlowInFraction";
    bool done = Output2DimensionArrayTxt(flowin_index_name_, header, flow_in_cells_, flowin_fracs_);
    if (use_mongo_) {
#ifdef USE_MONGODB
        done = done && OutputArrayAsGfs(flowin_index_name_, flow_in_count_ + n_valid_cells_ + 1, flow_in_cells_) &&
                OutputArrayAsGfs(flowin_frac_name_, flow_in_count_ + n_valid_cells_ + 1, flowin_fracs_);
#endif
    }
    return done;
}

bool GridLayeringMFDmd::OutputFlowOut() {
    CountFlowOutCells();
    if (!BuildFlowOutCellsArray()) return false;

    if (nullptr == flowout_fracs_) Initialize1DArray(flow_out_count_ + n_valid_cells_ + 1, flowout_fracs_, 0.f);
    flowout_fracs_[0] = CVT_FLT(n_valid_cells_);
    int count = 1;
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        int i = pos_rowcol_[valid_idx][0];                           // row
        int j = pos_rowcol_[valid_idx][1];                           // col
        flowout_fracs_[count++] = CVT_FLT(flow_out_num_[valid_idx]); // maybe 0
        if (flow_out_num_[valid_idx] == 0) continue;

        int flow_dir = CVT_INT(flowdir_matrix_[valid_idx]);
        if (flow_dir < 0) continue; // This will not happen, just in case!

        vector<int> flow_dirs = uncompress_flow_directions(flow_dir);
        if (flow_dirs.empty()) continue;

        int cur_fdir_count = 0;
        for (vector<int>::iterator it_fdir = flow_dirs.begin(); it_fdir != flow_dirs.end(); ++it_fdir) {
            int fd_idx = find_flow_direction_index_ccw(*it_fdir);
            if (!mask_->ValidateRowCol(i + drow[fd_idx], j + dcol[fd_idx]) ||
                mask_->IsNoData(i + drow[fd_idx], j + dcol[fd_idx])) {
                continue;
            }
            float curfract = flowfrac_matrix_[valid_idx][fd_idx - 1];
            if (curfract < 0) {
                cout << "No flow fraction found in the flow direction, "
                        "valid index: " << valid_idx << ", row: " << pos_rowcol_[valid_idx][0] <<
                        ", col: " << pos_rowcol_[valid_idx][1] <<
                        ", flow out count: " << flow_out_num_[valid_idx] <<
                        ", compressed direction: " << flowdir_matrix_[valid_idx] <<
                        ", component direction: " << fdccw[fd_idx] << "\n";
                return false;
            }
            flowout_fracs_[count++] = curfract;
            cur_fdir_count++;
        }
    }

    if (count != flow_out_count_ + n_valid_cells_ + 1) {
        cout << "Build flow out fraction array failed!" << endl;
        return false;
    }

    string header = "ID\tDownstreamCount\tDownstreamID\tFlowOutFraction";
    bool done = Output2DimensionArrayTxt(flowout_index_name_, header, flow_out_cells_, flowout_fracs_);
    if (use_mongo_) {
#ifdef USE_MONGODB
        done = OutputArrayAsGfs(flowout_index_name_, flow_out_count_ + n_valid_cells_ + 1, flow_out_cells_) &&
                OutputArrayAsGfs(flowout_frac_name_, flow_out_count_ + n_valid_cells_ + 1, flowout_fracs_);
#endif
    }
    return done;
}
