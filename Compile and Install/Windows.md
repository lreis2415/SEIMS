Install SEIMS on Windows
----------------------------

First release: Huiran Gao

Reviewed and updated: Liang-Jun Zhu

Latest Updated：Dec.16, 2016

--------------------------
# 目录

[**1. Prerequisites**](#1-prerequisites)

  1.1. [Microsoft Visual Studio](#11-microsoft-visual-studio)

  1.2. [CMake](#12-cmake)

  1.3. [GDAL and Python](#13-gdal-and-python)

  1.4. [mongo-c-driver](#14-mongo-c-driver)

  1.5. [MS-MPI](#15-ms-mpi)

[**# 2. Compilation and Installation**](#2-compilation-and-installation)

  2.1. [Installation for users](#21-installation-for-users)

  2.2. [Installation for developers](#22-installation-for-developers)

2. [预处理程序中C++程序的编译](#ii-预处理程序中c++程序的编译)

[**3. SEIMS主程序的编译**](#3-seims主程序的编译)

1. [SEIMS源码结构](#i-seims源码结构)

2. [Windows下SEIMS主程序的编译](#ii-windows下seims主程序的编译)

# 1. Prerequisites

SEIMS模型采用C++和Python编写，支持子流域-栅格双层并行计算，在Windows下推荐采用Microsoft Visual Studio进行编译安装，安装前的准备包括：
+ Microsoft Visual Studio 2010 或更高 -- C++源码编译器及IDE
+ CMake -- 管理代码编译
+ GDAL 1.x with Python -- 矢栅数据读写库
+ mongo-c-driver 1.3.5 或更高 --
+ MS-MPI v6 （或更高）-- 编译MPI并行程序，如TauDEM、SEIMS-MPI版本等

> Note: SEIMS目前只提供32-bit版本编译帮助，因此，接下来GDAL的安装、mongo-c-driver的编译均指的是32-bit版本。

## 1.1 Microsoft Visual Studio

+ **For User**： 如果只是希望源码编译SEIMS模型，而不想安装臃肿庞大的VS，可以选择使用Microsoft Visual C++ Build Tools，目前最新版本为[2015 Update 3](https://www.visualstudio.com/downloads/#microsoft-visual-c-build-tools-2015-update-3 "microsoft-visual-c-build-tools-2015-update-3").
+ **For Developer**： 如果希望对SEIMS模型进行改进，建议采用[Microsoft Visual Studio 2010 或更高版本](https://www.visualstudio.com/downloads/ "visual-studio-downloads")进行开发。

> Note: 虽然SEIMS编译理论上支持更高版本的VS，但是为了接下来的GDAL库安装方便，建议使用与[Tamas Szekeres's Windows GDAL binaries](http://www.gisinternals.com/release.php)发布版本对应的VS，比如目前其发布的GDAL安装版本对应的最高VS版本为2013。当然，如果你希望自己源码编译接下来用到的GDAL、mongo-c-driver库，那VS版本选择则不受限制。

## 2. CMake

CMake是一个跨平台的安装或编译工具，可以用简单的语句来描述安装或编译过程。CMake通过`CmakeLists.txt`文件能够输出各种各样的makefile或者IDE工程。

CMake可以从其[官网免费下载](http://www.cmake.org/files)，推荐安装3.0以上版本，安装后添加CMake路径如`C:\Program Files (x86)\CMake`到系统环境变量`PATH`里。

## 3. GDAL and Python

SEIMS的矢栅数据读写基于`GDAL 1.x`编写，Windows下GDAL最方便的安装方式应该就是采用[Tamas Szekeres's Windows GDAL binaries](http://www.gisinternals.com/release.php)编译发布的版本。

+ 具体安装请参考博客[Installing-gdal-with-python-for-windows](https://sandbox.idre.ucla.edu/sandbox/tutorials/installing-gdal-for-windows "installing-gdal-with-python-for-windows")。

+ 安装完成之后，除了`GDAL_DATA`和`GDAL_DRIVER_PATH`外，还需要在环境变量里添加一个`GDAL_DIR`，赋值为你的GDAL安装目录，如`C:\GDAL`，以便CMake在编译时能够找得到GDAL依赖库。

+ 为了方便将来程序能够找到GDAL的诸多动态链接库，建议新建`GDAL_PATHS`环境变量`C:\GDAL;C:\GDAL\bin;C:\GDAL\bin\proj\apps;C:\GDAL\bin\gdal\apps;C:\GDAL\bin\ms\apps;C:\GDAL\bin\curl;`，并将`%GDAL_PATHS%`添加至环境变量`PATH`中。

## 4. mongo-c-driver

SEIMS数据管理采用NoSQL型数据库——MongoDB，依赖于mongo-c-driver。
Windows下的配置步骤为：
+ 从[官网](http://mongoc.org/ "mongo-c-driver-download")下载源码压缩包，目前最新稳定版本为[1.5.0](https://github.com/mongodb/mongo-c-driver/releases/download/1.5.0/mongo-c-driver-1.5.0.tar.gz "mongo-c-driver-1.5.0")，解压缩至当前文件夹，如`C:\z_code\Repos\mongo-c-driver-1.5.0`
+ 打开cmd，依次输入如下命令，默认的安装目录为`C:\mongo-c-driver`
```bat
cd C:\
mkdir mongo-c-driver
cd C:\z_code\Repos\mongo-c-driver-1.5.0
cd src\libbson
cmake -DCMAKE_INSTALL_PREFIX=C:\mongo-c-driver -G "Visual Studio 10 2010"
msbuild.exe ALL_BUILD.vcxproj
msbuild.exe INSTALL.vcxproj
cd ..\..
cmake -DCMAKE_INSTALL_PREFIX=C:\mongo-c-driver -DBSON_ROOT_DIR=C:\mongo-c-driver -G "Visual Studio 10 2010"
msbuild.exe ALL_BUILD.vcxproj
msbuild.exe INSTALL.vcxproj
```

+ 至此，`mongo-c-driver`即编译安装完成了，在`C:\mongo-c-driver`目录下能看到`bin`, `include`, `lib`文件夹。
+ 将`C:\mongo-c-driver`添加至环境变量，命名为`MONGOC_ROOT_DIR`。
+ 将`C:\mongo-c-driver\bin`添加至环境变量，命名为`MONGOC_LIB_DIR`，并将`%MONGOC_LIB_DIR%`添加至环境变量`PATH`中。

> Note: 如果cmd提示找不到msbuild.exe，可以在msbuild.exe前加上绝对路径，这个文件是.NetFramework里的，比如`C:\Windows\Microsoft.NET\Framework64\v4.0.30319\msbuild.exe`

## 5. MS-MPI

Microsoft MPI (MS-MPI) 是微软基于MPICH实现的用于Windows平台开发和运行并行应用程序的消息传递接口标准，预处理程序中用于提取子流域和地形信息的TauDEM、SEIMS_MPI并行版本等都需要MPI的支持。

+ 从[MS-MPI v6](https://www.microsoft.com/en-us/download/details.aspx?id=47259)下载并分别安装msmpisdk.msi, MSMpiSetup.exe，并检查系统环境变量是否配置正确，如Windows-64bit版：

```
MSMPI_BIN=C:\Program Files\Microsoft MPI\Bin\
MSMPI_INC=C:\Program Files (x86)\Microsoft SDKs\MPI\Include\
MSMPI_LIB32=C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x86\
MSMPI_LIB64=C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64\
```

> 建议采用默认安装路径安装MPI

# 2. Compilation and Installation

## 2.1 Installation for users

> 注意：由于目前SEIMS源码里自带的GDAL为32位编译版本，因此，目前只允许编译为32位预处理程序及SEIMS程序，后续会考虑同时加入64位版GDAL
>
> 因此如果是Windows x64电脑，不要打开的是Visual Studio x64 Win64 命令提示(2010)，而是需要打开Visual Studio 命令提示(2010)

+ 打开 “开始” -> Microsoft Visual Studio 2010 -> Visual Studio Tools -> Visual Studio 命令提示(2010)，以**管理员方式**运行，依次输入以下命令：

```shell
cd C:\z_code\Hydro\SEIMS2017\seims
mkdir build
cd build
cmake -G "NMake Makefiles" C:\z_code\Hydro\SEIMS2017\seims -DCMAKE_BUILD_TYPE=Release
nmake
nmake install
```

完成后，不需要打开VS2010生成解决方案，可执行文件已经编译完成。此时，预处理程序目录为`< build >`，用于任务划分的METIS程序目录为`< build >\metis\programs`。
## 2.2 Installation for developers

在数据预处理之前，首先需要编译一下数据预处理需要的C++程序，使用CMake进行编译。

C++程序包括：

+ `preprocess/cpp_src`
+ `preprocess/cpp_src/metis-5.1.0-pk`

编译方式有两种：

+ 一是CMake生成VS2010工程文件，然后在VS2010中将工程编译链接为可执行程序
+ 二是直接利用CMake生成可执行文件 （推荐采用）。

**第一种方式编译步骤**：
+ 在SEIMS源代码外新建一个build文件夹，比如`D:\SEIMS_model\SEIMS_preprocessing\build`；
+ 打开 “开始” -> Microsoft Visual Studio 2010 -> Visual Studio Tools -> Visual Studio 命令提示(2010)，以**管理员方式**运行，依次输入以下命令：

> 注意：由于目前SEIMS源码里自带的GDAL为32位编译版本，因此，目前只允许编译为32位预处理程序及SEIMS程序，后续会考虑同时加入64位版GDAL
>
> 因此如果是Windows x64电脑，不要打开的是Visual Studio x64 Win64 命令提示(2010)，而是需要打开Visual Studio 命令提示(2010)

```shell
cd D:\SEIMS_model\SEIMS_preprocessing\build
D:
cmake <SEIMS Folder>/preprocess
mkdir metis
cd .\metis
cmake <SEIMS Folder>/preprocess/cpp_src/metis-5.1.0-pk
```

+ 然后用VS2010打开< build >文件夹下的SEIMS_Preprocess.sln，并生成解决方案，此时，预处理程序目录为`< build >\Debug`，或者`< build >\Release`

打开<build>\metis文件夹下的METIS.sln，同样生成解决方案，此时，用于子流域任务划分的METIS程序目录为`< build >\metis\programs\Debug`，或者`< build >\metis\programs\Release`

**第二种方式编译步骤**：




## ii. MongoDB及MongoVUE
### MongoDB
[MongoDB](https://www.mongodb.com)是一个C++语言编写的基于分布式文件存储的数据库，将数据存储为一个文档。

MongoDB安装比较简单,有两种方法：一是下载压缩包文件，解压使用；二是下载msi文件，安装使用。

下载路径：[https://www.mongodb.com/download-center#community](https://www.mongodb.com/download-center#community)

### MongoVUE
[MongoVUE](http://www.mongovue.com/)是一款比较好用的MongoDB客户端工具，可以提供一个简洁可用的MongoDB管理界面。
使用方法：
+ [下载MongoVUE](http://pan.baidu.com/s/1c2M9xPu)（提取码：3vhl），安装完成后点击桌面MongoVUE的图标，进入工具的主界面；
+ 进入工具主界面后，在左上角有一个connect连接的一个选项，点击**connect**；
+ 如果配置列表里面没有想要连接的数据库那么需要先经过配置，点击“+”按钮后弹出“mongoDB connection”弹窗，依次填写Name、server、port；

  ![](http://i.imgur.com/SHYDa4A.png)

MongoDB的安装及配置详解，见[MongoDB install and config](MongoDB-install-and-config)

### No SQL for MongoDB

同样是一款非常优秀的MongoDB客户端




[了解更多](https://cmake.org/)

[返回目录](#目录)

# 2. 预处理程序的配置与编译

## i. Python配置

数据预处理是通过[Python](https://www.python.org/)将TauDEM提取流域信息、土壤理化性质初始化计算、MongoDB数据库导入等程序组成的工作流。

[下载并安装Python](https://www.python.org/downloads/)，并在系统环境变量中配置Python变量，详见[Python环境配置](Construct-python-env)。

## ii. 预处理程序中C++程序的编译

在数据预处理之前，首先需要编译一下数据预处理需要的C++程序，使用CMake进行编译。

C++程序包括：

+ `preprocess/cpp_src`
+ `preprocess/cpp_src/metis-5.1.0-pk`

编译方式有两种：

+ 一是CMake生成VS2010工程文件，然后在VS2010中将工程编译链接为可执行程序
+ 二是直接利用CMake生成可执行文件 （推荐采用）。

**第一种方式编译步骤**：
+ 在SEIMS源代码外新建一个build文件夹，比如`D:\SEIMS_model\SEIMS_preprocessing\build`；
+ 打开 “开始” -> Microsoft Visual Studio 2010 -> Visual Studio Tools -> Visual Studio 命令提示(2010)，以**管理员方式**运行，依次输入以下命令：

> 注意：由于目前SEIMS源码里自带的GDAL为32位编译版本，因此，目前只允许编译为32位预处理程序及SEIMS程序，后续会考虑同时加入64位版GDAL
>
> 因此如果是Windows x64电脑，不要打开的是Visual Studio x64 Win64 命令提示(2010)，而是需要打开Visual Studio 命令提示(2010)

```shell
cd D:\SEIMS_model\SEIMS_preprocessing\build
D:
cmake <SEIMS Folder>/preprocess
mkdir metis
cd .\metis
cmake <SEIMS Folder>/preprocess/cpp_src/metis-5.1.0-pk
```

+ 然后用VS2010打开< build >文件夹下的SEIMS_Preprocess.sln，并生成解决方案，此时，预处理程序目录为`< build >\Debug`，或者`< build >\Release`

打开<build>\metis文件夹下的METIS.sln，同样生成解决方案，此时，用于子流域任务划分的METIS程序目录为`< build >\metis\programs\Debug`，或者`< build >\metis\programs\Release`

**第二种方式编译步骤**：
+ 新建build文件夹（同一）；
+ 打开 “开始” -> Microsoft Visual Studio 2010 -> Visual Studio Tools -> Visual Studio 命令提示(2010)，以**管理员方式**运行，依次输入以下命令：

> 注意：由于目前SEIMS源码里自带的GDAL为32位编译版本，因此，目前只允许编译为32位预处理程序及SEIMS程序，后续会考虑同时加入64位版GDAL
>
> 因此如果是Windows x64电脑，不要打开的是Visual Studio x64 Win64 命令提示(2010)，而是需要打开Visual Studio 命令提示(2010)


```shell
cd D:\SEIMS_model\SEIMS_preprocessing\build
D:
cmake -G "NMake Makefiles" <SEIMS Folder>\preprocess
nmake
mkdir metis
cd .\metis
cmake -G "NMake Makefiles" <SEIMS Folder>/preprocess/cpp_src/metis-5.1.0-pk
nmake
```

完成后，不需要打开VS2010生成解决方案，可执行文件已经编译完成。此时，预处理程序目录为`< build >`，用于任务划分的METIS程序目录为`< build >\metis\programs`。

# 3. SEIMS主程序的编译

SEIMS主程序的编译是通过CMake将SEIMS源码编译为VS2010工程。

## i. SEIMS源码结构
![](https://camo.githubusercontent.com/8231d7b5009d18900ae1d53a96354cace9383e1c/687474703a2f2f7a68756c6a2d626c6f672e6f73732d636e2d6265696a696e672e616c6979756e63732e636f6d2f7365696d732d696d672532467372636469722e706e67)

其中：
+ .git Github版本管理的文件，**勿动**

+ .gitignore为github管理时忽略的文件后缀名

+ include和lib为外部依赖库的文件，勿改动

+ model_data为测试数据，使用时应复制到他处

+ src为SEIMS源码

+ preprocess为预处理程序源码

+ postprocess为后处理程序源码

+ Changelog.md为版本更新日志

+ CMakeLists.txt为CMake命令文件，跨平台编译源码

+ LICENSE为GNU LICENSE

+ READEME.md为模型介绍


src内有3个文件夹，base为模型基本模块组，modules为功能模块组，main为主程序入口模块组。

modules中包含了SEIMS功能模块组：hydrology和Hydrology_longterm为水文模块组，erosion为侵蚀模块组，nutrient为养分循环模块组，growth为植被生长模块组。 每个模块组下又包括不同的算法模块。


## ii. Windows下SEIMS主程序的编译

+ 首先，在SEIMS源码外，新建一个文件夹用于存放编译出的VS工程，比如 `D:\SEIMS\SEIMS_prj`；

+ “开始” -> Microsoft Visual Studio 2010 -> Visual Studio Tools->Visual Studio 命令提示 (2010)，以**管理员方式运行**，切换到VS工程目录；

> 注意：由于目前SEIMS源码里自带的GDAL为32位编译版本，因此，目前只允许编译为32位预处理程序及SEIMS程序，后续会考虑同时加入64位版GDAL
>
> 因此如果是Windows x64电脑，不要打开的是Visual Studio x64 Win64 命令提示(2010)，而是需要打开Visual Studio 命令提示(2010)


```shell
cd D:\SEIMS\SEIMS_prj
D:
```
+ 然后，打开根目录下的`CMakeLists.txt`，根据本地情况指定MS MPI路径，默认路径为：

```
set(MPIEXEC "C:/Program Files/Microsoft MPI/Bin/mpiexec.exe")
```

> 如果需要更改路径，为了避免使用Github后更改默认路径，请先保留原始`CMakelist.txt`，上传更新时替换回原始的CMakelist文件。


```
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "AMD64")
	set(MS_MPI_ARCH_DIR x64)
	set(MS_MPI_PATH "C:/Program Files (x86)/Microsoft SDKs/MPI")
else()
	set(MS_MPI_PATH "C:/Program Files/Microsoft SDKs/MPI")
	set(MS_MPI_ARCH_DIR x86)
```

用于判断32位与64位机器，可修改为本地相应路径**（但是不建议这么做）**。
+ 配置编译版本
  + 默认生成OpenMP的版本，如果想生成MPI的版本，则：

    ```shell
	cmake <SEIMS Folder> -DSEIMSVERSION=MPI
    ```

  + 选择生成`Release`或者`Debug`版本
    默认生成`Release`版本，如果想要生成`Debug`版本，需要进行如下注释：

    ```
    SET (CMAKE_BUILD_TYPE "Debug")
    #SET (CMAKE_BUILD_TYPE "Release")
    ```

+ CMake生成`.sln` VS项目文件

	```shell
	cmake <source path>
	```

  比如：`cmake D:\GaohrWS\GithubPrj\SEIMS`

  如果cmake发生类似这样的错误：

  ![](http://i.imgur.com/cZor9Ej.png)

  说明编译的VS版本与目前计算机版本不一致，需要更改编译命令：

	```shell
	cmake -G "Visual Studio 10 2010" <source path>
	```
  比如：`cmake -G "Visual Studio 10 2010" D:\GaohrWS\GithubPrj\SEIMS`

  ![](http://i.imgur.com/KrKn0CG.jpg)
  ![](https://camo.githubusercontent.com/ac07c4ea29759c609fb91d681d177483520abd3e/687474703a2f2f7a68756c6a2d626c6f672e6f73732d636e2d6265696a696e672e616c6979756e63732e636f6d2f7365696d732d696d67253246767370726f6a6563742e706e67)

+ 打开VS工程，生成解决方案，在Debug或Release下可以看到一系列lib、dll库以及seims_omp.exe或seims.exe可执行文件。

至此，由源码到VS工程的转换已经完成。

```
cmake -G "NMake Makefiles" <source path> -DCMAKE_BUILD_TYPE=Realse
nmake install
```
编译好的可执行程序默认安装在`<source path>/bin`目录下。


[返回目录](#目录)
