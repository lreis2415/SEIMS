/*!
 * \file SUR_GR4J.h
 * \brief Infiltration & Surface Runoff using GR4J's method.
 *        This implementation is adopted from the infiltration module of \cite Raven. (http://raven.uwaterloo.ca)
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
#ifndef SEIMS_SUR_GR4J_H
#define SEIMS_SUR_GR4J_H

#include "SimulationModule.h"

class SUR_GR4J: public SimulationModule {
public:
    SUR_GR4J(); //! Constructor

    ~SUR_GR4J(); //! Destructor

    ///////////// SetData series functions /////////////

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;
    
    ///////////// CheckInputData and InitialOutputs /////////////

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    bool CheckInputSize(const char *key, int n);

    ///////////// Main control structure of execution code /////////////

    int Execute() OVERRIDE;

    ///////////// GetData series functions /////////////

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;
    

private:
    int m_nCells; ///< valid cells number

    //input variables

    /// Net precipitation calculated in the interception module (mm)
    FLTPT *m_pNet;
    /// soil thickness of the top layer
    FLTPT *m_topSoilThickness;

    /// soil rock/stone fraction of the top layer. assumed to be 0.
    // FLTPT *m_topSoilRockFraction;

    /// soil porosity
    FLTPT *m_topSoilPorosity;

    //intermediate variables

    ///Parameter X1 of GR4J model. X1 = SoilCapacity = Thickness * Porosity * (1-StoneFraction). 
    ///StoneFraction is not considered in this implementation, assumed to be 0.
    FLTPT *m_GR4J_X1;


    //output variables

    /// the excess precipitation (mm) of the total nCells
    FLTPT *m_pcpExcess;
    /// infiltration map of watershed (mm) of the total nCells
    FLTPT *m_infil;
    /// 
    FLTPT **m_soilWtrSto;

};

#endif
