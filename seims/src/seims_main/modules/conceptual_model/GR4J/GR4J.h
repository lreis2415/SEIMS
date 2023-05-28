/*!
 * \file GR4J.h
 * \brief This implementation is adopted from \cite Raven. (http://raven.uwaterloo.ca)
 * \author Yujing Wang
 * \date 2023-05-07
 *
 * Changelog:
 *   - 1. 2023-05-07 - Author - Desc
 *
 *   @article{Raven,
  title={Flexible watershed simulation with the Raven hydrological modelling framework},
  author={Craig, James R and Brown, Genevieve and Chlumsky, Robert and Jenkinson, R Wayne and Jost, Georg and Lee, Konhee and Mai, Juliane and Serrer, Martin and Sgro, Nicholas and Shafii, Mahyar and others},
  journal={Environmental Modelling \& Software},
  volume={129},
  pages={104728},
  year={2020},
  publisher={Elsevier}
}
 */
#ifndef SEIMS_GR4J_H
#define SEIMS_GR4J_H

#include "SimulationModule.h"

enum ConvoleType {
    CONVOL_GR4J_1,
    CONVOL_GR4J_2
};

class GR4J : public SimulationModule {
public:
    GR4J(); //! Constructor

    ~GR4J(); //! Destructor

    ///////////// SetData series functions /////////////

    void SetSubbasins(clsSubbasins* subbsns);
    // void SetValue(const char* key, FLTPT value) OVERRIDE;
    void SetValue(const char* key, int value) OVERRIDE;
    void Set1DData(const char* key, int n, int* data) OVERRIDE;
    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;
    void Set2DData(const char* key, int nrows, int ncols, FLTPT** data) OVERRIDE;

    ///////////// GetData series functions /////////////
    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    ///////////// CheckInputData and InitialOutputs /////////////
    bool CheckInputData() OVERRIDE;
    void InitialOutputs() OVERRIDE;
    bool CheckInputSize(const char* key, int n);


    ///////////// Main control structure of execution code /////////////
    int Execute() OVERRIDE;

    void Infiltration();
    void SoilEvaporation();
    void Percolation(int fromSoilLayer, int toSoilLayer);
    void PercolationExch(int fromSoilLayer, int toSoilLayer);
    void PercolationExch2(int fromSoilLayer, int toSoilLayer);
    void Flush(FLTPT* fromStore, int toSoilLayer);
    void Flush(int fromSoilLayer, FLTPT* toStore);
    void Split();
    void Convolve(ConvoleType t);
    void GenerateUnitHydrograph(ConvoleType t, int &N);
    void Baseflow(int fromSoilLayer);

    void OverlandRouting();

    ///////////// Intermediate variable functions /////////////
    void CalculateSoilCapacity();
    FLTPT GR4J_SH2(const FLTPT& t, const FLTPT& x4); //<unit S-hydrograph (cumulative hydrograph) for GR4J #2

private:
    bool m_isInitialized;
    int m_nCells; ///< valid cells number

    //! number of subbasins
    int m_nSubbasins;
    /// current subbasin ID, 0 for the entire watershed
    int m_inputSubbsnId;
    //! subbasin IDs
    vector<int> m_subbasinIds;
    //! All subbasins information
    clsSubbasins* m_subbasins;
    /// subbasin grid (subbasins ID)
    int* m_cellsMappingToSubbasinId;

    int N_SOIL_LAYERS = 4;
    const int SOIL_PRODUCT_LAYER = 0;
    const int SOIL_ROUTING_LAYER = 1;
    const int SOIL_TEMP_LAYER = 2;
    const int SOIL_GW_LAYER = 3;

    /****************
     * Infintration: (PCP - PET) -> ProductLayer
     ****************/
    //input variables
    FLTPT* m_pcp; ///< precipitation
    FLTPT** m_soilThickness; ///< soil thickness
    FLTPT** m_soilPorosity; ///< soil porosity
    FLTPT** m_soilCapacity; ///< conceptual soil capacity

    //output variables
    /// the excess precipitation (mm) of the total nCells
    FLTPT* m_pcpExcess;
    /// infiltration map of watershed (mm) of the total nCells
    FLTPT* m_infil;
    /// 
    FLTPT** m_soilWtrSto;


    /****************
     * Soil Evaporation: ProductLayer -> SoilET
     ****************/
    //input variables
    FLTPT* m_pet; ///< precipitation

    //output
    FLTPT* m_soilET; ///< Output, actual soil evaporation

    /****************
     * Percolation: 
     ****************/
    //input
    FLTPT** m_GR4J_X2;
    FLTPT** m_GR4J_X3;

    /****************
     * Split and Convolve:
     ****************/
    //input
    FLTPT* m_GR4J_X4;
    FLTPT* m_convEntering1;
    FLTPT* m_convEntering2;

    //intermediate
    FLTPT** m_unitHydro;
    FLTPT** m_convTransport1;
    FLTPT** m_convTransport2;


    /****************
     * Routing
     ****************/
    FLTPT* m_Q_SBOF;
    FLTPT* m_Q_SB_ZEROS;
};

#endif
