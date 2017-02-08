# RasterClass
==================

+ Raster I/O class based on GDAL and MongoDB (optional).
+ 基于GDAL开源库和NoSQL数据库MongoDB的栅格数据读写类
+ Author: [Liangjun Zhu](http://zhulj.net)

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
