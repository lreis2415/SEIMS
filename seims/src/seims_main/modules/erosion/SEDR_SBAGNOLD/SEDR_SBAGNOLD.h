/*!
 * \file SEDR_SBAGNOLD.h
 * \brief Sediment routing using simplified version of Bagnold(1997) stream power equation,
 *        which is based on fall velocity and degradation on stream.
 *
 *        reWrite from route.f and rtsed_bagnold.f of SWAT
 *
 * Changelog:
 *   - 1. 2012-07-31 - wh - Initial implementation based on rtsed.f of SWAT.
 *   - 2. 2016-05-30 - lj -
 *        -# Move m_erodibilityFactor, m_coverFactor, to reach collection of MongoDB as DT_Array1D inputs.
 *        -# Add point source loadings from Scenario database.
 *        -# Add SEDRECHConc output with the unit g/cm^3 (i.e., Mg/m^3).
 *   - 3. 2016-08-31 - jz - Code review.
 *   - 4. 2016-09-30 - lj -
 *        -# ReCheck and Update code according to route.f and rtsed.f
 *        -# Change the module name from SEDR_VCD to SEDR_SBAGNOLD.
 *   - 5. 2018-05-14 - lj - Code review and reformat.
 *   - 6. 2018-06-28 - lj -
 *        -# The initialization of m_sedSto should be done in Set1DData after m_chSto.
 *        -# Bug fixed about code related to the IN/OUTPUT variables.
 *   - 7. 2018-08-15 - lj - Update from rtsed.f to rtsed_bagnold.f of SWAT.
 *
 * \author Liangjun Zhu, Hui Wu, Junzhi Liu
 */
#ifndef SEIMS_MODULE_SEDR_SBAGNOLD_H
#define SEIMS_MODULE_SEDR_SBAGNOLD_H

#include "SimulationModule.h"

/** \defgroup SEDR_SBAGNOLD
 * \ingroup Erosion
 * \brief Sediment routing using Simplified version of Bagnold(1997) stream power equation
 */
/*!
 * \class SEDR_SBAGNOLD
 * \ingroup SEDR_SBAGNOLD
 *
 * \brief Sediment routing using variable channel dimension(VCD) method at daily time scale
 *
 */
class SEDR_SBAGNOLD: public SimulationModule {
public:
    SEDR_SBAGNOLD();

    ~SEDR_SBAGNOLD();

    void SetValue(const char* key, float value) OVERRIDE;

    void SetValueByIndex(const char* key, int index, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void SetReaches(clsReaches* reaches) OVERRIDE;

    void SetScenario(Scenario* sce) OVERRIDE;

    void InitialOutputs() OVERRIDE;

    bool CheckInputData() OVERRIDE;

    int Execute() OVERRIDE;

    TimeStepType GetTimeStepType() OVERRIDE { return TIMESTEP_CHANNEL; }

    void GetValue(const char* key, float* value) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

private:
    void PointSourceLoading();

    void SedChannelRouting(int i);

    void ChannelDowncuttingWidening(int i);
private:
    int m_dt;            ///< time step (sec)
    int m_inputSubbsnID; ///< current subbasin ID, 0 for the entire watershed

    /// INPUT PARAMETERS FROM DATABASE

    int m_nreach;           ///< reach/channel number (= subbasin number)
    bool m_vcd;             ///< compute channel degredation or not
    float m_peakRateAdj;    ///< the peak rate adjustment factor
    float m_sedTransEqCoef; ///< Coefficient in sediment transport equation, spcon in SWAT
    float m_sedTransEqExp;  ///< Exponent in sediment transport equation, spexp in SWAT
    float m_initChSto;      ///< initial channel storage per meter of reach length (m^3/m)

    float* m_reachDownStream; ///< downstream id (The value is 0 if there if no downstream reach)
    float* m_chOrder;         ///< Channel order ID
    float* m_chWth;           ///< Channel width at bankfull, m
    float* m_chDepth;         ///< Channel depth at bankfull, m
    float* m_chWthDepthRt;    ///< Channel width to depth ratio, ch_wdr in SWAT
    float* m_chLen;           ///< Channel length, m
    float* m_chSlope;         ///< Channel slope
    float* m_chSideSlope;     ///< inverse of the channel side slope, by default is 2. chside in SWAT.
    float* m_chBnkBD;         ///< Bulk density of channel bank
    float* m_chBedBD;         ///< Bulk density of channel bed
    float* m_chBnkCov;        /// Channel bank cover factor, ch_cov1 in SWAT
    // float* m_chBedCov;     /// Channel bed cover factor, ch_cov2 in SWAT, currently not used
    float* m_chBnkErod;   ///< channel bank erodibility factor, cm^3/N/s
    float* m_chBedErod;   ///< channel bed erodibility factor, cm^3/N/s
    float* m_chBnkTc;     ///< Critical shear stress of channel bank, N/m^2
    float* m_chBedTc;     ///< Critical shear stress of channel bed, N/m^2
    float* m_chBnkSand;   ///< Fraction of sand in channel bank sediment
    float* m_chBnkSilt;   ///< Fraction of silt in channel bank sediment
    float* m_chBnkClay;   ///< Fraction of clay in channel bank sediment
    float* m_chBnkGravel; ///< Fraction of gravel in channel bank sediment
    float* m_chBedSand;   ///< Fraction of sand in channel bed sediment
    float* m_chBedSilt;   ///< Fraction of silt in channel bed sediment
    float* m_chBedClay;   ///< Fraction of clay in channel bed sediment
    float* m_chBedGravel; ///< Fraction of gravel in channel bed sediment

    map<int, vector<int> > m_reachLayers; ///< Reach layers according to \a LayeringMethod
    /*!
     * Index of upstream Ids (The value is -1 if there if no upstream reach)
     * m_reachUpStream.size() = N+1
     * m_reachUpStream[1] = [2, 3] means Reach 2 and Reach 3 flow into Reach 1.
     */
    vector<vector<int> > m_reachUpStream;

    /* point source operations
     * key: unique index, BMPID * 100000 + subScenarioID
     * value: point source management factory instance
     */
    map<int, BMPPointSrcFactory *> m_ptSrcFactory;
    /// The point source loading (kg), m_ptSub[id], id is the reach id, load from m_Scenario
    float* m_ptSub;

    /// Temporary variables

    float* m_initChDepth;  ///< Initial channel depth at bankfull, m
    float* m_initChLen;    ///< Initial channel length, m
    float* m_initChSlope;  ///< Initial channel slope
    float* m_preRchDep;    ///< Deposition sediment in previous timestep, depprch in SWAT
    float* m_preFldplnDep; ///< Deposition sediment on floodplain in previous timestep, depprfp in SWAT

    /// INPUT from other modules

    float* m_sedtoCh;    ///< sediment from hillslope erosion module (e.g., IUH_SED_OL), kg
    float* m_sandtoCh;   ///< sand from hillslope erosion, kg
    float* m_silttoCh;   ///< sand from hillslope erosion, kg
    float* m_claytoCh;   ///< sand from hillslope erosion, kg
    float* m_sagtoCh;    ///< sand from hillslope erosion, kg
    float* m_lagtoCh;    ///< sand from hillslope erosion, kg
    float* m_graveltoCh; ///< sand from hillslope erosion, kg

    float* m_qRchOut;    ///< channel outflow, m^3/s
    float* m_chSto;      ///< channel storage (m^3) after channel routing, rchstor in SWAT
    float* m_rteWtrOut;  ///< Water leaving reach on day after channel routing, m^3, rtwtr in SWAT
    float* m_chBtmWth;   ///< Channel bottom width, m
    float* m_chWtrDepth; ///< channel water depth, m, rchdep in SWAT
    float* m_chWtrWth;   ///< channel top water width, m, topw in SWAT

    // OUTPUT

    float* m_sedRchOut;     ///< reach sediment out (kg) at time t, sedrch * 1000 in SWAT
    float* m_sedConcRchOut; ///< sediment concentration (g/L, i.e., kg/m^3)
    float* m_sandRchOut;    ///< sand out (kg), rch_san * 1000 in SWAT
    float* m_siltRchOut;    ///< silt out (kg), rch_sil * 1000 in SWAT
    float* m_clayRchOut;    ///< clay out (kg), rch_cla * 1000 in SWAT
    float* m_sagRchOut;     ///< small aggregate out (kg), rch_sag * 1000 in SWAT
    float* m_lagRchOut;     ///< large aggregate out (kg), rch_lag * 1000 in SWAT
    float* m_gravelRchOut;  ///< gravel out (kg), rch_gra * 1000 in SWAT

    float* m_rchBnkEro; ///< Bank erosion sediment, bnkrte * 1000 in SWAT
    float* m_rchDeg;    ///< Channel degradation sediment, degrte * 1000 in SWAT

    float* m_rchDep;       ///< Deposition sediment, depch in SWAT
    float* m_dltRchDep;    ///< Channel new deposition during the current time step, rchdy(57,jrch) in SWAT
    float* m_rchDepSand;   ///< depsanch in SWAT
    float* m_rchDepSilt;   ///< depsilch in SWAT
    float* m_rchDepClay;   ///< depclach in SWAT
    float* m_rchDepSag;    ///< depsagch in SWAT
    float* m_rchDepLag;    ///< deplagch in SWAT
    float* m_rchDepGravel; ///< depgrach in SWAT

    float* m_fldplnDep;     ///< Deposition sediment on floodplain, depfp in SWAT
    float* m_dltFldplnDep;  ///< Floodplain new deposits during the current timestep, rchdy(58,jrch) in SWAT
    float* m_fldplnDepSilt; ///< Deposition silt on floodplain, depsilfp in SWAT
    float* m_fldplnDepClay; ///< Deposition clay on floodplain, depclafp in SWAT

    float* m_sedSto;    ///< channel sediment storage (kg), sedst * 1000 in SWAT
    float* m_sandSto;   ///< Sand storage in reach (kg), sanst * 1000 in SWAT
    float* m_siltSto;   ///< Silt storage in reach (kg), silst * 1000 in SWAT
    float* m_claySto;   ///< Clay storage in reach (kg), clast * 1000 in SWAT
    float* m_sagSto;    ///< Small aggregate storage in reach (kg), sagst * 1000 in SWAT
    float* m_lagSto;    ///< Large aggregate storage in reach (kg), lagst * 1000 in SWAT
    float* m_gravelSto; ///< Gravel storage in reach (kg), sanst * 1000 in SWAT
};
#endif /* SEIMS_MODULE_SEDR_SBAGNOLD_H */
