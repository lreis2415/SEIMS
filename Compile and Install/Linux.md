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

## 2. SEIMS编译和安装

```shell
cd <SEIMS root path>/seims
mkdir build
cd build
/home/zhulj/CMake/bin/cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/home/zhulj/gcc4.8.4/bin/gcc -DCMAKE_CXX_COMPILER=/home/zhulj/gcc4.8.4/bin/g++
make (or make 2>&1 | tee build.log 用于保存编译日志)
make install
```

+ 在执行`make`命令时，可追加 `-j 4`命令利用CPU多核性能并行编译程序，提高编译速度，并行进程数最好设置为物理核数的2倍。
+ 编译完成之后，SEIMS模型所有可执行文件和模块库均安装于`<SEIMS root path>/seims/bin`目录下。

## 3. Python for SEIMS 依赖包的安装
 
SEIMS预处理、后处理、率定、情景分析等所需的Python依赖库及版本要求如下：

```py
pygeoc
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

+ SEIMS提供了自动安装脚本，脚本依赖于pip，因此请确保pip已正确安装，[参考教程](https://pip.pypa.io/en/stable/installing/)：
  + 下载[get-pip.py](https://bootstrap.pypa.io/get-pip.py)
  + `python get-pip.py`

+ 执行`sudo sh pyseims_install_linux.sh`即可自动安装完成所有依赖库，包括SEIMS自带的`PyGeoC`。

+ 安装完成后，运行`python pyseims_check.py`检查所需python库是否已经安装成功，如果有未成功的请单独安装。
