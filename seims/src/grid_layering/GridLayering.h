/*!
 * \brief Grid layering class.
 * \author original, Junzhi Liu, 29-July-2012
 * \author Liangjun Zhu, 21-July-2016
 *          lj - 28-Dec-2017 - Refactor as class.\n
 *          lj -  5-Mar-2018 - Use CCGL, and reformat code style.\n
 *          lj - 27-Otc-2020 - Code review and implement MFD-md algorithm.\n
 * \description:
 *               The output list:
 *               1. X_FLOWOUT_INDEX_{FD}, X_FLOWIN_INDEX_{FD}
 *                     in which X is subbasinID (0 for the whole basin)
 *                              `FD` is the flow direction algorithm, include `D8`, `DINF`, and `MFDMD`
 *                  X_FLOWIN_PERCENTAGE_{MFD}
 *                     in which `MFD` is two- or multi-flow direction algorithm, include `DINF` and `MFDMD`
 *               2. X_ROUTING_LAYERS_UP_DOWN{_METHOD} and X_ROUTING_LAYERS_DOWN_UP{_METHOD}
 *                     in which `_METHOD` represent flow direction method, the empty denotes `D8`, others
 *                        include `_DINF` and `_MFDMD`
 *
 */

#ifndef GRID_LAYERING_H
#define GRID_LAYERING_H

#include "basic.h"
#include "data_raster.h"
#include "db_mongoc.h"

#include <string>

using namespace ccgl;
using namespace data_raster;

#ifndef IntRaster
#define IntRaster   clsRasterData<int>
#endif
#ifndef FloatRaster
#define FloatRaster clsRasterData<float>
#endif

class GridLayering: Interface {
public:
    /*!
     * \brief Constructor.
     */
    GridLayering(int id, MongoGridFs* gfs, const char* out_dir);
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
     * \brief Calculate index of valid cell
     */
    void CalPositionIndex();
    /*!
     * \brief Get compressed reverse flow direction matrix, i.e., accumulate each cell's upstream direction
     *        e.g. cell (i, j) has three upstream source: (i, j+1), (i-1, j), (i+1, j)
     *             then the compressed reverse direction of cell (i, j) is 1 + 64 + 4 = 69
     */
    void GetReverseDirMatrix();
    /*!
     * \brief Count each cell's upstream number by bitwise AND operator
     *        e.g. cell (i, j) has a reversed direction value of 69, which stored as 1000101
     *        1000101 & 1 is True, and so as to 100, 1000000. So the upstream cell number is 3.
     */
    void CountFlowInCells();
    /*!
     * \brief Construct flow in indexes of each cells
     */
    void BuildFlowInCellsArray();
    /*!
     * \brief Output flow in cells index data, both txt file and GridFS.
     */
    virtual bool OutputFlowIn();
    /*!
     * \brief Count each cell's downstream number by bitwise AND operator, CountFlowInCells
     */
    void CountFlowOutCells();
    /*!
     * \brief Construct flow out indexes of each cells
     */
    void BuildFlowOutCellsArray();
    /*!
     * \brief Output flow out data, both txt file and GridFS.
     */
    virtual bool OutputFlowOut() = 0;
    /*!
     * \brief Build grid layers in Up-Down order from source
     */
    bool GridLayeringFromSource();
    /*!
     * \brief Build grid layers in Down-Up order from outlet
     */
    bool GridLayeringFromOutlet();
protected:
    /*!
     * \brief Output grid layering related data to MongoDB GridFS
     */
    bool OutputToMongodb(const char* name, int number, char* s);
    /*!
     * \brief Constructor multiple flow out array
     */
    void BuildMultiFlowOutArray(int*& compressed_dir,
                                int*& connect_count, float*& p_output);
    /*!
     * \brief Ouput 2D array as txt file
     */
    bool Output2DimensionArrayTxt(const string& name, string& header, float* matrix);
    /*!
    * \brief Ouput 2D array as MongoDB-GridFS
    */
    bool OutputArrayAsGfs(const string& name, int length, float* matrix);
    /*!
     * \brief Output grid layering as tiff file and MongoDB-GridFS
     */
    bool OutputGridLayering(const string& name, int datalength,
                            int* layer_grid, float* layer_cells);
protected:
    MongoGridFs* gfs_;       ///< MongoDB-GridFS instance
    const char* output_dir_; ///< Output directory
    int subbasin_id_;        ///< Subbasin ID, 0 for entire basin
    int n_rows_;             ///< Rows
    int n_cols_;             ///< Cols
    int dir_nodata_;         ///< Nodata value of direction raster
    int out_nodata_;         ///< Nodata value in output
    int n_valid_cells_;      ///< Valid Cells number
    int* pos_index_;         ///< Valid cell's index
    IntRaster* flowdir_;     ///< Flow direction raster data, e.g., `int` for D8
    int* flowdir_matrix_;    ///< Flow direction data, e.g., D8, compressed Dinf
    int* reverse_dir_;       ///< Compressed reversed direction
    int* flow_in_num_;       ///< Flow in cells number
    int flow_in_count_;      ///< All flow in counts from \a m_flowInNum
    /*!
     * \brief Stores flow in cells' indexes of each valid cells, which can be
     *          parsed as 2D array. Data length is m_flowInTimes + m_nValidCells + 1
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
    int* flow_out_num_;         ///< Flow out cells number
    int flow_out_count_;        ///< Flow out times
    float* flow_out_cells_;     ///< #m_flowInCells
    int* layers_updown_;        ///< the value of layering number from source, length is nRows * nCols
    int* layers_downup_;        ///< the value of layering number from outlet
    float* layer_cells_updown_; ///< store cell indexes in each layers, length is ValidNum + layerNum + 1
    float* layer_cells_downup_; ///< store cell indexes in each layers
    /** Output file names **/

    string flowdir_name_;         ///< Flow direction file name
    string flowin_index_name_;    ///< Flow in index
    string flowout_index_name_;   ///< Flow out index
    string layering_updown_name_; ///< Routing layers from sources
    string layering_downup_name_; ///< Routing layers from outlet
};

class GridLayeringD8 : public GridLayering {
public:
    GridLayeringD8(int id, MongoGridFs* gfs, const char* out_dir);

    ~GridLayeringD8();

    bool LoadData() OVERRIDE;
    bool OutputFlowOut() OVERRIDE;
};


class GridLayeringDinf : public GridLayering {
public:
    GridLayeringDinf(int id, MongoGridFs* gfs, const char* out_dir);
    ~GridLayeringDinf();
    bool LoadData() OVERRIDE;
    bool OutputFlowIn() OVERRIDE;
    bool OutputFlowOut() OVERRIDE;

    /** Dinf specific functions **/

    /*!
     * \brief Get flow partition of Dinf model in delta row (i) and delta col (j)
     */
    static float GetPercentage(float angle, int di, int dj);
protected:
    /*!
     * \brief
     * \param compressed_dir
     * \param connect_count
     * \param p_output
     */
    void BuildMultiFlowOutAngleArray(int*& compressed_dir,
                                     int*& connect_count, float*& p_output);
private:
    FloatRaster* flow_angle_; ///< Flow direction in radiation
    float* angle_;            ///< Flow angle array
    float* flow_in_angle_;    ///< Flow in partition, #m_flowInCells

    string flow_angle_name_;   ///< Dinf flow direction name
    string flowin_angle_name_; ///< Output of flow in partition
};

#endif /* GRID_LAYERING_H */
