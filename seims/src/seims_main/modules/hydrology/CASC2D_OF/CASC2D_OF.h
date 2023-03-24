/*!
 * \file CASC2D_OF.h
 * \brief Brief description of this module
 *        Detail description about the implementation.
 * \author Dawei Xiao
 * \date 2022-12-30
 *
 */
#ifndef SEIMS_MODULE_TEMPLATE_H
#define SEIMS_MODULE_TEMPLATE_H
//#define IS_DEBUG 0
//#define IS_DEBUG 1 // Set Macro in the project property, or define during CMake
#include "SimulationModule.h"

// using namespace std; // Avoid use this statement

class CASC2D_OF : public SimulationModule {
public:
    CASC2D_OF(); //! Constructor

    ~CASC2D_OF(); //! Destructor

    ///////////// SetData series functions /////////////

    void SetValue(const char* key, float value) OVERRIDE;

    void SetValueByIndex(const char* key, int index, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int n, int col, float** data) OVERRIDE;

    void SetReaches(clsReaches* rches) OVERRIDE;

    void SetSubbasins(clsSubbasins* subbsns) OVERRIDE;

    void SetScenario(Scenario* sce) OVERRIDE;

    ///////////// CheckInputData and InitialOutputs /////////////

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;


    ///////////// Main control structure of execution code /////////////

    int Execute() OVERRIDE;

    ///////////// GetData series functions /////////////

    TimeStepType GetTimeStepType() OVERRIDE;

    void GetValue(const char* key, float* value) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void Get2DData(const char* key, int* n, int* col, float*** data) OVERRIDE;

    void SetRasterPositionDataPointer(const char* key, int** positions) OVERRIDE;

    void OvrlDepth();

    void ChannDepth();

    void OvrlRout();

    float ovrl(int icell, int rbCell);

    void ChannRout();

    void chnchn(int reachIndex, int curReachId, int nReaches, int iCell, vector<int> vecCells);

    void RoutOutlet();

    float newChnDepth(float wch, float dch, float sfactor,
                      int idCell, float addedVolume);

    float chnDischarge(float hchan, float hh, float wch, float dch,
                       float stordep, float rmanch, float a, float sf, float sfactor);

    void printPosition();

    void buildPositionIndex();

    void printLineBreak(int lastrow_firstcol);
    void printOvFlow();
    void printChFlow();
    void printCellFlow(int iCell);
    void printCellArrow(int iCell, int lastCol, int curCol);
    void printArrow(int iCell);


private:
    int counter;
    std::ofstream  ovFlowFptr;
    std::ofstream  chFlowFptr;
    std::ofstream  position_Fptr;
    std::ofstream  wtrDepFptr;
	
    int m_nCells;
    int m_maxSoilLyrs;
    float* m_nSoilLyrs;
    float** m_ks;
    float* m_soilWtrStoPrfl;
	
    int m_nSubbsns;                // subbasin number
    int m_inputSubbsnID;           // current subbasin ID, 0 for the entire watershed
    vector<int> m_subbasinIDs;     // subbasin IDs
    clsSubbasins* m_subbasinsInfo; // All subbasins information
    /// id of source cells of reaches
    int *m_sourceCellIds;
    /**
    *	The first element in each sub-array is the number of flow in cells in this sub-array
    */
    float **m_flowInIndex;
    /// map from subbasin id to index of the array
    map<int, int> m_idToIndex;

    /**** Inputs ***/
    int m_nrows;
    int m_ncols;
    float m_dt; /* DT_HS or DT_CH from FILE_IN*/
    float m_cellWth; /* Tag_CellWidth */
    float* m_ManningN; /* pman[iman[][]] manning coefficient */ /* VAR_MANNING from ParameterDB*/
    float* m_streamLink; /* VAR_STREAM_LINK from ParameterDB*/
    int m_nreach; 
    float* m_flowOutIndex;	/* flow out index*/
    int m_idOutlet;
    float* m_surSdep; /* VAR_SUR_SDEP from ParameterDB, need initialize in preprocess*/
    map<int, vector<int> > m_reachLayers; /* from reach propertites*/
    map<int, vector<int> > m_reachs;				/* */
    int** m_RasterPostion;		/* */
    map<int, int> m_downStreamReachId;	/* */
    float* m_chDownStream;	/* downstream id (The value is 0 if there if no downstream reach)*/
    /**** Input 1DArray ***/
    float* m_surWtrDepth;		/* surface water depth */ /* VAR_SURU from DepressionFS module */
    float* m_chDepth;                             /* chp[i][j][3] */
    float* m_chWidth;                             /* chp[i][j][2] */
    float* m_chSinuosity;                         /* chp[i][j][6] */
    float* m_dem;                                 /* e */
    float* m_chWtrDepth;                          /* hch */
    float* m_Slope;                               /* */
    /**** Output 1DArray ***/
    float* m_chQ; /* dqov m3/s*/
    float* m_ovQ; /* dqch m3/s*/
    float m_outQ; /* qout m3/s*/
    float m_outV; /* vout m3*/

    float qoutov; /* m3/s */	
    float sovout; /*  */										
    /**** others ***/
    bool m_InitialInputs;						/* */
    map<int, vector<int>> m_rbcellsMap;  /* */
    int output_icell;
    int printOvFlowMaxT;
    int printIOvFlowMinT;
    int printChFlowMaxT;
    int printChFlowMinT;
    int** m_RasterNeighbor; /* */
    float ** m_Dqq;
    float * cellWtrDep;
};

#endif /* SEIMS_MODULE_TEMPLATE_H */
