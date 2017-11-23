/*!
 * \brief Plant management operation class
 * \author Liang-Jun Zhu
 * \date June 2016
 */
#pragma once
#include "utilities.h"
#include "BMPText.h"

using namespace std;
namespace MainBMP {
/*!
 * \ingroup MainBMP
 * \brief Base namespace for Plant management related operations
 * \author Liang-Jun Zhu
 */
namespace PlantManagement {
/*!
 * \class PlantManagementOperation
 * \ingroup PlantManagement
 * \brief Base class of plant management operation
 */
class PlantManagementOperation {
public:
    /*!
     * \brief Constructor
     * \param[in] mgtCode 1 to 16
     * \param[in] usebaseHU true or false
     * \param[in] husc Fraction of heat units (base or plant)
     * \param[in] year Rotation year, e.g., 1,2,...
     * \param[in] month
     * \param[in] day
     * \param[in] location Indexes of field to practice the operation (string)
     */
    PlantManagementOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
                             float *parameters);

    //! virtual Destructor
    virtual ~PlantManagementOperation(void);

    //! Output
    virtual void dump(ostream *fs) = 0;

    bool &UseBaseHUSC(void) {
        return m_useBaseHUSC;
    }

    float &GetHUFraction(void) {
        return m_frHU;
    }

    int &GetMonth(void) {
        return m_month;
    }

    int &GetDay(void) {
        return m_day;
    }

    int &GetOperationCode(void) {
        return m_mgtOp;
    }

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
    float *m_parameters;
};

/*!
 * \class PlantOperation
 * \ingroup PlantManagement
 * \brief Plant management operation
 */
class PlantOperation : public PlantManagementOperation {
public:
    PlantOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters);

    ~PlantOperation(void);

    int PlantID(void) {
        return m_plantID;
    }

    float CurYearMaturity(void) {
        return m_curYrMat;
    }

    float HeatUnits(void) {
        return m_heatUnits;
    }

    float LAIInit(void) {
        return m_laiInit;
    }

    float BIOInit(void) {
        return m_bioInit;
    }

    float HITarg(void) {
        return m_hiTarg;
    }

    float BIOTarg(void) {
        return m_bioTarg;
    }

    float CNOP(void) {
        return m_CNOP;
    }

    //! Output
    void dump(ostream *fs);

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
 * \class IrrigationOperation
 * \ingroup PlantManagement
 * \brief Irrigation operation
 */
class IrrigationOperation : public PlantManagementOperation {
public:
    IrrigationOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters);

    ~IrrigationOperation(void);

    int IRRSource(void) {
        return m_irrSrc;
    }

    int IRRNo(void) {
        return m_irrNo;
    }

    float IRRApplyDepth(void) {
        return m_irrAmt;
    }

    float IRRSalt(void) {
        return m_irrSalt;
    }

    float IRREfficiency(void) {
        return m_irrEfm;
    }

    float IRRSQfrac(void) {
        return m_irrSq;
    }

    //! Output
    void dump(ostream *fs);

private:
    int m_irrSrc;
    float m_irrAmt;
    float m_irrSalt;
    float m_irrEfm;
    float m_irrSq;
    int m_irrNo;
};

/*!
 * \class FertilizerOperation
 * \ingroup PlantManagement
 * \brief Fertilizer operation
 */
class FertilizerOperation : public PlantManagementOperation {
public:
    FertilizerOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters);

    ~FertilizerOperation(void);

    int FertilizerID(void) {
        return m_fertID;
    }

    float FertilizerKg_per_ha(void) {
        return m_frtKgHa;
    }

    float FertilizerSurfaceFrac(void) {
        return m_frtSurface;
    }

    //! Output
    void dump(ostream *fs);

private:
    int m_fertID;
    float m_frtKgHa;
    /// fraction of fertilizer which is applied to the top 10 mm of soil (the remaining
    /// fraction is applied to first soil layer defined by user)
    float m_frtSurface;
};

/*!
 * \class PesticideOperation
 * \ingroup PlantManagement
 * \brief Pesticide operation
 */
class PesticideOperation : public PlantManagementOperation {
public:
    PesticideOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters);

    ~PesticideOperation(void);

    int PesticideID(void) {
        return m_pestID;
    }

    float PesticideKg(void) {
        return m_pstKg;
    }

    float PesticideDepth(void) {
        return m_pstDep;
    }

    //! Output
    void dump(ostream *fs);

private:
    int m_pestID;
    float m_pstKg;
    float m_pstDep;
};

/*!
 * \class HarvestKillOperation
 * \ingroup PlantManagement
 * \brief HarvestKill operation
 */
class HarvestKillOperation : public PlantManagementOperation {
public:
    HarvestKillOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
                         float *parameters);

    ~HarvestKillOperation(void);

    float CNOP(void) {
        return m_CNOP;
    }

    float HarvestIndexOverride(void) {
        return m_hiOvr;
    }

    float StoverFracRemoved(void) {
        return m_fracHarvk;
    }

    //! Output
    void dump(ostream *fs);

private:
    float m_CNOP;
    float m_hiOvr;
    float m_fracHarvk;
};

/*!
 * \class TillageOperation
 * \ingroup PlantManagement
 * \brief Tillage operation
 */
class TillageOperation : public PlantManagementOperation {
public:
    TillageOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters);

    ~TillageOperation(void);

    float CNOP(void) {
        return m_CNOP;
    }

    int TillageID(void) {
        return m_tillID;
    }

    //! Output
    void dump(ostream *fs);

private:
    int m_tillID;
    float m_CNOP;
};

/*!
 * \class HarvestOnlyOperation
 * \ingroup PlantManagement
 * \brief HarvestOnly operation
 */
class HarvestOnlyOperation : public PlantManagementOperation {
public:
    HarvestOnlyOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
                         float *parameters);

    ~HarvestOnlyOperation(void);

    float HarvestEfficiency(void) {
        return m_harvEff;
    }

    float HarvestIndexResidue(void) {
        return m_hiRsd;
    }

    float HarvestIndexBiomass(void) {
        return m_hiBms;
    }

    //! Output
    void dump(ostream *fs);

private:
    float m_harvEff;
    float m_hiRsd;
    float m_hiBms;
};

/*!
 * \class KillOperation
 * \ingroup PlantManagement
 * \brief Kill operation
 */
class KillOperation : public PlantManagementOperation {
public:
    KillOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters);

    ~KillOperation(void);

    //! Output
    void dump(ostream *fs);
};

/*!
 * \class GrazingOperation
 * \ingroup PlantManagement
 * \brief Grazing operation
 */
class GrazingOperation : public PlantManagementOperation {
public:
    GrazingOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters);

    ~GrazingOperation(void);

    int GrazingDays(void) {
        return m_grzDays;
    }

    int ManureID(void) {
        return m_manureID;
    }

    float BiomassConsumed(void) {
        return m_bioEat;
    }

    float BiomassTrampled(void) {
        return m_bioTrmp;
    }

    float ManureDeposited(void) {
        return m_manureKg;
    }

    //! Output
    void dump(ostream *fs);

private:
    int m_grzDays;
    int m_manureID;
    float m_bioEat;
    float m_bioTrmp;
    float m_manureKg;
};

/*!
 * \class AutoIrrigationOperation
 * \ingroup PlantManagement
 * \brief Auto irrigation operation
 */
class AutoIrrigationOperation : public PlantManagementOperation {
public:
    AutoIrrigationOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
                            float *parameters);

    ~AutoIrrigationOperation(void);

    int WaterStrsIdent(void) {
        return m_wstrsID;
    }

    int AutoIrrSrcCode(void) {
        return m_irrSrc;
    }

    int AutoIrrSrcLocs(void) {
        return m_irrNoa;
    }

    float AutoWtrStrsThrsd(void) {
        return m_autoWstrs;
    }

    float IrrigationEfficiency(void) {
        return m_irrEff;
    }

    float IrrigationWaterApplied(void) {
        return m_irrMx;
    }

    float SurfaceRunoffRatio(void) {
        return m_irrAsq;
    }

    //! Output
    void dump(ostream *fs);

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
 * \class AutoFertilizerOperation
 * \ingroup PlantManagement
 * \brief Auto Fertilizer operation
 */
class AutoFertilizerOperation : public PlantManagementOperation {
public:
    AutoFertilizerOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
                            float *parameters);

    ~AutoFertilizerOperation(void);

    int FertilizerID(void) {
        return m_afertID;
    }

    int NitrogenMethod(void) {
        return m_NStress;
    }

    float NitrogenStrsFactor(void) {
        return m_autoNStrs;
    }

    float MaxMineralN(void) {
        return m_autoNAPP;
    }

    float MaxMineralNYearly(void) {
        return m_autoNYR;
    }

    float FertEfficiency(void) {
        return m_autoEff;
    }

    float SurfaceFracApplied(void) {
        return m_afrtSurface;
    }

    //! Output
    void dump(ostream *fs);

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
 * \class ReleaseImpoundOperation
 * \ingroup PlantManagement
 * \brief ReleaseImpound operation
 */
class ReleaseImpoundOperation : public PlantManagementOperation {
public:
    ReleaseImpoundOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
                            float *parameters);

    ~ReleaseImpoundOperation(void);

    int ImpoundTriger(void) {
        return m_impTrig;
    }

    float MaxPondDepth(void) {
        return m_maxPondDepth;
    }

    float MaxFitDepth(void) {
        return m_maxFitDepth;
    }

    float MinFitDepth(void) {
        return m_minFitDepth;
    }

    //! Output
    void dump(ostream *fs);

private:
    int m_impTrig;
    float m_maxPondDepth;  ///< Maximum ponding depth, mm
    float m_maxFitDepth;  ///< Maximum fitting depth, mm
    float m_minFitDepth;  ///< Minimum fitting depth, mm
};

/*!
 * \class ContinuousFertilizerOperation
 * \ingroup PlantManagement
 * \brief Continuous Fertilizer operation
 */
class ContinuousFertilizerOperation : public PlantManagementOperation {
public:
    ContinuousFertilizerOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
                                  float *parameters);

    ~ContinuousFertilizerOperation(void);

    int FertilizerID(void) {
        return m_cfertID;
    }

    int ApplyFrequency(void) {
        return m_ifrtFreq;
    }

    int FertilizerDays(void) {
        return m_fertDays;
    }

    float FertilizerKg(void) {
        return m_cfrtKg;
    }

    //! Output
    void dump(ostream *fs);

private:
    int m_cfertID;
    int m_fertDays;
    float m_cfrtKg;
    int m_ifrtFreq;
};

/*!
 * \class ContinuousPesticideOperation
 * \ingroup PlantManagement
 * \brief Continuous Pesticide operation
 */
class ContinuousPesticideOperation : public PlantManagementOperation {
public:
    ContinuousPesticideOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
                                 float *parameters);

    ~ContinuousPesticideOperation(void);

    int PesticideID(void) {
        return m_ipstID;
    }

    int PesticideFrequency(void) {
        return m_pstFreq;
    }

    float PesticideKg(void) {
        return m_cpstKg;
    }

    int PesticideDays(void) {
        return m_pstDays;
    }

    //! Output
    void dump(ostream *fs);

private:
    int m_ipstID;
    int m_pstDays;
    float m_cpstKg;
    int m_pstFreq;
};

/*!
 * \class BurningOperation
 * \ingroup PlantManagement
 * \brief Burning operation
 */
class BurningOperation : public PlantManagementOperation {
public:
    BurningOperation(int mgtOp, bool useBaseHU, float husc, int year, int month, int day, float *parameters);

    ~BurningOperation(void);

    float FractionLeft(void) {
        return m_burnFrlb;
    }

    //! Output
    void dump(ostream *fs);

private:
    float m_burnFrlb;
};
}
}
