# MongoUtilClass
----------------

+ Handing data with MongoDB using mongo-c-driver.

+ 利用mongo-c-driver操作MongoDB数据库。

+ Author: [Liangjun Zhu](http://zhulj.net)

Selected build environments:

+ Windows-MSVC 2013-64bit: [![Build status](https://ci.appveyor.com/api/projects/status/b3eu2hfca1mte3ta?svg=true)](https://ci.appveyor.com/project/lreis-2415/mongoutilclass)
+ Linux(Ubuntu trusty)-GCC-4.8: [![Build Status](http://badges.herokuapp.com/travis/lreis2415/MongoUtilClass?branch=master&env=BUILD_NAME=linux_gcc48&label=linux_gcc48)](https://travis-ci.org/lreis2415/MongoUtilClass)
+ macOS-Clang-7.3: [![Build Status](http://badges.herokuapp.com/travis/lreis2415/MongoUtilClass?branch=master&env=BUILD_NAME=osx_xcode&label=osx_clang)](https://travis-ci.org/lreis2415/MongoUtilClass)

Code coverage: [![codecov](https://codecov.io/gh/lreis2415/MongoUtilClass/branch/master/graph/badge.svg)](https://codecov.io/gh/lreis2415/MongoUtilClass)

## 1 Introduction
+ MongoUtilClass提供基本的MongoDB数据库操作。
+ MongoUtilClass可单独调试，也可作为其他项目的基础类。

## 2 Install mongo-c-driver

### 2.1 On Windows
+ Windows开发环境推荐采用Microsoft Visual Studio 2013 或更高
+ 从[官网](http://mongoc.org/ "mongo-c-driver-download") 或[Github](https://github.com/mongodb/mongo-c-driver/releases)下载源码压缩包，如版本[1.9.2](https://github.com/mongodb/mongo-c-driver/archive/1.9.2.tar.gz "mongo-c-driver-1.9.2")，解压缩至本地文件夹，如`C:\z_code\Repos\mongo-c-driver-1.9.2`
+ 打开cmd，依次输入如下命令，默认的安装目录为`C:\mongo-c-driver`

  ```bat
  >cd C:\z_code\Repos\mongo-c-driver-1.9.2
  # 1. Configure and install libbson first.
  >cd src\libbson
  >cmake -DCMAKE_INSTALL_PREFIX=C:\mongo-c-driver -G "Visual Studio 12 2013 Win64"
  >msbuild.exe ALL_BUILD.vcxproj /p:Configuration=Release
  >msbuild.exe INSTALL.vcxproj /p:Configuration=Release
  # 2. Go back to the root folder, configure and install mongoc
  >cd ..\..
  >cmake -DCMAKE_INSTALL_PREFIX=C:\mongo-c-driver -DBSON_ROOT_DIR=C:\mongo-c-driver -G "Visual Studio 12 2013 Win64" -DCMAKE_PREFIX_PATH=C:\mongo-c-driver\lib\cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP:BOOL=OFF -DENABLE_SSL=WINDOWS -DENABLE_SASL=SSPI
  # Use Windows Native TLS, rather then OpenSSL in case of strange link errors
  # Use Windows Native SSPI, rather then Cyrus SASL
  >msbuild.exe ALL_BUILD.vcxproj /p:Configuration=Release
  >msbuild.exe INSTALL.vcxproj /p:Configuration=Release
  ```

+ 至此，`mongo-c-driver`即编译安装完成了，在`C:\mongo-c-driver`目录下能看到`bin`, `include`, `lib`文件夹。
+ 将`C:\mongo-c-driver`添加至环境变量，命名为`MONGOC_ROOT_DIR`。
+ 新建系统环境变量`MONGOC_LIB_DIR` ，其值为`C:\mongo-c-driver\bin`，并将`%MONGOC_LIB_DIR%`添加至环境变量`PATH`中。

> Note: 如果cmd提示找不到msbuild.exe，可以在msbuild.exe前加上绝对路径或将其路径添加至环境变量`PATH` 中，这个文件是`.NetFramework`里的，比如`C:\Windows\Microsoft.NET\Framework64\v4.0.30319\msbuild.exe`

### 2.2 On Linux

> 以CentOS为例 (Take CentOS as example)

+ 从[官网](http://mongoc.org/ "mongo-c-driver-download") 或[Github](https://github.com/mongodb/mongo-c-driver/releases)下载源码压缩包，如版本[1.9.2](https://github.com/mongodb/mongo-c-driver/archive/1.9.2.tar.gz "mongo-c-driver-1.9.2")，解压缩至本地文件夹，如`mongo-c-driver-1.9.2.tar.gz`
+ 解压、编译、安装，命令如下：

  ```shell
  tar xzf mongo-c-driver-1.9.2.tar.gz
  cd mongo-c-driver-1.9.2
  ./configure --disable-automatic-init-and-cleanup
  make
  make install
  ```

+ 值得注意的是，在`./configure`时可以修改一些参数，比如GCC版本、安装目录等，比如我的配置为：`./configure --prefix=~/code/mongo-c-driver CC=~/gcc4.8.4/bin/gcc CXX=~/gcc4.8.4/bin/g++ --disable-automatic-init-and-cleanup`
+ 安装完成之后，`bin`、`include`、`lib`等目录均在`/usr/local`或自定义的安装目录下，
+ 在`/etc/profile`（对所有用户有效）或`~/.bash_profile`（对当前用户有效）内添加环境变量以及追加动态链接库目录

  ```shell
  e.g.

  export MONGOC_ROOT_DIR=~/code/mongoc-c-driver
  export PKG_CONFIG_PATH=~/code/mongoc-c-driver/lib/pkgconfig:$PKG_CONFIG_PATH
  export LD_LIBRARY_PATH=~/code/mongoc-c-driver/lib:$LD_LIBRARY_PATH
  ```

+ 保存后，通过`source /etc/profile`或`source ~/.bash_profile`命令使设置生效


### 2.3 On macOS
+ 从[官网](http://mongoc.org/ "mongo-c-driver-download") 或[Github](https://github.com/mongodb/mongo-c-driver/releases)下载源码压缩包，如版本[1.9.2](https://github.com/mongodb/mongo-c-driver/archive/1.9.2.tar.gz "mongo-c-driver-1.9.2")，解压缩至本地文件夹，如`/Users/zhulj/apps/mongo-c-driver-1.5.0`
+ 使用brew安装依赖包`brew install automake autoconf libtool pkgconfig openssl`
+ 打开终端，依次输入如下命令
  ```shell
  cd /Users/zhulj/apps/mongo-c-driver-1.9.2
  export LDFLAGS="-L/usr/local/opt/openssl/lib"
  export CPPFLAGS="-I/usr/local/opt/openssl/include"
  ./configure CC=gcc-4.9 CXX=g++-4.9 --prefix=/usr/local
  make
  sudo make install
  ```

+ 至此，`mongo-c-driver`即编译安装完成了，在`/usr/local/include`目录下能看到`libbson-1.0`, `libmongoc-1.0`文件夹，链接库则在`/usr/local/lib`。

## 3 单元测试
+ MongoUtilClass采用CMake进行跨平台编译。
+ MongoUtilClass采用[Google Test](https://github.com/google/googletest)单元测试框架。
+ MongoUtilClass需调用[UtilsClass](https://github.com/lreis2415/UtilsClass)，编译前需将其保存至MongoUtilClass**同级目录**下。
+ 所有单元测试代码统一唯一`test`文件夹下，并以`Test_XX.cpp`格式命名。

### 3.1 Windows
+ 打开 “开始” -> Microsoft Visual Studio 2013 -> Visual Studio Tools -> Visual Studio 命令提示(2013)，以**管理员方式**运行，依次输入以下命令：

  ```shell
  cd <path-to-MongoUtilClass>
  mkdir build
  cd build
  ### 编译Microsoft Visual Studio工程 ###
  cmake -G "Visual Studio 12 2013 Win64" -DUNITTEST=1 ..
  ```

+ `MongoUtil.sln`将保存在`<path-to-MongoUtilClass>\build`目录下，编译
  运行`UnitTests_Mongo`项即可看到当前所有单元测试项的结果，如：

    ```shell
    Running main() from gtest_main.cc
    [==========] Running 1 test from 1 test case.
    [----------] Global test environment set-up.
    [----------] 1 test from Test_Mongo
    [ RUN      ] Test_Mongo.initMongoDB
    [       OK ] Test_Mongo.initMongoDB (260 ms)
    [----------] 1 test from Test_Mongo (261 ms total)
    
    [----------] Global test environment tear-down
    [==========] 1 test from 1 test case ran. (262 ms total)
    [  PASSED  ] 1 test.
    ```

### 3.2 Linux
+ 打开终端，依次输入如下命令

  ```shell
  cd MongoUtilClass
  mkdir build
  cd build
  cmake ..
  cmake -DCMAKE_BUILD_TYPE=Debug -DUNITTEST=1 ..
  make
  make UnitTestCoverage
  ```

+ 如果希望采用其他GCC安装版本，请在cmake命令后面追加：`-DCMAKE_C_COMPILER=~/gcc4.8.4/bin/gcc -DCMAKE_CXX_COMPILER=~/gcc4.8.4/bin/g++`

### 3.3 macOS

+ 参考Linux下操作。

