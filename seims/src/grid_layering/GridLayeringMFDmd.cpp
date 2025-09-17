#include "GridLayering.h"

#ifdef USE_MONGODB
GridLayeringMFDmd::GridLayeringMFDmd(const int id, MongoGridFs* gfs, const char* out_dir,
                                     const char* stream_file/*=nullptr*/,
                                     int decimals/*=4*/,
                                     string fdir_name/*=""*/) :
    GridLayering(id, gfs, out_dir) {
    fdtype_str_ = "MFDMD";
    if (fdir_name != "") { fdtype_str_ = fdir_name; }
    // outputs
    OutputFilenames();
    // inputs
    string prefix = ValueToString(subbasin_id_);
    flowdir_name_ = prefix + "_FLOW_DIR_" + fdtype_str_;
    flowfrac_corename_ = prefix + "_FLOW_FRACTION_" + fdtype_str_;
    mask_name_ = prefix + "_SUBBASIN";
    stream_file_ = stream_file;
    decimals_ = decimals;
}
#endif

GridLayeringMFDmd::GridLayeringMFDmd(const int id, const char* out_dir,
                                     const char* fd_file, const char* fraction_file,
                                     const char* mask_file/*=nullptr*/, const char* stream_file/*=nullptr*/,
                                     int decimals/*=4*/,
                                     string fdir_name/*=""*/) :
    GridLayering(id, out_dir) {
    fdtype_str_ = "MFDMD";
    if (fdir_name != "") { fdtype_str_ = fdir_name; }
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
    decimals_ = decimals;
    // outputs
    OutputFilenames();
}

GridLayeringMFDmd::~GridLayeringMFDmd() {
    // Anything to do here.
}

bool GridLayeringMFDmd::LoadData() {
    if (use_mongo_) {
#ifdef USE_MONGODB
        has_mask_ = true;
        mask_ = IntRaster::Init(gfs_, mask_name_.c_str(), true);
        STRING_MAP opts;
        UpdateStringMap(opts, HEADER_INC_NODATA, "FALSE");
        flowdir_ = IntRaster::Init(gfs_, flowdir_name_.c_str(),
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
            flowdir_ = IntRaster::Init(flowdir_name_, true);
            mask_ = flowdir_;
        } else {
            has_mask_ = true;
            mask_ = IntRaster::Init(mask_name_, true);
            flowdir_ = IntRaster::Init(flowdir_name_, true, mask_, true);
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
    // Calculate pos_index_, full size length (rows * cols) and two columns
    CalPositionIndex();
    // Force stream cell only flow into downstream cell
    return LoadChannelData();
}
