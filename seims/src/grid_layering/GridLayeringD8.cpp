#include "GridLayering.h"

#ifdef USE_MONGODB
GridLayeringD8::GridLayeringD8(const int id, MongoGridFs* gfs, const char* out_dir,
                               const char* stream_file/*=nullptr*/, bool force_outlet/*=false*/) :
    GridLayering(id, gfs, out_dir) {
    string prefix = ValueToString(subbasin_id_);
    flowdir_name_ = prefix + "_FLOW_DIR";
    mask_name_ = prefix + "_SUBBASIN";
    stream_file_ = stream_file;
    force_outlet_ = force_outlet;
    OutputFilenames(FD_D8);
}
#endif

GridLayeringD8::GridLayeringD8(int id, const char* out_dir, const char* in_file,
                               const char* mask_file/*=nullptr*/, const char* stream_file/*=nullptr*/,
                               bool force_outlet/*=false*/) :
    GridLayering(id, out_dir) {
    flowdir_name_ = in_file;
    mask_name_ = mask_file;
    stream_file_ = stream_file;
    force_outlet_ = force_outlet;
    OutputFilenames(FD_D8);
}

GridLayeringD8::~GridLayeringD8() {
    // Nothing to do.
}


bool GridLayeringD8::LoadData() {
    if (use_mongo_) {
#ifdef USE_MONGODB
        has_mask_ = true;
        mask_ = FloatRaster::Init(gfs_, mask_name_.c_str(), true);
        STRING_MAP opts;
        UpdateStringMap(opts, HEADER_INC_NODATA, "FALSE");
        flowdir_ = FloatRaster::Init(gfs_, flowdir_name_.c_str(),
                                     true, mask_, true, NODATA_VALUE, opts);
#else
        return false;
#endif
    } else {
        if (StringMatch(flowdir_name_, mask_name_)) {
            flowdir_ = FloatRaster::Init(flowdir_name_, true);
            mask_ = flowdir_;
        } else {
            has_mask_ = true;
            mask_ = FloatRaster::Init(mask_name_, true);
            flowdir_ = FloatRaster::Init(flowdir_name_, true, mask_, true);
        }
    }
    if (nullptr == flowdir_ || nullptr == mask_) { return false; }
    n_rows_ = mask_->GetRows();
    n_cols_ = mask_->GetCols();
    mask_->GetRasterPositionData(&n_valid_cells_, &pos_rowcol_);

    flowdir_matrix_ = flowdir_->GetRasterDataPointer();
    if (FloatEqual(flowdir_->GetNoDataValue(), out_nodata_)) {
        flowdir_->ReplaceNoData(out_nodata_);
    }

    // Create stream grid from Shapefile
    if (stream_file_.empty()) {
        return true;
    }

    if (nullptr == stream_matrix_) Initialize1DArray(n_valid_cells_, stream_matrix_, mask_->GetNoDataValue());
    vector<vector<ROW_COL> > stream_rc;
    bool flag = read_stream_vertexes(stream_file_, mask_, stream_rc, stream_matrix_);
    if (!flag) return false;

    return true;
}
