# RasterClass
-------------

+ Raster I/O class based on GDAL and MongoDB (optional).
+ 基于GDAL开源库和NoSQL数据库MongoDB的栅格数据读写类
+ Author: [Liangjun Zhu](http://zhulj.net)

## 0 Build status
RasterClass采用CMake进行跨平台代码管理，理论上，支持任何主流操作系统（Windows、Linux、macOS）和编译器（MSVC、GCC、Clang），可编译生成32位和64位程序（操作系统支持的前提下）。

目前测试环境配置采用`GDAL-1.10.1+`，`mongo-c-driver-1.5.0+`，测试系统及编译器包括：

+ Linux(Unbuntu 14.04) + GCC-4.8: [![Build Status](http://badges.herokuapp.com/travis/lreis2415/RasterClass?branch=master&env=BUILD_NAME=linux_gcc48&label=linux_gcc48)](https://travis-ci.org/lreis2415/RasterClass)
+ macOS 10.12 + Clang: [![Build Status](http://badges.herokuapp.com/travis/lreis2415/RasterClass?branch=master&env=BUILD_NAME=osx_xcode&label=osx_code)](https://travis-ci.org/lreis2415/RasterClass)
+ Windows + MSVC 2015 (build x64 version): [![Build status](https://ci.appveyor.com/api/projects/status/xa5c17um0kv4yc4i/branch/master?svg=true)](https://ci.appveyor.com/project/crazyzlj/rasterclass/branch/master)

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

+ 安装完成之后，除了`GDAL_DATA`和`GDAL_DRIVER_PATH`外，还需要在环境变量里添加一个`GDAL_DIR`，赋值为你的GDAL安装目录，如`C:\GDAL`，以便CMake在编译时能够找得到GDAL依赖库。

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

## 3. 单独测试RasterClass模块

+ RasterClass采用CMake进行跨平台编译。
+ RasterClass需调用[UtilsClass](https://github.com/lreis2415/UtilsClass)，编译前需将其保存至RasterClass**同级目录**下。
+ 如需添加MongoDB数据库相关操作，RasterClass支持对[MongoUtilClass](https://github.com/lreis2415/MongoUtilClass)的依赖，同样地，将其保存至RasterClass**同级目录**下，mongo-c-driver的相关配置请参阅[MongoUtilClass](https://github.com/lreis2415/MongoUtilClass)库的相关帮助。

### 3.1 Windows
+ 打开 “开始” -> Microsoft Visual Studio 2010 -> Visual Studio Tools -> Visual Studio 命令提示(2010)，以**管理员方式**运行，依次输入以下命令：

	```shell
	cd <path-to-RasterClass>
	mkdir build
	cd build
	### 仅编译安装 ###
	cmake -G "NMake Makefiles" <path-to-RasterClass> -DCMAKE_BUILD_TYPE=Release
	nmake
	nmake install
	### 编译Microsoft Visual Studio工程 ###
	cmake <path-to-RasterClass>
	nmake
	```

+ 对于“仅编译安装”操作，`RasterClass.exe`会自动安装在`<path-to-RasterClass>\bin`目录下。
+ 对于“编译Microsoft Visual Studio工程”，`RasterClass.sln`将保存在`<path-to-RasterClass>\build`目录下。

### 3.2 Unix
对于Linux和macOS系统而言，操作与Windows类似，这里不再赘述。
