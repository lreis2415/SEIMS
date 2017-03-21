SEIMS 数据预处理
----------------
prepared by Liang-Jun Zhu

First Release: 2015-6-20

Revised date : 2016-5-31

----------------

文档至此，相信你已经[配置好了SEIMS预处理程序](Construct-python-env)（Python及C++），也已经详细了解了[SEIMS模型所需数据](Data-preparation)的准备情况，如果没有，还请前往以上两个链接继续学习！

SEIMS数据预处理，即将原始输入数据、参数等进行初步加工并导入MongoDB数据库，以供SEIMS运行时调用。

下面，介绍如何配置预处理程序完成SEIMS数据预处理任务。

## 1. config.py

SEIMS预处理程序的所有输入均在`SEIMS/preprocess/config.py`中进行设置。

### 1.1 模型相关路径设置

程序的第10~15行或17~22行：

+ BASE_DATA_DIR: 准备数据所在目录，里面应该包含`climate`和`spatial`文件夹及相关数据
+ TXT_DB_DIR: SEIMS文本类数据库目录，如各类查找表
+ PREPROC_SCRIPT_DIR: SEIMS预处理Python脚本目录
+ CPP_PROGRAM_DIR: SEIMS预处理相关C++程序编译目录
+ MPIEXEC_DIR: MPI程序目录，如环境变量里有设置，则此项可为`None`

以下为一个示例：

```python
BASE_DATA_DIR = r'E:\data\model_data\model_dianbu_30m_longterm\data_prepare'
TXT_DB_DIR = r'E:\code\Hydro\SEIMS\database'
PREPROC_SCRIPT_DIR = r'E:\code\Hydro\SEIMS\preprocess'
CPP_PROGRAM_DIR = r'D:\Compile\SEIMS_Preprocess\Release'
MPIEXEC_DIR = None
```

### 1.2 MongoDB相关设置

+ HOSTNAME: MongoDB的IP地址，默认为本机`127.0.0.1`
+ PORT: MongoDB端口号，默认为27017
+ ClimateDBName: 气象数据库名
+ SpatialDBName: 主数据库名，即模型名
+ forCluster: `True` 或 `False`，用于标识是否用于SEIMS_MPI版本，默认为`False`
+ stormMode: 是否用于次暴雨建模，默认为`False`

> `forCluster` 如为`True`，则说明所建数据库是为`SEIMS_MPI`并行版本准备，此时，所有栅格数据均按照子流域编号进行分割后分别存储，如坡度数据在`forCluster`为`False`时，数据库中存储为`1_SLOPE`,而`forCluster`为True时，则存储为`1_SLOPE`, `2_SLOPE`...`N_SLOPE`

以下为一个示例：

```python
HOSTNAME = '127.0.0.1'
PORT = 27017
ClimateDBName = 'climate_dianbu'
SpatialDBName = 'model_dianbu_30m_longterm'
forCluster = False 
stormMode = False
```

### 1.3 气象数据输入

+ HydroClimateVarFile: 气象数据单位定义
+ MetroSiteFile: 气象站点信息
+ PrecSiteFile: 降水站点信息
+ MeteoDailyFile: 气象数据
+ PrecExcelPrefix: 降水数据前缀
+ PrecDataYear: 降水数据年份，为数组，多个年份应该对应多个降水EXCEL文件

以下为一个示例:

```python
HydroClimateVarFile = CLIMATE_DATA_DIR + os.sep + 'Variables.txt'
MetroSiteFile = CLIMATE_DATA_DIR + os.sep + 'Sites_M.txt'
PrecSiteFile = CLIMATE_DATA_DIR + os.sep + 'Sites_P.txt'
MeteoDailyFile = CLIMATE_DATA_DIR + os.sep+ 'meteorology_dianbu_daily.txt'
PrecExcelPrefix = CLIMATE_DATA_DIR + os.sep + 'precipitation_by_day_'
PrecDataYear = [2014]
```

> TODO: 降水数据的输入应改为以站点编号（或站名）命名的文本文件，而非以年份组织的EXCEL

### 1.4 空间数据输入

+ PrecSitesThiessen: 降水站点泰森多边形分布
+ MeteorSitesThiessen: 气象站点泰森多边形分布
+ dem: DEM数据
+ outlet_file:流域出口，可设置为`None` 
+ threshold: 河网提取汇流累积量阈值，默认为0，则采用TauDEM算法[DropAnalysis](http://hydrology.usu.edu/taudem/taudem5/help53/StreamDropAnalysis.html)自动确定
+ np: TauDEM运行时并行计算进程数，默认为4
+ landuseFile: 土地利用数据
+ soilSEQNFile: 土壤类别序列数据
+ soilSEQNText: 土壤类别序列属性查找表

以下为一个示例:

```python
PrecSitesThiessen = SPATIAL_DATA_DIR + os.sep + 'Preci_dianbu_Vor.shp'
MeteorSitesThiessen = SPATIAL_DATA_DIR + os.sep + 'Metero_hefei_Vor.shp'
dem = SPATIAL_DATA_DIR + os.sep + 'dem_30m.tif'
outlet_file = SPATIAL_DATA_DIR + os.sep + 'outlet_30m.shp'
threshold = 0
np = 4  # number of parallel processors for TauDEM
landuseFile = SPATIAL_DATA_DIR + os.sep + 'landuse_30m.tif'
soilSEQNFile = SPATIAL_DATA_DIR + os.sep + 'soil_SEQN.tif'
soilSEQNText = SPATIAL_DATA_DIR + os.sep + 'soil_properties_lookup.txt'
```

## 2. main.py

设置完成`config.py`，便可以运行SEIMS预处理的主函数`main.py`了。

> 建议运行时，逐行运行，以便程序出错时，错误检查。

## 3. MongoDB数据检查

成功运行主函数之后，SEIMS程序所需的所有准备数据均已成功导入MongoDB，如下图所示：


![](http://zhulj-blog.oss-cn-beijing.aliyuncs.com/seims-img%2Fdatabases.png)


## 4. 预处理子程序调用

多数情况下，模型的预处理不可能一次成功，往往需要多次修改，而并非每次修改都需要运行整个预处理流程，因此，SEIMS提供了预处理过程中的子程序的调用。

基本调用命令与之前一致，均为：

```python
> python XXX.py -ini <configurationFile>.ini
```

### 4.1 MeteorologicalDaily.py
### 4.2 PrecipitationDaily.py
### 4.3 import_measurement.py
### 4.4 import_parameters.py
### 4.5 import_bmp_scenario.py

