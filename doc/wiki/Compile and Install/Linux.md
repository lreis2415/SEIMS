Linux下SEIMS模型的编译与运行
----------------

> 以下是以192.168.6.55服务器上zhulj账号下的路径为例。

## 0 Paths

### 由于zhulj账号下的一些配置有别于服务器配置，因此单独介绍如下：
+ GCC编译器版本至4.8.4，路径`/home/zhulj/gcc4.8.4/bin`，即在模型`CMakeLists.txt`中需显示定义GCC编译器路径：`SET(CMAKE_CXX_COMPILER "/home/zhulj/gcc4.8.4/bin/g++")`
+ MPICH版本至3.1.4，路径`/home/zhulj/mpich/bin`,同理，如果编译MPI版本，则需显示定义编译器路径：`SET (CMAKE_CXX_COMPILER /home/zhulj/mpich/bin/mpic++)`，此时，模型配置文件（`/home/zhulj/SEIMS/SEIMS/preprocess/*.ini`）第9行：`MPIEXEC_DIR = /home/zhulj/mpich/bin`
+ Python版本至2.7.9，路径`/usr/bin`（软链接至2.7.9，旧版本为`/usr/bin/python2.6`）
+ mongo-c-driver版本至1.3.5，路径`/home/zhulj/code/mongo-c-driver`，当然该目录可以复制至任何目录使用

### 程序及数据路径约定：

+ SEIMS文件夹为 `/home/zhulj/SEIMS`
+ 源代码文件夹为 `/home/zhulj/SEIMS/SEIMS`
+ 预处理Python主程序目录 `/home/zhulj/SEIMS/SEIMS/preprocess`
+ 预处理C++程序目录 `/home/zhulj/SEIMS/SEIMS_preprocess`
+ SEIMS主程序及模块目录 `/home/zhulj/SEIMS/seims_omp`
+ 研究区模型目录（配置文件及OUTPUT） `/home/zhulj/SEIMS/models`
+ 研究区模型构建中间文件（即preprocess) `/home/zhulj/SEIMS/models_prepare`

## 1 SEIMS_Preprocess Cpp programs

```shell

	cd /home/zhulj/SEIMS/SEIMS_preprocess
	cmake /home/zhulj/SEIMS/SEIMS/preprocess
	make (or make 2>&1 | tee build_proprocess.log 用于保存编译日志)
	cd /home/zhulj/SEIMS/SEIMS/preprocess/cpp_src/metis-5.1.0-pk
	make config
	make

```

编译完成之后，预处理路径分别为：

```shell

	CPP_PROGRAM_DIR = /home/zhulj/SEIMS/SEIMS_preprocess
	METIS_DIR = /home/zhulj/SEIMS/SEIMS/preprocess/cpp_src/metis-5.1.0-pk/build/Linux-x86_64/programs

```

对应于模型配置文件（`/home/zhulj/SEIMS/SEIMS/preprocess/*.ini`）的第7~8行

## 2 SEIMS Cpp program

```
	
	cd /home/zhulj/SEIMS/seims_omp
	cmake /home/zhulj/SEIMS/SEIMS

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
	-- Build files have been written to: /home/zhulj/SEIMS/seims_omp

```

随后，

```
make (or make 2>&1 | tee build_seims.log 用于保存编译日志)
```

正常情况下，SEIMS即编译完成！

## 3 Model preparation

