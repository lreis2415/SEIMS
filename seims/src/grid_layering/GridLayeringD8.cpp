#include "GridLayering.h"

#ifdef USE_MONGODB
GridLayeringD8::GridLayeringD8(const int id, MongoGridFs* gfs, const char* out_dir) :
    GridLayering(id, gfs, out_dir) {
    string prefix = ValueToString(subbasin_id_);
    flowdir_name_ = prefix + "_FLOW_DIR";
    mask_name_ = prefix + "_MASK";
    flowin_index_name_ = prefix + "_FLOWIN_INDEX_D8";
    flowout_index_name_ = prefix + "_FLOWOUT_INDEX_D8";
    layering_updown_name_ = prefix + "_ROUTING_LAYERS_UP_DOWN_D8";
    layering_downup_name_ = prefix + "_ROUTING_LAYERS_DOWN_UP_D8";
    layering_evenly_name_ = prefix + "_ROUTING_LAYERS_EVEN_D8";
}
#endif

GridLayeringD8::GridLayeringD8(const int id, const char* in_file, const char* mask_file, const char* out_dir) :
    GridLayering(id, out_dir) {
    flowdir_name_ = in_file;
    mask_name_ = mask_file;
    string prefix = ValueToString(subbasin_id_);
    flowin_index_name_ = prefix + "_FLOWIN_INDEX_D8";
    flowout_index_name_ = prefix + "_FLOWOUT_INDEX_D8";
    layering_updown_name_ = prefix + "_ROUTING_LAYERS_UP_DOWN_D8";
    layering_downup_name_ = prefix + "_ROUTING_LAYERS_DOWN_UP_D8";
    layering_evenly_name_ = prefix + "_ROUTING_LAYERS_EVEN_D8";
}

GridLayeringD8::~GridLayeringD8() {
    // Nothing to do.
}


bool GridLayeringD8::LoadData() {
    if (use_mongo_) {
#ifdef USE_MONGODB
        has_mask_ = true;
        mask_ = FloatRaster::Init(gfs_, mask_name_.c_str(), true);
        flowdir_ = FltMaskFltRaster::Init(gfs_, flowdir_name_.c_str(), 
                                          true, mask_, true);
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
            flowdir_ = FltMaskFltRaster::Init(flowdir_name_, true, mask_, true);
        }
    }
    if (nullptr == flowdir_ || nullptr == mask_) return false;
    n_rows_ = mask_->GetRows();
    n_cols_ = mask_->GetCols();
    mask_->GetRasterPositionData(&n_valid_cells_, &pos_rowcol_);

    flowdir_matrix_ = flowdir_->GetRasterDataPointer();
    if (FloatEqual(flowdir_->GetNoDataValue(), out_nodata_)) flowdir_->ReplaceNoData(out_nodata_);
    return true;
}
