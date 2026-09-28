#ifndef SEIMS_CONSERVATIVE_INFILTRATION_H
#define SEIMS_CONSERVATIVE_INFILTRATION_H
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <string>
namespace conservative_infiltration {

// Limit infiltration to the active portion of each layer without changing storage below it.
// Depths are mm; moisture is the whole-layer mean volumetric water content.
template <typename T>
double Capacity(int nLayers, const T *layerBottomDepth, const T *porosity, const T *fieldCapacity,
    const T *moisture, double activeDepth, double storageFraction) {
    if (nLayers <= 0 || !layerBottomDepth || !porosity || !fieldCapacity || !moisture ||
        !std::isfinite(activeDepth) || !std::isfinite(storageFraction) || storageFraction < 0 ||
        storageFraction > 1) {
        throw std::runtime_error("Invalid infiltration profile/settings");
    }
    double top = 0, capacity = 0;
    const double limit = activeDepth <= 0 ? layerBottomDepth[nLayers - 1] : activeDepth;
    for (int k = 0; k < nLayers; ++k) {
        const double bottom = layerBottomDepth[k];
        if (!std::isfinite(bottom) || bottom <= top || !std::isfinite(moisture[k]) ||
            !std::isfinite(porosity[k]) || !std::isfinite(fieldCapacity[k]) || moisture[k] < 0 ||
            porosity[k] <= 0 || fieldCapacity[k] < 0 || fieldCapacity[k] > porosity[k]) {
            throw std::runtime_error("Invalid infiltration layer state");
        }
        const double target = fieldCapacity[k] + storageFraction * (porosity[k] - fieldCapacity[k]);
        const double roundoff =
            8 * std::numeric_limits<T>::epsilon() * std::max(1., static_cast<double>(porosity[k]));
        if (moisture[k] > porosity[k] + roundoff) {
            throw std::runtime_error("Supersaturated infiltration layer " + std::to_string(k) +
                ": excess=" + std::to_string(moisture[k] - porosity[k]));
        }
        capacity +=
            std::max(0., target - moisture[k]) * std::max(0., std::min(bottom, limit) - top);
        top = bottom;
    }
    return capacity;
}
template <typename T>
double Accept(int nLayers, const T *layerBottomDepth, const T *porosity, const T *fieldCapacity,
    T *moisture, double activeDepth, double storageFraction, double requested) {
    if (!std::isfinite(requested) || requested < 0) {
        throw std::runtime_error("Invalid infiltration request");
    }
    Capacity(nLayers, layerBottomDepth, porosity, fieldCapacity, moisture, activeDepth,
        storageFraction);
    double top = 0, accepted = 0;
    const double limit = activeDepth <= 0 ? layerBottomDepth[nLayers - 1] : activeDepth;
    for (int k = 0; k < nLayers && accepted < requested; ++k) {
        const double bottom = layerBottomDepth[k], thickness = bottom - top;
        const double overlap = std::max(0., std::min(bottom, limit) - top);
        const double target = fieldCapacity[k] + storageFraction * (porosity[k] - fieldCapacity[k]);
        const double fill =
            std::min(requested - accepted, std::max(0., target - moisture[k]) * overlap);
        if (fill > 0) {
            const T before = moisture[k];
            T after = static_cast<T>(static_cast<double>(before) + fill / thickness);
            // Round toward the old state if rounding would create extra water.
            if ((static_cast<double>(after) - before) * thickness > fill) {
                after = std::nextafter(after, before);
            }
            moisture[k] = after;
            accepted += (static_cast<double>(after) - before) * thickness;
        }
        top = bottom;
    }
    return accepted;
}
} // namespace conservative_infiltration
#endif
