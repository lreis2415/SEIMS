#include "GridLayering.h"
#include "NormalizeFlowFractions.h"

#ifdef USE_MONGODB
GridLayeringMFDmd::GridLayeringMFDmd(const int id, MongoGridFs* gfs, const char* out_dir,
                                     const char* stream_file/*=nullptr*/,
                                     bool force_outlet/*=false*/, bool force_inbasin/*=true*/, int decimals/*=4*/) :
    GridLayering(id, gfs, out_dir), flow_fraction_(nullptr), flowfrac_matrix_(nullptr), flowin_fracs_(nullptr),
    flowout_fracs_(nullptr) {
    // outputs
    OutputFilenames(FD_MFDmd);
    // inputs
    string prefix = ValueToString(subbasin_id_);
    flowdir_name_ = prefix + "_FLOW_DIR_" + fdtype_str_;
    flowfrac_corename_ = prefix + "_FLOW_FRACTION_" + fdtype_str_;
    mask_name_ = prefix + "_SUBBASIN";
    stream_file_ = stream_file;
    force_outlet_ = force_outlet;
    force_inbasin_ = force_inbasin;
    decimals_ = decimals;
}
#endif

GridLayeringMFDmd::GridLayeringMFDmd(const int id, const char* out_dir,
                                     const char* fd_file, const char* fraction_file,
                                     const char* mask_file/*=nullptr*/, const char* stream_file/*=nullptr*/,
                                     bool force_outlet/*=false*/, bool force_inbasin/*=true*/, int decimals/*=4*/) :
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
    force_outlet_ = force_outlet;
    force_inbasin_ = force_inbasin;
    decimals_ = decimals;
    // outputs
    OutputFilenames(FD_MFDmd);
}

GridLayeringMFDmd::~GridLayeringMFDmd() {
    delete flow_fraction_;
    if (nullptr != flowin_fracs_) Release1DArray(flowin_fracs_);
    if (nullptr != flowout_fracs_) Release1DArray(flowout_fracs_);
}

void GridLayeringMFDmd::OutputFilenames(flowDirTypes ftype) {
    GridLayering::OutputFilenames(ftype);
    string prefix = ValueToString(subbasin_id_);
    flowin_frac_name_ = prefix + "_FLOWIN_FRACTION_" + fdtype_str_;
    flowout_frac_name_ = prefix + "_FLOWOUT_FRACTION_" + fdtype_str_;
}

bool GridLayeringMFDmd::LoadData() {
    if (use_mongo_) {
#ifdef USE_MONGODB
        has_mask_ = true;
        mask_ = FloatRaster::Init(gfs_, mask_name_.c_str(), true);
        STRING_MAP opts;
        UpdateStringMap(opts, HEADER_INC_NODATA, "FALSE");
        flowdir_ = FloatRaster::Init(gfs_, flowdir_name_.c_str(),
                                          true, mask_, true, NODATA_VALUE, opts);
        flow_fraction_ = FloatRaster::Init(gfs_, flowfrac_corename_.c_str(),
                                                true, mask_, true, NODATA_VALUE, opts);
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
            flowdir_ = FloatRaster::Init(flowdir_name_, true, mask_, true);
        }
        flow_fraction_ = FloatRaster::Init(flowfrac_names_, true, mask_, true);
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

    // Force stream grid to flow into a single downstream cell
    if (stream_file_.empty())
        return true;

    if (nullptr == stream_matrix_) Initialize1DArray(n_valid_cells_, stream_matrix_, mask_->GetNoDataValue());
    vector<vector<ROW_COL> > stream_rc;
    bool flag = read_stream_vertexes(stream_file_, mask_, stream_rc, stream_matrix_);
    if (!flag) return false;

    for (vector<vector<ROW_COL> >::iterator it = stream_rc.begin(); it != stream_rc.end(); ++it) {
        if (it->size() < 2) continue; // A line should have at least two points!
        for (vector<ROW_COL>::reverse_iterator it2 = it->rbegin(); it2 != it->rend() - 1; ++it2) {
            int delta_row = (it2 + 1)->first - it2->first;
            int delta_col = (it2 + 1)->second - it2->second;
            int fd_index = find_flow_direction_index_ccw(delta_row, delta_col);
            if (fd_index < 0) {
                flowdir_->SetValue(it2->first, it2->second, -1.f);
                for (int i = 1; i <= 8; i++) {
                    flow_fraction_->SetValue(it2->first, it2->second,
                                             flow_fraction_->GetNoDataValue(), i);
                }
            } else {
                flowdir_->SetValue(it2->first, it2->second, CVT_FLT(fdccw[fd_index]));
                for (int i = 1; i <= 8; i++) {
                    flow_fraction_->SetValue(it2->first, it2->second, -1.f, i);
                }
                flow_fraction_->SetValue(it2->first, it2->second, 1.f, fd_index);
            }
        }
    }

    return true;
}

bool GridLayeringMFDmd::OutputFlowIn() {
    GetReverseDirMatrix();
    if (!BuildFlowInCellsArray()) return false;

    int datalength = n_valid_cells_ + flow_in_count_ + 1;
    if (nullptr == flowin_fracs_) Initialize1DArray(datalength, flowin_fracs_, 0.f);
    flowin_fracs_[0] = CVT_FLT(n_valid_cells_);
    int count = 1;
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        flowin_fracs_[count++] = CVT_FLT(flow_in_num_[valid_idx]);
        if (flow_in_num_[valid_idx] == 0) continue;
        for (int iin = 0; iin < flow_in_num_[valid_idx]; iin++) {
            int in_cell_idx = 1 + valid_idx + 1 + iin;
            if (valid_idx > 0) in_cell_idx += flow_in_acc_[valid_idx - 1];
            int source_index = CVT_INT(flow_in_cells_[in_cell_idx]);
            int fd_idx = find_flow_direction_index_ccw(pos_rowcol_[valid_idx][0] - pos_rowcol_[source_index][0],
                                                       pos_rowcol_[valid_idx][1] - pos_rowcol_[source_index][1]);

            float flowfrac = flowfrac_matrix_[source_index][fd_idx - 1];
            if (flowfrac < 0) continue;
            flowin_fracs_[count++] = flowfrac;
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

    if (nullptr == flowout_fracs_) {
        Initialize1DArray(flow_out_count_ + n_valid_cells_ + 1, flowout_fracs_, 0.f);
    }
    flowout_fracs_[0] = CVT_FLT(n_valid_cells_);
    int count = 1;
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        int i = pos_rowcol_[valid_idx][0];                           // row
        int j = pos_rowcol_[valid_idx][1];
        int flowcount = flow_out_num_[valid_idx];
        flowout_fracs_[count++] = CVT_FLT(flowcount); // maybe 0
        if (flowcount <= 0) {
            flowout_fracs_[count - 1] = 0.;
            continue;
        }
        vector<FLTPT> fracin(flowcount, 0.);
        vector<int> flowidxs(flowcount, -1);
        for (int iout = 0; iout < flow_out_num_[valid_idx]; iout++) {
            int down_cell_idx = 1 + valid_idx + 1 + iout;
            if (valid_idx > 0) {
                down_cell_idx += flow_out_acc_[valid_idx - 1];
            }
            int down_cell = CVT_INT(flow_out_cells_[down_cell_idx]);
            int fd_idx = find_flow_direction_index_ccw(pos_rowcol_[down_cell][0] - i,
                                                       pos_rowcol_[down_cell][1] - j);
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
            fracin[iout] = curfract;
            flowidxs[iout] = fd_idx - 1;
        }
        vector<FLTPT> fracout;
        normalize_flow_fraction(fracin, decimals_, fracout);
        for (int flow_idx = flowcount - 1; flow_idx >= 0; flow_idx--) {
            int actual_idx = flowcount - 1 - flow_idx;
            flowout_fracs_[count - 1 - flow_idx] = fracout[actual_idx];
            flowfrac_matrix_[valid_idx][flowidxs[actual_idx]] = fracout[actual_idx]; // write back
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
