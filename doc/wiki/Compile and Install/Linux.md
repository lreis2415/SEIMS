Linux下SEIMS模型的编译与运行
----------------
> Latest update: 2017-2-11

> 以下是以192.168.6.55服务器上zhulj账号下的路径为例。

## 1. 准备工作

### 1.1 zhulj账号下对服务器原有配置进行了更新：
+ CMake 版本更新为3.7.2，路径`~/CMake/bin/cmake`;
+ GCC编译器版本至4.8.4，路径`~/gcc4.8.4/bin`，在CMake命令中加入`-DCMAKE_C_COMPILER=~/gcc4.8.4/bin/gcc -DCMAKE_CXX_COMPILER=~/gcc4.8.4/bin/g++`以显示定义GCC编译器路径；
+ MPICH版本至3.1.4，路径`~/mpich/bin`,同理，如果编译MPI版本，则需显示定义编译器路径：`-DCMAKE_CXX_COMPILER=~/mpich/bin/mpic++`;
+ mongo-c-driver版本至1.6.0，路径`~/code/mongo-c-driver`，并在`~/.bash_profile`中更新了环境变量`MONGOC_ROOT_DIR`、`PKG_CONFIG_PATH`和`LD_LIBRARY_PATH`，以确保基于mongo-c-driver的程序能够顺利编译、链接、安装、运行。
+ GDAL采用服务器现有版本1.9.0，并新建环境变量`export GDAL_ROOT=/soft/share`
+ Python版本至2.7.9，路径`/usr/bin`（软链接至2.7.9，旧版本为`/usr/bin/python2.6`）
+ Python依赖库及版本号如下：

	```py
	# Preprocess
	GDAL>=1.9.0,<2.0
	numpy>=1.9.0
	pymongo>=3.0
	networkx>=1.10
	Shapely>=1.5.0
	# Postprocess
	matplotlib>=1.5.0
	pathlib2>=2.0.0
	# Scenario analysis
	deap>=1.0.2
	scoop>=0.7.1.1
	```

### 1.2 

### 程序及数据路径约定：

+ SEIMS文件夹为 `~/SEIMS`
+ 源代码文件夹为 `~/SEIMS/SEIMS`
+ 预处理Python主程序目录 `~/SEIMS/SEIMS/preprocess`
+ 预处理C++程序目录 `~/SEIMS/SEIMS_preprocess`
+ SEIMS主程序及模块目录 `~/SEIMS/seims_omp`
+ 研究区模型目录（配置文件及OUTPUT） `~/SEIMS/models`
+ 研究区模型构建中间文件（即preprocess) `~/SEIMS/models_prepare`

## 1 SEIMS_Preprocess Cpp programs

```shell

	cd ~/SEIMS/SEIMS_preprocess
	cmake ~/SEIMS/SEIMS/preprocess
	make (or make 2>&1 | tee build_proprocess.log 用于保存编译日志)
	cd ~/SEIMS/SEIMS/preprocess/cpp_src/metis-5.1.0-pk
	make config
	make

```

编译完成之后，预处理路径分别为：

```shell

	CPP_PROGRAM_DIR = ~/SEIMS/SEIMS_preprocess
	METIS_DIR = ~/SEIMS/SEIMS/preprocess/cpp_src/metis-5.1.0-pk/build/Linux-x86_64/programs

```

对应于模型配置文件（`~/SEIMS/SEIMS/preprocess/*.ini`）的第7~8行

## 2 SEIMS Cpp program

```
	
	cd ~/SEIMS/seims_omp
	cmake ~/SEIMS/SEIMS

```

如果看到如下信息，则可进行下一步，否则需要检查CMakeLists.txt中是否有路径错误：

```
	
	-- Compiling folder: base...
	-- Compiling folder: modules...
	--     Compiling folder: climate...
	--     Compiling folder: hydrology_longterm...
	--     Compiling folder: erosion...
	--     Compiling folder: nutrient...
	--     Compiling folder: ecology...
	--     Compiling folder: management...
	-- Compiling folder: main...
	-- SEIMS is Ready for you!
	-- Configuring done
	-- Generating done
	-- Build files have been written to: ~/SEIMS/seims_omp

```

随后，

```
make (or make 2>&1 | tee build_seims.log 用于保存编译日志)
```

正常情况下，SEIMS即编译完成！

## 3 Model preparation

