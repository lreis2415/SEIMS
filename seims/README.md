# 代码组织结构
----------------------

SEIMS模型主要编程语言为C++和Python，许多代码如对栅格文件的读写等可抽象出来进行独立维护、扩充，因此采用了Git的子模块（submodule）方式进行组织管理。

采用CMAKE进行C++代码管理，以支持Windows、Linux、macOS等平台。

## 0. database

模型默认参数设置、土地利用等查找表。

## 1. src

包含SEIMS模型相关的所有C++程序。

### 1.1 commonlibs

通用算法库，是SEIMS模型的基础操作类库，均为子模块（submodule），可独立维护，具体包括：

子模块名|简介
---|---
[UtilsClass](https://github.com/lreis2415/UtilsClass)|支持跨平台的时间、字符串、文件IO等基础操作类
[RasterClass](https://github.com/lreis2415/RasterClass)|基于GDAL的栅格数据读写类
[MongoUtilClass](https://github.com/lreis2415/MongoUtilClass)|MongoDB操作类

### 1.2 taudem

TauDEM为Tarboton教授开发的基于格网DEM进行水文、地形信息提取的工具包。

### 1.3 grid_layering

由刘军志博士开发的用于水文模型并行计算的栅格分层算法，支持D8、Dinf算法。

## 1.4 field_partition

由吴辉博士开发的考虑坡面上下游关系的地块划分算法。

## 1.5 iuh

由吴辉博士改写自刘永波老师的WetSpa模型中的瞬时地貌单位线算法。

## 1.6 import_raster

将栅格数据导入mongodb数据库。

## 1.7 metis

根据子流域划分计算任务，用于MPI版SEIMS模型。

## 1.8 seims_main

SEIMS模型主程序。

TODO

## 2. pygeoc

Git子模块（submodule）[pygeoc](https://github.com/lreis2415/PyGeoC)为SEIMS中python计算脚本（即preprocess、postprocess、scenario_analysis等）的基础，其包括水文、栅格、矢量、通用等功能模块。

详细介绍请参见[PyGeoC的Wiki](https://github.com/lreis2415/PyGeoC/wiki)。

## 3. preprocess

模型数据预处理程序，包括气象数据导入、子流域划分、土地利用及土壤参数提取、栅格分层等操作，预处理结束后，模型所需数据均导入mongodb数据库中。

## 4. postprocess

模型运行后处理程序，包括生成水文过程图等。

## 5. scenario_analysis

基于NSAG-II优化算法的情景优化分析。

## 6. calibration

TODO，模型率定。


