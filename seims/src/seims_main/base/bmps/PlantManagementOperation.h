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
#include <seims.h>

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
    PltMgtOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    //! Destructor
    ~PltMgtOp();

    //! Output
    virtual void dump(std::ostream* fs) = 0;

    bool UseBaseHUSC();

    FLTPT GetHUFraction();

    int GetMonth();

    int GetDay();

    int GetOperationCode();

protected:
    /// use base hu or plant accumulated hu
    bool m_useBaseHUSC;
    /// husc
    FLTPT m_frHU;
    /// year
    int m_year;
    /// month
    int m_month;
    /// day
    int m_day;
    /// management operation code
    int m_mgtOp;
    /// parameters, mgt1~mgt10
    FLTPT* m_parameters;
};

/*!
 * \class PltOp
 * \brief Plant management operation
 */
class PltOp: public PltMgtOp {
public:
    PltOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~PltOp();

    int PlantID() { return m_plantID; }

    FLTPT CurYearMaturity() { return m_curYrMat; }

    FLTPT HeatUnits() { return m_heatUnits; }

    FLTPT LAIInit() { return m_laiInit; }

    FLTPT BIOInit() { return m_bioInit; }

    FLTPT HITarg() { return m_hiTarg; }

    FLTPT BIOTarg() { return m_bioTarg; }

    FLTPT CNOP() { return m_CNOP; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_plantID;
    FLTPT m_curYrMat;
    FLTPT m_heatUnits;
    FLTPT m_laiInit;
    FLTPT m_bioInit;
    FLTPT m_hiTarg;
    FLTPT m_bioTarg;
    FLTPT m_CNOP;
};

/*!
 * \class IrrOp
 * \brief Irrigation operation
 */
class IrrOp: public PltMgtOp {
public:
    IrrOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~IrrOp();

    int IRRSource() { return m_irrSrc; }

    int IRRNo() { return m_irrNo; }

    FLTPT IRRApplyDepth() { return m_irrAmt; }

    FLTPT IRRSalt() { return m_irrSalt; }

    FLTPT IRREfficiency() { return m_irrEfm; }

    FLTPT IRRSQfrac() { return m_irrSq; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_irrSrc;
    FLTPT m_irrAmt;
    FLTPT m_irrSalt;
    FLTPT m_irrEfm;
    FLTPT m_irrSq;
    int m_irrNo;
};

/*!
 * \class FertOp
 * \brief Fertilizer operation
 */
class FertOp: public PltMgtOp {
public:
    FertOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~FertOp();

    int FertilizerID() { return m_fertID; }

    FLTPT FertilizerKg_per_ha() { return m_frtKgHa; }

    FLTPT FertilizerSurfaceFrac() { return m_frtSurface; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_fertID;
    FLTPT m_frtKgHa;
    /// fraction of fertilizer which is applied to the top 10 mm of soil (the remaining
    /// fraction is applied to first soil layer defined by user)
    FLTPT m_frtSurface;
};

/*!
 * \class PestOp
 * \brief Pesticide operation
 */
class PestOp: public PltMgtOp {
public:
    PestOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~PestOp();

    int PesticideID() { return m_pestID; }

    FLTPT PesticideKg() { return m_pstKg; }

    FLTPT PesticideDepth() { return m_pstDep; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_pestID;
    FLTPT m_pstKg;
    FLTPT m_pstDep;
};

/*!
 * \class HvstKillOp
 * \brief HarvestKill operation
 */
class HvstKillOp: public PltMgtOp {
public:
    HvstKillOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~HvstKillOp();

    FLTPT CNOP();

    FLTPT HarvestIndexOverride();

    FLTPT StoverFracRemoved();

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    FLTPT m_CNOP;
    FLTPT m_hiOvr;
    FLTPT m_fracHarvk;
};

/*!
 * \class TillOp
 * \brief Tillage operation
 */
class TillOp: public PltMgtOp {
public:
    TillOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~TillOp();

    FLTPT CNOP() { return m_CNOP; }

    int TillageID() { return m_tillID; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_tillID;
    FLTPT m_CNOP;
};

/*!
 * \class HvstOnlyOp
 * \brief HarvestOnly operation
 */
class HvstOnlyOp: public PltMgtOp {
public:
    HvstOnlyOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~HvstOnlyOp();

    FLTPT HarvestEfficiency() { return m_harvEff; }

    FLTPT HarvestIndexResidue() { return m_hiRsd; }

    FLTPT HarvestIndexBiomass() { return m_hiBms; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    FLTPT m_harvEff;
    FLTPT m_hiRsd;
    FLTPT m_hiBms;
};

/*!
 * \class KillOp
 * \brief Kill operation
 */
class KillOp: public PltMgtOp {
public:
    KillOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~KillOp();

    //! Output
    void dump(std::ostream* fs) OVERRIDE;
};

/*!
 * \class GrazOp
 * \brief Grazing operation
 */
class GrazOp: public PltMgtOp {
public:
    GrazOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~GrazOp();

    int GrazingDays() { return m_grzDays; }

    int ManureID() { return m_manureID; }

    FLTPT BiomassConsumed() { return m_bioEat; }

    FLTPT BiomassTrampled() { return m_bioTrmp; }

    FLTPT ManureDeposited() { return m_manureKg; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_grzDays;
    int m_manureID;
    FLTPT m_bioEat;
    FLTPT m_bioTrmp;
    FLTPT m_manureKg;
};

/*!
 * \class AutoIrrOp
 * \brief Auto irrigation operation
 */
class AutoIrrOp: public PltMgtOp {
public:
    AutoIrrOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~AutoIrrOp();

    int WaterStrsIdent() { return m_wstrsID; }

    int AutoIrrSrcCode() { return m_irrSrc; }

    int AutoIrrSrcLocs() { return m_irrNoa; }

    FLTPT AutoWtrStrsThrsd() { return m_autoWstrs; }

    FLTPT IrrigationEfficiency() { return m_irrEff; }

    FLTPT IrrigationWaterApplied() { return m_irrMx; }

    FLTPT SurfaceRunoffRatio() { return m_irrAsq; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_wstrsID;
    int m_irrSrc;
    FLTPT m_autoWstrs;
    int m_irrNoa;
    FLTPT m_irrEff;
    FLTPT m_irrMx;
    FLTPT m_irrAsq;
};

/*!
 * \class AutoFertOp
 * \brief Auto Fertilizer operation
 */
class AutoFertOp: public PltMgtOp {
public:
    AutoFertOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~AutoFertOp();

    int FertilizerID() { return m_afertID; }

    int NitrogenMethod() { return m_NStress; }

    FLTPT NitrogenStrsFactor() { return m_autoNStrs; }

    FLTPT MaxMineralN() { return m_autoNAPP; }

    FLTPT MaxMineralNYearly() { return m_autoNYR; }

    FLTPT FertEfficiency() { return m_autoEff; }

    FLTPT SurfaceFracApplied() { return m_afrtSurface; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_afertID;
    int m_NStress;
    FLTPT m_autoNStrs;
    FLTPT m_autoNAPP;
    FLTPT m_autoNYR;
    FLTPT m_autoEff;
    FLTPT m_afrtSurface;
};

/*!
 * \class RelImpndOp
 * \brief ReleaseImpound operation
 */
class RelImpndOp: public PltMgtOp {
public:
    RelImpndOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~RelImpndOp();

    int ImpoundTriger() { return m_impTrig; }

    FLTPT MaxPondDepth() { return m_maxPondDepth; }

    FLTPT MaxFitDepth() { return m_maxFitDepth; }

    FLTPT MinFitDepth() { return m_minFitDepth; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_impTrig;
    FLTPT m_maxPondDepth; ///< Maximum ponding depth, mm
    FLTPT m_maxFitDepth;  ///< Maximum fitting depth, mm
    FLTPT m_minFitDepth;  ///< Minimum fitting depth, mm
};

/*!
 * \class ContFertOp
 * \brief Continuous Fertilizer operation
 */
class ContFertOp: public PltMgtOp {
public:
    ContFertOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~ContFertOp();

    int FertilizerID() { return m_cfertID; }

    int ApplyFrequency() { return m_ifrtFreq; }

    int FertilizerDays() { return m_fertDays; }

    FLTPT FertilizerKg() { return m_cfrtKg; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_cfertID;
    int m_fertDays;
    FLTPT m_cfrtKg;
    int m_ifrtFreq;
};

/*!
 * \class ContPestOp
 * \brief Continuous Pesticide operation
 */
class ContPestOp: public PltMgtOp {
public:
    ContPestOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~ContPestOp();

    int PesticideID() { return m_ipstID; }

    int PesticideFrequency() { return m_pstFreq; }

    FLTPT PesticideKg() { return m_cpstKg; }

    int PesticideDays() { return m_pstDays; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    int m_ipstID;
    int m_pstDays;
    FLTPT m_cpstKg;
    int m_pstFreq;
};

/*!
 * \class BurnOp
 * \brief Burning operation
 */
class BurnOp: public PltMgtOp {
public:
    BurnOp(int mgtOp, bool usebaseHU, FLTPT husc, int year, int month, int day, FLTPT* parameters);
    
    ~BurnOp();

    FLTPT FractionLeft() { return m_burnFrlb; }

    //! Output
    void dump(std::ostream* fs) OVERRIDE;

private:
    FLTPT m_burnFrlb;
};

} /* plant_management */
} /* bmps */

#endif /* SEIMS_PLANT_MANAGEMENT_OPERATION_H */
