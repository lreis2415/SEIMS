/*!
 * \file PlantManagementOperation.h
 * \brief Plant management operation class
 * \author Liang-Jun Zhu
 * \date June 2016
 */
#ifndef SEIMS_PLANT_MANAGEMENT_OPERATION_H
#define SEIMS_PLANT_MANAGEMENT_OPERATION_H

#include "basic.h"
#include "BMPText.h"

using namespace ccgl;
using std::ostream;

namespace bmps {
/*!
 * \namespace bmps::plant_management
 * \brief Base namespace for Plant management related operations
 */
namespace plant_management {
/*!
 * \class PltMgtOp
 * \brief Base class of plant management operation
 */
class PltMgtOp: Interface {
public:
    /*!
     * \brief Constructor
     * \param[in] mgtOp 1 to 16
     * \param[in] usebaseHU true or false
     * \param[in] husc Fraction of heat units (base or plant)
     * \param[in] year Rotation year, e.g., 1,2,...
     * \param[in] month
     * \param[in] day
     * \param[in] parameters
     */
    PltMgtOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
             float* parameters);

    //! Output
    virtual void dump(std::ostream* fs) = 0;

    bool UseBaseHUSC() { return m_useBaseHUSC; }

    float GetHUFraction() { return m_frHU; }

    int GetMonth() { return m_month; }

    int GetDay() { return m_day; }

    int GetOperationCode() { return m_mgtOp; }

protected:
    /// use base hu or plant accumulated hu
    bool m_useBaseHUSC;
    /// husc
    float m_frHU;
    /// year
    int m_year;
    /// month
    int m_month;
    /// day
    int m_day;
    /// management operation code
    int m_mgtOp;
    /// parameters, mgt1~mgt10
    float* m_parameters;
};

/*!
 * \class PltOp
 * \brief Plant management operation
 */
class PltOp: public PltMgtOp {
public:
    PltOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float* parameters);

    int PlantID() { return m_plantID; }

    float CurYearMaturity() { return m_curYrMat; }

    float HeatUnits() { return m_heatUnits; }

    float LAIInit() { return m_laiInit; }

    float BIOInit() { return m_bioInit; }

    float HITarg() { return m_hiTarg; }

    float BIOTarg() { return m_bioTarg; }

    float CNOP() { return m_CNOP; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_plantID;
    float m_curYrMat;
    float m_heatUnits;
    float m_laiInit;
    float m_bioInit;
    float m_hiTarg;
    float m_bioTarg;
    float m_CNOP;
};

/*!
 * \class IrrOp
 * \brief Irrigation operation
 */
class IrrOp: public PltMgtOp {
public:
    IrrOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float* parameters);

    int IRRSource() { return m_irrSrc; }

    int IRRNo() { return m_irrNo; }

    float IRRApplyDepth() { return m_irrAmt; }

    float IRRSalt() { return m_irrSalt; }

    float IRREfficiency() { return m_irrEfm; }

    float IRRSQfrac() { return m_irrSq; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_irrSrc;
    float m_irrAmt;
    float m_irrSalt;
    float m_irrEfm;
    float m_irrSq;
    int m_irrNo;
};

/*!
 * \class FertOp
 * \brief Fertilizer operation
 */
class FertOp: public PltMgtOp {
public:
    FertOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float* parameters);

    int FertilizerID() { return m_fertID; }

    float FertilizerKg_per_ha() { return m_frtKgHa; }

    float FertilizerSurfaceFrac() { return m_frtSurface; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_fertID;
    float m_frtKgHa;
    /// fraction of fertilizer which is applied to the top 10 mm of soil (the remaining
    /// fraction is applied to first soil layer defined by user)
    float m_frtSurface;
};

/*!
 * \class PestOp
 * \brief Pesticide operation
 */
class PestOp: public PltMgtOp {
public:
    PestOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float* parameters);

    int PesticideID() { return m_pestID; }

    float PesticideKg() { return m_pstKg; }

    float PesticideDepth() { return m_pstDep; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_pestID;
    float m_pstKg;
    float m_pstDep;
};

/*!
 * \class HvstKillOp
 * \brief HarvestKill operation
 */
class HvstKillOp: public PltMgtOp {
public:
    HvstKillOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
               float* parameters);

    float CNOP() { return m_CNOP; }

    float HarvestIndexOverride() { return m_hiOvr; }

    float StoverFracRemoved() { return m_fracHarvk; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    float m_CNOP;
    float m_hiOvr;
    float m_fracHarvk;
};

/*!
 * \class TillOp
 * \brief Tillage operation
 */
class TillOp: public PltMgtOp {
public:
    TillOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float* parameters);

    float CNOP() { return m_CNOP; }

    int TillageID() { return m_tillID; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_tillID;
    float m_CNOP;
};

/*!
 * \class HvstOnlyOp
 * \brief HarvestOnly operation
 */
class HvstOnlyOp: public PltMgtOp {
public:
    HvstOnlyOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
               float* parameters);

    float HarvestEfficiency() { return m_harvEff; }

    float HarvestIndexResidue() { return m_hiRsd; }

    float HarvestIndexBiomass() { return m_hiBms; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    float m_harvEff;
    float m_hiRsd;
    float m_hiBms;
};

/*!
 * \class KillOp
 * \brief Kill operation
 */
class KillOp: public PltMgtOp {
public:
    KillOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float* parameters);

    //! Output
    void dump(std::ostream* fs) OVERRIDE;
};

/*!
 * \class GrazOp
 * \brief Grazing operation
 */
class GrazOp: public PltMgtOp {
public:
    GrazOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float* parameters);

    int GrazingDays() { return m_grzDays; }

    int ManureID() { return m_manureID; }

    float BiomassConsumed() { return m_bioEat; }

    float BiomassTrampled() { return m_bioTrmp; }

    float ManureDeposited() { return m_manureKg; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_grzDays;
    int m_manureID;
    float m_bioEat;
    float m_bioTrmp;
    float m_manureKg;
};

/*!
 * \class AutoIrrOp
 * \brief Auto irrigation operation
 */
class AutoIrrOp: public PltMgtOp {
public:
    AutoIrrOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
              float* parameters);

    int WaterStrsIdent() { return m_wstrsID; }

    int AutoIrrSrcCode() { return m_irrSrc; }

    int AutoIrrSrcLocs() { return m_irrNoa; }

    float AutoWtrStrsThrsd() { return m_autoWstrs; }

    float IrrigationEfficiency() { return m_irrEff; }

    float IrrigationWaterApplied() { return m_irrMx; }

    float SurfaceRunoffRatio() { return m_irrAsq; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_wstrsID;
    int m_irrSrc;
    float m_autoWstrs;
    int m_irrNoa;
    float m_irrEff;
    float m_irrMx;
    float m_irrAsq;
};

/*!
 * \class AutoFertOp
 * \brief Auto Fertilizer operation
 */
class AutoFertOp: public PltMgtOp {
public:
    AutoFertOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
               float* parameters);

    int FertilizerID() { return m_afertID; }

    int NitrogenMethod() { return m_NStress; }

    float NitrogenStrsFactor() { return m_autoNStrs; }

    float MaxMineralN() { return m_autoNAPP; }

    float MaxMineralNYearly() { return m_autoNYR; }

    float FertEfficiency() { return m_autoEff; }

    float SurfaceFracApplied() { return m_afrtSurface; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_afertID;
    int m_NStress;
    float m_autoNStrs;
    float m_autoNAPP;
    float m_autoNYR;
    float m_autoEff;
    float m_afrtSurface;
};

/*!
 * \class RelImpndOp
 * \brief ReleaseImpound operation
 */
class RelImpndOp: public PltMgtOp {
public:
    RelImpndOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
               float* parameters);

    int ImpoundTriger() { return m_impTrig; }

    float MaxPondDepth() { return m_maxPondDepth; }

    float MaxFitDepth() { return m_maxFitDepth; }

    float MinFitDepth() { return m_minFitDepth; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_impTrig;
    float m_maxPondDepth; ///< Maximum ponding depth, mm
    float m_maxFitDepth;  ///< Maximum fitting depth, mm
    float m_minFitDepth;  ///< Minimum fitting depth, mm
};

/*!
 * \class ContFertOp
 * \brief Continuous Fertilizer operation
 */
class ContFertOp: public PltMgtOp {
public:
    ContFertOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
               float* parameters);

    int FertilizerID() { return m_cfertID; }

    int ApplyFrequency() { return m_ifrtFreq; }

    int FertilizerDays() { return m_fertDays; }

    float FertilizerKg() { return m_cfrtKg; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_cfertID;
    int m_fertDays;
    float m_cfrtKg;
    int m_ifrtFreq;
};

/*!
 * \class ContPestOp
 * \brief Continuous Pesticide operation
 */
class ContPestOp: public PltMgtOp {
public:
    ContPestOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
               float* parameters);

    int PesticideID() { return m_ipstID; }

    int PesticideFrequency() { return m_pstFreq; }

    float PesticideKg() { return m_cpstKg; }

    int PesticideDays() { return m_pstDays; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_ipstID;
    int m_pstDays;
    float m_cpstKg;
    int m_pstFreq;
};

/*!
 * \class BurnOp
 * \brief Burning operation
 */
class BurnOp: public PltMgtOp {
public:
    BurnOp(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float* parameters);

    float FractionLeft() { return m_burnFrlb; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    float m_burnFrlb;
};

} /* plant_management */
} /* bmps */

#endif /* SEIMS_PLANT_MANAGEMENT_OPERATION_H */
