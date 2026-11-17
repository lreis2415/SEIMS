/*! \brief Saturated soil routing with optional bottom capacity excess. */
#ifndef SEIMS_MODULE_SSR_KIT_H
#define SEIMS_MODULE_SSR_KIT_H

#include "SimulationModule.h"

#include <vector>

class SSR_KIT : public SimulationModule {
public:
    SSR_KIT();
    ~SSR_KIT() OVERRIDE;

    void SetValue(const char* key, FLTPT value) OVERRIDE;
    void SetValue(const char* key, int value) OVERRIDE;
    void SetReaches(clsReaches* reaches) OVERRIDE;
    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;
    void Set1DData(const char* key, int n, int* data) OVERRIDE;
    void Set2DData(const char* key, int nrows, int ncols, FLTPT** data) OVERRIDE;
    void Set2DData(const char* key, int nrows, int ncols, int** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;
    void InitialOutputs() OVERRIDE;
    int Execute() OVERRIDE;
    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    // SOILDEPTH stores cumulative layer-bottom depths in mm, not layer thicknesses.

    static FLTPT RedistributeSaturationExcess(int nLayers, const FLTPT* layerBottomDepth,
                                              FLTPT* moisture, const FLTPT* porosity);
    static FLTPT CalculateWaterTableDepth(int nLayers, const FLTPT* layerBottomDepth,
                                          const FLTPT* moisture, const FLTPT* fieldCapacity,
                                          const FLTPT* porosity);
    static FLTPT CalculateLayerIntegratedTransmissivity(int nLayers, const FLTPT* layerBottomDepth,
                                                        const FLTPT* lateralConductivity,
                                                        FLTPT waterTableDepth);

    static FLTPT CalculateTransmissivity(FLTPT bottomDepth, FLTPT waterTableDepth, FLTPT lateralK,
                                         FLTPT decayPerMeter, FLTPT thresholdDepth);
    static FLTPT CalculateBankGradientWidth(FLTPT waterTableDepth, FLTPT bankHeight);

    static FLTPT CalculateDrainableWaterDepthBetween(int nLayers, const FLTPT* layerBottomDepth,
                                                     const FLTPT* fieldCapacity,
                                                     const FLTPT* porosity, FLTPT upperDepth,
                                                     FLTPT lowerDepth);
    static FLTPT AddWaterBottomUp(int nLayers, const FLTPT* layerBottomDepth, FLTPT* moisture,
                                  const FLTPT* porosity, FLTPT waterDepth);
    static FLTPT RemoveWaterFromWaterTableDown(int nLayers, const FLTPT* layerBottomDepth,
                                               FLTPT* moisture, const FLTPT* fieldCapacity,
                                               const FLTPT* porosity, FLTPT waterTableDepth,
                                               FLTPT waterDepth);
    static FLTPT CalculateHydraulicGradient(FLTPT terrainSlope, FLTPT upstreamWaterTableDepth,
                                            FLTPT downstreamWaterTableDepth, FLTPT flowLength);

private:
    void CalculateRoutingGradients(const std::vector<FLTPT>& waterTableDepth,
                                   std::vector<FLTPT>* gradients) const;

    FLTPT ConnectedDrainableDepth(int id, FLTPT waterTableDepth) const;

    FLTPT CellTransmissivity(int id, FLTPT waterTableDepth, FLTPT bottomDepth) const;

    FLTPT BankHeight(int id) const;

    static FLTPT LayerThickness(const FLTPT* layerBottomDepth, int layer);

    int m_nCells;
    int m_nReaches;
    int m_maxSoilLyrs;
    int m_dt;

    FLTPT m_cellWidth;
    FLTPT m_khRatio;
    FLTPT m_decayFactor;
    FLTPT m_depthThresholdRatio;
    int m_gradientMode;

    int* m_nSoilLyrs;
    int* m_streamLink;
    FLTPT* m_slope;
    FLTPT* m_chWidth;
    FLTPT* m_reachDepth;

    FLTPT* m_surfaceRunoff;
    /// Bottom pore-capacity excess supplied by PERCO_DARCY (mm per cell).
    FLTPT* m_bottomExcess;
    FLTPT* m_infilCapacitySurplus;

    FLTPT** m_soilDepth;
    FLTPT** m_conductivity;
    FLTPT** m_porosity;
    FLTPT** m_fieldCapacity;
    FLTPT** m_soilMoisture;

    int** m_flowInIndex;
    FLTPT** m_flowInFraction;

    bool m_soilGeometryValidated;

    FLTPT* m_qSoil;
    FLTPT* m_returnFlow;
    FLTPT* m_waterTableDepth;
    FLTPT* m_saturatedDepth;
    FLTPT* m_qOut;
};

#endif
