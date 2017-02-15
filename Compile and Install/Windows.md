Install SEIMS on Windows
----------------------------

First release: Huiran Gao

Reviewed and updated: Liang-Jun Zhu

Latest Updated：Feb.13, 2017

--------------------------
# 目录

[**1. Prerequisites**](#1-prerequisites)

  1.1. [Microsoft Visual Studio](#11-microsoft-visual-studio)

  1.2. [CMake](#12-cmake)

  1.3. [GDAL and Python](#13-gdal-and-python)

  1.4. [mongo-c-driver](#14-mongo-c-driver)

  1.5. [MS-MPI](#15-ms-mpi)

[**2. Compilation and Installation**](#2-compilation-and-installation)

  2.1. [Installation for users](#21-installation-for-users)

  2.2. [Installation for developers](#22-installation-for-developers)

2. [预处理程序中C++程序的编译](#ii-预处理程序中c++程序的编译)

[**3. SEIMS主程序的编译**](#3-seims主程序的编译)

1. [SEIMS源码结构](#i-seims源码结构)

2. [Windows下SEIMS主程序的编译](#ii-windows下seims主程序的编译)

# 1. Prerequisites

SEIMS模型采用C++和Python编写，支持子流域-栅格双层并行计算，在Windows下推荐采用Microsoft Visual Studio进行编译安装，安装前的准备包括：
+ Microsoft Visual Studio 2010 或更高 -- C++源码编译器及IDE
+ CMake 3.1+ -- 管理代码编译
+ GDAL 1.x with Python -- 矢栅数据读写库
+ mongo-c-driver 1.5+ -- 读写MongoDB
+ MS-MPI v6 （或更高）-- 编译MPI并行程序，如TauDEM、SEIMS-MPI版本等

> Note: SEIMS目前只提供32-bit版本编译帮助，因此，接下来GDAL的安装、mongo-c-driver的编译均指的是32-bit版本。

## 1.1 Microsoft Visual Studio

+ **For User**： 如果只是希望源码编译SEIMS模型，而不想安装臃肿庞大的VS，可以选择使用Microsoft Visual C++ Build Tools，目前最新版本为[2015 Update 3](https://www.visualstudio.com/downloads/#microsoft-visual-c-build-tools-2015-update-3 "microsoft-visual-c-build-tools-2015-update-3").
+ **For Developer**： 如果希望对SEIMS模型进行改进，建议采用[Microsoft Visual Studio 2010 或更高版本](https://www.visualstudio.com/downloads/ "visual-studio-downloads")进行开发。

> Note: 虽然SEIMS编译理论上支持更高版本的VS，但是为了接下来的GDAL库安装方便，建议使用与[Tamas Szekeres's Windows GDAL binaries](http://www.gisinternals.com/release.php)发布版本对应的VS，比如目前其发布的GDAL安装版本对应的最高VS版本为2013。当然，如果你希望自己源码编译接下来用到的GDAL、mongo-c-driver库，那VS版本选择则不受限制。

## 1.2. CMake

CMake是一个跨平台的安装或编译工具，可以用简单的语句来描述安装或编译过程。CMake通过`CmakeLists.txt`文件能够输出各种各样的makefile或者IDE工程。

CMake可以从其[官网免费下载](http://www.cmake.org/files)，最低安装版本要求为3.1，安装后添加CMake路径如`C:\Program Files (x86)\CMake`到系统环境变量`PATH`里。

## 1.3. GDAL and Python

SEIMS的矢栅数据读写基于`GDAL 1.x`编写，Windows下GDAL最方便的安装方式应该就是采用[Tamas Szekeres's Windows GDAL binaries](http://www.gisinternals.com/release.php)编译发布的版本。

+ 具体安装请参考博客[Installing-gdal-with-python-for-windows](https://sandbox.idre.ucla.edu/sandbox/tutorials/installing-gdal-for-windows "installing-gdal-with-python-for-windows")。

+ **注意！**安装完成之后，除了`GDAL_DATA`和`GDAL_DRIVER_PATH`外，还需要在环境变量里添加一个`GDAL_DIR`，赋值为你的GDAL安装目录，如`C:\GDAL`，以便CMake在编译时能够找得到GDAL依赖库。

+ **注意！**为了方便将来程序能够找到GDAL的诸多动态链接库，建议新建`GDAL_PATHS`环境变量`C:\GDAL;C:\GDAL\bin;C:\GDAL\bin\proj\apps;C:\GDAL\bin\gdal\apps;C:\GDAL\bin\ms\apps;C:\GDAL\bin\curl;`，并将`%GDAL_PATHS%`添加至环境变量`PATH`中。

## 1.4. mongo-c-driver

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

## 1.5. MS-MPI

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
	cd ~\SEIMS2017\seims
	mkdir build
	cd build
	cmake -G "NMake Makefiles" .. -DCMAKE_BUILD_TYPE=Release
	nmake
	nmake install
	```

完成后，不需要打开VS2010生成解决方案，可执行文件已经编译完成，程序目录为`~\SEIMS2017\seims\bin`。
+ 右键以管理员方式运行`~\seims\bin\Firewall_for_Windows.bat`，配置防火墙规则，以防在运行Python脚本调用SEIMS程序时被防火墙阻止。
## 2.2 Installation for developers

+ 打开 “开始” -> Microsoft Visual Studio 2010 -> Visual Studio Tools -> Visual Studio 命令提示(2010)，以**管理员方式**运行，依次输入以下命令：

	```shell
	cd ~\SEIMS2017\seims
	mkdir build
	cd build
	cmake ..
	```

+ 然后用VS2010打开< build >文件夹下的`SEIMS_OMP_ALL.sln`，即为整个SEIMS模型的VS工程，打开`build\bin`目录可以分别找到子工程，比如SEIMS模块库工程等。
+ VS工程编译完成之后，右键单击`INSTALL`模块，选择“生成”，即可完成SEIMS模型的安装，安装路径为`~\SEIMS2017\seims\bin`

[返回目录](#目录)
