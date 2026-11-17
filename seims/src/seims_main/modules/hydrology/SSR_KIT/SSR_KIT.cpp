/*! \brief Saturated soil routing with optional bottom capacity excess. */
#include "SSR_KIT.h"

#include "text.h"

#include <cmath>
#include <vector>

SSR_KIT::SSR_KIT()
    : m_nCells(-1), m_nReaches(-1), m_maxSoilLyrs(-1), m_dt(-1), m_cellWidth(-1.), m_khRatio(1.),
      m_decayFactor(0.), m_depthThresholdRatio(0.70), m_gradientMode(0), m_nSoilLyrs(nullptr),
      m_streamLink(nullptr), m_slope(nullptr), m_chWidth(nullptr), m_reachDepth(nullptr),
      m_surfaceRunoff(nullptr), m_bottomExcess(nullptr), m_infilCapacitySurplus(nullptr),
      m_soilDepth(nullptr), m_conductivity(nullptr), m_porosity(nullptr), m_fieldCapacity(nullptr),
      m_soilMoisture(nullptr), m_flowInIndex(nullptr), m_flowInFraction(nullptr),
      m_soilGeometryValidated(false), m_qSoil(nullptr), m_returnFlow(nullptr),
      m_waterTableDepth(nullptr), m_saturatedDepth(nullptr), m_qOut(nullptr) {}

SSR_KIT::~SSR_KIT() {
    Release1DArray(m_qSoil);
    Release1DArray(m_returnFlow);
    Release1DArray(m_waterTableDepth);
    Release1DArray(m_saturatedDepth);
    Release1DArray(m_qOut);
}

FLTPT SSR_KIT::LayerThickness(const FLTPT* layerBottomDepth, const int layer) {
    if (layerBottomDepth == nullptr || layer < 0) {
        return 0.;
    }
    const FLTPT layerBottom = Max(layerBottomDepth[layer], 0.);
    const FLTPT layerTop = layer > 0 ? Max(layerBottomDepth[layer - 1], 0.) : 0.;
    return Max(layerBottom - layerTop, 0.);
}

FLTPT SSR_KIT::RedistributeSaturationExcess(const int nLayers, const FLTPT* layerBottomDepth,
                                               FLTPT* moisture, const FLTPT* porosity) {
    if (nLayers <= 0 || layerBottomDepth == nullptr || moisture == nullptr || porosity == nullptr) {
        throw ModelException(M_SSR_KIT[0], "RedistributeSaturationExcess",
                             "Missing soil profile data.");
    }
    const int bottomLayer = nLayers - 1;
    if (moisture[bottomLayer] < porosity[bottomLayer]) {
        return 0.;
    }

    FLTPT excessDepth = (moisture[bottomLayer] - porosity[bottomLayer]) *
                        LayerThickness(layerBottomDepth, bottomLayer);
    moisture[bottomLayer] = porosity[bottomLayer];

    for (int k = bottomLayer - 1; k >= 0; k--) {
        const FLTPT layerDepth = LayerThickness(layerBottomDepth, k);

        moisture[k] += excessDepth / layerDepth;
        if (moisture[k] < porosity[k]) {
            return 0.;
        }
        excessDepth = (moisture[k] - porosity[k]) * layerDepth;
        moisture[k] = porosity[k];
    }
    return excessDepth;
}

FLTPT SSR_KIT::CalculateWaterTableDepth(const int nLayers, const FLTPT* layerBottomDepth,
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
        if (layerDepth <= UTIL_ZERO) {
            continue;
        }
        const FLTPT fc = Max(fieldCapacity[k], 0.);
        const FLTPT por = Max(porosity[k], fc);
        const FLTPT theta = Max(0., Min(moisture[k], por));
        const FLTPT storageRange = por - fc;
        if (storageRange <= UTIL_ZERO || theta <= fc + UTIL_ZERO) {
            break;
        }
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

FLTPT SSR_KIT::CalculateLayerIntegratedTransmissivity(const int nLayers,
                                                         const FLTPT* layerBottomDepth,
                                                         const FLTPT* lateralConductivity,
                                                         const FLTPT waterTableDepth) {
    if (nLayers <= 0 || layerBottomDepth == nullptr || lateralConductivity == nullptr) {
        return 0.;
    }
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

FLTPT SSR_KIT::CalculateTransmissivity(const FLTPT bottomDepth, const FLTPT waterTableDepth,
                                          const FLTPT lateralK, const FLTPT decayPerMeter,
                                          const FLTPT thresholdDepth) {
    if (!std::isfinite(bottomDepth) || !std::isfinite(waterTableDepth) ||
        !std::isfinite(lateralK) || !std::isfinite(decayPerMeter) ||
        !std::isfinite(thresholdDepth) || bottomDepth < 0. || lateralK < 0. || decayPerMeter < 0. ||
        thresholdDepth < 0.) {
        throw ModelException(M_SSR_KIT[0], "CalculateTransmissivity",
                             "Invalid transmissivity parameters.");
    }
    const FLTPT bottom = bottomDepth * 0.001;
    const FLTPT waterTable = Max(waterTableDepth, 0.) * 0.001;
    const FLTPT threshold = thresholdDepth * 0.001;
    if (bottom <= waterTable || lateralK <= 0.) {
        return 0.;
    }
    if (decayPerMeter == 0.) {
        return lateralK * (bottom - waterTable);
    }
    if (waterTable < threshold) {
        return lateralK * std::exp(-decayPerMeter * waterTable) *
               -std::expm1(-decayPerMeter * (bottom - waterTable)) / decayPerMeter;
    }

    const FLTPT atThreshold = lateralK * std::exp(-decayPerMeter * threshold) *
                              -std::expm1(-decayPerMeter * (bottom - threshold)) / decayPerMeter;
    return atThreshold * (bottom - waterTable) / (bottom - threshold);
}

FLTPT SSR_KIT::CalculateBankGradientWidth(const FLTPT waterTableDepth, const FLTPT bankHeight) {
    if (!std::isfinite(waterTableDepth) || !std::isfinite(bankHeight)) {
        return 0.;
    }
    return 4. * Max(bankHeight - waterTableDepth, 0.) * 0.001;
}

FLTPT SSR_KIT::CalculateDrainableWaterDepthBetween(const int nLayers,
                                                      const FLTPT* layerBottomDepth,
                                                      const FLTPT* fieldCapacity,
                                                      const FLTPT* porosity, const FLTPT upperDepth,
                                                      const FLTPT lowerDepth) {
    if (nLayers <= 0 || layerBottomDepth == nullptr || fieldCapacity == nullptr ||
        porosity == nullptr) {
        return 0.;
    }
    const FLTPT profileBottom = layerBottomDepth[nLayers - 1];
    const FLTPT upper = Max(0., Min(upperDepth, profileBottom));
    const FLTPT lower = Max(upper, Min(lowerDepth, profileBottom));
    FLTPT drainableDepth = 0.;
    FLTPT layerTop = 0.;
    for (int k = 0; k < nLayers; k++) {
        const FLTPT layerBottom = layerBottomDepth[k];
        const FLTPT overlapTop = Max(layerTop, upper);
        const FLTPT overlapBottom = Min(layerBottom, lower);
        if (overlapBottom > overlapTop) {
            drainableDepth +=
                Max(porosity[k] - fieldCapacity[k], 0.) * (overlapBottom - overlapTop);
        }
        layerTop = layerBottom;
    }
    return drainableDepth;
}

FLTPT SSR_KIT::AddWaterBottomUp(const int nLayers, const FLTPT* layerBottomDepth,
                                   FLTPT* moisture, const FLTPT* porosity, FLTPT waterDepth) {
    if (nLayers <= 0 || layerBottomDepth == nullptr || moisture == nullptr || porosity == nullptr) {
        return Max(waterDepth, 0.);
    }
    FLTPT remaining = Max(waterDepth, 0.);
    for (int k = nLayers - 1; k >= 0 && remaining > UTIL_ZERO; k--) {
        const FLTPT layerDepth = LayerThickness(layerBottomDepth, k);
        if (layerDepth <= UTIL_ZERO) {
            continue;
        }
        const FLTPT por = Max(porosity[k], 0.);
        const FLTPT capacity = Max(por - moisture[k], 0.) * layerDepth;
        const FLTPT added = Min(remaining, capacity);
        moisture[k] += added / layerDepth;
        remaining -= added;
    }
    return Max(remaining, 0.);
}

FLTPT SSR_KIT::RemoveWaterFromWaterTableDown(const int nLayers, const FLTPT* layerBottomDepth,
                                                FLTPT* moisture, const FLTPT* fieldCapacity,
                                                const FLTPT* porosity, const FLTPT waterTableDepth,
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
        if (layerDepth <= UTIL_ZERO) {
            continue;
        }
        const FLTPT fc = Max(fieldCapacity[k], 0.);
        const FLTPT drainable = Max(moisture[k] - fc, 0.) * layerDepth;
        const FLTPT removed = Min(remaining, drainable);
        moisture[k] -= removed / layerDepth;
        remaining -= removed;
    }
    return Max(remaining, 0.);
}

FLTPT SSR_KIT::CalculateHydraulicGradient(const FLTPT terrainSlope,
                                             const FLTPT upstreamWaterTableDepth,
                                             const FLTPT downstreamWaterTableDepth,
                                             const FLTPT flowLength) {
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
    for (int id = 0; id < m_nCells; id++) {
        if (m_bottomExcess != nullptr &&
            (!std::isfinite(m_bottomExcess[id]) || m_bottomExcess[id] < 0.)) {
            throw ModelException(M_SSR_KIT[0], "CheckInputData",
                                 "BOTTOM_SAT_EXCESS must be finite and nonnegative at cell " +
                                     ValueToString(id) + ".");
        }
        if (m_nSoilLyrs[id] <= 0 || m_nSoilLyrs[id] > m_maxSoilLyrs) {
            throw ModelException(M_SSR_KIT[0], "CheckInputData", "Invalid soil layer count.");
        }
        for (int k = 0; k < m_nSoilLyrs[id]; k++) {
            const FLTPT theta = m_soilMoisture[id][k];
            const FLTPT por = m_porosity[id][k];
            const FLTPT fc = m_fieldCapacity[id][k];
            if (!std::isfinite(theta) || !std::isfinite(por) || !std::isfinite(fc) || por < 0. ||
                fc < 0. || fc > por || theta < 0.) {
                throw ModelException(M_SSR_KIT[0], "CheckInputData",
                                     "Invalid soil state at cell " + ValueToString(id) +
                                         ", layer " + ValueToString(k) + ".");
            }
        }
    }
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
                                     "Invalid number of soil layers at cell " + ValueToString(id) +
                                         ".");
            }
            FLTPT previousBottom = 0.;
            for (int k = 0; k < m_nSoilLyrs[id]; k++) {
                const FLTPT layerBottom = m_soilDepth[id][k];
                if (!std::isfinite(layerBottom) || layerBottom <= previousBottom + UTIL_ZERO) {
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

    for (int id = 0; id < m_nCells; id++) {
        FLTPT excessDepth = 0.;
        bool connected = true;
        for (int k = m_nSoilLyrs[id] - 1; k >= 0; k--) {
            const FLTPT theta = m_soilMoisture[id][k];
            const FLTPT por = m_porosity[id][k];
            if (!connected) {
                if (theta > por + 1.e-6) {
                    throw ModelException(M_SSR_KIT[0], "CheckInputData",
                                         "Disconnected supersaturated soil at cell " +
                                             ValueToString(id) + ", layer " + ValueToString(k) +
                                             ".");
                }
                continue;
            }
            const FLTPT layerDepth = LayerThickness(m_soilDepth[id], k);
            const FLTPT filledMoisture = theta + excessDepth / layerDepth;
            if (filledMoisture < por) {
                connected = false;
                excessDepth = 0.;
            } else {
                excessDepth = (filledMoisture - por) * layerDepth;
            }
        }
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
}

void SSR_KIT::CalculateRoutingGradients(const vector<FLTPT>& waterTableDepth,
                                           vector<FLTPT>* gradients) const {

    vector<FLTPT>& cellGradients = *gradients;
    cellGradients.assign(m_nCells, 0.);
    if (m_gradientMode == 0) {
        for (int id = 0; id < m_nCells; id++) {
            cellGradients[id] = Max(m_slope[id], 0.);
        }
        return;
    }
    for (int id = 0; id < m_nCells; id++) {
        const int nUpstream = m_flowInIndex[id][0];
        for (int upIndex = 1; upIndex <= nUpstream; upIndex++) {
            const int up = m_flowInIndex[id][upIndex];
            if (up < 0 || up >= m_nCells) {
                continue;
            }
            const FLTPT flowWeight =
                m_flowInFraction != nullptr ? m_flowInFraction[id][upIndex] : 1.;

            const FLTPT terrainSlope = Max(m_slope[up], 0.);
            const FLTPT hydraulicGradient = CalculateHydraulicGradient(
                terrainSlope, waterTableDepth[up], waterTableDepth[id], m_cellWidth);

            cellGradients[up] += flowWeight * hydraulicGradient;
        }
    }
}

FLTPT SSR_KIT::ConnectedDrainableDepth(const int id, const FLTPT waterTableDepth) const {
    FLTPT drainableDepth = 0.;
    for (int k = 0; k < m_nSoilLyrs[id]; k++) {
        const FLTPT layerDepth = LayerThickness(m_soilDepth[id], k);
        const FLTPT layerBottom = Max(m_soilDepth[id][k], 0.);
        if (layerBottom <= waterTableDepth + UTIL_ZERO) {
            continue;
        }
        const FLTPT fc = Max(m_fieldCapacity[id][k], 0.);
        const FLTPT por = Max(m_porosity[id][k], fc);
        const FLTPT theta = Max(0., Min(m_soilMoisture[id][k], por));
        drainableDepth += Max(theta - fc, 0.) * layerDepth;
    }
    return Max(drainableDepth, 0.);
}

FLTPT SSR_KIT::CellTransmissivity(const int id, const FLTPT waterTableDepth,
                                     const FLTPT bottomDepth) const {
    const FLTPT totalDepth = m_soilDepth[id][m_nSoilLyrs[id] - 1];
    const FLTPT lower = Max(0., Min(bottomDepth, totalDepth));
    const FLTPT upper = Max(0., Min(waterTableDepth, lower));
    if (lower <= upper) {
        return 0.;
    }
    if (m_decayFactor > 0.) {
        FLTPT surfaceLateralK = 0.;
        for (int k = 0; k < m_nSoilLyrs[id]; k++) {
            if (LayerThickness(m_soilDepth[id], k) <= UTIL_ZERO) {
                continue;
            }
            const FLTPT verticalK = Max(m_conductivity[id][k], 0.) * 0.001 / 3600.;
            if (verticalK > 0.) {
                surfaceLateralK = verticalK * m_khRatio;
                break;
            }
        }
        return CalculateTransmissivity(lower, upper, surfaceLateralK,
                                       m_decayFactor / (totalDepth * 0.001),
                                       m_depthThresholdRatio * totalDepth);
    }
    FLTPT transmissivity = 0.;
    FLTPT layerTop = 0.;
    for (int k = 0; k < m_nSoilLyrs[id]; k++) {
        const FLTPT layerBottom = m_soilDepth[id][k];
        const FLTPT overlap = Max(0., Min(layerBottom, lower) - Max(layerTop, upper));
        const FLTPT verticalK = Max(m_conductivity[id][k], 0.) * 0.001 / 3600.;
        transmissivity += verticalK * m_khRatio * overlap * 0.001;
        layerTop = layerBottom;
    }
    return transmissivity;
}

FLTPT SSR_KIT::BankHeight(const int id) const {
    if (id < 0 || id >= m_nCells || m_streamLink[id] <= 0 || m_streamLink[id] > m_nReaches ||
        m_reachDepth == nullptr) {
        return 0.;
    }
    const FLTPT totalDepth = Max(m_soilDepth[id][m_nSoilLyrs[id] - 1], 0.);
    return Max(0., Min(m_reachDepth[m_streamLink[id]] * 1000., totalDepth));
}

int SSR_KIT::Execute() {
    InitialOutputs();

    std::vector<bool> hasDownstream(m_nCells, false);
    std::vector<FLTPT> outgoingWeight(m_nCells, 0.);
    for (int id = 0; id < m_nCells; id++) {
        for (int upIndex = 1; upIndex <= m_flowInIndex[id][0]; upIndex++) {
            const int up = m_flowInIndex[id][upIndex];
            if (up < 0 || up >= m_nCells) {
                throw ModelException(M_SSR_KIT[0], "Execute", "Invalid upstream cell index.");
            }
            if (m_flowInFraction == nullptr && hasDownstream[up]) {
                throw ModelException(M_SSR_KIT[0], "Execute",
                                     "Multiple downstream connections require FLOWIN_FRACTION.");
            }
            const FLTPT flowWeight =
                m_flowInFraction != nullptr ? m_flowInFraction[id][upIndex] : 1.;
            if (!std::isfinite(flowWeight) || flowWeight < 0. || flowWeight > 1.) {
                throw ModelException(M_SSR_KIT[0], "Execute",
                                     "FLOWIN_FRACTION must be finite and between zero and one.");
            }
            if (flowWeight > 0.) {
                hasDownstream[up] = true;
            }
            outgoingWeight[up] += flowWeight;
        }
    }
    for (int id = 0; id < m_nCells; ++id) {
        if (hasDownstream[id] && std::fabs(outgoingWeight[id] - 1.) > 1.e-6) {
            throw ModelException(M_SSR_KIT[0], "Execute",
                                 "FLOWIN_FRACTION leaving each cell must sum to one.");
        }
    }
    const FLTPT area = m_cellWidth * m_cellWidth;
    std::vector<FLTPT> routingWaterTable(m_nCells, 0.);
    std::vector<FLTPT> routingGradient(m_nCells, 0.);
    std::vector<FLTPT> qOutVolume(m_nCells, 0.);
    std::vector<FLTPT> qSoilVolume(m_nCells, 0.);
    std::vector<FLTPT> netVolume(m_nCells, 0.);
    for (int id = 0; id < m_nCells; id++) {
        m_qOut[id] = 0.;
        m_qSoil[id] = 0.;

        m_returnFlow[id] = RedistributeSaturationExcess(m_nSoilLyrs[id], m_soilDepth[id],
                                                        m_soilMoisture[id], m_porosity[id]);
        if (m_bottomExcess != nullptr) {
            m_returnFlow[id] +=
                AddWaterBottomUp(m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
                                 m_porosity[id], m_bottomExcess[id]);
        }
        routingWaterTable[id] =
            CalculateWaterTableDepth(m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
                                     m_fieldCapacity[id], m_porosity[id]);
    }
    CalculateRoutingGradients(routingWaterTable, &routingGradient);

    for (int id = 0; id < m_nCells; id++) {
        const FLTPT waterTable = routingWaterTable[id];
        const FLTPT totalDepth = m_soilDepth[id][m_nSoilLyrs[id] - 1];
        if (m_streamLink[id] > 0) {
            const FLTPT bankHeight = BankHeight(id);
            if (waterTable >= bankHeight) {
                continue;
            }
            const FLTPT intervalDepth = CalculateDrainableWaterDepthBetween(
                m_nSoilLyrs[id], m_soilDepth[id], m_fieldCapacity[id], m_porosity[id], waterTable,
                bankHeight);

            const FLTPT availableDepth =
                Min(intervalDepth, ConnectedDrainableDepth(id, waterTable));
            const FLTPT availableVolume = availableDepth * 0.001 * area;
            const FLTPT potentialVolume = CellTransmissivity(id, waterTable, bankHeight) *
                                          CalculateBankGradientWidth(waterTable, bankHeight) * m_dt;
            qSoilVolume[id] = Min(Max(potentialVolume, 0.), availableVolume);
        } else if (hasDownstream[id]) {
            const FLTPT availableVolume = ConnectedDrainableDepth(id, waterTable) * 0.001 * area;
            const FLTPT potentialVolume = m_cellWidth * routingGradient[id] *
                                          CellTransmissivity(id, waterTable, totalDepth) * m_dt;
            qOutVolume[id] = Min(Max(potentialVolume, 0.), availableVolume);
        }
    }

    for (int id = 0; id < m_nCells; id++) {
        FLTPT incomingVolume = 0.;
        for (int upIndex = 1; upIndex <= m_flowInIndex[id][0]; upIndex++) {
            const int up = m_flowInIndex[id][upIndex];
            if (up < 0 || up >= m_nCells) {
                throw ModelException(M_SSR_KIT[0], "Execute", "Invalid upstream cell index.");
            }

            const FLTPT flowWeight =
                m_flowInFraction != nullptr ? m_flowInFraction[id][upIndex] : 1.;
            incomingVolume += qOutVolume[up] * flowWeight;
        }
        netVolume[id] = incomingVolume - qOutVolume[id] - qSoilVolume[id];
    }

    for (int id = 0; id < m_nCells; id++) {
        const FLTPT netDepth = netVolume[id] / area * 1000.;
        if (netDepth > 0.) {

            m_returnFlow[id] += AddWaterBottomUp(m_nSoilLyrs[id], m_soilDepth[id],
                                                 m_soilMoisture[id], m_porosity[id], netDepth);
        } else if (netDepth < 0.) {
            const FLTPT unmetDepth = RemoveWaterFromWaterTableDown(
                m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id], m_fieldCapacity[id],
                m_porosity[id], routingWaterTable[id], -netDepth);
            if (unmetDepth > 1.e-4) {
                throw ModelException(M_SSR_KIT[0], "Execute",
                                     "Net outflow exceeded connected drainable water.");
            }
        }
    }

    for (int id = 0; id < m_nCells; id++) {
        m_qOut[id] = qOutVolume[id] / m_dt;
        m_qSoil[id] = qSoilVolume[id] / m_dt;
        if (m_returnFlow[id] > 0.) {
            m_surfaceRunoff[id] += m_returnFlow[id];

            if (m_infilCapacitySurplus != nullptr) {
                m_infilCapacitySurplus[id] = 0.;
            }
        }

        m_waterTableDepth[id] =
            CalculateWaterTableDepth(m_nSoilLyrs[id], m_soilDepth[id], m_soilMoisture[id],
                                     m_fieldCapacity[id], m_porosity[id]);
        const FLTPT totalDepth = Max(m_soilDepth[id][m_nSoilLyrs[id] - 1], 0.);
        m_saturatedDepth[id] = Max(totalDepth - m_waterTableDepth[id], 0.);
    }
    return 0;
}

void SSR_KIT::SetValue(const char* key, const FLTPT value) {
    const string sk(key);
    if (StringMatch(sk, Tag_CellWidth[0])) {
        m_cellWidth = value;
    } else if (StringMatch(sk, VAR_KH_RATIO[0])) {
        m_khRatio = Max(value, 0.);
    } else if (StringMatch(sk, VAR_K_DECAY_FACTOR[0])) {
        if (!std::isfinite(value) || value < 0.) {
            throw ModelException(M_SSR_KIT[0], "SetValue",
                                 "K_DECAY_FACTOR must be finite and nonnegative.");
        }
        m_decayFactor = value;
    } else if (StringMatch(sk, VAR_DEPTH_THRESHOLD_RATIO[0])) {
        if (!std::isfinite(value) || value <= 0. || value >= 1.) {
            throw ModelException(M_SSR_KIT[0], "SetValue",
                                 "DEPTH_THRESHOLD_RATIO must be between 0 and 1.");
        }
        m_depthThresholdRatio = value;
    } else if (StringMatch(sk, VAR_GRADIENT_MODE[0])) {
        const int mode = CVT_INT(value);
        if (!std::isfinite(value) || mode < 0 || mode > 1 || fabs(value - mode) > UTIL_ZERO) {
            throw ModelException(M_SSR_KIT[0], "SetValue", "GRADIENT_MODE must be 0 or 1.");
        }
        m_gradientMode = mode;
    } else {
        throw ModelException(M_SSR_KIT[0], "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void SSR_KIT::SetValue(const char* key, const int value) {
    const string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep[0]) || StringMatch(sk, Tag_TimeStep[0])) {
        m_dt = value;
    } else if (StringMatch(sk, Tag_CellSize[0])) {
        m_nCells = value;
    } else if (StringMatch(sk, VAR_GRADIENT_MODE[0])) {
        if (value < 0 || value > 1) {
            throw ModelException(M_SSR_KIT[0], "SetValue", "GRADIENT_MODE must be 0 or 1.");
        }
        m_gradientMode = value;
    } else {
        throw ModelException(M_SSR_KIT[0], "SetValue",
                             "Integer parameter " + sk + " does not exist.");
    }
}

void SSR_KIT::SetReaches(clsReaches* reaches) {
    if (reaches == nullptr) {
        throw ModelException(M_SSR_KIT[0], "SetReaches", "The reaches input cannot be nullptr.");
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
    if (m_nCells <= 0) {
        m_nCells = n;
    }
    if (n != m_nCells) {
        throw ModelException(M_SSR_KIT[0], "Set1DData", "Input size mismatch.");
    }
    if (StringMatch(sk, VAR_SLOPE[0])) {
        m_slope = data;
    } else if (StringMatch(sk, VAR_CHWIDTH[0])) {
        m_chWidth = data;
    } else if (StringMatch(sk, VAR_SURU[0])) {
        m_surfaceRunoff = data;
    } else if (StringMatch(sk, VAR_BOTTOM_SAT_EXCESS[0])) {
        m_bottomExcess = data;
    } else if (StringMatch(sk, VAR_INFILCAPSURPLUS[0])) {
        m_infilCapacitySurplus = data;
    } else
        throw ModelException(M_SSR_KIT[0], "Set1DData", "Parameter " + sk + " does not exist.");
}

void SSR_KIT::Set1DData(const char* key, const int n, int* data) {
    if (m_nCells <= 0) {
        m_nCells = n;
    }
    if (n != m_nCells) {
        throw ModelException(M_SSR_KIT[0], "Set1DData", "Input size mismatch.");
    }
    const string sk(key);
    if (StringMatch(sk, VAR_STREAM_LINK[0])) {
        m_streamLink = data;
    } else if (StringMatch(sk, VAR_SOILLAYERS[0])) {
        m_nSoilLyrs = data;
        m_soilGeometryValidated = false;
    } else
        throw ModelException(M_SSR_KIT[0], "Set1DData", "Parameter " + sk + " does not exist.");
}

void SSR_KIT::Set2DData(const char* key, const int nrows, const int ncols, FLTPT** data) {
    if (m_nCells <= 0) {
        m_nCells = nrows;
    }
    if (nrows != m_nCells) {
        throw ModelException(M_SSR_KIT[0], "Set2DData", "Input row mismatch.");
    }
    const string sk(key);
    if (StringMatch(sk, Tag_FLOWIN_FRACTION[0])) {
        m_flowInFraction = data;
        return;
    }
    if (m_maxSoilLyrs <= 0) {
        m_maxSoilLyrs = ncols;
    }
    if (ncols != m_maxSoilLyrs) {
        throw ModelException(M_SSR_KIT[0], "Set2DData", "Input column mismatch.");
    }
    if (StringMatch(sk, VAR_SOILDEPTH[0])) {
        m_soilDepth = data;
        m_soilGeometryValidated = false;
    } else if (StringMatch(sk, VAR_CONDUCT[0])) {
        m_conductivity = data;
    } else if (StringMatch(sk, VAR_POROST[0])) {
        m_porosity = data;
    } else if (StringMatch(sk, VAR_FIELDCAP[0])) {
        m_fieldCapacity = data;
    } else if (StringMatch(sk, VAR_SOL_ST[0])) {
        m_soilMoisture = data;
    } else {
        throw ModelException(M_SSR_KIT[0], "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void SSR_KIT::Set2DData(const char* key, const int nrows, const int, int** data) {
    const string sk(key);
    if (StringMatch(sk, Tag_FLOWIN_INDEX[0])) {
        if (m_nCells <= 0) {
            m_nCells = nrows;
        }
        if (nrows != m_nCells) {
            throw ModelException(M_SSR_KIT[0], "Set2DData", "Flow input row mismatch.");
        }
        m_flowInIndex = data;
    } else {
        throw ModelException(M_SSR_KIT[0], "Set2DData",
                             "Integer parameter " + sk + " does not exist.");
    }
}

void SSR_KIT::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    const string sk(key);
    if (StringMatch(sk, VAR_QSOIL[0])) {
        *data = m_qSoil;
    } else if (StringMatch(sk, VAR_RETURNFLOW[0])) {
        *data = m_returnFlow;
    } else if (StringMatch(sk, VAR_WT_DEPTH[0])) {
        *data = m_waterTableDepth;
    } else if (StringMatch(sk, VAR_SAT_DEPTH[0])) {
        *data = m_saturatedDepth;
    } else if (StringMatch(sk, VAR_QOUT[0])) {
        *data = m_qOut;
    } else
        throw ModelException(M_SSR_KIT[0], "Get1DData", "Output " + sk + " does not exist.");
    *n = m_nCells;
}
