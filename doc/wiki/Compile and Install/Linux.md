Linux下SEIMS模型的编译与运行
----------------
> Latest update: 2017-2-13

## 1. 准备工作

以192.168.6.55服务器上zhulj账号下的配置为例：
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

## 2. SEIMS编译和安装

```shell
cd <SEIMS root path>/seims
mkdir build
cd build
/home/zhulj/CMake/bin/cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/home/zhulj/gcc4.8.4/bin/gcc -DCMAKE_CXX_COMPILER=/home/zhulj/gcc4.8.4/bin/g++
make (or make 2>&1 | tee build.log 用于保存编译日志)
make install
```

编译完成之后，SEIMS模型所有可执行文件和模块库均安装于`<SEIMS root path>/seims/bin`目录下。
