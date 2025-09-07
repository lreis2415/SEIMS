/*!
 * \brief Grid layering class.
 * \author original, Junzhi Liu, 29-July-2012
 * \author Liangjun Zhu, since 21-July-2016
 *          lj - 28-Dec-2017 - Refactor as class.\n
 *          lj -  5-Mar-2018 - Use CCGL, and reformat code style.\n
 *          lj - 31-Mar-2021 - Rewrite most core parts and now support MFD-md algorithm.\n
 *          lj -  7-Apr-2021 - Since all spatial raster data is stored in float type in MongoDB,
 *                              for cross-platform compatible, both MongoDB and File mode use FloatRaster.\n
 *          lj - 18-May-2021 - Force each stream grid flow into one downstream grid.\n
 *          lj - 30-Jul-2021 - Add new layering method named _EVEN.\n
 * \description:
 *               Output lists of both local files and MongoDB GridFS:
 *               1. X_FLOWOUT_INDEX_{FD}, X_FLOWIN_INDEX_{FD}
 *               2. X_ROUTING_LAYERS_UP_DOWN{_FD}, X_ROUTING_LAYERS_DOWN_UP{_FD}, and X_ROUTING_LAYERS_EVEN{_FD}
 *               3. X_FLOWIN_FRACTION_{FD}, X_FLOWOUT_FRACTION_{FD}. For `DINF` and `MFDMD`.
 *               Where, `X` is subbasinID (0 for the whole basin)
 *                      `FD` is the flow direction algorithm, include `D8`, `DINF`, and `MFDMD`.
 *
 */

#ifndef GRID_LAYERING_H
#define GRID_LAYERING_H

#include "data_raster.hpp"

#include "gdal_handler.h"

using namespace ccgl;
using namespace data_raster;

#ifdef USE_FLOAT64
typedef double FLTPT;
#else
typedef float FLTPT;
#endif

#ifndef IntRaster
#define IntRaster   clsRasterData<int>
#endif
#ifndef FloatRaster
#define FloatRaster clsRasterData<float>
#endif
#ifndef FltIntRaster
#define FltIntRaster clsRasterData<float, int>
#endif
#ifndef IntFltRaster
#define IntFltRaster clsRasterData<int, float>
#endif
//
// #ifdef IntRaster
// #undef IntRaster
// #endif
// #ifndef IntRaster
// /*! Integer-typed raster */
// #define IntRaster   ccgl::data_raster::clsRasterData<int>
// #endif
// #ifdef FloatRaster
// #undef FloatRaster
// #endif
// #ifndef FloatRaster
// /*! Float-typed raster with int-typed mask, specific for legacy SEIMS code */
// #define FloatRaster ccgl::data_raster::clsRasterData<FLTPT, int>
// #endif

/*!
* \enum flowDirTypes
* \brief Algorithm of flow direction
*/
enum flowDirTypes {
    FD_D8 = 0,   /**< Default, D8 single flow direction */
    FD_Dinf = 1, /**< Dinf (Tarboton, 1997) */
    FD_MFDmd = 2 /**< MFD-md (Qin et al., 2007) */
};

// flow direction coding system in ArcGIS
// 32  64 128
// 16   x   1
//  8   4   2

// The indexes of fdccw follow the flow direction coding system in TauDEM
//  4   3   2
//  5   x   1
//  6   7   8
const int fdccw[9] = {0, 1, 128, 64, 32, 16, 8, 4, 2};
const int drow[9] = {0, 0, -1, -1, -1, 0, 1, 1, 1};
const int dcol[9] = {0, 1, 1, 0, -1, -1, -1, 0, 1};

int find_flow_direction_index_ccw(int fd);

int find_flow_direction_index_ccw(int delta_row, int delta_col);

int get_reversed_fdir(int fd);

vector<int> uncompress_flow_directions(int compressed_fd);

bool read_stream_vertexes(string stream_file, FloatRaster* mask,
                          vector<vector<ROW_COL> >& stream_rc,
                          float*& stream_matrix);

void print_flow_fractions_mfdmd(FloatRaster* ffrac, int row, int col);

// ---------- Candidate score ----------
struct Candidate {
    int u; // cell ID (0-based)
    double score; // higher is better
    int slack; // towards moving direction, layer count allowed to move, i.e., |t - s|
    int window; // dynamic width, i.e., high[u] - low[u]
    int t; // target layerID (0-based)
};

struct CandidateCmp {
    bool operator()(const Candidate& a, const Candidate& b) const {
        if (FloatEqual(a.score, b.score)) return a.score > b.score; // 1st priority: score
        if (a.slack != b.slack) return a.slack > b.slack; // 2nd priority: layer count allowed to move
        if (a.window != b.window) return a.window > b.window; // 3rd priority: width
        return a.u < b.u; // 4th and deterministic priority: lower ID
    }
};

static int sgn(int x) { return (x > 0) - (x < 0); }

static inline long long Phi_L1(const vector<int>& Count, const vector<int>& T) {
    long long s = 0;
    for (size_t i = 0; i < Count.size(); i++) {
        s += Abs(static_cast<long long>(Count[i]) - T[i]);
    }
    return s;
}
static inline long long Psi_L2(const vector<int>& Count, const vector<int>& T) {
    long long s = 0;
    for (size_t i = 0; i < Count.size(); i++) {
        long long d = static_cast<long long>(Count[i]) - T[i];
        s += d * d;
    }
    return s;
}
// only calculate L1/L2 of movable prefix layers [0, prefix_end)
static inline long long Phi_L1_Prefix(const vector<int>& Count, const vector<int>& T, int prefix_end) {
    long long s = 0;
    for (int i = 0; i < prefix_end; i++) {
        s += Abs(static_cast<long long>(Count[i]) - T[i]);
    }
    return s;
}
static inline long long Psi_L2_Prefix(const vector<int>& Count, const vector<int>& T, int prefix_end) {
    long long s = 0;
    for (int i = 0; i < prefix_end; i++) {
        long long d = static_cast<long long>(Count[i]) - T[i];
        s += d * d;
    }
    return s;
}

enum DeficitScanMode {
    NEAREST_FIRST = 0,
    FARTHEST_FIRST = 1,
    BIGGEST_DEFICIT_FIRST = 2,
    BEST_BATCH_SCORE = 3
};

struct DistOrderCmp {
    int s;
    bool farthest;
    DistOrderCmp(int s_, bool f_) : s(s_), farthest(f_) {}
    bool operator()(const std::pair<int,int>& a, const std::pair<int,int>& b) const {
        if (a.first != b.first) return farthest ? (a.first > b.first) : (a.first < b.first);
        bool ad = (a.second > s);
        bool bd = (b.second > s);
        if (ad != bd) return ad > bd; // downstream first
        return a.second < b.second;
    }
};

struct BigDefCmp {
    const vector<int>* S; int s;
    BigDefCmp(const vector<int>& S_, int s_) : S(&S_), s(s_) {}
    bool operator()(int da, int db) const {
        int a = -(*S)[da];
        int b = -(*S)[db];
        if (a != b) return a > b;
        int daDist = Abs(da - s), dbDist = Abs(db - s);
        if (daDist != dbDist) return daDist < dbDist;
        bool ad = (da > s);
        bool bd = (db > s);
        if (ad != bd) return ad > bd;
        return da < db;
    }
};

struct UPick { int u, t; double score; int slack, window; };
struct UPickCmp {
    bool operator()(const UPick& a, const UPick& b) const {
        if (a.score  != b.score)  return a.score  > b.score;
        if (a.slack  != b.slack)  return a.slack  > b.slack;
        if (a.window != b.window) return a.window > b.window;
        return a.u < b.u;
    }
};

static void BuildTargetsCappedWaterfillPrefix(int R, int K, int M, const vector<int>& Cap, vector<int>& T);
static void ComputeLayerCaps(int N, int K, const vector<int>& Lmin, const vector<int>& Lmax, vector<int>& Cap);
static int AutoDetectFixedSuffix_PrefixDynamic(const vector<vector<int> >& Buckets, const vector<int>& Count,
                                               const vector<int>& low, const vector<int>& high);
static int AutoDetectFixedSuffix_Heuristic(int N, int K, bool fix_last,
                                           const vector<vector<int> >& Buckets,
                                           const vector<int>& Count, const vector<int>& low_dyn,
                                           const vector<int>* low_star_opt);
static int AutoDetectFixedSuffix(const vector<vector<int> >& Buckets, const vector<int>& Lmin,
                                 const vector<int>& Lmax, const vector<int>& Cap);
static int AutoDetectDynamicFixedSuffix(const vector<vector<int> >& Buckets,
                                        const vector<int>& low, const vector<int>& high,
                                        const vector<int>& mov_up, const vector<int>& mov_down,
                                        bool require_low_eq_high);
static void ComputePrefixCapsExcludingSuffix(int N, int K, int prefix_end, const vector<int> &L,
                                             const vector<int> &Lmin, const vector<int> &Lmax, vector<int> &CapPrefix);
static void RecomputeBoundsOne(int u, int K, const vector<vector<int> >& up, const vector<vector<int> >& down,
                               const vector<int>& l, const vector<int>& lmin, const vector<int>& lmax,
                               int& low_u, int& high_u);
static void TopoOrder_Kahn(int N, const vector<vector<int> >& Up, const vector<vector<int> >& Down, vector<int>& topo);
static void InitMedianProjection_Strict(int N, int K, const vector<int>& Lmin, const vector<int>& Lmax,
                                        const vector<vector<int> >& Up, const vector<vector<int> >& Down,
                                        vector<int>& L);
static void InitFromDownUpFeasible(int N, int K, const vector<int>& topo, const vector<int>& L_down,
                                   const vector<int>& Lmin, const vector<int>& Lmax,
                                   const vector<vector<int> >& Up, const vector<vector<int> >& Down, vector<int>& L);
static void ComputeStaticFeasibleBounds(int N, int K, const vector<int>& topo,
                                        const vector<int>& Lmin, const vector<int>& Lmax,
                                        const vector<vector<int> >& Up, const vector<vector<int> >& Down,
                                        vector<int>& low_star, vector<int>& high_star);
static int NearestDeficitLayer(int s, const vector<int>& S);
static int PickNearestDeficitTarget(int s, int d, const std::vector<int>& S, int low_u, int high_u, int prefix_end);
static int NeighborTightenRisk(int u, int t, int K, const vector<vector<int> >& Up,
                               const vector<vector<int> >& Down, const vector<int>& L,
                               const vector<int>& low, const vector<int>& high,
                               const vector<int>& Lmin, const vector<int>& Lmax);
static void CollectCandidates(int s, int d, int K, int prefix_end, const vector<vector<int> >& Buckets,
                              const vector<int>& L, const vector<int>& low, const vector<int>& high,
                              const vector<int>& Lmin, const vector<int>& Lmax,
                              const vector< vector<int> >& Up, const vector< vector<int> >& Down,
                              const vector<int>& S, bool require_deficit_target, const vector<int>* pCap,
                              int capMin, int capMax, double depth_penalty_weight, double scarcity_weight,
                              vector<Candidate>& out);
static bool PickFeasibleDeficitAndCandidates(
    int s, const std::vector<int>& S, int K, int prefix_end,
    const std::vector< std::vector<int> >& Buckets,
    const std::vector<int>& L,
    const std::vector<int>& low, const std::vector<int>& high,
    const std::vector<int>& Lmin, const std::vector<int>& Lmax,
    const std::vector< std::vector<int> >& Up,
    const std::vector< std::vector<int> >& Down,
    bool strict_deficit_target,
    bool allow_intermediate_fallback,
    int& d_out,
    std::vector<Candidate>& cand_out,
    DeficitScanMode mode,
    // scoring aids:
    const std::vector<int>* pCap,
    int capMin, int capMax,
    double depth_penalty_weight,
    double scarcity_weight);
static void EvaluateBatchPotentialCounts(int s, const vector<Candidate>& cand, int need,
                                         const vector<int>& Count, const vector<int>& T,
                                         long long& phi1, long long& psi1);
static void EvaluateBatchPotentialCountsPrefix(int s, const vector<Candidate>& cand, int need,
                                               const vector<int>& Count, const vector<int>& T,
                                               int prefix_end, long long& phi1_pref, long long& psi1_pref);
static bool PickDeficitByPotentialAuto(int s, const vector<int>& S, int K, int prefix_end,
                                       const vector<vector<int> >& Buckets,
                                       const vector<int>& L, const vector<int>& low, const vector<int>& high,
                                       const vector<int>& Lmin, const vector<int>& Lmax,
                                       const vector<vector<int> >& Up, const vector<vector<int> >& Down,
                                       const vector<int>& Count, const vector<int>& T,
                                       bool allow_intermediate_fallback,
                                       int& d_out, vector<Candidate>& cand_out,
                                       const vector<int>* pCap, int capMin, int capMax,
                                       double depth_penalty_weight, double scarcity_weight);
static bool TryEvictAndFillOnce(const vector<int>& S, int K, int prefix_end,
                                vector<vector<int> >& Buckets, vector<int>& L, vector<int>& Count,
                                const vector<int>& Lmin, const vector<int>& Lmax,
                                const vector<vector<int> >& Up, const vector<vector<int> >& Down,
                                vector<int>& low, vector<int>& high, const vector<int>& T);
static void PlaceOne(int u, int t, vector<int>& L, vector<vector<int> >& Buckets, vector<int>& Count);
static void RefreshLocalBounds(const vector<int>& moved, int K,
                               const vector<vector<int> >& Up, const vector<vector<int> >& Down,
                               const vector<int>& L, const vector<int>& Lmin, const vector<int>& Lmax,
                               vector<int>& low, vector<int>& high);

class GridLayering: Interface {
public:
#ifdef USE_MONGODB
    /*!
     * \brief Constructor using MongoDB.
     */
    GridLayering(int id, MongoGridFs* gfs, const char* out_dir);
#endif
    /*!
     * \brief Constructor using Raster file directly.
     */
    GridLayering(int id, const char* out_dir);
    ///< Destructor
    virtual ~GridLayering();
    /*!
     * \brief Execute workflow
     */
    bool Execute();
    /*!
     * \brief Load flow data
     */
    virtual bool LoadData() = 0;
    /*!
     * \brief Calculate index of valid cell according to mask raster data
     */
    void CalPositionIndex();
    /*!
     * \brief Get reverse flow direction of each cell, accumulate each cell's upstream cells,
     *          and count upstream number.
     *          The core algorithm is based on bitwise AND operator.
     *            e.g. cell (i, j) has a compressed reversed direction value of 69,
     *            which is stored as 1000101, 1000101 & 1 is True, and so as to 100, 1000000.
     *            So the upstream cells are (i, j+1), (i-1, j), (i+1, j), the number is 3.
     */
    void GetReverseDirMatrix();
    /*!
     * \brief Construct flow in indexes of each cell
     */
    bool BuildFlowInCellsArray();
    /*!
     * \brief Output flow in cells index data, both txt file and GridFS.
     */
    virtual bool OutputFlowIn();
    /*!
     * \brief Count each cell's downstream number by bitwise AND operator
     */
    void CountFlowOutCells();
    /*!
     * \brief Construct flow out indexes of each cells
     */
    bool BuildFlowOutCellsArray();
    /*!
     * \brief Output flow out data, both txt file and GridFS.
     */
    virtual bool OutputFlowOut();
    /*!
     * \brief Build grid layers in Up-Down order from source
     */
    bool GridLayeringFromSource();
    /*!
     * \brief Build grid layers in Down-Up order from outlet
     */
    bool GridLayeringFromOutlet();
    /*!
     * \brief Build grid layers evenly based on Up-Down and Down-Up orders
     */
    bool GridLayeringEvenly_deprecated();
    bool GridLayeringEvenly();
protected:
    /*！
     * \brief Create output filenames
     */
    virtual void OutputFilenames(flowDirTypes ftype);
    /*!
     * \brief Build multiple flow out array
     */
    int BuildMultiFlowOutArray(float*& compressed_dir,
                               int*& connect_count, float*& p_output);
    /*!
     * \brief Output 2D array as txt file
     */
    bool Output2DimensionArrayTxt(const string& name, string& header, float* matrix, float* matrix2 = nullptr);
#ifdef USE_MONGODB
    /*!
     * \brief Output grid layering related data to MongoDB GridFS
     */
    bool OutputToMongodb(const char* name, vint number, char* s);

    /*!
    * \brief Output 2D array as MongoDB-GridFS
    */
    bool OutputArrayAsGfs(const string& name, vint length, float* matrix);
    /*!
     * \brief Output grid layering as tiff file and MongoDB-GridFS
     */
    bool OutputGridLayering(const string& name, int datalength,
                            float* layer_grid, float* layer_cells);

    MongoGridFs* gfs_; ///< MongoDB-GridFS instance
#endif
    bool use_mongo_;         ///< Use MongoDB or file
    bool has_mask_;          ///< User-specific mask raster file
    bool force_outlet_;      ///< Force stream cells as outlets to reduce hillslope routing layers
    flowDirTypes fdtype_;    ///< Flow direction model
    string fdtype_str_;      ///< Flow direction model's name
    const char* output_dir_; ///< Output directory
    int subbasin_id_;        ///< Subbasin ID, 0 for entire basin
    int n_rows_;             ///< Rows
    int n_cols_;             ///< Cols
    float out_nodata_;       ///< Nodata value in output
    int n_valid_cells_;      ///< Valid Cells number
    int n_layer_count_;      ///< Layer count, MUST be the same for all layering methods
    int* pos_index_;         ///< Valid cell's index
    int** pos_rowcol_;       ///< Positions of valid cells, e.g., (row, col) coordinates
    FloatRaster* mask_;      ///< Mask raster data
    FloatRaster* flowdir_;   ///< Flow direction raster data, e.g., `int` for D8
    float* flowdir_matrix_;     ///< Valid flow direction data, e.g., D8, compressed Dinf and MFD-md
    float* reverse_dir_;        ///< Compressed reversed direction
    float* stream_matrix_;      ///< (Optional) Stream data with a length of n_valid_cells_
    int* flow_in_num_;          ///< Count of flow in cells, with a length of n_valid_cells_
    int* flow_in_acc_;          ///< Accumulative count of flow in cells
    int flow_in_count_;         ///< All flow in times
    /*!
     * \brief Stores flow in cells' indexes of each valid cells, which can be
     *          parsed as 2D array. Data length is flow_in_count_ + n_valid_cells_ + 1
     *        For example:
     *            53933 0 1 0 1 1 2 7 8 ...
     *            can be parsed as:
     *                The valid cell number is 53933
     *                ID    UpstreamCount     UpstreamID
     *                0          0
     *                1          1                0
     *                2          1                1
     *                3          2                7,8
     * \note The only reason to use float* rather than int* is that we use float
     *       to keep consistent in data IO of MongoDB.
     */
    float* flow_in_cells_;
    int* flow_out_num_;         ///< Count of flow out cells, with a length of n_valid_cells_
    int* flow_out_acc_;         ///< Accumulative count of flow out cells
    int flow_out_count_;        ///< All flow out times
    float* flow_out_cells_;     ///< Indexes of each cell's flow out
    vector<vector<int> > n_layer_cells_updown_; ///< layer index (not number) - indexes of cells in Up-Down order
    vector<vector<int> > n_layer_cells_downup_; ///< layer index (not number) - indexes of cells in Down-Up order
    vector<vector<int> > n_layer_cells_evenly_; ///< layer index (not number) - indexes of cells in Evenly order
    float* layers_updown_;      ///< layer numbers from source (Up-Down order) with a length of n_valid_cells_
    float* layers_downup_;      ///< layer numbers from outlet (Down-Up order) with a length of n_valid_cells_
    float* layers_evenly_;      ///< layer numbers based on evenly method with a length of n_valid_cells_
    float* layer_cells_updown_; ///< cell indexes of each layer in Up-Down order with a length of n_valid_cells_ + n_layer_count_ + 1
    float* layer_cells_downup_; ///< cell indexes of each layer in Down-Up order with a length of n_valid_cells_ + n_layer_count_ + 1
    float* layer_cells_evenly_; ///< cell indexes of each layer in Evenly order with a length of n_valid_cells_ + n_layer_count_ + 1
    string flowdir_name_;       ///< Flow direction file name
    string mask_name_;          ///< Mask raster file name
    string stream_file_;        ///< Stream shapefile name

    /** Output file names **/
    string flowin_index_name_;    ///< Flow in index
    string flowout_index_name_;   ///< Flow out index
    string layering_updown_name_; ///< Routing layers from sources
    string layering_downup_name_; ///< Routing layers from outlet
    string layering_evenly_name_; ///< Routing layers evenly
};

class GridLayeringD8: public GridLayering {
public:
#ifdef USE_MONGODB
    GridLayeringD8(int id, MongoGridFs* gfs, const char* out_dir,
                   const char* stream_file=nullptr, bool force_outlet=false);
#endif
    GridLayeringD8(int id, const char* out_dir, const char* in_file,
                   const char* mask_file=nullptr, const char* stream_file=nullptr,
                   bool force_outlet=false);

    ~GridLayeringD8();

    bool LoadData() OVERRIDE;
};


class GridLayeringDinf: public GridLayering {
public:
#ifdef USE_MONGODB
    GridLayeringDinf(int id, MongoGridFs* gfs, const char* out_dir,
                     const char* stream_file=nullptr, bool force_outlet=false, bool force_inbasin=true, int decimals=4);
#endif
    GridLayeringDinf(int id, const char* out_dir, const char* fd_file, const char* fraction_file,
                     const char* mask_file=nullptr, const char* stream_file=nullptr,
                     bool force_outlet=false, bool force_inbasin=true, int decimals=4);

    ~GridLayeringDinf();

    void OutputFilenames(flowDirTypes ftype) OVERRIDE;

    bool LoadData() OVERRIDE;
    bool OutputFlowIn() OVERRIDE;
    bool OutputFlowOut() OVERRIDE;

private:
    bool force_inbasin_;               ///< Force all cells flow inside the watershed
    int decimals_;                     ///< Round to N decimal places for flow fractions
    string flowfrac_name_;             ///< Flow fraction raster file recording the fraction of first direction
    FloatRaster* flow_fraction_;       ///< Flow fraction of the first flow out direction
    float* flowfrac_matrix_;           ///< Flow fraction of the first flow out direction (valid cell number)
    float* flowin_fracs_;              ///< Flow in fractions from each cell's upstream
    float* flowout_fracs_;             ///< Flow out fractions of each cell

    /** Output file names **/
    string flowin_frac_name_;  ///< Flow fraction of each flow in cell
    string flowout_frac_name_; ///< Flow fraction of each flow out cell
};

class GridLayeringMFDmd: public GridLayering {
public:
#ifdef USE_MONGODB
    GridLayeringMFDmd(int id, MongoGridFs* gfs, const char* out_dir,
                     const char* stream_file=nullptr, bool force_outlet=false, bool force_inbasin=true, int decimals=4);
#endif
    GridLayeringMFDmd(int id, const char* out_dir, const char* fd_file, const char* fraction_file,
                      const char* mask_file=nullptr, const char* stream_file=nullptr,
                      bool force_outlet=false, bool force_inbasin=true, int decimals=4);

    ~GridLayeringMFDmd();

    void OutputFilenames(flowDirTypes ftype) OVERRIDE;

    bool LoadData() OVERRIDE;
    bool OutputFlowIn() OVERRIDE;
    bool OutputFlowOut() OVERRIDE;

private:
    bool force_inbasin_;               ///< Force all cells flow inside the watershed
    int decimals_;                     ///< Round to N decimal places for flow fractions
    string flowfrac_corename_;         ///< Core name of flow fraction raster files (multiple layer raster) in MongoDB
    vector<string> flowfrac_names_;    ///< Flow fraction raster files recording the fractions of each direction by ccw
    FloatRaster* flow_fraction_;  ///< Flow fraction of the first flow out direction
    float** flowfrac_matrix_;          ///< Flow fraction of the first flow out direction (valid cell number)
    float* flowin_fracs_;              ///< Flow in fraction
    float* flowout_fracs_;             ///< Flow fractions of each cell's flow in

    /** Output file names **/
    string flowin_frac_name_;  ///< Flow fraction of each flow in cell
    string flowout_frac_name_; ///< Flow fraction of each flow out cell
};
#endif /* GRID_LAYERING_H */
