# RasterClass
-------------

+ Raster I/O class based on GDAL and MongoDB (optional).
+ 基于GDAL开源库和NoSQL数据库MongoDB的栅格数据读写类
+ Author: [Liangjun Zhu](http://zhulj.net)

## 0 Build status
RasterClass采用CMake进行跨平台代码管理，理论上，支持任何主流操作系统（Windows、Linux、macOS）和编译器（MSVC、GCC、Clang），可编译生成32位和64位程序（操作系统支持的前提下）。


目前测试环境配置采用`GDAL-1.10.1+`，`mongo-c-driver-1.5.0+`，测试系统及编译器包括：

+ Linux(Unbuntu 14.04) + GCC-4.8: [![Build Status](http://badges.herokuapp.com/travis/lreis2415/RasterClass?branch=master&env=BUILD_NAME=linux_gcc48&label=linux_gcc48)](https://travis-ci.org/lreis2415/RasterClass)
+ macOS 10.12 + Clang: [![Build Status](http://badges.herokuapp.com/travis/lreis2415/RasterClass?branch=master&env=BUILD_NAME=osx_xcode&label=osx_xcode)](https://travis-ci.org/lreis2415/RasterClass)
+ Windows + MSVC 2013 (build x64 version): [![Build status](https://ci.appveyor.com/api/projects/status/k11kcl47ehjco01h/branch/master?svg=true)](https://ci.appveyor.com/project/lreis-2415/rasterclass/branch/master)

Code coverage: [![codecov](https://codecov.io/gh/lreis2415/RasterClass/branch/master/graph/badge.svg)](https://codecov.io/gh/lreis2415/RasterClass)

> 值得一提的是，在64位系统下，可以编译32位或64位程序，此时需要注意对应的GDAL库和mongo-c-driver库都需要编译成64位才可。利用AppVeyor构建的CI测试即全部采用64位编译。

## 1 Introduction
+ RasterClass提供基本栅格数据的读取。
+ RasterClass提供两种栅格数据读取结果：
    + 二维矩阵方式，矩阵中包含`NODATA`值
    + 一维数组，配合一个行列号索引的二维数组使用，该一维数组为栅格数据按行展开，并排除`NODATA`值。
+ RasterClass可单独调试，也可作为其他项目的基础类。

## 2 Install GDAL
### 2.1 Windows
Windows下GDAL最方便的安装方式应该就是采用[Tamas Szekeres's Windows GDAL binaries](http://www.gisinternals.com/release.php)编译发布的版本。

+ 具体安装请参考博客[Installing-gdal-with-python-for-windows](https://sandbox.idre.ucla.edu/sandbox/tutorials/installing-gdal-for-windows "installing-gdal-with-python-for-windows")。

+ 安装完成之后，除了`GDAL_DATA`外，还需要在环境变量里添加一个`GDAL_DIR`，赋值为你的GDAL安装目录，如`C:\GDAL`，以便CMake在编译时能够找得到GDAL依赖库。

+ 为了方便将来程序能够找到GDAL的诸多动态链接库，建议新建`GDAL_PATHS`环境变量`C:\GDAL;C:\GDAL\bin;C:\GDAL\bin\proj\apps;C:\GDAL\bin\gdal\apps;C:\GDAL\bin\ms\apps;C:\GDAL\bin\curl;`，并将`%GDAL_PATHS%`添加至环境变量`PATH`中。

### 2.2 On Linux

> 以CentOS为例 (Take CentOS as example)

+ 源码编译安装GDAL后，新建环境变量`export GDAL_ROOT=<GDAL_root_path>`, 如`export GDAL_ROOT=/soft/share`。

### 2.3 On macOS
macOS下推荐使用[William Kyngesburye](http://www.kyngchaos.com/software:frameworks)维护的Framework安装包，最新的为`GDAL_Complete-1.11.dmg`。

+ GDAL将安装在`/Library/Frameworks/GDAL.framework`。
+ 同时，测试python的GDAL包是否安装成功：

	```python
	>>> import osgeo
	>>> osgeo.__version__
	'1.11.4'
	>>> from osgeo import ogr
	>>> from osgeo import osr
	>>> from osgeo import gdalconst
	>>> from osgeo import gdal_array
	>>> from osgeo import gdal
	```

## 3. 单元测试

+ RasterClass采用CMake进行跨平台编译。
+ RasterClass采用[Google Test](https://github.com/google/googletest)单元测试框架。
+ RasterClass需调用[UtilsClass](https://github.com/lreis2415/UtilsClass)，编译前需将其保存至RasterClass**同级目录**下。
+ 如需添加MongoDB数据库相关操作，RasterClass支持对[MongoUtilClass](https://github.com/lreis2415/MongoUtilClass)的依赖，
同样地，将其保存至RasterClass**同级目录**下，mongo-c-driver的相关配置请参阅
[MongoUtilClass](https://github.com/lreis2415/MongoUtilClass)库的相关帮助。
+ 所有单元测试代码统一存放在`test`文件夹下，并以`Test_XX.cpp`格式命名。
+ 通用编译命令
    ```shell
    cd <path-to-UtilsClass>
    mkdir build
    cd build
    cmake .. -DUNITTEST=1
    make
    ./test/UnitTests_Raster
    ```
+ 强烈推荐CLion，直接打开RasterClass目录并在CMake Options中添加`-DUNITTEST=1`，
即可自动构建工程，方便且跨平台。
