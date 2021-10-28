/*!
 * \file SoilErosion_MUSLE.h
 * \brief MUSLE (Modified Universal Soil Loss Equation, Williams, 1995) method to
 *          calculate sediment yield of each cell.
 *
 * Refers to the source code part of SWAT
 *   - soil_phys.f Initialization of usle_mult (i.e., K*P*LS*11.8*exp(ROCK))
 *   - cfactor.f   Calculate daily USLE_C factor.
 *   - ysed.f      Actually calculation of daily soil loss caused by water erosion.
 *
 * Changelog:
 *   - 1. 2012-02-01 - zq - Initial implementation.
 *   - 2. 2018-05-14 - lj - Code review and reformat.
 *   - 3. 2018-07-09 - lj -
 *        -# Change module ID from MUSLE_AS to SERO_MUSLE.
 *        -# Updates USLE_C factor during the growth cycle of the plant.
 *        -# Change the calculation of LS factor, and add USLE_L and USLE_S as outputs.
 *
 * \author Liangjun Zhu, Zhiqiang Yu
 */
#ifndef SEIMS_MODULE_SERO_MUSLE_H
#define SEIMS_MODULE_SERO_MUSLE_H

#include "SimulationModule.h"

/** \defgroup SERO_MUSLE
 * \ingroup Erosion
 * \brief use MUSLE method to calculate sediment yield of each cell
 */
/*!
 * \class SERO_MUSLE
 * \ingroup SERO_MUSLE
 *
 * \brief use MUSLE method to calculate sediment yield of each cell
 *
 */
class SERO_MUSLE: public SimulationModule {
public:
    SERO_MUSLE();

    ~SERO_MUSLE();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    void InitializeIntermediateVariables() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void Get2DData(const char* key, int* nrows, int* ncols, float*** data) OVERRIDE;

private:
    //! valid cell number
    int m_nCells;
    //! cell width (m)
    float m_cellWth;
    //! soil layer number
    int m_maxSoilLyrs;

    // grid from parameter

    //! percent of rock content
    float** m_soilRock;
    //! USLE K factor (erodibility), multi-layer paramters.
    float** m_usleK;
    //! USLE P factor (Practice)
    float* m_usleP;
    //! flow accumulation (number of accumulated cells)
    float* m_flowAccm;
    //! Slope gradient (drop/distance)
    float* m_slope;
    //! Slope length (optional)
    float* m_slpLen;
    //! stream link
    float* m_rchID;

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

    //! C-factor calculation using Cmin (0, default) or new method from RUSLE (1)
    int m_iCfac;
    //! Average annual USLE C factor for the land cover, or log(aveAnnUsleC) when m_soilRsd is available.
    float* m_aveAnnUsleC;
    //! landcover, which can be used as same as `idplt` in SWAT
    float* m_landCover;
    //! Amount of organic matter in the soil classified as residue(kg/ha)
    float* m_rsdCovSoil;
    //! Residue cover factor for computing fraction of cover
    float m_rsdCovCoef;
    //! Canopy height, m
    float* m_canHgt;

    // grid from other modules

    //! LAI of current day
    float* m_lai;
    //! surface runoff (mm)
    float* m_surfRf;
    //! snow accumulation
    float* m_snowAccum;

    // temporary variables

    //! product of USLE K,P,LS,exp(rock)
    float* m_usleMult;
    //! cell area (A, km^2)
    float m_cellAreaKM;
    //! cell area factor (3.79 * A^0.7)
    float m_cellAreaKM1;
    //! cell area factor (0.903 * A^0.017)
    float m_cellAreaKM2;
    //! Slope^0.16
    float* m_slopeForPq;

    // Outputs

    //! USLE L factor
    float* m_usleL;
    //! USLE S factor
    float* m_usleS;
    //! Daily updated USLE C factor
    float* m_usleC;
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
#endif /* SEIMS_MODULE_SERO_MUSLE_H */
