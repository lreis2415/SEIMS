/*!
 * \brief Grid layering based on Dinf model
 * \author originally by Junzhi Liu
 * \author Liangjun Zhu
 * \date 12-28-2017
 */

#ifndef GRID_LAYERING_DINF_H
#define GRID_LAYERING_DINF_H

#include "GridLayering.h"

class GridLayeringDinf : public GridLayering {
public:
    GridLayeringDinf(int id, MongoGridFS *gfs, const char *out_dir);
    virtual ~GridLayeringDinf();
    virtual bool LoadData();
    virtual bool OutputFlowIn();
    virtual bool OutputFlowOut();
    /** Dinf specific functions **/

    /*!
     * \brief Get flow partition of Dinf model in delta row (i) and delta col (j)
     */
    float GetPercentage(float angle, int di, int dj);
protected:
    /*!
     * \brief
     * \param compressedDir
     * \param connectCount
     * \param pOutput
     */
    void _build_multi_flow_out_angle_array(const int *compressedDir,
                                           const int *connectCount, float *&pOutput);
private:
    clsRasterData<float> *m_flowangle;  ///< Flow direction in radiation
    float *m_angle;  ///< Flow angle array
    float *m_flowInAngle;  ///< Flow in partition, \sa m_flowInCells

    string m_flowangle_name;  ///< Dinf flow direction name
    string m_flowin_angle_name;  ///< Output of flow in partition
};

#endif /* GRID_LAYERING_DINF_H */
