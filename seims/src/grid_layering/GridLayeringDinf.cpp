#include "GridLayering.h"
#include "NormalizeFlowFractions.h"

#ifdef USE_MONGODB
GridLayeringDinf::GridLayeringDinf(const int id, MongoGridFs* gfs, const char* out_dir,
                                   const char* stream_file/*=nullptr*/,
                                   int decimals/*=4*/) :
    GridLayering(id, gfs, out_dir) {
    fdtype_str_ = "DINF";
    // outputs
    OutputFilenames();
    // inputs
    string prefix = ValueToString(subbasin_id_);
    flowdir_name_ = prefix + "_FLOW_DIR_" + fdtype_str_;
    flowfrac_name_ = prefix + "_WEIGHT_" + fdtype_str_;
    mask_name_ = prefix + "_SUBBASIN";
    dem_file_ = prefix + "_DEM";
    stream_file_ = stream_file;
    decimals_ = decimals;
}
#endif

GridLayeringDinf::GridLayeringDinf(const int id, const char* out_dir,
                                   const char* fd_file, const char* fraction_file,
                                   const char* mask_file/*=nullptr*/, const char* stream_file/*=nullptr*/,
                                   const char* dem_file/*=nullptr*/,
                                   int decimals/*=4*/) :
    GridLayering(id, out_dir) {
    fdtype_str_ = "DINF";
    string prefix = ValueToString(subbasin_id_);
    // inputs
    flowdir_name_ = fd_file;
    flowfrac_name_ = fraction_file;
    mask_name_ = mask_file;
    stream_file_ = stream_file;
    dem_file_ = dem_file;
    decimals_ = decimals;
    // outputs
    OutputFilenames();
}

GridLayeringDinf::~GridLayeringDinf() {
    if (nullptr != flowfrac_matrix_) Release2DArray(flowfrac_matrix_);
}


bool GridLayeringDinf::LoadData() {
    if (use_mongo_) {
#ifdef USE_MONGODB
        has_mask_ = true;
        mask_ = IntRaster::Init(gfs_, mask_name_.c_str(), true);
        STRING_MAP opts;
        UpdateStringMap(opts, HEADER_INC_NODATA, "FALSE");
        flowdir_ = IntRaster::Init(gfs_, flowdir_name_.c_str(), true,
                                   mask_, true, NODATA_VALUE, opts);
        flow_fraction_ = FloatRaster::Init(gfs_, flowfrac_name_.c_str(), true,
                                           mask_, true, NODATA_VALUE, opts);
        dem_ = FloatRaster::Init(gfs_, dem_file_.c_str(),
                                 true, mask_, true, NODATA_VALUE, opts);
#else
        return false;
#endif
    } else {
        if (StringMatch(flowdir_name_, mask_name_)) {
            flowdir_ = IntRaster::Init(flowdir_name_, true);
            mask_ = flowdir_;
        } else {
            has_mask_ = true;
            mask_ = IntRaster::Init(mask_name_, true);
            flowdir_ = IntRaster::Init(flowdir_name_, true, mask_, true);
        }
        flow_fraction_ = FloatRaster::Init(flowfrac_name_, true, mask_, true);
        if (!dem_file_.empty()) {
            dem_ = FloatRaster::Init(dem_file_, true, mask_, true);
        }
    }
    if (nullptr == flowdir_ || nullptr == flow_fraction_ || nullptr == mask_) return false;

    n_rows_ = mask_->GetRows();
    n_cols_ = mask_->GetCols();
    mask_->GetRasterPositionData(&n_valid_cells_, &pos_rowcol_);

    flowdir_matrix_ = flowdir_->GetRasterDataPointer();
    if (FloatEqual(flowdir_->GetNoDataValue(), out_nodata_)) flowdir_->ReplaceNoData(out_nodata_);
    FLTPT* flowfrac_matrix_org = flow_fraction_->GetRasterDataPointer();
    if (flowdir_->GetValidNumber() != flow_fraction_->GetValidNumber()) {
        cout << "The valid cell number must be the same between "
                "Dinf flow direction and flow fraction raster data!" << endl;
        return false;
    }
    // Preprocessing flow fraction matrix to 2D array
    Initialize2DArray(n_valid_cells_, 8, flowfrac_matrix_, out_nodata_);
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        int flow_dir = flowdir_matrix_[valid_idx];
        if (flow_dir < 0) {
            continue; // This will not happen, just in case!
        }
        vector<int> flow_dirs = uncompress_flow_directions(flow_dir);
        if (flow_dirs.empty()) {
            continue; // This will not happen, just in case!
        }
        if (CVT_INT(flow_dirs.size()) == 1) { // Only one downslope cell
            int fd_idx = find_flow_direction_index_ccw(flow_dirs[0]);
            flowfrac_matrix_[valid_idx][fd_idx - 1] = 1.;
            flowfrac_matrix_org[valid_idx] = 1.; // currently, no further used
            continue;
        }
        int fd_idx = find_flow_direction_index_ccw(flow_dirs[0]);
        int fd_idx2 = find_flow_direction_index_ccw(flow_dirs[1]);
        // We don't need to consider the downslope cell is a valid cell or nodata here!
        vector<FLTPT> fracin(2);
        fracin[0] = flowfrac_matrix_org[valid_idx];
        fracin[1] = 1. - flowfrac_matrix_org[valid_idx];
        vector<FLTPT> fracout;
        normalize_flow_fraction(fracin, decimals_, fracout);
        flowfrac_matrix_[valid_idx][fd_idx - 1] = fracout[0];
        flowfrac_matrix_[valid_idx][fd_idx2 - 1] = fracout[1];
        flowfrac_matrix_org[valid_idx] = fracout[0]; // write back, although, currently, no further used
    }
    // Calculate pos_index_, full size length (rows * cols) and two columns
    CalPositionIndex();
    // Force stream cell only flow into downstream cell
    return LoadChannelData();
}
