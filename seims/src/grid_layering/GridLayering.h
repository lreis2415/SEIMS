/*!
 * \brief Grid layering class.
 * \author original, Junzhi Liu, 29-July-2012
 * \author Liangjun Zhu, 21-July-2016
 *          lj - 28-Dec-2017 - Refactor as class
 * \description:
 *               The output list:
 *               1. X_FLOWOUT_INDEX_{FD}, X_FLOWIN_INDEX_{FD}
 *                     in which X is subbasinID (0 for the whole basin)
 *                              FD is the flow direction algorithm, include D8, Dinf
 *                  X_FLOWIN_PERCENTAGE_{MFD}
 *                     in which MFD is multi-flow direction algorithm, include Dinf
 *               2. X_ROUTING_LAYERS_UP_DOWN or X_ROUTING_LAYERS_DOWN_UP for D8
 *                  X_ROUTING_LAYERS_DINF for Dinf
 *
 */

#ifndef GRID_LAYERING_H
#define GRID_LAYERING_H

#include "clsRasterData.h"
#include "MongoUtil.h"
#include "utilities.h"

#include <string>

using namespace std;

class GridLayering {
public:
    /*!
     * \brief Constructor.
     */
    GridLayering(int id, MongoGridFS *gfs, const char *out_dir);
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
     * \brief Count each cell's downstream number by bitwise AND operator, \sa CountFlowInCells
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
    bool _output_to_mongodb(const char *type, int number, char *s);
    /*!
     * \brief Constructor multiple flow out array
     */
    void _build_multi_flow_out_array(const int *compressedDir,
                                     const int *connectCount, float *&pOutput);
    /*!
     * \brief Ouput 2D array as txt file
     */
    bool _output_2dimension_array_txt(string &name, string &header, const float *matrix);
    /*!
    * \brief Ouput 2D array as MongoDB-GridFS
    */
    bool _output_array_as_gfs(string &name, int length, const float *matrix);
    /*!
     * \brief Output grid layering as tiff file and MongoDB-GridFS
     */
    bool _output_grid_layering(string &name, int layer_num, int datalength,
                               const int *layer_grid, const float *layer_cells);
protected:
    MongoGridFS *m_gfs;  ///< MongoDB-GridFS instance
    const char *m_outputDir;  ///< Output directory
    int m_subbasinID;  ///< Subbasin ID, 0 for entire basin
    int m_nRows;  ///< Rows
    int m_nCols;  ///< Cols
    int m_dirNoData;  ///< Nodata value of direction raster
    int m_outNoData;  ///< Nodata value in output
    int m_nValidCells;  ///< Valid Cells number
    int *m_posIndex;  ///< Valid cell's index
    clsRasterData<int> *m_flowdir;  ///< Flow direction raster data, e.g., <int> for D8
    int *m_flowdirMatrix; ///< Flow direction data, e.g., D8, compressed Dinf
    int *m_reverseDir; ///< Compressed reversed direction
    int *m_flowInNum;  ///< Flow in cells number
    int m_flowInTimes;  ///< All flow in counts from \a m_flowInNum
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
     */
    float *m_flowInCells;
    int *m_flowOutNum;  ///< Flow out cells number
    int m_flowOutTimes;  ///< Flow out times
    float *m_flowOutCells;  ///< \sa m_flowInCells
    int *m_layers_updown;  ///< the value of layering number from source, length is nRows * nCols
    int *m_layers_downup;  ///< the value of layering number from outlet
    float *m_layerCells_updown;  ///< store cell indexes in each layers, length is ValidNum + layerNum + 1
    float *m_layerCells_downup;  ///< store cell indexes in each layers
    /** Output file names **/

    string m_flowdir_name;  ///< Flow direction file name
    string m_flowin_index_name;  ///< Flow in index
    string m_flowout_index_name;  ///< Flow out index
    string m_layering_updown_name;  ///< Routing layers from sources
    string m_layering_downup_name;  ///< Routing layers from outlet
};

#endif /* GRID_LAYERING_H */
