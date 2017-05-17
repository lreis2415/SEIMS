# MongoUtilClass
================

+ Handing data with MongoDB using mongo-c-driver.

+ 利用mongo-c-driver操作MongoDB数据库。

+ Author: [Liangjun Zhu](http://zhulj.net)

## 1 Introduction
+ MongoUtilClass提供基本的MongoDB数据库操作。
+ MongoUtilClass可单独调试，也可作为其他项目的基础类。

## 2 Install mongo-c-driver

### 2.1 On Windows
+ Windows开发环境推荐采用Microsoft Visual Studio 2010 或更高
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

### 2.2 On Linux

> 以CentOS为例 (Take CentOS as example)

+ 下载源码包，如`mongo-c-driver-1.6.0.tar.gz`
+ 解压、编译、安装，命令如下：

	```shell
	tar xzf mongo-c-driver-1.6.0.tar.gz
	cd mongo-c-driver-1.6.0
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

## 3 MongoUtilClass测试
+ MongoUtilClass采用CMake进行跨平台编译。
+ MongoUtilClass需调用[UtilsClass](https://github.com/lreis2415/UtilsClass)，编译前需将其保存至MongoUtilClass**同级目录**下。

### 3.1 Windows
+ 打开 “开始” -> Microsoft Visual Studio 2010 -> Visual Studio Tools -> Visual Studio 命令提示(2010)，以**管理员方式**运行，依次输入以下命令：
	
	```shell
	cd <path-to-MongoUtilClass>
	mkdir build
	cd build
	### 仅编译安装 ###
	cmake -G "NMake Makefiles" <path-to-MongoUtilClass> -DCMAKE_BUILD_TYPE=Release
	nmake
	nmake install
	### 编译Microsoft Visual Studio工程 ###
	cmake <path-to-MongoUtilClass>
	nmake
	```

+ 对于“仅编译安装”操作，`MongoUtilClass.exe`会自动安装在`<path-to-MongoUtilClass>\bin`目录下。
+ 对于“编译Microsoft Visual Studio工程”，`MongoUtil.sln`将保存在`<path-to-MongoUtilClass>\build`目录下。

### 3.2 Linux

	```shell
	cd MongoUtilClass
	mkdir build
	cd build
	cmake ..
	make
	make install
	```
+ 可执行程序`MongoUtil`默认安装于`MongoUtilClass/bin`下
+ 如果希望采用其他GCC安装版本，请在cmake命令后面追加：`-DCMAKE_C_COMPILER=~/gcc4.8.4/bin/gcc -DCMAKE_CXX_COMPILER=~/gcc4.8.4/bin/g++`

### 3.3 macOS

+ macOS的编译、安装步骤与Linux相同。
+ macOS下推荐采用CLion进行代码调试。
