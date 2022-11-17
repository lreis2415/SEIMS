/*!
 * \file template.h
 * \brief Brief description of this module
 *        Detail description about the implementation.
 * \author Liangjun Zhu
 * \date 2018-02-07
 *
 * Changelog:
 *   - 1. 2018-02-07 - lj - Initial implementaition
 *   - 2. 2019-01-30 - lj - Add (or update) all available APIs
 */
#ifndef SEIMS_MODULE_TEMPLATE_H
#define SEIMS_MODULE_TEMPLATE_H

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

	//void SetRasterRows(int rows) OVERRIDE;

	//void SetRasterCols(int cols) OVERRIDE;

	//void SetReachDepthData( FloatRaster* positions) OVERRIDE;

	void OvrlDepth();

	void ChannDepth();

	void OvrlRout();

	void ovrl(int icell, int rbCell);

	void ChannRout();

	void chnchn(int reachIndex, int curReachId, int nReaches, int iCell, vector<int> vecCells);

	void RoutOutlet();

	float newChnDepth(float wch, float dch, float sfactor,
		int idCell, float addedVolume);

	float chnDischarge(float hchan, float hh, float wch, float dch,
		float stordep, float rmanch, float a, float sf, float sfactor);


private:
    int m_nCells; ///< valid cells number
	int m_maxSoilLyrs;
	float* m_nSoilLyrs;
	float** m_ks;
	float* m_soilWtrStoPrfl;

	//! subbasin number
	int m_nSubbsns;
	//! current subbasin ID, 0 for the entire watershed
	int m_inputSubbsnID;
	//! subbasin IDs
	vector<int> m_subbasinIDs;
	//! All subbasins information
	clsSubbasins* m_subbasinsInfo;

	/// id of source cells of reaches
	/// 河道的
	int *m_sourceCellIds;
	/**
	*	@brief 2d array of flow in cells
	*
	*	The first element in each sub-array is the number of flow in cells in this sub-array
	*   存储每个栅格单元的入流单元
	*   例如:
	*          [[2, 100,200],[3,101,201,202]]
	*          表示第一个栅格单元的入流单元有2个，id分别为100和200；第二个栅格单元的入流单元有3个，id分别为101，201，202
	*/
	float **m_flowInIndex;

	/// map from subbasin id to index of the array
	map<int, int> m_idToIndex;
	//! reach depth data from SPATIAL collection
	//FloatRaster* m_reachDepth;
	//float* m_chDepth;

	/**** problems ***/
	/*
	1. 在这里降雨使用了下渗模块输出的VAR_EXCP，在casc_2d中m_rint却是只扣除了植被截留的降雨，
		之后才用infil()函数从径流深度中扣除了下渗深度，这里必须设法保持一致，如要使用VAR_EXCP，
		就必须删除infil()函数，并修改h的赋值逻辑；
	2. 在casc2d中，h初始化时等于sdep，之后每个时间步长根据降雨-截留-下渗进行更新；在seims中，
		下渗模块每个时间步长都会输出地表径流深度，这按理说应该是h？此时可以不再需要sdep这个变量，
		而直接使用每个时间步长由下渗模块输出的地表径流
	*/
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
	float* m_chDownStream;/// downstream id (The value is 0 if there if no downstream reach)
	/**** 输入一维数组 ***/
	//float* m_exsPcp;				/* m_rint栅格单元上的过载降雨强度 mm/s */		/* VAR_EXCP from MUR_MR module*/
	float* m_surWtrDepth;		/* h栅格单元上的地表水深m*/								/* VAR_SURU from DepressionFS module */
	float* m_chDepth;				/* chp[i][j][3] 栅格单元上的河道深度 全河道一个值*/
	float* m_chWidth;				/* chp[i][j][2] 栅格单元上的河道宽度*/
	float* m_chSinuosity;			/* chp[i][j][6] 栅格单元上的河道弯曲因子 全河道一个值*/
	float* m_dem;						/* e 栅格单元上的高程*/
	float* m_chWtrDepth;			/* hch 栅格单元上的河道水深，初始化时等同于m_surWtrDepth，不从其他模块输入*/
	//float *m_chManning;		/* chp[i][j][3] 栅格单元上的河道深度*/
	//float *m_chSSlope;			/* chp[i][j][3] 栅格单元上的河道深度*/
	float* m_Slope;					/* 坡度*/
	/**** 输出一维数组 ***/
	float* m_chQ;						/* dqov 栅格单元的地表径流速率 立方米/s*/				
	float m_outQ;						/* qout 出水口栅格单元的地表径流速率 立方米/s*/
	float m_outV;						/* vout 出水口栅格单元的地表出流量 立方米*/
	/**** 其他变量 ***/
	bool m_InitialInputs = true;				/* 是否检测输入数据*/

	/**** OverDepth.c使用 ***/
	int m, n;						/* 集水区的行列数 */
	float** m_rint;				/* m_rint[j][k],栅格单元上的过载降雨强度（扣除截留和下渗） mm/s */		/* VAR_EXCP*/
	float dt;						/* 时间步长 */																		/* DT_HS 或 DT_CH*/
	float** dqov;				/* dqov[j][k] 栅格单元的地表径流速率 立方米/s*/						/* VAR_CH_V*/
	float w;							/* 栅格单元的宽度*/																/* Tag_CellWidth*/


	/**** OverRout.c使用 ***/
	float** h;								/* overland depth, 即地表水深*/									/* 从填洼模块接收*/
	int** iman;							/* iman[i][j] 栅格单元上对应的土地利用类型*/				/*VAR_MANNING*/
	int** link;								/* link[i][j] 栅格单元上的河道编号*/								/* VAR_STREAM_LINK*/
	int** node;							/* link[i][j] 栅格单元上的节点编号*/								/* 不需要*/
	float* pman;							/* pman[l] 土地利用类型l对应的曼宁系数*/					/*VAR_MANNING*/
	float** sdep;						/* sdep[i][j] 栅格单元上的初始蓄水深度*/						/* 不需要*/
	float** e;								/* e[i][j] 栅格单元上对应的高程值*/								/* VAR_DEM*/
	float chp[50][100][7];			/* chp[i][j][k] 在link i,node j上的k类型属性值，K= 1: channel type, 2: width, 3: depth, 4: side slope, 5: Manning's 'n', 6: sinuosity*/    /*REACH_SLOPE  GetReachesSingleProperty(REACH_SLOPE, &m_chSlope);*/
	float** hch;							/* 河道水流深度 */														/* CH_DEPTH*/


	/*ChannDepth使用*/
	int maxlink;							/* 河道数量*/																		/*参考 CH_DW 模块中DiffusiveWave::Execute()中对河道的遍历nReaches*/
	int *nchan_node;					/* 河道中的节点数量*/														/*参考 CH_DW 模块中DiffusiveWave::Execute()中对节点的遍历vecCells.size() */
	int ichn[50][100][3];				/* ichn[i][j][k]河道i、节点j的行列号, k = 1行号, k = 2列号*/	/*参考 CH_DW 模块中DiffusiveWave::Execute()中对节点id的取值vecCells[iCell] */
	float **dqch;						/* 时间步长内栅格单元的河道径流速率 立方米 / s*/				/* VAR_CH_V*/


	/*RoutOutlet使用*/
	int jout;								/* 出水口的行号/*														/*根据VAR_ID_OUTLET推算*/
	int kout;								/* 出水口的列号/*														/*根据VAR_ID_OUTLET推算*/

	float qoutov;						/* 出水口栅格单元每个时间步长的出流量（来自地表径流） 立方米/s */	/*	内部变量*/
	float sovout;						/* 出水口栅格单元的坡度 */										/*自定义*/					
	float wchout;						/* 出水口栅格单元的河道宽度 */								/*自定义*/		
	float dchout;						/* 出水口栅格单元的河道深度 */								/*自定义*/		
	float rmanout;						/* 出水口栅格单元的曼宁系数 */								/*自定义*/		
	float sout;							/* 出水口栅格单元的河床坡度 */								/*自定义*/		
	float sfactorout;				
	float qout;							/* 出水口栅格单元每个时间步长的出流量  立方米/s */	/*VAR_QRECH*/
	float *q;								/* 用户自定义站点的出流速率*/								/*自定义*/

};

#endif /* SEIMS_MODULE_TEMPLATE_H */
