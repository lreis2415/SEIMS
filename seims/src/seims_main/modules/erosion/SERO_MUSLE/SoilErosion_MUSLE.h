/*!
 * \file SoilErosion_MUSLE.h
 * \brief MUSLE (Modified Universal Soil Loss Equation, Williams, 1995) method to
 *          calculate sediment yield of each cell.
 *
 * Refers to the source code part of SWAT
 *   - soil_phys.f Initialization of usle_mult (i.e., K*P*LS*11.8*CalExp(ROCK))
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
 *   - 4. 2021-10-29 - ss,lj -
 *   - 5. 2022-08-22 - lj - Change float to FLTPT.
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
 * \brief use MUSLE method to calculate sediment yield of each cell
 *
 */
class SERO_MUSLE: public SimulationModule {
public:
    SERO_MUSLE();

    ~SERO_MUSLE();

    void SetValue(const char* key, FLTPT value) OVERRIDE;

    void SetValue(const char* key, int value) OVERRIDE;

    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    void Set1DData(const char* key, int n, int* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, FLTPT** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    void InitialIntermediates() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    void Get2DData(const char* key, int* nrows, int* ncols, FLTPT*** data) OVERRIDE;

private:
    //! valid cell number
    int m_nCells;
    //! cell width (m)
    FLTPT m_cellWth;
    //! soil layer number
    int m_maxSoilLyrs;

    // grid from parameter

    //! percent of rock content
    FLTPT** m_soilRock;
    //! USLE K factor (erodibility), multi-layer paramters.
    FLTPT** m_usleK;
    //! USLE P factor (Practice)
    FLTPT* m_usleP;
    //! flow accumulation (number of accumulated cells)
    FLTPT* m_flowAccm;
    //! Slope gradient (drop/distance)
    FLTPT* m_slope;
    //! Slope length (optional)
    FLTPT* m_slpLen;
    //! stream link
    int* m_rchID;

    //! sand fraction
    FLTPT* m_detSand;
    //! silt fraction
    FLTPT* m_detSilt;
    //! clay fraction
    FLTPT* m_detClay;
    //! small aggregate fraction
    FLTPT* m_detSmAgg;
    //! large aggregate fraction
    FLTPT* m_detLgAgg;

    //! C-factor calculation using Cmin (0, default) or new method from RUSLE (1)
    int m_iCfac;
    //! Average annual USLE C factor for the land cover, or CalLn(aveAnnUsleC) when m_soilRsd is available.
    FLTPT* m_aveAnnUsleC;
    //! landcover, which can be used as same as `idplt` in SWAT
    int* m_landCover;
    //! Amount of organic matter in the soil classified as residue(kg/ha)
    FLTPT* m_rsdCovSoil;
    //! Residue cover factor for computing fraction of cover
    FLTPT m_rsdCovCoef;
    //! Canopy height, m
    FLTPT* m_canHgt;

    // grid from other modules

    //! LAI of current day
    FLTPT* m_lai;
    //! surface runoff (mm)
    FLTPT* m_surfRf;
    //! snow accumulation
    FLTPT* m_snowAccum;

    // intermediate parameters

    //! product of USLE K,P,LS,CalExp(rock)
    FLTPT* m_usleMult;
    //! cell area (A, km^2)
    FLTPT m_cellAreaKM;
    //! cell area factor (3.79 * A^0.7)
    FLTPT m_cellAreaKM1;
    //! cell area factor (0.903 * A^0.017)
    FLTPT m_cellAreaKM2;
    //! Slope^0.16
    FLTPT* m_slopeForPq;

    // Outputs

    //! USLE L factor
    FLTPT* m_usleL;
    //! USLE S factor
    FLTPT* m_usleS;
    //! Daily updated USLE C factor
    FLTPT* m_usleC;
    //! sediment yield on each cell
    FLTPT* m_eroSed;
    //! sand yield
    FLTPT* m_eroSand;
    //! silt yield
    FLTPT* m_eroSilt;
    //! clay yield
    FLTPT* m_eroClay;
    //! small aggregate yield
    FLTPT* m_eroSmAgg;
    //! large aggregate yield
    FLTPT* m_eroLgAgg;
};
#endif /* SEIMS_MODULE_SERO_MUSLE_H */
