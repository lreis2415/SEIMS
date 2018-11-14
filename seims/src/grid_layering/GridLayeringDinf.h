/*!
 * \brief Grid layering based on Dinf model
 * \author originally by Junzhi Liu
 * \author Liangjun Zhu
 * \date 12-28-2017
 */

#ifndef GRID_LAYERING_DINF_H
#define GRID_LAYERING_DINF_H

#include "GridLayering.h"

class GridLayeringDinf: public GridLayering {
public:
    GridLayeringDinf(int id, MongoGridFs* gfs, const char* out_dir);
    virtual ~GridLayeringDinf();
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

#endif /* GRID_LAYERING_DINF_H */
