# SEIMS 代码框架
----------------------

SEIMS模型主要编程语言为C++和Python，C++代码编译采用CMake管理，以支持Windows、Linux、macOS等平台。

地学计算中有很多通用的操作，比如，如对栅格文件的读写、对数据库的读写、对基本对象（字符串、文件及文件夹等）的操作。因此将这些通用的操作部分进行独立维护、扩充，采用Git的subtree功能进行组织管理独立子库，独立子库包括：
+ 1.[PyGeoC: 用于地学计算的简单Python包](https://github.com/lreis2415/PyGeoC)，包括hydro, raster, vector, utils等子模块
+ 2.用于地学计算的C++基础类，[CCGL (Common Cross-platform Geographic-computing Library)](https://github.com/crazyzlj/CCGL)

此外，用于地形参数提取、子流域划分的[TauDEM](https://github.com/lreis2415/TauDEM)和用于子流域层次的任务划分的[METIS](https://github.com/lreis2415/metis)也均采用独立子库管理。

-----------------------------

下图为SEIMS模型总体结构图：

![seims_structure](http://i.imgur.com/uYNK0po.jpg)


## 1. doc

包括代码文档自动生成脚本（基于doxygen），代码格式规范（style），以及中英文帮助文档（en/doc, zh-ch/doc）。

## 2. data

示例数据。

## 3. seims

模型源代码。

### 3.1. src

+ src文件夹中包含SEIMS模型所有C++源码，经过编译、安装之后，所有可执行文件和动态链接库均位于`bin`文件夹内，如TauDEM、metis、grid_layering（用于水文模型并行计算的栅格分层算法，支持D8、Dinf算法）、iuh（WetSpa模型中的瞬时地貌单位线算法）等。

+ 将来需要新增的模块功能代码也在此添加。

+ 预处理（preprocess）和运行模型（`run_seims.py`）阶段将调用`bin`目录下的相关文件。

### 3.2. preprocess

模型数据预处理程序，包括气象数据导入、子流域划分、土地利用及土壤参数提取、栅格分层等操作，预处理结束后，模型所需数据均导入mongodb数据库中。

此外还包括了模型默认参数设置、土地利用和土壤等属性查找表（preprocess/database）。

### 3.3. postprocess

模型运行后处理程序，包括生成水文过程图等。

### 3.4. parameters_sensitivity

参数敏感性分析。

### 3.5. calibration

基于NSAG-II优化算法的模型率定。

### 3.6. scenario_analysis

基于NSAG-II优化算法的情景优化分析。
