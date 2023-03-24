/*!
 * \file NutrientTransportSediment.h
 * \brief Nutrient removed and lost in surface runoff.
 *
 *        Support three carbon model, static method (orgn.f), C-FARM one carbon model (orgncswat.f),
 *        and CENTURY C/N cycling model (NCsed_leach.f90) from SWAT
 *        As for phosphorus, psed.f of SWAT calculates the attached to sediment in surface runoff.
 *
 * Changelog:
 *   - 1. 2016-04-30 - hr - Initial implementation.
 *   - 2. 2016-09-28 - lj - Add CENTURY model of calculating organic nitrogen removed in surface runoff.
 *   - 3. 2017-08-23 - lj -  Solve inconsistent results when using openmp to reducing raster data according to subbasin ID.
 *   - 4. 2018-05-08 - lj -
 *        -# Reformat, especially naming style (sync update in "text.h").
 *        -# Code optimization, e.g., use multiply instead of divide.
 *   - 5. 2022-08-22 - lj - Change float to FLTPT.
 *
 * \todo Ammonian adsorbed to soil should be considered.
 * \author Huiran Gao, Liangjun Zhu
 */
#ifndef SEIMS_MODULE_NUTRSED_H
#define SEIMS_MODULE_NUTRSED_H

#include "SimulationModule.h"

/** \defgroup NUTRSED
 * \ingroup Nutrient
 * \brief Nutrient removed and lost with the eroded sediment in surface runoff
 */

/*!
 * \class NutrientTransportSediment
 * \ingroup NUTRSED
 *
 * \brief Nutrient removed and lost with the eroded sediment in surface runoff
 *
 */

class NutrientTransportSediment: public SimulationModule {
public:
    NutrientTransportSediment();

    ~NutrientTransportSediment();

    void SetValue(const char* key, FLTPT value) OVERRIDE;

    void SetValue(const char* key, int value) OVERRIDE;

    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    void Set1DData(const char* key, int n, int* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, FLTPT** data) OVERRIDE;

    void SetSubbasins(clsSubbasins*) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    void InitialIntermediates() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    void Get2DData(const char* key, int* nrows, int* ncols, FLTPT*** data) OVERRIDE;

private:
    /*!
    * \brief check the input data for running CENTURY model. Make sure all the inputs data is available.
    * \return bool The validity of the inputs data.
    */
    bool CheckInputDataCenturyModel();

    /*!
    * \brief check the input data for running C-FARM one carbon model. Make sure all the inputs data is available.
    * \return bool The validity of the inputs data.
    */
    bool CheckInputDataCFarmModel();

    /*!
    * \brief calculates the amount of organic nitrogen removed in surface runoff.
    *        orgn.f of SWAT, when CSWAT = 0
    * \return void
    */
    void OrgNRemovedInRunoffStaticMethod(int i);

    /*!
    * \brief calculates the amount of organic nitrogen removed in surface runoff.
    *        orgnswat.f of SWAT, when CSWAT = 1
    * \TODO THIS IS ON TODO LIST
    * \return void
    */
    void OrgNRemovedInRunoffCFarmOneCarbonModel(int i);

    /*!
    * \brief calculates the amount of organic nitrogen removed in surface runoff.
    *        NCsed_leach.f90 of SWAT, when CSWAT = 2
    * \return void
    */
    void OrgNRemovedInRunoffCenturyModel(int i);

    /*!
    * \brief Calculates the amount of organic and mineral phosphorus attached to sediment in surface runoff.
    * psed.f of SWAT
    * \return void
    */
    void OrgPAttachedtoSed(int i);

private:
    /// subbasins number
    int m_nSubbsns;
    /// current subbasin ID, 0 for the entire watershed
    int m_inputSubbsnID;
    /// cell width of grid map (m)
    FLTPT m_cellWth;
    /// cell area of grid map (ha)
    FLTPT m_cellArea;
    /// number of cells
    int m_nCells;
    /// soil layers
    int* m_nSoilLyrs;
    /// maximum soil layers
    int m_maxSoilLyrs;
    /// soil rock content, %
    FLTPT** m_soilRock;
    /// sol_ul, soil saturated water amount, mm
    FLTPT** m_soilSat;
    /* carbon modeling method
     *   = 0 Static soil carbon (old mineralization routines)
     *   = 1 C-FARM one carbon pool model
     *   = 2 Century model
     */
    int m_cbnModel;
    /// enrichment ratio
    FLTPT* m_enratio;

    ///inputs

    // soil loss caused by water erosion
    FLTPT* m_olWtrEroSed;
    // surface runoff generated
    FLTPT* m_surfRf;
    //bulk density of the soil
    FLTPT** m_soilBD;
    // thickness of soil layer
    FLTPT** m_soilThk;
    /// Soil mass (kg/ha)
    FLTPT** m_soilMass;

    /// subbasin related
    //! subbasin IDs
    vector<int> m_subbasinIDs;
    /// subbasin grid (subbasins ID)
    int* m_subbsnID;
    /// subbasins information
    clsSubbasins* m_subbasinsInfo;

    ///output data
    //amount of organic nitrogen in surface runoff
    FLTPT* m_surfRfSedOrgN;
    //amount of organic phosphorus in surface runoff
    FLTPT* m_surfRfSedOrgP;
    //amount of active mineral phosphorus sorbed to sediment in surface runoff
    FLTPT* m_surfRfSedAbsorbMinP;
    //amount of stable mineral phosphorus sorbed to sediment in surface runoff
    FLTPT* m_surfRfSedSorbMinP;

    /// output to channel

    FLTPT* m_surfRfSedOrgNToCh;       // amount of organic N in surface runoff to channel, kg
    FLTPT* m_surfRfSedOrgPToCh;       // amount of organic P in surface runoff to channel, kg
    FLTPT* m_surfRfSedAbsorbMinPToCh; // amount of active mineral P in surface runoff to channel, kg
    FLTPT* m_surfRfSedSorbMinPToCh;   // amount of stable mineral P in surface runoff to channel, kg

    ///input & output
    //amount of nitrogen stored in the active organic (humic) nitrogen pool, kg N/ha
    FLTPT** m_soilActvOrgN;
    //amount of nitrogen stored in the fresh organic (residue) pool, kg N/ha
    FLTPT** m_soilFrshOrgN;
    //amount of nitrogen stored in the stable organic N pool, kg N/ha
    FLTPT** m_soilStabOrgN;
    //amount of phosphorus stored in the organic P pool, kg P/ha
    FLTPT** m_soilHumOrgP;
    //amount of phosphorus stored in the fresh organic (residue) pool, kg P/ha
    FLTPT** m_soilFrshOrgP;
    //amount of phosphorus in the soil layer stored in the stable mineral phosphorus pool, kg P/ha
    FLTPT** m_soilStabMinP;
    //amount of phosphorus stored in the active mineral phosphorus pool, kg P/ha
    FLTPT** m_soilActvMinP;
    /// for C-FARM one carbon model
    FLTPT** m_soilManP;
    /// for CENTURY C/Y cycling model
    /// inputs from other modules
    FLTPT** m_sol_LSN;
    FLTPT** m_sol_LMN;
    FLTPT** m_sol_HPN;
    FLTPT** m_sol_HSN;
    FLTPT** m_sol_HPC;
    FLTPT** m_sol_HSC;
    FLTPT** m_sol_LMC;
    FLTPT** m_sol_LSC;
    FLTPT** m_sol_LS;
    FLTPT** m_sol_LM;
    FLTPT** m_sol_LSL;
    FLTPT** m_sol_LSLC;
    FLTPT** m_sol_LSLNC;
    FLTPT** m_sol_BMC;
    FLTPT** m_sol_WOC;
    FLTPT** m_soilPerco;
    FLTPT** m_subSurfRf;
    /// outputs
    FLTPT** m_soilIfluCbn;     ///< lateral flow Carbon loss in each soil layer
    FLTPT** m_soilPercoCbn;    ///< percolation Carbon loss in each soil layer
    FLTPT* m_soilIfluCbnPrfl;  ///< lateral flow Carbon loss in soil profile
    FLTPT* m_soilPercoCbnPrfl; ///< percolation Carbon loss in soil profile
    FLTPT* m_sedLossCbn;       ///< amount of C lost with sediment pools
};
#endif /* SEIMS_MODULE_NUTRSED_H */
