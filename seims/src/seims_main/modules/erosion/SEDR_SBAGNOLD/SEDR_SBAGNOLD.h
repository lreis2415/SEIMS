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
 *   - 8. 2022-08-22 - lj - Change float to FLTPT.
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

    void SetValue(const char* key, FLTPT value) OVERRIDE;

    void SetValue(const char* key, int value) OVERRIDE;

    void SetValueByIndex(const char* key, int index, FLTPT value) OVERRIDE;

    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    void SetReaches(clsReaches* reaches) OVERRIDE;

    void SetScenario(Scenario* sce) OVERRIDE;

    void InitialOutputs() OVERRIDE;

    bool CheckInputData() OVERRIDE;

    int Execute() OVERRIDE;

    TimeStepType GetTimeStepType() OVERRIDE { return TIMESTEP_CHANNEL; }

    void GetValue(const char* key, FLTPT* value) OVERRIDE;

    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

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
    FLTPT m_peakRateAdj;    ///< the peak rate adjustment factor
    FLTPT m_sedTransEqCoef; ///< Coefficient in sediment transport equation, spcon in SWAT
    FLTPT m_sedTransEqExp;  ///< Exponent in sediment transport equation, spexp in SWAT
    FLTPT m_initChSto;      ///< initial channel storage per meter of reach length (m^3/m)

    int* m_reachDownStream;   ///< downstream id (The value is 0 if there if no downstream reach)
    int* m_chOrder;         ///< Channel order ID
    FLTPT* m_chWth;           ///< Channel width at bankfull, m
    FLTPT* m_chDepth;         ///< Channel depth at bankfull, m
    FLTPT* m_chWthDepthRt;    ///< Channel width to depth ratio, ch_wdr in SWAT
    FLTPT* m_chLen;           ///< Channel length, m
    FLTPT* m_chSlope;         ///< Channel slope
    FLTPT* m_chSideSlope;     ///< inverse of the channel side slope, by default is 2. chside in SWAT.
    FLTPT* m_chBnkBD;         ///< Bulk density of channel bank
    FLTPT* m_chBedBD;         ///< Bulk density of channel bed
    FLTPT* m_chBnkCov;        /// Channel bank cover factor, ch_cov1 in SWAT
    // FLTPT* m_chBedCov;     /// Channel bed cover factor, ch_cov2 in SWAT, currently not used
    FLTPT* m_chBnkErod;   ///< channel bank erodibility factor, cm^3/N/s
    FLTPT* m_chBedErod;   ///< channel bed erodibility factor, cm^3/N/s
    FLTPT* m_chBnkTc;     ///< Critical shear stress of channel bank, N/m^2
    FLTPT* m_chBedTc;     ///< Critical shear stress of channel bed, N/m^2
    FLTPT* m_chBnkSand;   ///< Fraction of sand in channel bank sediment
    FLTPT* m_chBnkSilt;   ///< Fraction of silt in channel bank sediment
    FLTPT* m_chBnkClay;   ///< Fraction of clay in channel bank sediment
    FLTPT* m_chBnkGravel; ///< Fraction of gravel in channel bank sediment
    FLTPT* m_chBedSand;   ///< Fraction of sand in channel bed sediment
    FLTPT* m_chBedSilt;   ///< Fraction of silt in channel bed sediment
    FLTPT* m_chBedClay;   ///< Fraction of clay in channel bed sediment
    FLTPT* m_chBedGravel; ///< Fraction of gravel in channel bed sediment

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
    FLTPT* m_ptSub;

    /// Temporary variables

    FLTPT* m_initChDepth;  ///< Initial channel depth at bankfull, m
    FLTPT* m_initChLen;    ///< Initial channel length, m
    FLTPT* m_initChSlope;  ///< Initial channel slope
    FLTPT* m_preRchDep;    ///< Deposition sediment in previous timestep, depprch in SWAT
    FLTPT* m_preFldplnDep; ///< Deposition sediment on floodplain in previous timestep, depprfp in SWAT

    /// INPUT from other modules

    FLTPT* m_sedtoCh;    ///< sediment from hillslope erosion module (e.g., IUH_SED_OL), kg
    FLTPT* m_sandtoCh;   ///< sand from hillslope erosion, kg
    FLTPT* m_silttoCh;   ///< sand from hillslope erosion, kg
    FLTPT* m_claytoCh;   ///< sand from hillslope erosion, kg
    FLTPT* m_sagtoCh;    ///< sand from hillslope erosion, kg
    FLTPT* m_lagtoCh;    ///< sand from hillslope erosion, kg
    FLTPT* m_graveltoCh; ///< sand from hillslope erosion, kg

    FLTPT* m_qRchOut;    ///< channel outflow, m^3/s
    FLTPT* m_chSto;      ///< channel storage (m^3) after channel routing, rchstor in SWAT
    FLTPT* m_rteWtrOut;  ///< Water leaving reach on day after channel routing, m^3, rtwtr in SWAT
    FLTPT* m_chBtmWth;   ///< Channel bottom width, m
    FLTPT* m_chWtrDepth; ///< channel water depth, m, rchdep in SWAT
    FLTPT* m_chWtrWth;   ///< channel top water width, m, topw in SWAT

    // OUTPUT

    FLTPT* m_sedRchOut;     ///< reach sediment out (kg) at time t, sedrch * 1000 in SWAT
    FLTPT* m_sedConcRchOut; ///< sediment concentration (g/L, i.e., kg/m^3)
    FLTPT* m_sandRchOut;    ///< sand out (kg), rch_san * 1000 in SWAT
    FLTPT* m_siltRchOut;    ///< silt out (kg), rch_sil * 1000 in SWAT
    FLTPT* m_clayRchOut;    ///< clay out (kg), rch_cla * 1000 in SWAT
    FLTPT* m_sagRchOut;     ///< small aggregate out (kg), rch_sag * 1000 in SWAT
    FLTPT* m_lagRchOut;     ///< large aggregate out (kg), rch_lag * 1000 in SWAT
    FLTPT* m_gravelRchOut;  ///< gravel out (kg), rch_gra * 1000 in SWAT

    FLTPT* m_rchBnkEro; ///< Bank erosion sediment, bnkrte * 1000 in SWAT
    FLTPT* m_rchDeg;    ///< Channel degradation sediment, degrte * 1000 in SWAT

    FLTPT* m_rchDep;       ///< Deposition sediment, depch in SWAT
    FLTPT* m_dltRchDep;    ///< Channel new deposition during the current time step, rchdy(57,jrch) in SWAT
    FLTPT* m_rchDepSand;   ///< depsanch in SWAT
    FLTPT* m_rchDepSilt;   ///< depsilch in SWAT
    FLTPT* m_rchDepClay;   ///< depclach in SWAT
    FLTPT* m_rchDepSag;    ///< depsagch in SWAT
    FLTPT* m_rchDepLag;    ///< deplagch in SWAT
    FLTPT* m_rchDepGravel; ///< depgrach in SWAT

    FLTPT* m_fldplnDep;     ///< Deposition sediment on floodplain, depfp in SWAT
    FLTPT* m_dltFldplnDep;  ///< Floodplain new deposits during the current timestep, rchdy(58,jrch) in SWAT
    FLTPT* m_fldplnDepSilt; ///< Deposition silt on floodplain, depsilfp in SWAT
    FLTPT* m_fldplnDepClay; ///< Deposition clay on floodplain, depclafp in SWAT

    FLTPT* m_sedSto;    ///< channel sediment storage (kg), sedst * 1000 in SWAT
    FLTPT* m_sandSto;   ///< Sand storage in reach (kg), sanst * 1000 in SWAT
    FLTPT* m_siltSto;   ///< Silt storage in reach (kg), silst * 1000 in SWAT
    FLTPT* m_claySto;   ///< Clay storage in reach (kg), clast * 1000 in SWAT
    FLTPT* m_sagSto;    ///< Small aggregate storage in reach (kg), sagst * 1000 in SWAT
    FLTPT* m_lagSto;    ///< Large aggregate storage in reach (kg), lagst * 1000 in SWAT
    FLTPT* m_gravelSto; ///< Gravel storage in reach (kg), sanst * 1000 in SWAT
};
#endif /* SEIMS_MODULE_SEDR_SBAGNOLD_H */
