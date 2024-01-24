#ifndef SEIMS_CONVOLVE_H
#define SEIMS_CONVOLVE_H
#include <seims.h>
#include <text.h>
#include <vector>


class ConvolveTransporter {
public:
    ConvolveTransporter() = default;
    
    ConvolveTransporter(const ConvolveTransporter& other) {
        m_convType = other.m_convType;
        m_nCells = other.m_nCells;
        m_isInitialized = other.m_isInitialized;
        m_unitHydro = other.m_unitHydro;
        m_convTransport = other.m_convTransport;
    }
    ConvolveTransporter(int nCells) :
        m_convType(CONV_BASE), m_nCells(nCells), m_isInitialized(false) {
        m_unitHydro.resize(nCells);
        m_convTransport.resize(nCells);
    }
    // copy assignment operator
    ConvolveTransporter& operator=(const ConvolveTransporter& other) {
        if (this != &other) { // self-assignment check expected
            m_convType = other.m_convType;
            m_nCells = other.m_nCells;
            m_isInitialized = other.m_isInitialized;
            m_unitHydro = other.m_unitHydro;
            m_convTransport = other.m_convTransport;
        }
        return *this;
    }
    // move constructor
    ConvolveTransporter(ConvolveTransporter&& other) noexcept {
        m_convType = other.m_convType;
        m_nCells = other.m_nCells;
        m_isInitialized = other.m_isInitialized;
        m_unitHydro = other.m_unitHydro;
        m_convTransport = other.m_convTransport;
    }
    // move assignment operator
    ConvolveTransporter& operator=(ConvolveTransporter&& other) noexcept {
        if (this != &other) { // self-assignment check expected
            m_convType = other.m_convType;
            m_nCells = other.m_nCells;
            m_isInitialized = other.m_isInitialized;
            m_unitHydro = other.m_unitHydro;
            m_convTransport = other.m_convTransport;
        }
        return *this;
    }

    virtual ~ConvolveTransporter() {
        m_unitHydro.clear();
        m_convTransport.clear();
    }

    virtual void Convolve(FLTPT* entering, FLTPT* leaving) {
        if (entering == nullptr || leaving == nullptr) {
            throw ModelException("ConvolveTransporter", "Convolve",
                "Input and output arrays cannot be null.");
        }
//#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            m_convTransport.at(i).at(0) = entering[i];
            entering[i] = 0;
            for (int n = 0; n < m_unitHydro.at(i).size(); n++) {
                leaving[i] += m_convTransport.at(i).at(n) * m_unitHydro.at(i).at(n);
            }
            for (int n = m_unitHydro.at(i).size() - 1; n > 0; n--) {
                m_convTransport.at(i).at(n) = m_convTransport.at(i).at(n - 1);
            }
            m_convTransport.at(i).at(0) = 0;  // redundant, but clear
        }
    }

    bool m_isInitialized;
    string m_convType;  /// name
    int m_nCells;
    std::vector<vector<FLTPT>> m_unitHydro;
    std::vector<vector<FLTPT>> m_convTransport;


};

inline FLTPT IncompleteGamma(const double& x, const double& a)
{
    //cumulative distribution
    /// \ref from http://algolist.manual.ru/maths/count_fast/gamma_function.php
    const int N = 100;
    if (x == 0) { return 0.0; }
    double num = 1;
    double sum = 0.0;
    double prod = 1.0;
    for (int n = 0; n < N; n++) {
        if (n > 0) { num *= x; }
        prod *= (a + n);
        sum += num / prod;
    }
    return sum * pow(x, a) * exp(-x);
}
inline FLTPT gamma2(double x)
{
    int i, k, m;
    double ga = 0, gr, r = 0, z;

    static double g[] = {
      1.0,                  0.5772156649015329, -0.6558780715202538,
      -0.420026350340952e-1, 0.1665386113822915, -0.421977345555443e-1,
      -0.9621971527877e-2,   0.7218943246663e-2, -0.11651675918591e-2,
      -0.2152416741149e-3,   0.1280502823882e-3, -0.201348547807e-4,
      -0.12504934821e-5,     0.1133027232e-5,    -0.2056338417e-6,
      0.6116095e-8,         0.50020075e-8,      -0.11812746e-8,
      0.1043427e-9,         0.77823e-11,        -0.36968e-11,
      0.51e-12,            -0.206e-13,          -0.54e-14,
      0.14e-14 };

    if (x > 171.0) { return 0.0; }    // This value is an overflow flag.
    if (x == (int)x)
    {
        if (x > 0.0) {
            ga = 1.0;               // use factorial
            for (i = 2; i < x; i++) { ga *= i; }
        }
        else {
            ga = 0.0;
            throw ModelException("Gamma", "gamma2", "negative integer values not allowed");
        }
    }
    else
    {
        z = x;
        r = 1.0;
        if (fabs(x) > 1.0) {
            z = fabs(x);
            m = (int)(z);
            r = 1.0;
            for (k = 1; k <= m; k++) { r *= (z - k); }
            z -= m;
        }
        gr = g[24];
        for (k = 23; k >= 0; k--) { gr = gr * z + g[k]; }
        ga = 1.0 / (gr * z);
        if (fabs(x) > 1.0) {
            ga *= r;
            if (x < 0.0) { ga = -PI / (x * ga * sin(PI * x)); }
        }
    }
    return ga;
}
inline FLTPT GammaCumDist(const double& t, const double& alpha, const double& beta)
{
    return IncompleteGamma(beta * t, alpha) / gamma2(alpha);
}
class ConvolveTransporterGAMMA : public ConvolveTransporter {
public:
    ConvolveTransporterGAMMA() = default;

    ConvolveTransporterGAMMA(int nCells, FLTPT* gammaScale, FLTPT* gammaShape) :
        ConvolveTransporter(nCells), m_gammaScale(gammaScale), m_gammaShape(gammaShape) {
        m_convType = CONV_GAMMA;
        InitUnitHydrograph();
    }

    void InitUnitHydrograph() {
        if (m_isInitialized) {
            return;
        }
//#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            FLTPT a = m_gammaScale[i];
            FLTPT b = m_gammaShape[i];
            int maxTime = Min((double)50, 4.5 * pow(a, 0.6) / b);
            FLTPT uhAcc = 0;
            //FLTPT uh = (pow(b * t, a) * exp(-b * t)) / (t * tgamma(a));
            for (int t = 0; t < maxTime; t++) {
                FLTPT uh = GammaCumDist(t+1, a, b) - uhAcc;
                uhAcc += uh;
                m_unitHydro[i].push_back(uh);
                m_convTransport[i].push_back(0);
            }
            FLTPT sum = 0.0;
            for (int n = 0; n < maxTime; n++) { sum += m_unitHydro[i].at(n); }
            if (sum == 0) {
                throw ModelException("ConvolveTransporterGAMMA", "InitUnitHydrograph",
                    "Sum of unit hydrograph is zero.");
            }
            for (int n = 0; n < maxTime; n++) {
                m_unitHydro[i].at(n) /= sum;
            }
        }

        m_isInitialized = true;
    }
private:
    FLTPT* m_gammaScale;
    FLTPT* m_gammaShape;
};
#endif 
