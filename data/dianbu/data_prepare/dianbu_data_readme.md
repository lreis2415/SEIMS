## 店埠河小流域SEIMS模型准备数据

> 以下教程中文件路径可能包含了Windows服务器（6.57）和Linux服务器（6.55）

### 1 气候数据

#### 1.1 数据来源

##### 1.1.1 降水数据
降水数据来源于[安徽省水文遥测信息网](http://yc.wswj.net/ahyc/ "安徽省水文遥测信息网")，利用Python实现自动下载功能，[点此查看源代码](https://github.com/crazyzlj/Python/blob/master/HydroDataDownload/anhui_precipitation_download.py "python-download-anhui-rainfall")。

研究区周围共有5个站点，分别是：管湾'62915310', 胡岗'62942747', 众兴'62903180', 群治'62942717', 肥东'62922800'，数据时间为2013-2015年，分辨率为1天。

##### 1.1.2 气象数据
由于研究区周围仅有一个国家气象站点，合肥站（58321），因此采用这一个站点的日值观测数据。数据来源为[中国气象数据网](http://data.cma.cn/ "中国气象数据网")的中国地面气候资料日值数据集(V3.0)产品，[点此查看数据下载方法](http://zhulj.net/python/2016/04/11/Constructing-SURF_CLI_CHN_MUL_DAY_V3.0-database.html)。

2015年12月资料没有更新。暂时用2014年12月的替代。

对于日尺度模型的构建，所需气象数据如下：
+ 气象数据(**日值**)
	+ StationID: 必需，站点编号(整型)
	+ Y, M, D  : 必需，年/月/日
	+ TMAX     : 必需，最高气温(deg C)
	+ TMIN     : 必需，最低气温(deg C)
	+ TMEAN    : 可选，平均气温(deg C)，缺失情况下，由(TMAX+TMIN)/2计算
	+ RM       : 必需，相对湿度(%)
	+ WS       : 必需，风速(m/s)
	+ SR/SSD   : 二选一，太阳辐射(MJ/m2/day)/日照时数(h)
	+ PET      : 可选，蒸散发(mm)

##### 1.2 数据整理

+ 降水站点信息文件
	+ `/home/zhulj/models/dianbu/data_prepare/climate/Sites_P.txt`
	+ 站点信息包括 站点编号(整数型)、站点名称(字符串)、投影系X坐标及Y坐标、经纬度、以及高程，如下表所示，站点信息保存为txt格式，如:

|StationID|Name|LocalX|LocalY|Lon|Lat|Elevation|
|----|----|----|----|----|----|----|
|62915310|GuanWan|39548201.84|3543440.687|117.525278|32.025556|43|
|...|...|...|...|...|...|...|

+ 气象站点信息文件

	+ 格式与降水站点一致，`/home/zhulj/models/dianbu/data_prepare/climate/Sites_M.txt`

|StationID|Name|LocalX|LocalY|Lon|Lat|Elevation|
|----|----|----|----|----|----|----|
|58321|hefei|39535576.056|3536052.03009|117.23|31.87|20|

+ 降水日值数据文件
	+ 降水数据采用文本文件组织，基本格式如下：
	+ `/home/zhulj/models/dianbu/data_prepare/climate/precipitation_dianbu_daily.txt`
	
|Y|M|D|StationID1|StationID2|...|
|----|----|----|----|----|----|
|2014|1|1|0|0.5|...|

+ 气象日值数据文件
	+ 气象数据采用文本文件组织，基本格式如下：
	+ 没有数据的属性列不写，但是必需属性需准备
	+ `/home/zhulj/models/dianbu/data_prepare/climate/meteorology_dianbu_daily.txt`

|StationID|Y|M|D|TMEAN|TMAX|TMIN|RM|WS|SSD|...|
|----|----|----|----|----|----|----|----|----|----|----|
|58321|2010|1|1|3.5|8.6|-0.8|69|1.9|4.7|...|


### 2 空间数据
#### 2.1 土壤

+ soil_SEQN_30m.tif，肥东土肥站绘制土壤类型图，野外采样得到土壤属性数据

#### 2.2 土地利用
+ landuse_30m.tif


### 3 建模过程

#### 3.1 子流域划分

+ 设置`mgtFieldFile = None（line 42 in *.ini file）`
+ 运行程序`python G:\code_zhulj\SEIMS\preprocess\subbasin_delineation.py -ini G:\code_zhulj\SEIMS\preprocess\dianbu_30m_longterm_omp_zhulj_winserver.ini`进行子流域划分，结合DEM分析，最终确定一个合适的汇流累积量阈值，即`D8AccThreshold（line 47 in *.ini file）`。
+ 运行程序`python G:\code_zhulj\SEIMS\preprocess\parameters_extraction.py -ini G:\code_zhulj\SEIMS\preprocess\dianbu_30m_longterm_omp_zhulj_winserver.ini`

#### 3.2 管理单元划分

+ 利用流向、土地利用、子流域等进行管理单元划分，目前可以利用吴辉博士开发的考虑上下游关系地块划分方法，即预处理程序中的`fieldpartition`。

+ 详细使用说明参见[这里](https://github.com/lreis2415/FieldPartition)。

+ 地块划分结果为一个地块栅格文件，用于设置*.ini文件中的mgtFieldFile，以及一个txt文件，文件中记录了每个地块编号、下游地块编号以及主要土地利用类型等。

#### 3.3 管理措施情景设置

#### 3.3.1 输入文件组织

+ BMP_index.txt：SEIMS模型支持的BMP类型编号 （目前多数尚未实现）
+ BMP_scenarios.txt：情景分析设置的主文件，包含以下属性列：
	+ ID：唯一标识情景ID
	+ NAME：情景名称
	+ BMPID：BMP类型，如12为作物管理，1为点源污染
	+ SUBSCENARIO：子情景唯一标识ID，某一BMP类型可能存在多个子情景，如作物管理下可分为0传统施肥方式和1生态种植技术
	+ DISTRIBUTION：定义该BMP的分布，多个字段用‘|’分割，第一个可为RASTER和ARRAY，分别意为栅格数据和列表；如为栅格数据，则第二个为栅格名（如MGT_FIELDS），如果为列表，第二个则为MongoDB表明，其中存储点状要素列表（如point_source_distribution）；如果为列表，第三个数为点状要素类别（如10000为养牛厂）
	+ COLLECTION：管理措施参数表，如点源污染措施表为point_source_management
	+ LOCATION：用于定义该措施实施位置，如果DISTRIBUTION中为RASTER，则LOCATION定义栅格值，多个值之间用‘，’分割，如“1,3,4”，如果作用于整个流域，则输入“ALL”；同理，如果DISTRIBUTION为ARRAY，LOCATION中输入COLLECTION中定义的点编号，如“20001,20003”等
+ point_source_distribution.txt：如上所述，定义点源污染位置，包括
	+ PTSRC：点源类型，包括养牛厂（10000），养猪厂（20000），上游水库放水点（30000），居民生活污水排放口（40000）
	+ PTSRCID：具体点源编号，支持1~9999
	+ Name：点源名称
	+ Lon，Lat：经纬度
	+ LocalX，LocalY：投影坐标值，用于判断该点源所在子流域
	+ Size：规模，用于计算排放总量，如10001养牛厂养殖300头牛
+ point_source_management.txt：点源管理，用于确定排放量及排放日期等，详见该文件头中注释（#开头）
+ areal_source_distribution.txt 与 areal_source_management.txt 同上，不再赘述。
+ plant_management.txt：作物管理措施，根据积温或者种植日期，管理作物生长，输入参数借鉴SWAT。

#### 3.3.2 管理措施数据

> 有关管理措施更新之后，可以用`python import_bmp_scenario.py -ini *.ini`程序重新导入数据库。


|ScenarioID|基本介绍|
|----|----|
|0|基准情景（base_scenario）：养殖场不采取措施、农田采用传统施肥、生活污水直接排放|
|1|工程示范情景（demo_scenario）：工程示范猪场和牛场（固体+液体措施）、农田生态种植、生活污水治理|
|2|全流域治理情景（ideal_scenario）：流域内所有猪场和牛场均采用工程措施、所有农田均为生态种植、所有生活污水排放点均治理|
|3|全流域养殖场治理情景（animal_scenario）：流域内所有猪场和牛场采用工程措施|
|4|全流域农田生态种植情景（crop_scenario）:流域内所有农田采用生态种植技术|
|5|全流域生活污水治理情景（sewage_scenario）：流域内的生活污水排放口进行治理|


千柳公园西污水工程
COD  261.39 ==> 39.21
TN   41.61  ==> 8.78
TP   2.51   ==> 0.73
100 t/day

牌坊中学
COD	128.71	33.98
TN	29.08	3.73
TP	2.02	0.51
100 t/day

三王村
COD	108.64	28.34
TN	37.16	4.70
TP	2.46	0.31

60 t/day

表5-18  生活垃圾示范工程对污染物削减贡献
污染物	千柳公园西/吨	牌坊中学东/吨	三王村/吨	生活垃圾/吨	年削减总量/吨	合同指标/吨	达标率/%
COD	8.11	3.46	2.93	147.17	161.67	100	100
TN	1.20	0.92	1.18	20.60	23.9	10	100
TP	0.07	0.06	0.08	4.12	4.33	1	100




