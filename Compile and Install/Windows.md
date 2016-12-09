在Windows下编译SEIMS可执行文件
----------------------------

by Huiran Gao, Liang-Jun Zhu

Latest Updated：May.30, 2016 

--------------------------
# 目录

[**1. Prerequisite**](#1-prerequisite)

1. [MS MPI v6（或更高版本）](#i-ms-mpi-v6（或更高版本）)

2. [MongoDB及MongoVUE](#ii-mongodb及mongovue)

3. [CMake](#iii-cmake)

[**2. 预处理程序的配置与编译**](#2-预处理程序的配置与编译)

1. [Python配置](#i-python配置)

2. [预处理程序中C++程序的编译](#ii-预处理程序中c++程序的编译)

[**3. SEIMS主程序的编译**](#3-seims主程序的编译)

1. [SEIMS源码结构](#i-seims源码结构)

2. [Windows下SEIMS主程序的编译](#ii-windows下seims主程序的编译)

# 1. Prerequisite

SEIMS模型依赖一系列软件，包括Microsoft-MPI v6（或更高版本），MongoDB及MongoVUE（或No SQL for MongoDB），CMake等。

## i. MS MPI v6（或更高版本）

Microsoft MPI (MS-MPI) 是微软基于MPICH实现的用于Windows平台开发和运行并行应用程序的消息传递接口标准，预处理程序中用于提取子流域和地形信息的TauDEM、SEIMS—_MPI并行版本都需要MPI的支持。

从[MS-MPI v6](https://www.microsoft.com/en-us/download/details.aspx?id=47259)下载并分别安装msmpisdk.msi, MSMpiSetup.exe，并配置好系统环境变量。

如上述链接打不开，可从[百度网盘](http://pan.baidu.com/s/1jIdbVka)下载，提取码：50te

> 强烈建议采用默认安装路径安装MPI，否则后续采用CMAKE编译时应手动修改CMakeLists.txt来指定MPI路径

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


## iii. CMake

CMake是一个跨平台的安装或编译工具，可以用简单的语句来描述安装或编译过程。能够输出各种各样的makefile或者project文件，CMake 的组态档取名为`CmakeLists.txt`，下载[cmake-3.2.2](http://www.cmake.org/files/v3.2/cmake-3.2.3-win32-x86.exe)版本并安装，添加CMake环境变量到系统PATH变量里。

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

[返回目录](#目录)







