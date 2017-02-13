Install SEIMS on macOS
----------------------------

First release: Liang-Jun Zhu

Latest Updated：Feb.13, 2017

> 测试环境为Macbook Pro 13 （Late 2016） with macOS Sierra 10.12.3

# 1. Prerequisites

SEIMS模型采用C++和Python编写，支持子流域-栅格双层并行计算，在macOS下推荐采用GCC进行编译安装，安装前的准备包括：
+ GCC49 -- C++源码编译器
+ OpenMPI -- 编译MPI并行程序，如TauDEM、SEIMS-MPI版本等
+ CMake -- 管理代码编译
+ GDAL 1.x with Python -- 矢栅数据读写库
+ mongo-c-driver 1.3.5 或更高 -- NoSQL数据库MongoDB的驱动库

> Note: SEIMS目前只提供32-bit版本编译帮助，因此，接下来GDAL的安装、mongo-c-driver的编译均指的是32-bit版本。

## 1.1. GCC49
macOS中最便捷配置GCC版本的方式是通过[Homebrew](http://brew.sh/)自动安装。
+ 在终端输入代码`/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
` 进行安装`brew`工具
+ 输入`brew tap homebrew/versions`更新`brew`中软件版本
+ 输入`brew install gcc49`安装
+ 安装大约需要60分钟，耐心等待安装完成，可通过`gcc-4.9 --version`查看版本信息：
```
gcc-4.9 --version
gcc-4.9 (Homebrew gcc49 4.9.3) 4.9.3
Copyright (C) 2015 Free Software Foundation, Inc.
```

## 1.2. OpenMPI
+ 从[官网下载OpenMPI源码](https://www.open-mpi.org/software/ompi/v1.10/)，推荐安装1.x系列的最新版，目前最新的是1.10.4，解压至文件夹，如`/Users/zhulj/apps/openmpi-1.10.4
`
+ 配置config，指定使用GCC-4.9编译，如下：`./configure CC=gcc-4.9 CXX=g++-4.9 --prefix=/usr/local`
+ `make all`
+ `sudo make install`

安装完成之后，在终端中输入`mpic++ --showme`，会提示`g++-4.9 -I/usr/local/include -L/usr/local/lib -lmpi_cxx -lmpi`，则表明OpenMPI是由GCC49编译的。

## 1.3. CMake

CMake是一个跨平台的安装或编译工具，可以用简单的语句来描述安装或编译过程。CMake通过`CmakeLists.txt`文件能够输出各种各样的makefile或者IDE工程。

CMake可以从其[官网免费下载](http://www.cmake.org/files)安装GUI版本，推荐安装3.0以上版本，也可通过`brew install cmake`安装。

## 1.4. GDAL with Python

SEIMS的矢栅数据读写基于`GDAL 1.x`编写，macOS下推荐使用[William Kyngesburye](http://www.kyngchaos.com/software:frameworks)维护的Framework安装包，最新的为`GDAL_Complete-1.11.dmg`。

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

## 1.5. mongo-c-driver

SEIMS数据管理采用NoSQL型数据库——MongoDB，依赖于mongo-c-driver。
macOS下的配置步骤为：
+ 从[官网](http://mongoc.org/ "mongo-c-driver-download")下载源码压缩包，目前最新稳定版本为[1.5.0](https://github.com/mongodb/mongo-c-driver/releases/download/1.5.0/mongo-c-driver-1.5.0.tar.gz "mongo-c-driver-1.5.0")，解压缩至当前文件夹，如`/Users/zhulj/apps/mongo-c-driver-1.5.0`
+ 使用brew安装依赖包`brew install automake autoconf libtool pkgconfig openssl`
+ 打开终端，依次输入如下命令
```shell
cd /Users/zhulj/apps/mongo-c-driver-1.5.0
export LDFLAGS="-L/usr/local/opt/openssl/lib"
export CPPFLAGS="-I/usr/local/opt/openssl/include"
./configure CC=gcc-4.9 CXX=g++-4.9 --prefix=/usr/local
make
sudo make install
```

+ 至此，`mongo-c-driver`即编译安装完成了，在`/usr/local/include`目录下能看到`libbson-1.0`, `libmongoc-1.0`文件夹，链接库则在`/usr/local/lib`。

# 2. Compilation and Installation

## 2.1 Installation for users

```shell
cd /Users/zhulj/Documents/code/SEIMS2017/seims
mkdir build
cd build
1. clang: cmake .. -DCMAKE_BUILD_TYPE=Release
2. GCC: cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/local/bin/gcc-4.9 -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-4.9
make
make install
```
+ 编译、安装无误后，SEIMS所有可执行程序及模块动态链接库均在`./SEIMS2017/seims/bin`。

+ 注意1：使用clang编译时，由于其不支持openmp，因此基于共享内存的并行计算将无法实现，因此推荐使用GCC进行编译安装。

## 2.2 Installation for developers

### 2.2.1 Xcode (test on Xcode 8.2)

```shell
cd /Users/zhulj/Documents/code/SEIMS2017/seims
mkdir build
cd build
cmake -G "Xcode" /Users/zhulj/Documents/code/SEIMS2017/seims -DCMAKE_BUILD_TYPE=Release
```
+ 完成之后，即可得到SEIMS项目总工程，`SEIMS_OMP_ALL.xcodeproj`，各子工程则在`build/bin`下，如SEIMS模块库工程为`build/bin/seims_omp_project/SEIMS_OMP_prj.xcodeproj`。

### 2.2.2 CLion (test on CLion 2016.3.2）

CLion继承了JetBrains家族的特点，工程采用CMake管理，方便、高效。在CLion中直接打开`SEIMS2017/seims`文件夹即可导入整个工程。

CLion默认使用clang编译器，因此需要简单设置一下，[官网提供了三种方法](https://cmake.org/Wiki/CMake_FAQ#How_do_I_use_a_different_compiler.3F)，推荐采用第二种，即：

+ 打开CLion->Preferences->Build，Execution，Deployment->CMake
+ 在Generation选项卡下的CMake options中输入以下命令，以覆盖CMake的默认设置，同理为Debug和Release分别设置。

```
-D CMAKE_C_COMPILER=<your_path_to_c_compiler>
-D CMAKE_CXX_COMPILER=<your_path_to_cxx_compiler>

e.g.
-DCMAKE_C_COMPILER=/usr/local/bin/gcc-4.9 -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-4.9
```

接下来，你可以选择Build All，也可以选择某一个程序进行Build等。
