/*!
 * \brief
 * \author Liang-Jun Zhu
 * \date June 2016
 */
#include "PlantManagementOperation.h"
#include <sstream>
#include <iomanip>
#include "utils.h"
#include "BMPText.h"
#include "util.h"

using namespace MainBMP;
using namespace PlantManagement;

/// Base class
PlantManagementOperation::PlantManagementOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
                                                   float *parameters)
        : m_mgtOp(mgtOp), m_useBaseHUSC(usebaseHU), m_frHU(husc), m_year(year), m_month(month), m_day(day), m_parameters(parameters)
{
}

PlantManagementOperation::~PlantManagementOperation()
{
        /// This destruct function should not be executed!
        cout<<"The destruct function of PlantManagementOperation class should not be executed!"<<endl;
}

/// Plant
PlantOperation::PlantOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_plantID = int(parameters[0]);
        m_curYrMat = parameters[2];
        m_heatUnits = (parameters[3] < 700.f) ? 1700.f : parameters[3];
        m_laiInit = parameters[4];
        m_bioInit = parameters[5];
        m_hiTarg = parameters[6];
        m_bioTarg = parameters[7] * 1000.f; /// tons/ha => kg/ha
        m_CNOP = parameters[8];
}

PlantOperation::~PlantOperation()
{
}

void PlantOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    Plant Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Plant ID: " << m_plantID << " CurrentYearToMaturity: " << m_curYrMat <<
                " Heat Units: " << m_heatUnits << " Initial LAI: " << m_laiInit <<
                " Initial Biomass: " << m_bioInit << " Harvest Index: " << m_hiTarg <<
                " Biomass target: " << m_bioTarg << " CNOP: " << m_CNOP << endl;
}

/// Irrigation
IrrigationOperation::IrrigationOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_irrSrc = int(parameters[1]);
        m_irrSrc = (m_irrSrc <= 0) ? IRR_SRC_OUTWTSD : m_irrSrc;
        m_irrAmt = parameters[3];
        m_irrSalt = parameters[4];
        m_irrEfm = (parameters[5] < UTIL_ZERO) ? 1.0f : parameters[5];
        m_irrSq = parameters[6];
        m_irrNo = int(parameters[7]);
}

IrrigationOperation::~IrrigationOperation()
{
}

void IrrigationOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    Irrigation Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Irrigation Source: " << m_irrSrc << " Applied Depth: " << m_irrAmt <<
                " Salt: " << m_irrSalt << " Efficiency: " << m_irrEfm <<
                " Surface Runoff Ratio: " << m_irrSq << " Source LocationID: " << m_irrNo << endl;
}

/// Fertilizer
FertilizerOperation::FertilizerOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_fertID = int(parameters[0]);
        m_frtKgHa = parameters[3];
        m_frtSurface = (parameters[4] < UTIL_ZERO) ? 0.2f : parameters[4];
}

FertilizerOperation::~FertilizerOperation()
{
}

void FertilizerOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    Fertilizer Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Fertilizer ID: " << m_fertID << " Amount (kg): " << m_frtKgHa <<
                " Fraction Applied in Surface: " << m_frtSurface << endl;
}

/// Pesticide
PesticideOperation::PesticideOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_pestID = int(parameters[0]);
        m_pstKg = parameters[3];
        m_pstDep = parameters[4];
}

PesticideOperation::~PesticideOperation()
{
}

void PesticideOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    Pesticide Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Pesticide ID: " << m_pestID << " Amount (kg): " << m_pstKg <<
                " Applied Depth (mm): " << m_pstDep << endl;
}

/// HarvestKill
HarvestKillOperation::HarvestKillOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_CNOP = parameters[3];
        m_hiOvr = parameters[4];
        m_fracHarvk = parameters[5];
}

HarvestKillOperation::~HarvestKillOperation()
{
}

void HarvestKillOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    HarvestKill Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " CNOP: " << m_CNOP << " Harvest Index Override: " << m_hiOvr <<
                " Stover Fraction Removed: " << m_fracHarvk << endl;
}

/// Tillage
TillageOperation::TillageOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_tillID = int(parameters[0]);
        m_CNOP = parameters[3];
}

TillageOperation::~TillageOperation()
{
}

void TillageOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    Tillage Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Tillage ID: " << m_tillID << " CNOP: " << m_CNOP << endl;
}

/// HarvestOnly
HarvestOnlyOperation::HarvestOnlyOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_harvEff = (parameters[3] <= 0.) ? 1.0f : parameters[3];
        m_hiBms = parameters[4];
        m_hiRsd = parameters[5];
}

HarvestOnlyOperation::~HarvestOnlyOperation()
{
}

void HarvestOnlyOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    HarvestOnly Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Harvest Efficiency: " << m_harvEff << " Harvest Index Biomass: " << m_hiBms <<
                " Harvest Index Residue: " << m_hiRsd << endl;
}

/// Kill
KillOperation::KillOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
}

KillOperation::~KillOperation()
{
}

void KillOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    Kill Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day << endl;
}

/// Grazing
GrazingOperation::GrazingOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_grzDays = int(parameters[0]);
        m_manureID = int(parameters[1]);
        m_bioEat = parameters[3];
        m_bioTrmp = parameters[4];
        m_manureKg = parameters[5];
        if (m_manureKg <= 0.)
                m_manureKg = 0.95f * m_bioEat;
}

GrazingOperation::~GrazingOperation()
{
}

void GrazingOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    Grazing Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Grazing Days: " << m_grzDays << " Manure ID: " << m_manureID <<
                " Biomass Consumed: " << m_bioEat << " Biomass Trampled: " << m_bioTrmp <<
                " Manure Deposited: " << m_manureKg << endl;
}

/// AutoIrrigation
AutoIrrigationOperation::AutoIrrigationOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters)
        :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_wstrsID = int(parameters[0]);
        m_irrSrc = int(parameters[1]);
        m_autoWstrs = parameters[3];
        m_irrEff = parameters[4];
        m_irrMx = parameters[5];
        m_irrAsq = parameters[6];
        m_irrNoa = int(parameters[9]);
        m_irrSrc = (m_irrSrc <= 0) ? IRR_SRC_OUTWTSD : m_irrSrc;
        if (m_wstrsID <= 0) m_wstrsID = 1;
        if (m_irrEff > 1.f) m_irrEff = 0.f;
        if (FloatEqual(m_irrEff, 0.f)) m_irrEff = 1.f;
        if (m_irrMx < UTIL_ZERO) m_irrMx = 25.4f;
}

AutoIrrigationOperation::~AutoIrrigationOperation()
{
}

void AutoIrrigationOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    AutoIrrigation Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Water Stress Identifier: " << m_wstrsID << " AutoIrrigation Source: " << m_irrSrc <<
                " Source Location ID: " << m_irrNoa << " Water Stress Threshold: " << m_autoWstrs <<
                " Efficiency: " << m_irrEff << " Water Applied (mm): " << m_irrMx <<
                " Surface Runoff Ratio: " << m_irrAsq << endl;
}

/// AutoFertilizer
AutoFertilizerOperation::AutoFertilizerOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters)
        : PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_afertID = int(parameters[0]);
        m_NStress = int(parameters[1]);
        m_autoNStrs = parameters[3];
        m_autoNAPP = parameters[4];
        m_autoNYR = parameters[5];
        m_autoEff = parameters[6];
        m_afrtSurface = parameters[7];
        if (m_autoNAPP < UTIL_ZERO) m_autoNAPP = 250.f;
        if (m_autoNYR < UTIL_ZERO) m_autoNYR = 350.f;
        if (m_afrtSurface <= UTIL_ZERO) m_afrtSurface = 0.8f;
}

AutoFertilizerOperation::~AutoFertilizerOperation()
{
}

void AutoFertilizerOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    AutoFertilizer Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Fertilizer ID: " << m_afertID << " Nitrogen Method: " << m_NStress <<
                " Nitrogen Stress Factor: " << m_autoNStrs << " MaxMineral Nitrogen: " << m_autoNAPP <<
                " MaxMineral Nitrogen Yearly: " << m_autoNYR << " Fertilizer Efficiency: " << m_autoEff <<
                " Surface Fraction Applied: " << m_afrtSurface << endl;
}

/// ReleaseImpound
ReleaseImpoundOperation::ReleaseImpoundOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters)
        :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_impTrig = int(parameters[0]);
        m_maxDepth = float(parameters[1]);
        m_lowDepth = float(parameters[2]);
        m_upDepth = float(parameters[3]);
}

ReleaseImpoundOperation::~ReleaseImpoundOperation()
{
}

void ReleaseImpoundOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    Release/Impound Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Impound Trigger: " << m_impTrig << endl;
}

/// ContinuousFertilizer
ContinuousFertilizerOperation::ContinuousFertilizerOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
                                                             float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_cfertID = int(parameters[1]);
        m_fertDays = int(parameters[0]);
        m_ifrtFreq = int(parameters[2]);
        m_cfrtKg = parameters[3];
}

ContinuousFertilizerOperation::~ContinuousFertilizerOperation()
{
}

void ContinuousFertilizerOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    Continuous Fertilizer Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Fertilizer ID: " << m_cfertID << " Amount (kg): " << m_cfrtKg <<
                " Frequency: " << m_ifrtFreq << " Duration Days: " << m_fertDays << endl;
}

/// Continuous Pesticide
ContinuousPesticideOperation::ContinuousPesticideOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day,
                                                           float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_ipstID = int(parameters[0]);
        m_pstDays = int(parameters[1]);
        m_pstFreq = int(parameters[2]);
        m_cpstKg = parameters[3];
}

ContinuousPesticideOperation::~ContinuousPesticideOperation()
{
}

void ContinuousPesticideOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    Continuous Pesticide Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Pesticide ID: " << m_ipstID << " Amount (kg): " << m_cpstKg <<
                " Frequency: " << m_pstFreq << " Duration Days: " << m_pstDays << endl;
}

/// Burning
BurningOperation::BurningOperation(int mgtOp, bool usebaseHU, float husc, int year, int month, int day, float *parameters) :
        PlantManagementOperation(mgtOp, usebaseHU, husc, year, month, day, parameters)
{
        m_burnFrlb = parameters[3];
}

BurningOperation::~BurningOperation()
{
}

void BurningOperation::dump(ostream *fs)
{
        if (fs == NULL) return;
        *fs << "    Burning Operation: " << endl <<
                "      HUSC: " << m_frHU << " rotationYear: " << m_year <<
                " Month: " << m_month << " Day: " << m_day <<
                " Fraction Left: " << m_burnFrlb << endl;
}
