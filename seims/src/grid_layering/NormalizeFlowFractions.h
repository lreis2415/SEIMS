/*!
* \file NormalizeFlowFractions.h
 * \brief Normalize an array to sum ~= 1 (math) and quantize to 'decimals' so printed sum is exactly 1.000...
 *
 * \remarks
 *   - 1. 2025-09-07 - lj - Initial implementation.
 *
 * \note No exceptions will be thrown.
 * \author Liangjun Zhu, zlj(at)lreis.ac.cn
 * \version 0.1
 */
#ifndef NORMALIZE_FLOW_FRACTION_H_
#define NORMALIZE_FLOW_FRACTION_H_

#include <vector>
#include <algorithm>
#include <cmath>
#include <cstddef> // size_t

// - Negative inputs are clamped to 0.
// - If all inputs are non-positive, split evenly (1/n).
// - Order is preserved; sorting is only used to decide where to add/subtract 1 unit.
template<typename T>
int normalize_flow_fraction(const std::vector<T>& in_w,
                            int decimals,
                            std::vector<T>& out_norm) {
    const size_t n = in_w.size();
    out_norm.assign(n, static_cast<T>(0));
    if (n == 0) return 0;

    // 1) Normalize positives to sum 1 (math-level)
    double sum_pos = 0.;
    for (size_t i = 0; i < n; ++i) {
        if (in_w[i] > static_cast<T>(0)) sum_pos += static_cast<double>(in_w[i]);
    }

    std::vector<double> norm(n, 0.);
    if (sum_pos > 0.) {
        for (size_t i = 0; i < n; ++i) {
            double v = static_cast<double>(in_w[i]);
            norm[i] = v > 0. ? (v / sum_pos) : 0.;
        }
        // Re-normalize to cancel tiny FP drift
        double chk = 0.;
        for (size_t i = 0; i < n; ++i) chk += norm[i];
        if (chk > 0.) {
            for (size_t i = 0; i < n; ++i) norm[i] /= chk;
        }
    } else {
        // All non-positive → split evenly
        for (size_t i = 0; i < n; ++i) norm[i] = 1. / static_cast<double>(n);
    }

    // 2) Quantize to 'decimals' using largest remainder method
    if (decimals < 0) decimals = 0;
    if (decimals > 9) decimals = 9;           // keep 10^d in 64-bit range
    long long scale = 1;
    for (int k = 0; k < decimals; ++k) scale *= 10;

    std::vector<long long> base(n, 0);
    std::vector<double> rem(n, 0.);
    long long sum_base = 0;
    for (size_t i = 0; i < n; ++i) {
        double raw = norm[i] * static_cast<double>(scale);
        if (raw < 0.) raw = 0.;
        double flo = std::floor(raw + 1e-12L); // guard against 0.999999…
        base[i] = static_cast<long long>(flo);
        rem[i]  = raw - flo;
        sum_base += base[i];
    }

    const long long target = scale;            // printed sum target
    long long diff = target - sum_base;        // >0 add units, <0 remove units

    // Indices for stable tie-breaking; final order is preserved
    std::vector<size_t> idx(n);
    for (size_t i = 0; i < n; ++i) idx[i] = i;

    struct CmpDesc {
        const std::vector<double>* r;
        CmpDesc(const std::vector<double>* rp): r(rp) {}
        bool operator()(size_t a, size_t b) const {
            if ((*r)[a] > (*r)[b]) return true;
            if ((*r)[a] < (*r)[b]) return false;
            return a < b;
        }
    };
    struct CmpAsc {
        const std::vector<double>* r;
        CmpAsc(const std::vector<double>* rp): r(rp) {}
        bool operator()(size_t a, size_t b) const {
            if ((*r)[a] < (*r)[b]) return true;
            if ((*r)[a] > (*r)[b]) return false;
            return a < b;
        }
    };

    if (diff > 0) {
        std::sort(idx.begin(), idx.end(), CmpDesc(&rem));     // larger remainders get +1 first
        const size_t m = n;
        for (long long t = 0; t < diff; ++t)
            base[idx[(size_t)(t % m)]] += 1;
    } else if (diff < 0) {
        std::sort(idx.begin(), idx.end(), CmpAsc(&rem));      // smaller remainders lose 1 first
        const size_t m = n;
        long long need = -diff;
        size_t p = 0;
        while (need > 0 && m > 0) {
            size_t j = idx[p % m];
            if (base[j] > 0) {
                base[j] -= 1;
                --need;
            }
            ++p; // round-robin to avoid over-shaving one slot
        }
    }

    // 3) Write back as T (preserve order). Printed with 'decimals' will sum to exactly 1
    for (size_t i = 0; i < n; ++i)
        out_norm[i] = static_cast<T>(static_cast<double>(base[i]) / static_cast<double>(scale));

    return static_cast<int>(n);
}

#endif // NORMALIZE_FLOW_FRACTION_H_
