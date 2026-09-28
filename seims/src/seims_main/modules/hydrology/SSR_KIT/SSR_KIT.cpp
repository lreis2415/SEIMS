#include "SSR_KIT.h"

#include "text.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <vector>

SSR_KIT::SSR_KIT() :
    m_nCells(-1), m_nReaches(-1), m_maxSoilLyrs(-1), m_dt(-1),
                     m_executeCount(0), m_cellWidth(-1.),
                     m_khRatio(1.), m_decayFactor(0.), m_depthThresholdRatio(0.70),
                     m_maxDrainFraction(0.5), m_maxSubsteps(64), m_gradientMode(0),
                     m_nSoilLyrs(nullptr), m_streamLink(nullptr), m_slope(nullptr), m_chWidth(nullptr),
                     m_reachDepth(nullptr),
                     m_surfaceRunoff(nullptr), m_infilCapacitySurplus(nullptr), m_soilDepth(nullptr),
                     m_conductivity(nullptr), m_porosity(nullptr), m_fieldCapacity(nullptr), m_soilMoisture(nullptr),
                     m_flowInIndex(nullptr), m_flowInFraction(nullptr), m_topologyReady(false),
                     m_soilGeometryValidated(false), m_outWeightSum(nullptr),
                     m_outConnectionCount(nullptr), m_qSoil(nullptr),
                     m_returnFlow(nullptr), m_waterTableDepth(nullptr), m_saturatedDepth(nullptr),
                     m_qOut(nullptr) {
}

SSR_KIT::~SSR_KIT() {
    Release1DArray(m_outWeightSum);
    Release1DArray(m_outConnectionCount);
    Release1DArray(m_qSoil);
    Release1DArray(m_returnFlow);
    Release1DArray(m_waterTableDepth);
    Release1DArray(m_saturatedDepth);
    Release1DArray(m_qOut);
}

FLTPT SSR_KIT::LayerThickness(const FLTPT* layerBottomDepth, const int layer) {
    if (layerBottomDepth == nullptr || layer < 0) return 0.;
    const FLTPT layerBottom = Max(layerBottomDepth[layer], 0.);
    const FLTPT layerTop = layer > 0 ? Max(layerBottomDepth[layer - 1], 0.) : 0.;
    return Max(layerBottom - layerTop, 0.);
}

FLTPT SSR_KIT::CalculateWaterTableDepth(const int nLayers,
    const FLTPT* layerBottomDepth,
    const FLTPT* moisture, const FLTPT* fieldCapacity,
    const FLTPT* porosity) {
    if (nLayers <= 0 || layerBottomDepth == nullptr || moisture == nullptr ||
        fieldCapacity == nullptr || porosity == nullptr) {
        return 0.;
    }
    const FLTPT totalDepth = Max(layerBottomDepth[nLayers - 1], 0.);
    FLTPT waterTable = totalDepth;
    for (int k = nLayers - 1; k >= 0; k--) {
        const FLTPT layerDepth = LayerThickness(layerBottomDepth, k);
        if (layerDepth <= UTIL_ZERO) continue;
        const FLTPT fc = Max(fieldCapacity[k], 0.);
        const FLTPT por = Max(porosity[k], fc);
        const FLTPT theta = Max(0., Min(moisture[k], por));
        const FLTPT storageRange = por - fc;
        if (storageRange <= UTIL_ZERO || theta <= fc + UTIL_ZERO) break;
        if (theta >= por - UTIL_ZERO) {
            waterTable -= layerDepth;
            continue;
        }
        const FLTPT saturatedFraction = Max(0., Min((theta - fc) / storageRange, 1.));
        waterTable -= saturatedFraction * layerDepth;
        break;
    }
    return Max(0., Min(waterTable, totalDepth));
}

FLTPT SSR_KIT::CalculateLayerIntegratedTransmissivity(
    const int nLayers, const FLTPT* layerBottomDepth, const FLTPT* lateralConductivity,
    const FLTPT waterTableDepth) {
    if (nLayers <= 0 || layerBottomDepth == nullptr || lateralConductivity == nullptr) return 0.;
    FLTPT transmissivity = 0.;
    FLTPT layerTop = 0.;
    for (int k = 0; k < nLayers; k++) {
        const FLTPT layerBottom = Max(layerBottomDepth[k], layerTop);
        const FLTPT saturatedDepth = Max(0., layerBottom - Max(waterTableDepth, layerTop));
        // Layer bottom depth is supplied in mm and conductivity in m/s.
        transmissivity += Max(lateralConductivity[k], 0.) * saturatedDepth * 0.001;
        layerTop = layerBottom;
    }
    return Max(transmissivity, 0.);
}

FLTPT SSR_KIT::CalculateTwoPartTransmissivity(
    const FLTPT soilDepth, const FLTPT waterTableDepth,
    const FLTPT surfaceLateralConductivity, const FLTPT decayFactor,
    const FLTPT depthThresholdRatio) {
    const FLTPT depthM = Max(soilDepth, 0.) * 0.001;
    const FLTPT lateralK = Max(surfaceLateralConductivity, 0.);
    if (depthM <= UTIL_ZERO || lateralK <= 0.) return 0.;

    const FLTPT waterTableM = Max(0., Min(waterTableDepth * 0.001, depthM));
    if (waterTableM >= depthM - UTIL_ZERO) return 0.;
    if (decayFactor <= 0.) return lateralK * (depthM - waterTableM);

    const FLTPT thresholdRatio = Max(1.e-6, Min(depthThresholdRatio, 1. - 1.e-6));
    const FLTPT thresholdM = thresholdRatio * depthM;
    const FLTPT decayPerMeter = decayFactor / depthM;
    FLTPT transmissivity = 0.;
    if (waterTableM < thresholdM) {
        const FLTPT saturatedInterval = depthM - waterTableM;
        transmissivity = lateralK * std::exp(-decayPerMeter * waterTableM) *
            -std::expm1(-decayPerMeter * saturatedInterval) /
            decayPerMeter;
    } else {
        const FLTPT thresholdInterval = depthM - thresholdM;
        const FLTPT atThreshold = lateralK * std::exp(-decayPerMeter * thresholdM) *
            -std::expm1(-decayPerMeter * thresholdInterval) /
            decayPerMeter;
        transmissivity = (depthM - waterTableM) / (depthM - thresholdM) * atThreshold;
    }
    return std::isfinite(transmissivity) ? Max(transmissivity, 0.) : 0.;
}

FLTPT SSR_KIT::CalculateBankGradientWidth(const FLTPT waterTableDepth,
    const FLTPT bankHeight) {
    if (!std::isfinite(waterTableDepth) || !std::isfinite(bankHeight)) return 0.;
    return 4. * Max(bankHeight - waterTableDepth, 0.) * 0.001;
}

FLTPT SSR_KIT::CalculateTwoPartTransmissivityBetween(
    const FLTPT soilDepth, const FLTPT upperDepth, const FLTPT lowerDepth,
    const FLTPT surfaceLateralConductivity, const FLTPT decayFactor,
    const FLTPT depthThresholdRatio) {
    const FLTPT boundedUpper = Max(0., Min(upperDepth, soilDepth));
    const FLTPT boundedLower = Max(boundedUpper, Min(lowerDepth, soilDepth));
    if (boundedLower <= boundedUpper + UTIL_ZERO) return 0.;
    const FLTPT fromUpper = CalculateTwoPartTransmissivity(
        soilDepth, boundedUpper, surfaceLateralConductivity,
        decayFactor, depthThresholdRatio);
    const FLTPT fromLower = CalculateTwoPartTransmissivity(
        soilDepth, boundedLower, surfaceLateralConductivity,
        decayFactor, depthThresholdRatio);
    return Max(fromUpper - fromLower, 0.);
}

FLTPT SSR_KIT::CalculateDrainableWaterDepthBetween(
    const int nLayers, const FLTPT* layerBottomDepth, const FLTPT* moisture,
    const FLTPT* fieldCapacity, const FLTPT* porosity,
    const FLTPT upperDepth, const FLTPT lowerDepth) {
    if (nLayers <= 0 || layerBottomDepth == nullptr || moisture == nullptr ||
        fieldCapacity == nullptr || porosity == nullptr) {
        return 0.;
    }
    const FLTPT profileBottom = Max(layerBottomDepth[nLayers - 1], 0.);
    const FLTPT boundedUpper = Max(0., Min(upperDepth, profileBottom));
    const FLTPT boundedLower = Max(boundedUpper, Min(lowerDepth, profileBottom));
    FLTPT drainableDepth = 0.;
    FLTPT layerTop = 0.;
    for (int k = 0; k < nLayers; k++) {
        const FLTPT layerBottom = Max(layerBottomDepth[k], layerTop);
        const FLTPT overlapTop = Max(layerTop, boundedUpper);
        const FLTPT overlapBottom = Min(layerBottom, boundedLower);
        if (overlapBottom > overlapTop + UTIL_ZERO) {
            const FLTPT fc = Max(fieldCapacity[k], 0.);
            const FLTPT por = Max(porosity[k], fc);
            const FLTPT theta = Max(0., Min(moisture[k], por));
            drainableDepth += Max(theta - fc, 0.) * (overlapBottom - overlapTop);
        }
        layerTop = layerBottom;
    }
    return Max(drainableDepth, 0.);
}

FLTPT SSR_KIT::RemoveWaterFromDepthInterval(
    const int nLayers, const FLTPT* layerBottomDepth, FLTPT* moisture,
    const FLTPT* fieldCapacity, const FLTPT* porosity,
    const FLTPT upperDepth, const FLTPT lowerDepth, const FLTPT waterDepth) {
    if (nLayers <= 0 || layerBottomDepth == nullptr || moisture == nullptr ||
        fieldCapacity == nullptr || porosity == nullptr) {
        return Max(waterDepth, 0.);
    }
    const FLTPT profileBottom = Max(layerBottomDepth[nLayers - 1], 0.);
    const FLTPT boundedUpper = Max(0., Min(upperDepth, profileBottom));
    const FLTPT boundedLower = Max(boundedUpper, Min(lowerDepth, profileBottom));
    FLTPT remaining = Max(waterDepth, 0.);
    FLTPT layerTop = 0.;
    for (int k = 0; k < nLayers && remaining > UTIL_ZERO; k++) {
        const FLTPT layerBottom = Max(layerBottomDepth[k], layerTop);
        const FLTPT overlapTop = Max(layerTop, boundedUpper);
        const FLTPT overlapBottom = Min(layerBottom, boundedLower);
        if (overlapBottom > overlapTop + UTIL_ZERO) {
            const FLTPT fc = Max(fieldCapacity[k], 0.);
            const FLTPT por = Max(porosity[k], fc);
            moisture[k] = Max(0., Min(moisture[k], por));
            const FLTPT overlapDepth = overlapBottom - overlapTop;
            const FLTPT drainable = Max(moisture[k] - fc, 0.) * overlapDepth;
            const FLTPT removed = Min(remaining, drainable);
            moisture[k] -= removed / Max(LayerThickness(layerBottomDepth, k), UTIL_ZERO);
            remaining -= removed;
        }
        layerTop = layerBottom;
    }
    return Max(remaining, 0.);
}

FLTPT SSR_KIT::AddWaterBottomUp(const int nLayers, const FLTPT* layerBottomDepth,
    FLTPT* moisture, const FLTPT* porosity,
    FLTPT waterDepth) {
    if (nLayers <= 0 || layerBottomDepth == nullptr || moisture == nullptr ||
        porosity == nullptr) {
        return Max(waterDepth, 0.);
    }
    FLTPT remaining = Max(waterDepth, 0.);
    for (int k = nLayers - 1; k >= 0 && remaining > UTIL_ZERO; k--) {
        const FLTPT layerDepth = LayerThickness(layerBottomDepth, k);
        if (layerDepth <= UTIL_ZERO) continue;
        const FLTPT por = Max(porosity[k], 0.);
        moisture[k] = Max(0., Min(moisture[k], por));
        const FLTPT capacity = Max(por - moisture[k], 0.) * layerDepth;
        const FLTPT added = Min(remaining, capacity);
        moisture[k] += added / layerDepth;
        remaining -= added;
    }
    return Max(remaining, 0.);
}

FLTPT SSR_KIT::RemoveWaterFromWaterTableDown(
    const int nLayers, const FLTPT* layerBottomDepth, FLTPT* moisture,
    const FLTPT* fieldCapacity, const FLTPT* porosity, const FLTPT waterTableDepth,
    FLTPT waterDepth) {
    if (nLayers <= 0 || layerBottomDepth == nullptr || moisture == nullptr ||
        fieldCapacity == nullptr || porosity == nullptr) {
        return Max(waterDepth, 0.);
    }
    int waterTableLayer = nLayers;
    for (int k = 0; k < nLayers; k++) {
        const FLTPT layerBottom = Max(layerBottomDepth[k], 0.);
        if (layerBottom > waterTableDepth + UTIL_ZERO) {
            waterTableLayer = k;
            break;
        }
    }
    FLTPT remaining = Max(waterDepth, 0.);
    for (int k = waterTableLayer; k < nLayers && remaining > UTIL_ZERO; k++) {
        const FLTPT layerDepth = LayerThickness(layerBottomDepth, k);
        if (layerDepth <= UTIL_ZERO) continue;
        const FLTPT fc = Max(fieldCapacity[k], 0.);
        const FLTPT por = Max(porosity[k], fc);
        moisture[k] = Max(0., Min(moisture[k], por));
        const FLTPT drainable = Max(moisture[k] - fc, 0.) * layerDepth;
        const FLTPT removed = Min(remaining, drainable);
        moisture[k] -= removed / layerDepth;
        remaining -= removed;
    }
    return Max(remaining, 0.);
}

FLTPT SSR_KIT::CalculateHydraulicGradient(
    const FLTPT terrainSlope, const FLTPT upstreamWaterTableDepth,
    const FLTPT downstreamWaterTableDepth, const FLTPT flowLength) {
    if (!std::isfinite(terrainSlope) || !std::isfinite(upstreamWaterTableDepth) ||
        !std::isfinite(downstreamWaterTableDepth) || !std::isfinite(flowLength) ||
        flowLength <= UTIL_ZERO) {
        return 0.;
    }
    const FLTPT waterTableGradient =
        (downstreamWaterTableDepth - upstreamWaterTableDepth) * 0.001 / flowLength;
    return Max(terrainSlope + waterTableGradient, 0.);
}

bool SSR_KIT::CheckInputData() {
    CHECK_POSITIVE(M_SSR_KIT[0], m_nCells);
    CHECK_POSITIVE(M_SSR_KIT[0], m_maxSoilLyrs);
    CHECK_POSITIVE(M_SSR_KIT[0], m_dt);
    CHECK_POSITIVE(M_SSR_KIT[0], m_cellWidth);
    CHECK_POSITIVE(M_SSR_KIT[0], m_maxSubsteps);
    CHECK_POINTER(M_SSR_KIT[0], m_nSoilLyrs);
    CHECK_POINTER(M_SSR_KIT[0], m_streamLink);
    CHECK_POINTER(M_SSR_KIT[0], m_slope);
    CHECK_POINTER(M_SSR_KIT[0], m_chWidth);
    CHECK_POINTER(M_SSR_KIT[0], m_surfaceRunoff);
    CHECK_POINTER(M_SSR_KIT[0], m_soilDepth);
    CHECK_POINTER(M_SSR_KIT[0], m_conductivity);
    CHECK_POINTER(M_SSR_KIT[0], m_porosity);
    CHECK_POINTER(M_SSR_KIT[0], m_fieldCapacity);
    CHECK_POINTER(M_SSR_KIT[0], m_soilMoisture);
    CHECK_POINTER(M_SSR_KIT[0], m_flowInIndex);
    bool hasStreamCell = false;
    for (int id = 0; id < m_nCells; id++) {
        if (m_streamLink[id] > 0) {
            hasStreamCell = true;
            break;
        }
    }
    if (hasStreamCell) {
        CHECK_POINTER(M_SSR_KIT[0], m_reachDepth);
    }
    if (!m_soilGeometryValidated) {
        for (int id = 0; id < m_nCells; id++) {
            if (m_nSoilLyrs[id] <= 0 || m_nSoilLyrs[id] > m_maxSoilLyrs) {
                throw ModelException(M_SSR_KIT[0], "CheckInputData",
                    "Invalid number of soil layers at cell " +
                        ValueToString(id) + ".");
            }
            FLTPT previousBottom = 0.;
            for (int k = 0; k < m_nSoilLyrs[id]; k++) {
                const FLTPT layerBottom = m_soilDepth[id][k];
                if (!std::isfinite(layerBottom) ||
                    layerBottom <= previousBottom + UTIL_ZERO) {
                    throw ModelException(
                        M_SSR_KIT[0], "CheckInputData",
                        "SOILDEPTH must contain strictly increasing layer-bottom depths at cell " +
                            ValueToString(id) + ".");
                }
                previousBottom = layerBottom;
            }
            if (m_streamLink[id] > 0) {
                const int reachId = m_streamLink[id];
                if (reachId > m_nReaches || !std::isfinite(m_reachDepth[reachId]) ||
                    m_reachDepth[reachId] <= UTIL_ZERO) {
                    throw ModelException(
                        M_SSR_KIT[0], "CheckInputData",
                        "STREAM_LINK references a missing or nonpositive CH_DEPTH at cell " +
                            ValueToString(id) + ".");
                }
            }
        }
        m_soilGeometryValidated = true;
    }
    return true;
}

void SSR_KIT::InitialOutputs() {
    CheckInputData();
    if (m_qSoil == nullptr) {
        Initialize1DArray(m_nCells, m_qSoil, 0.);
        Initialize1DArray(m_nCells, m_returnFlow, 0.);
        Initialize1DArray(m_nCells, m_waterTableDepth, 0.);
        Initialize1DArray(m_nCells, m_saturatedDepth, 0.);
        Initialize1DArray(m_nCells, m_qOut, 0.);
    }
    if (!m_topologyReady) BuildTopologyWeights();
}

void SSR_KIT::BuildTopologyWeights() {
    Release1DArray(m_outWeightSum);
    Release1DArray(m_outConnectionCount);
    Initialize1DArray(m_nCells, m_outWeightSum, 0.);
    Initialize1DArray(m_nCells, m_outConnectionCount, 0);
    for (int id = 0; id < m_nCells; id++) {
        const int nUpstream = m_flowInIndex[id][0];
        for (int upIndex = 1; upIndex <= nUpstream; upIndex++) {
            const int up = m_flowInIndex[id][upIndex];
            if (up < 0 || up >= m_nCells) continue;
            FLTPT rawWeight = 1.;
            if (m_flowInFraction != nullptr && std::isfinite(m_flowInFraction[id][upIndex])) {
                rawWeight = Max(m_flowInFraction[id][upIndex], 0.);
            }
            m_outWeightSum[up] += rawWeight;
            m_outConnectionCount[up]++;
        }
    }
    m_topologyReady = true;
}

FLTPT SSR_KIT::GetFlowInWeight(const int id, const int upIndex) const {
    const int up = m_flowInIndex[id][upIndex];
    if (up < 0 || up >= m_nCells || m_outConnectionCount[up] <= 0) return 0.;
    if (m_flowInFraction == nullptr) return 1. / m_outConnectionCount[up];
    const FLTPT rawWeight = std::isfinite(m_flowInFraction[id][upIndex]) ?
                            Max(m_flowInFraction[id][upIndex], 0.) : 0.;
    if (m_outWeightSum[up] <= UTIL_ZERO) return 1. / m_outConnectionCount[up];
    return rawWeight / m_outWeightSum[up];
}

void SSR_KIT::CalculateRoutingGradients(
    const vector<FLTPT>& waterTableDepth, vector<FLTPT>* gradients) const {
    gradients->assign(m_nCells, 0.);
    if (m_gradientMode == 0) {
        for (int id = 0; id < m_nCells; id++) {
            (*gradients)[id] = Max(m_slope[id], 0.);
        }
        return;
    }
    for (int id = 0; id < m_nCells; id++) {
        const int nUpstream = m_flowInIndex[id][0];
        for (int upIndex = 1; upIndex <= nUpstream; upIndex++) {
            const int up = m_flowInIndex[id][upIndex];
            if (up < 0 || up >= m_nCells) continue;
            (*gradients)[up] += GetFlowInWeight(id, upIndex) *
                CalculateHydraulicGradient(
                    Max(m_slope[up], 0.), waterTableDepth[up],
                    waterTableDepth[id], m_cellWidth);
        }
    }
}

FLTPT SSR_KIT::ConnectedDrainableDepth(const int id, const FLTPT waterTableDepth) const {
    FLTPT drainableDepth = 0.;
    for (int k = 0; k < m_nSoilLyrs[id]; k++) {
        const FLTPT layerDepth = LayerThickness(m_soilDepth[id], k);
        const FLTPT layerBottom = Max(m_soilDepth[id][k], 0.);
        if (layerBottom <= waterTableDepth + UTIL_ZERO) continue;
        const FLTPT fc = Max(m_fieldCapacity[id][k], 0.);
        const FLTPT por = Max(m_porosity[id][k], fc);
        const FLTPT theta = Max(0., Min(m_soilMoisture[id][k], por));
        drainableDepth += Max(theta - fc, 0.) * layerDepth;
    }
    return Max(drainableDepth, 0.);
}

FLTPT SSR_KIT::CellTransmissivity(const int id, const FLTPT waterTableDepth) const {
    if (m_decayFactor > 0.) {
        const FLTPT totalDepth = Max(m_soilDepth[id][m_nSoilLyrs[id] - 1], 0.);
        FLTPT surfaceLateralK = 0.;
        for (int k = 0; k < m_nSoilLyrs[id]; k++) {
            if (LayerThickness(m_soilDepth[id], k) <= UTIL_ZERO) continue;
            const FLTPT verticalK = Max(m_conductivity[id][k], 0.) * 0.001 / 3600.;
            if (verticalK <= 0.) continue;
            surfaceLateralK = verticalK * m_khRatio;
            break;
        }
        return CalculateTwoPartTransmissivity(
            totalDepth, waterTableDepth, surfaceLateralK,
            m_decayFactor, m_depthThresholdRatio);
    }
    FLTPT transmissivity = 0.;
    FLTPT layerTop = 0.;
    for (int k = 0; k < m_nSoilLyrs[id]; k++) {
        const FLTPT layerBottom = Max(m_soilDepth[id][k], layerTop);
        const FLTPT saturatedDepth = Max(0., layerBottom - Max(waterTableDepth, layerTop));
        const FLTPT verticalK = Max(m_conductivity[id][k], 0.) * 0.001 / 3600.;
        transmissivity += verticalK * m_khRatio * saturatedDepth * 0.001;
        layerTop = layerBottom;
    }
    return Max(transmissivity, 0.);
}

FLTPT SSR_KIT::CellTransmissivityBetween(
    const int id, const FLTPT upperDepth, const FLTPT lowerDepth) const {
    const FLTPT totalDepth = Max(m_soilDepth[id][m_nSoilLyrs[id] - 1], 0.);
    const FLTPT boundedUpper = Max(0., Min(upperDepth, totalDepth));
    const FLTPT boundedLower = Max(boundedUpper, Min(lowerDepth, totalDepth));
    if (boundedLower <= boundedUpper + UTIL_ZERO) return 0.;
    if (m_decayFactor > 0.) {
        FLTPT surfaceLateralK = 0.;
        for (int k = 0; k < m_nSoilLyrs[id]; k++) {
            if (LayerThickness(m_soilDepth[id], k) <= UTIL_ZERO) continue;
            const FLTPT verticalK = Max(m_conductivity[id][k], 0.) * 0.001 / 3600.;
            if (verticalK <= 0.) continue;
            surfaceLateralK = verticalK * m_khRatio;
            break;
        }
        return CalculateTwoPartTransmissivityBetween(
            totalDepth, boundedUpper, boundedLower, surfaceLateralK,
            m_decayFactor, m_depthThresholdRatio);
    }
    FLTPT transmissivity = 0.;
    FLTPT layerTop = 0.;
    for (int k = 0; k < m_nSoilLyrs[id]; k++) {
        const FLTPT layerBottom = Max(m_soilDepth[id][k], layerTop);
        const FLTPT overlapTop = Max(layerTop, boundedUpper);
        const FLTPT overlapBottom = Min(layerBottom, boundedLower);
        if (overlapBottom > overlapTop + UTIL_ZERO) {
            const FLTPT verticalK = Max(m_conductivity[id][k], 0.) * 0.001 / 3600.;
            transmissivity += verticalK * m_khRatio *
                (overlapBottom - overlapTop) * 0.001;
        }
        layerTop = layerBottom;
    }
    return Max(transmissivity, 0.);
}

FLTPT SSR_KIT::BankHeight(const int id) const {
    if (id < 0 || id >= m_nCells || m_streamLink[id] <= 0 ||
        m_streamLink[id] > m_nReaches || m_reachDepth == nullptr) {
        return 0.;
    }
    const FLTPT totalDepth = Max(m_soilDepth[id][m_nSoilLyrs[id] - 1], 0.);
    return Max(0., Min(m_reachDepth[m_streamLink[id]] * 1000., totalDepth));
}

int SSR_KIT::DetermineSubsteps() const {
    int substeps = 1;
    const FLTPT area = m_cellWidth * m_cellWidth;
    vector<FLTPT> waterTableDepth(m_nCells, 0.);
    for (int id = 0; id < m_nCells; id++) {
        waterTableDepth[id] = CalculateWaterTableDepth(
            m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
            m_fieldCapacity[id], m_porosity[id]);
    }
    vector<FLTPT> gradients;
    CalculateRoutingGradients(waterTableDepth, &gradients);
    for (int id = 0; id < m_nCells; id++) {
        if (m_streamLink[id] > 0 || m_outConnectionCount[id] <= 0) continue;
        const FLTPT wt = waterTableDepth[id];
        const FLTPT availableVolume = ConnectedDrainableDepth(id, wt) * 0.001 * area;
        if (availableVolume <= UTIL_ZERO) continue;
        const FLTPT q = m_cellWidth * gradients[id] * CellTransmissivity(id, wt);
        const FLTPT denominator = Max(m_maxDrainFraction * availableVolume, UTIL_ZERO);
        const int needed = static_cast<int>(ceil(q * m_dt / denominator));
        substeps = Max(substeps, needed);
    }
    return Min(Max(substeps, 1), m_maxSubsteps);
}

double SSR_KIT::SoilWaterVolume() const {
    const double area = static_cast<double>(m_cellWidth) * m_cellWidth;
    double volume = 0.;
    for (int id = 0; id < m_nCells; id++) {
        for (int k = 0; k < m_nSoilLyrs[id]; k++) {
            const FLTPT por = Max(m_porosity[id][k], 0.);
            const FLTPT theta = Max(0., Min(m_soilMoisture[id][k], por));
            volume += static_cast<double>(theta) * LayerThickness(m_soilDepth[id], k) *
                0.001 * area;
        }
    }
    return volume;
}

int SSR_KIT::Execute() {
    InitialOutputs();
    m_executeCount++;
    const char* diagEnv = getenv("SEIMS_SSR_KIT_DIAG");
    if (diagEnv == nullptr) {
        diagEnv = getenv("SEIMS_SSR_DHSVM_DIAG");
    }
    const bool diagEnabled = diagEnv != nullptr && string(diagEnv) != "0" &&
        !m_outpath.empty();
    int stateDiagInterval = 12; // Hourly for the standard 5-minute storm time step.
    const char* intervalEnv = getenv("SEIMS_SSR_KIT_STATE_DIAG_INTERVAL");
    if (intervalEnv == nullptr) {
        intervalEnv = getenv("SEIMS_SSR_DHSVM_STATE_DIAG_INTERVAL");
    }
    if (intervalEnv != nullptr) stateDiagInterval = Max(1, atoi(intervalEnv));
    const bool stateDiagDue = diagEnabled &&
        (m_executeCount == 1 || m_executeCount % stateDiagInterval == 0);
    const FLTPT area = m_cellWidth * m_cellWidth;
    const double startSoilVolume = diagEnabled ? SoilWaterVolume() : 0.;
    const int substeps = DetermineSubsteps();
    const FLTPT dtSub = static_cast<FLTPT>(m_dt) / substeps;

    vector<FLTPT> qSub(m_nCells, 0.);
    vector<FLTPT> incomingVolume(m_nCells, 0.);
    vector<FLTPT> accumulatedIncomingVolume(m_nCells, 0.);
    vector<FLTPT> qOutVolume(m_nCells, 0.);
    vector<FLTPT> qSoilVolume(m_nCells, 0.);
    vector<FLTPT> routingWaterTable(m_nCells, 0.);
    vector<FLTPT> routingGradient(m_nCells, 0.);
    double bankReleaseVolume = 0.;
    double bankInflowVolume = 0.;
    for (int id = 0; id < m_nCells; id++) {
        m_qSoil[id] = 0.;
        m_returnFlow[id] = 0.;
        m_qOut[id] = 0.;
    }

    // Match DHSVM's stream-cell exchange: use only the beginning-of-step
    // stream-soil state, the water-table-to-bank transmissivity interval,
    // and the water physically available in that interval.
    for (int id = 0; id < m_nCells; id++) {
        if (m_streamLink[id] <= 0) continue;
        const FLTPT waterTable = CalculateWaterTableDepth(
            m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
            m_fieldCapacity[id], m_porosity[id]);
        const FLTPT bankHeight = BankHeight(id);
        if (waterTable >= bankHeight - UTIL_ZERO) continue;
        const FLTPT availableDepth = CalculateDrainableWaterDepthBetween(
            m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
            m_fieldCapacity[id], m_porosity[id], waterTable, bankHeight);
        const FLTPT availableVolume = availableDepth * 0.001 * area;
        const FLTPT potentialVolume =
            CellTransmissivityBetween(id, waterTable, bankHeight) *
            CalculateBankGradientWidth(waterTable, bankHeight) * m_dt;
        const FLTPT releaseVolume = Min(Max(potentialVolume, 0.), availableVolume);
        if (releaseVolume <= UTIL_ZERO) continue;
        const FLTPT unmetDepth = RemoveWaterFromDepthInterval(
            m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
            m_fieldCapacity[id], m_porosity[id], waterTable, bankHeight,
            releaseVolume / area * 1000.);
        if (unmetDepth > 1.e-4) {
            throw ModelException(M_SSR_KIT[0], "Execute",
                "Bank exchange exceeded interval drainable water.");
        }
        qSoilVolume[id] += releaseVolume;
        bankReleaseVolume += releaseVolume;
    }

    for (int sub = 0; sub < substeps; sub++) {
        for (int id = 0; id < m_nCells; id++) {
            routingWaterTable[id] = CalculateWaterTableDepth(
                m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
                m_fieldCapacity[id], m_porosity[id]);
        }
        CalculateRoutingGradients(routingWaterTable, &routingGradient);
        for (int id = 0; id < m_nCells; id++) {
            qSub[id] = 0.;
            incomingVolume[id] = 0.;
            const FLTPT wt = routingWaterTable[id];
            if (m_streamLink[id] > 0 || m_outConnectionCount[id] <= 0) continue;
            const FLTPT availableVolume = ConnectedDrainableDepth(id, wt) * 0.001 * area;
            if (availableVolume <= UTIL_ZERO) continue;
            const FLTPT potentialQ = m_cellWidth * routingGradient[id] *
                CellTransmissivity(id, wt);
            const FLTPT allowedVolume = m_maxDrainFraction * availableVolume;
            const FLTPT outVolume = Min(potentialQ * dtSub, allowedVolume);
            qSub[id] = Max(outVolume / dtSub, 0.);
        }

        // Gather all upstream outflows from the same beginning-of-substep state.
        for (int id = 0; id < m_nCells; id++) {
            const int nUpstream = m_flowInIndex[id][0];
            for (int upIndex = 1; upIndex <= nUpstream; upIndex++) {
                const int up = m_flowInIndex[id][upIndex];
                if (up < 0 || up >= m_nCells) continue;
                incomingVolume[id] += qSub[up] * dtSub * GetFlowInWeight(id, upIndex);
            }
        }

        for (int id = 0; id < m_nCells; id++) {
            const FLTPT outVolume = qSub[id] * dtSub;
            qOutVolume[id] += outVolume;
            if (m_streamLink[id] > 0) {
                accumulatedIncomingVolume[id] += incomingVolume[id];
                continue;
            }
            accumulatedIncomingVolume[id] += incomingVolume[id];
            if (outVolume > UTIL_ZERO) {
                const FLTPT wt = CalculateWaterTableDepth(
                    m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
                    m_fieldCapacity[id], m_porosity[id]);
                const FLTPT unmetDepth = RemoveWaterFromWaterTableDown(
                    m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
                    m_fieldCapacity[id], m_porosity[id], wt,
                    outVolume / area * 1000.);
                if (unmetDepth > 1.e-4) {
                    throw ModelException(M_SSR_KIT[0], "Execute",
                        "Connected drainable water was insufficient for routed outflow.");
                }
            }
        }
    }

    // Match DHSVM's routing sweep: current-step inflow is distributed only after
    // every source-cell outflow has been calculated. It can affect storage now,
    // but cannot become a new lateral source until the next global time step.
    for (int id = 0; id < m_nCells; id++) {
        if (accumulatedIncomingVolume[id] <= UTIL_ZERO) continue;
        if (m_streamLink[id] > 0) {
            bankInflowVolume += accumulatedIncomingVolume[id];
        }
        const FLTPT excessDepth = AddWaterBottomUp(
            m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
            m_porosity[id], accumulatedIncomingVolume[id] / area * 1000.);
        m_returnFlow[id] += excessDepth;
    }

    double channelVolume = 0.;
    double returnVolume = 0.;
    int eligibleCells = 0;
    int saturatedCells = 0;
    double waterTableSum = 0.;
    double saturatedDepthSum = 0.;
    double activeSaturatedDepthSum = 0.;
    FLTPT saturatedDepthMax = 0.;
    double transmissivitySum = 0.;
    FLTPT transmissivityMax = 0.;
    double potentialQSum = 0.;
    double actualQOutSum = 0.;
    int streamCells = 0;
    int bankActiveCells = 0;
    double streamSoilStorage = 0.;
    double bankExchangeStorage = 0.;
    double bankBlockedDrainable = 0.;
    vector<FLTPT> finalWaterTable(m_nCells, 0.);
    for (int id = 0; id < m_nCells; id++) {
        m_qOut[id] = qOutVolume[id] / m_dt;
        m_qSoil[id] = qSoilVolume[id] / m_dt;
        channelVolume += qSoilVolume[id];
        returnVolume += m_returnFlow[id] * 0.001 * area;
        if (m_returnFlow[id] > 0.) {
            m_surfaceRunoff[id] += m_returnFlow[id];
            // SUR_SGA calculates the remaining infiltration capacity before lateral routing.
            // Once lateral inflow fills this profile to the surface, that earlier capacity is
            // stale and must not immediately re-infiltrate saturation-excess return flow.
            if (m_infilCapacitySurplus != nullptr) m_infilCapacitySurplus[id] = 0.;
        }
        m_waterTableDepth[id] = CalculateWaterTableDepth(
            m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
            m_fieldCapacity[id], m_porosity[id]);
        finalWaterTable[id] = m_waterTableDepth[id];
        const FLTPT totalDepth = Max(m_soilDepth[id][m_nSoilLyrs[id] - 1], 0.);
        m_saturatedDepth[id] = Max(totalDepth - m_waterTableDepth[id], 0.);
        if (stateDiagDue && m_streamLink[id] > 0) {
            streamCells++;
            for (int k = 0; k < m_nSoilLyrs[id]; k++) {
                const FLTPT theta = Max(0., Min(m_soilMoisture[id][k],
                                                Max(m_porosity[id][k], 0.)));
                streamSoilStorage += theta * LayerThickness(m_soilDepth[id], k) *
                    0.001 * area;
            }
            const FLTPT bankHeight = BankHeight(id);
            if (m_waterTableDepth[id] < bankHeight - UTIL_ZERO) {
                bankActiveCells++;
                bankExchangeStorage += CalculateDrainableWaterDepthBetween(
                                           m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
                                           m_fieldCapacity[id], m_porosity[id],
                                           m_waterTableDepth[id], bankHeight) * 0.001 * area;
            }
            bankBlockedDrainable += CalculateDrainableWaterDepthBetween(
                                        m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
                                        m_fieldCapacity[id], m_porosity[id],
                                        bankHeight, totalDepth) * 0.001 * area;
        }
    }
    vector<FLTPT> finalGradient;
    CalculateRoutingGradients(finalWaterTable, &finalGradient);
    for (int id = 0; id < m_nCells; id++) {
        if (stateDiagDue && m_streamLink[id] <= 0) {
            eligibleCells++;
            waterTableSum += m_waterTableDepth[id];
            saturatedDepthSum += m_saturatedDepth[id];
            saturatedDepthMax = Max(saturatedDepthMax, m_saturatedDepth[id]);
            const FLTPT transmissivity = CellTransmissivity(id, m_waterTableDepth[id]);
            transmissivitySum += transmissivity;
            transmissivityMax = Max(transmissivityMax, transmissivity);
            potentialQSum += m_cellWidth * finalGradient[id] * transmissivity;
            actualQOutSum += m_qOut[id];
            if (m_saturatedDepth[id] > UTIL_ZERO) {
                saturatedCells++;
                activeSaturatedDepthSum += m_saturatedDepth[id];
            }
        }
    }


    if (diagEnabled) {
        const double endSoilVolume = SoilWaterVolume();
        const double closure = startSoilVolume - endSoilVolume - channelVolume - returnVolume;
        const string diagPath = m_outpath + SEP + "SSR_KIT_balance.csv";
        std::ifstream existing(diagPath.c_str());
        const bool needHeader = !existing.good();
        existing.close();
        std::ofstream fs(diagPath.c_str(), std::ios::out | std::ios::app);
        if (fs.is_open()) {
            if (needHeader) {
                fs << "time,substeps,soil_start_m3,soil_end_m3,channel_m3,return_m3,closure_m3\n";
            }
            fs << ConvertToString2(m_date) << "," << substeps << ","
               << startSoilVolume << "," << endSoilVolume << ","
               << channelVolume << "," << returnVolume << "," << closure << "\n";
        }
        if (stateDiagDue) {
            const string statePath = m_outpath + SEP + "SSR_KIT_state.csv";
            std::ifstream existingState(statePath.c_str());
            const bool needStateHeader = !existingState.good();
            existingState.close();
            std::ofstream state(statePath.c_str(), std::ios::out | std::ios::app);
            if (state.is_open()) {
                if (needStateHeader) {
                    state << "time,eligible_cells,saturated_cells,saturated_fraction,"
                          << "water_table_mean_mm,sat_depth_mean_mm,"
                          << "sat_depth_active_mean_mm,sat_depth_max_mm,"
                          << "transmissivity_mean_m2_s,transmissivity_max_m2_s,"
                          << "potential_q_sum_m3_s,qout_sum_m3_s,channel_m3,return_m3,"
                          << "bank_inflow_m3,bank_release_m3,stream_soil_storage_m3,"
                          << "bank_exchange_storage_m3,bank_blocked_drainable_m3,"
                          << "stream_cells,bank_active_cells,bank_active_fraction,"
                          << "substeps\n";
                }
                const double eligibleDenominator = Max(eligibleCells, 1);
                const double saturatedDenominator = Max(saturatedCells, 1);
                state << ConvertToString2(m_date) << "," << eligibleCells << ","
                      << saturatedCells << ","
                      << saturatedCells / eligibleDenominator << ","
                      << waterTableSum / eligibleDenominator << ","
                      << saturatedDepthSum / eligibleDenominator << ","
                      << activeSaturatedDepthSum / saturatedDenominator << ","
                      << saturatedDepthMax << ","
                      << transmissivitySum / eligibleDenominator << ","
                      << transmissivityMax << ","
                      << potentialQSum << "," << actualQOutSum << ","
                      << channelVolume << "," << returnVolume << ","
                      << bankInflowVolume << "," << bankReleaseVolume << ","
                      << streamSoilStorage << "," << bankExchangeStorage << ","
                      << bankBlockedDrainable << "," << streamCells << ","
                      << bankActiveCells << ","
                      << bankActiveCells / static_cast<double>(Max(streamCells, 1)) << ","
                      << substeps << "\n";
            }
        }
    }
    return 0;
}

void SSR_KIT::SetValue(const char* key, const FLTPT value) {
    const string sk(key);
    if (StringMatch(sk, Tag_CellWidth[0])) m_cellWidth = value;
    else if (StringMatch(sk, VAR_DHSVM_KH_RATIO[0])) m_khRatio = Max(value, 0.);
    else if (StringMatch(sk, VAR_DHSVM_K_DECAY_FACTOR[0])) {
        if (!std::isfinite(value) || value < 0.) {
            throw ModelException(M_SSR_KIT[0], "SetValue",
                "DHSVM_K_DECAY_FACTOR must be finite and nonnegative.");
        }
        m_decayFactor = value;
    } else if (StringMatch(sk, VAR_DHSVM_DEPTH_THRESHOLD_RATIO[0])) {
        if (!std::isfinite(value) || value <= 0. || value >= 1.) {
            throw ModelException(M_SSR_KIT[0], "SetValue",
                "DHSVM_DEPTH_THRESHOLD_RATIO must be between 0 and 1.");
        }
        m_depthThresholdRatio = value;
    } else if (StringMatch(sk, VAR_DHSVM_MAX_DRAIN_FRAC[0])) {
        m_maxDrainFraction = Max(1.e-3, Min(value, 1.));
    } else if (StringMatch(sk, VAR_DHSVM_MAX_SUBSTEPS[0])) {
        m_maxSubsteps = Max(1, CVT_INT(value));
    } else if (StringMatch(sk, VAR_DHSVM_GRADIENT_MODE[0])) {
        const int mode = CVT_INT(value);
        if (!std::isfinite(value) || mode < 0 || mode > 1 ||
            fabs(value - mode) > UTIL_ZERO) {
            throw ModelException(M_SSR_KIT[0], "SetValue",
                "DHSVM_GRADIENT_MODE must be 0 or 1.");
        }
        m_gradientMode = mode;
    } else {
        throw ModelException(M_SSR_KIT[0], "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void SSR_KIT::SetValue(const char* key, const int value) {
    const string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep[0]) || StringMatch(sk, Tag_TimeStep[0])) m_dt = value;
    else if (StringMatch(sk, Tag_CellSize[0])) m_nCells = value;
    else if (StringMatch(sk, VAR_DHSVM_MAX_SUBSTEPS[0])) m_maxSubsteps = Max(1, value);
    else if (StringMatch(sk, VAR_DHSVM_GRADIENT_MODE[0])) {
        if (value < 0 || value > 1) {
            throw ModelException(M_SSR_KIT[0], "SetValue",
                "DHSVM_GRADIENT_MODE must be 0 or 1.");
        }
        m_gradientMode = value;
    }
    else {
        throw ModelException(M_SSR_KIT[0], "SetValue", "Integer parameter " + sk + " does not exist.");
    }
}

void SSR_KIT::SetReaches(clsReaches* reaches) {
    if (reaches == nullptr) {
        throw ModelException(M_SSR_KIT[0], "SetReaches",
            "The reaches input cannot be nullptr.");
    }
    m_nReaches = reaches->GetReachNumber();
    reaches->GetReachesSingleProperty(REACH_DEPTH, &m_reachDepth);
    m_soilGeometryValidated = false;
}

void SSR_KIT::Set1DData(const char* key, const int n, FLTPT* data) {
    const string sk(key);
    if (StringMatch(sk, REACH_DEPTH)) {
        if (n <= 1 || data == nullptr) {
            throw ModelException(M_SSR_KIT[0], "Set1DData",
                "CH_DEPTH must contain a count and at least one reach.");
        }
        m_nReaches = n - 1;
        m_reachDepth = data;
        m_soilGeometryValidated = false;
        return;
    }
    if (m_nCells <= 0) m_nCells = n;
    if (n != m_nCells) throw ModelException(M_SSR_KIT[0], "Set1DData", "Input size mismatch.");
    if (StringMatch(sk, VAR_SLOPE[0])) m_slope = data;
    else if (StringMatch(sk, VAR_CHWIDTH[0])) m_chWidth = data;
    else if (StringMatch(sk, VAR_SURU[0])) m_surfaceRunoff = data;
    else if (StringMatch(sk, VAR_INFILCAPSURPLUS[0])) m_infilCapacitySurplus = data;
    else throw ModelException(M_SSR_KIT[0], "Set1DData", "Parameter " + sk + " does not exist.");
}

void SSR_KIT::Set1DData(const char* key, const int n, int* data) {
    if (m_nCells <= 0) m_nCells = n;
    if (n != m_nCells) throw ModelException(M_SSR_KIT[0], "Set1DData", "Input size mismatch.");
    const string sk(key);
    if (StringMatch(sk, VAR_STREAM_LINK[0])) m_streamLink = data;
    else if (StringMatch(sk, VAR_SOILLAYERS[0])) {
        m_nSoilLyrs = data;
        m_soilGeometryValidated = false;
    }
    else throw ModelException(M_SSR_KIT[0], "Set1DData", "Parameter " + sk + " does not exist.");
}

void SSR_KIT::Set2DData(const char* key, const int nrows, const int ncols, FLTPT** data) {
    if (m_nCells <= 0) m_nCells = nrows;
    if (nrows != m_nCells) throw ModelException(M_SSR_KIT[0], "Set2DData", "Input row mismatch.");
    const string sk(key);
    if (StringMatch(sk, Tag_FLOWIN_FRACTION[0])) {
        m_flowInFraction = data;
        m_topologyReady = false;
        return;
    }
    if (m_maxSoilLyrs <= 0) m_maxSoilLyrs = ncols;
    if (ncols != m_maxSoilLyrs) {
        throw ModelException(M_SSR_KIT[0], "Set2DData", "Input column mismatch.");
    }
    if (StringMatch(sk, VAR_SOILDEPTH[0])) {
        m_soilDepth = data;
        m_soilGeometryValidated = false;
    }
    else if (StringMatch(sk, VAR_CONDUCT[0])) m_conductivity = data;
    else if (StringMatch(sk, VAR_POROST[0])) m_porosity = data;
    else if (StringMatch(sk, VAR_FIELDCAP[0])) m_fieldCapacity = data;
    else if (StringMatch(sk, VAR_SOL_ST[0])) m_soilMoisture = data;
    else {
        throw ModelException(M_SSR_KIT[0], "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void SSR_KIT::Set2DData(const char* key, const int nrows, const int, int** data) {
    const string sk(key);
    if (StringMatch(sk, Tag_FLOWIN_INDEX[0])) {
        if (m_nCells <= 0) m_nCells = nrows;
        if (nrows != m_nCells) throw ModelException(M_SSR_KIT[0], "Set2DData", "Flow input row mismatch.");
        m_flowInIndex = data;
        m_topologyReady = false;
    } else {
        throw ModelException(M_SSR_KIT[0], "Set2DData", "Integer parameter " + sk + " does not exist.");
    }
}

void SSR_KIT::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    const string sk(key);
    if (StringMatch(sk, VAR_QSOIL[0])) *data = m_qSoil;
    else if (StringMatch(sk, VAR_RETURNFLOW[0])) *data = m_returnFlow;
    else if (StringMatch(sk, VAR_DHSVM_WT_DEPTH[0])) *data = m_waterTableDepth;
    else if (StringMatch(sk, VAR_DHSVM_SAT_DEPTH[0])) *data = m_saturatedDepth;
    else if (StringMatch(sk, VAR_DHSVM_QOUT[0])) *data = m_qOut;
    else throw ModelException(M_SSR_KIT[0], "Get1DData", "Output " + sk + " does not exist.");
    *n = m_nCells;
}
