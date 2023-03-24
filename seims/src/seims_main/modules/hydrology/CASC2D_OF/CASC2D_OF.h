/*!
 * \file template.h
 * \brief Brief description of this module
 *        Detail description about the implementation.
 * \author Dawei Xiao
 * \date 2022-12-30
 *
 */
#ifndef SEIMS_MODULE_TEMPLATE_H
#define SEIMS_MODULE_TEMPLATE_H
//#define IS_DEBUG 0
#define IS_DEBUG 1
#include "SimulationModule.h"

using namespace std;

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

	void CASC2D_OF::buildPositionIndex();

	void CASC2D_OF::printLineBreak(int lastrow_firstcol);
	void CASC2D_OF::printOvFlow();
	void CASC2D_OF::printChFlow();
	void CASC2D_OF::printCellFlow(int iCell);
	void CASC2D_OF::printCellArrow(int iCell, int lastCol, int curCol);
	void CASC2D_OF::printArrow(int iCell);
	void CASC2D_OF::deleteExistFile(string file);


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
	
	int m_nSubbsns;                             // subbasin number
	int m_inputSubbsnID;                    // current subbasin ID, 0 for the entire watershed
	vector<int> m_subbasinIDs;         // subbasin IDs
	clsSubbasins* m_subbasinsInfo;   // All subbasins information
	/// id of source cells of reaches
	int *m_sourceCellIds;
	/**
	*	The first element in each sub-array is the number of flow in cells in this sub-array
	*   存储每个栅格单元的入流单元
	*   例如:
	*          [[2, 100,200],[3,101,201,202]]
	*          表示第一个栅格单元的入流单元有2个，id分别为100和200；第二个栅格单元的入流单元有3个，id分别为101，201，202
	*/
	float **m_flowInIndex;
	/// map from subbasin id to index of the array
	map<int, int> m_idToIndex;

	/**** 输入参数 ***/
	int m_nrows;					/* 行数*/
	int m_ncols;						/* 列数*/
	float m_dt;						/* dt时间步长 */												/* DT_HS 或 DT_CH from FILE_IN*/
	float m_cellWth;				/* w栅格单元的宽度*/										/* Tag_CellWidth	from */
	float* m_ManningN;		/* pman[iman[][]]栅格单元上的曼宁系数*/		/* VAR_MANNING from ParameterDB*/
	float* m_streamLink;		/* link栅格单元上的河道编号*/							/* VAR_STREAM_LINK from ParameterDB*/
	int m_nreach;					/* maxlink河道数量*/
	float* m_flowOutIndex;	/* D8流向算法的flow out index*/
	int m_idOutlet;					/* jout kout流域出水口的id*/								/* Tag_FLOWOUT_INDEX_D8*/
	float* m_surSdep;			/* sdep[i][j] 栅格单元上的初始蓄水深度*/				/* VAR_SUR_SDEP from ParameterDB, need initialize in preprocess*/
	map<int, vector<int> > m_reachLayers;	/* 河道图层*/							/* from reach propertites*/
	map<int, vector<int> > m_reachs;				/* */
	int** m_RasterPostion;		/* 储存栅格位置数据的数组*/
	map<int, int> m_downStreamReachId;	/* 储存上、下游河道id的映射关系*/
	float* m_chDownStream;	/* downstream id (The value is 0 if there if no downstream reach)*/
	/**** 输入一维数组 ***/
	float* m_surWtrDepth;		/* h栅格单元上的地表水深m*/								/* VAR_SURU from DepressionFS module */
	float* m_chDepth;				/* chp[i][j][3] 栅格单元上的河道深度 全河道一个值*/
	float* m_chWidth;				/* chp[i][j][2] 栅格单元上的河道宽度*/
	float* m_chSinuosity;			/* chp[i][j][6] 栅格单元上的河道弯曲因子 全河道一个值*/
	float* m_dem;						/* e 栅格单元上的高程*/
	float* m_chWtrDepth;			/* hch 栅格单元上的河道水深，初始化时等同于m_surWtrDepth，不从其他模块输入*/
	float* m_Slope;					/* 坡度*/
	/**** 输出一维数组 ***/
	float* m_chQ;						/* dqov 栅格单元的地表径流速率 立方米/s*/
	float* m_ovQ;						/* dqch 栅格单元的河道径流速率 立方米/s*/
	float m_outQ;						/* qout 出水口栅格单元的地表径流速率 立方米/s*/
	float m_outV;						/* vout 出水口栅格单元的地表出流量 立方米*/

	float qoutov;						/* 出水口栅格单元每个时间步长的出流量（来自地表径流） 立方米/s */	
	float sovout;						/* 出水口栅格单元的坡度 */										
	/**** 其他变量 ***/
	bool m_InitialInputs;						/* 是否检测输入数据*/
	map<int, vector<int>> m_rbcellsMap;  /* 存放栅格单元右、下方的单元下标，key栅格单元在一维数组中的下标，value[0]右方栅格的下标(如无右方栅格则是-1)，value[1]下方栅格的下标（如无下方栅格则是-1）*/
	int output_icell;
	int printOvFlowMaxT;
	int printIOvFlowMinT;
	int printChFlowMaxT;
	int printChFlowMinT;
	int** m_RasterNeighbor;		/* 储存栅格位置数据的数组*/
	float ** m_Dqq;
	float * cellWtrDep;
};

#endif /* SEIMS_MODULE_TEMPLATE_H */
