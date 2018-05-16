/*!
 * \brief use MUSLE method to calculate sediment yield of each cell
 * \author Zhiqiang Yu
 * \date Feb. 2012
 * \changelog 2016-05-30 - lj - Code reformat.\n
 *            2018-05-14 - lj - Code review and reformat.\n
 *
 */
#ifndef SEIMS_MODULE_MUSLE_AS_H
#define SEIMS_MODULE_MUSLE_AS_H

#include "SimulationModule.h"

/** \defgroup MUSLE_AS
 * \ingroup Erosion
 * \brief use MUSLE method to calculate sediment yield of each cell
 */
/*!
 * \class MUSLE_AS
 * \ingroup MUSLE_AS
 *
 * \brief use MUSLE method to calculate sediment yield of each cell
 *
 */
class MUSLE_AS: public SimulationModule {
public:
    MUSLE_AS();

    ~MUSLE_AS();

    int Execute() OVERRIDE;

    void SetValue(const char* key, float data) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int nRows, int nCols, float** data) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    bool CheckInputSize(const char* key, int n);

    bool CheckInputData();

private:
    void InitialOutputs();

private:
    //! valid cell number
    int m_nCells;
    //! cell width (m)
    float m_cellWth;
    //! soil layer number
    int m_nSoilLayers;
    //! deposition ratio
    float m_depRatio; // added by Wu Hui, 2013.11.16
    //grid from parameter
    //! sand fraction
    float* m_detSand;
    //! silt fraction
    float* m_detSilt;
    //! clay fraction
    float* m_detClay;
    //! small aggregate fraction
    float* m_detSmAgg;
    //! large aggregate fraction
    float* m_detLgAgg;

    //! USLE P factor (Practice)
    float* m_usleP;
    //! USLE K factor (erodibility), multi-layer paramters. By LJ
    float** m_usleK;
    //! USLE C factor (land cover)
    float* m_usleC;
    //! Slope gradient (drop/distance)
    float* m_slope;
    //! flow accumulation (number of accumulated cells)
    float* m_flowAccm;
    //! stream link
    float* m_rchID;
    ////! Subbasin map
    //   float *m_subbasin;

    //! USLE LS factor
    float* m_usleLS;
    //! cell area (A, km^2)
    float m_cellAreaKM;
    //! cell area factor (3.79 * A^0.7)
    float m_cellAreaKM1;
    //! cell area factor (0.903 * A^0.017)
    float m_cellAreaKM2;
    //! Slope^0.16
    float* m_slopeForPq;

    //grid from other modules

    //! snow accumulation
    float* m_snowAccum;
    //! surface runoff (mm)
    float* m_surfRf;

    //result

    //! sediment yield on each cell
    float* m_eroSed;
    //! sand yield
    float* m_eroSand;
    //! silt yield
    float* m_eroSilt;
    //! clay yield
    float* m_eroClay;
    //! small aggregate yield
    float* m_eroSmAgg;
    //! large aggregate yield
    float* m_eroLgAgg;
};
#endif /* SEIMS_MODULE_MUSLE_AS_H */
