SEIMS 模型配置、运行及率定
------------------

## 1. 模型配置

一个完整的SEIMS模型包括MongoDB数据库、模块配置文件config.fig、模拟时间步长和模拟时段文件file.in、参数输出设置文件file.out。

### 1.1 MongoDB数据库

通过[数据预处理](Data-preprocess)，我们得到了SEIMS运行依赖的MongoDB数据库，其中包括气象数据库和参数数据库。

1. 气象数据库：降水和气象数据
2. 参数数据库：以模型名称命名，包括下面几个表：
	+ parameters：包含模型的各个输入参数的信息，可以通过调整Impact参数进行调参
	+ reaches：河道相关属性信息
	+ SiteList：模拟所需的降水和气象站点
	+ Spatial：模拟所需的栅格数据、气象插值权重数据等，以`GridFS`方式存储

### 1.2 config.fig

模型运行所需的模块列表，SEIMS按照顺序依次加载指定的各个模块，在每个模拟时间步长内顺序执行。

通用格式为：
```
### FORMAT
### MODULE CLASS NO. | PROCESS NAME | METHOD NAME | MODULE ID
i.e.
#0 | Climate
0 | TimeSeries | | TSD_RD
0 | Interpolation | Thiessen_0 | ITP
0 | PET | Hargreaves | PET_H
```

> 以#开头的为注视行；允许有空行

### 1.3 file.in

设置SEIMS模型运行的:
  + MODE：`Daily`指日尺度模型；`Storm`指次降水模型；
  + Interval：步长，如果为Daily模拟，步长默认为1天，不可修改；如果为Storm模拟，步长单位为秒，可以分别指定坡面过程和河道过程的模拟步长，如“60, 60”指定次降水模拟的坡面和河道过程步长都是60s；
  + STARTTIME:模拟开始时间,格式：YYYY-MM-DD HH:MM:SS
  + ENDTIME:模拟结束时间

如：

```
MODE|Daily
INTERVAL|1
STARTTIME|2014-01-01 00:00:00
ENDTIME|2014-12-31 00:00:00
```

### 1.4 file.out

指定模型输出变量相关信息，基本格式为：

```
### FORMAT: TAG | VALUE
### ALL AVAILABLE TAGS:
### OUTPUTID	: VARIABLE NAME FOR OUTPUT
### COUNT		: OUTPUT NUMBER, e.g., 2 WHEN OUTPUT PET WITH AGGREGATION TYPE OF SUM AND MEAN.
### TYPE		: DATA AGGREGATION TYPE, e.g., SUM, AVE, MAX, etc.
### STARTTIME	: DATA DERIVED TIME START
### ENDTIME		: DATA DERIVED TIME END
### FILENAME	: OUTPUT FILENAME WITH SUFFIX, e.g., txt, asc, tif.
```

例如，希望输出模拟时段内降水总量，则`OUTPUTID`为`D_P`,`TYPE`为`SUM`，如下：

```
### 0 HydroClimate
## Precipitation
OUTPUTID|D_P
COUNT | 1
TYPE | SUM
STARTTIME|2014-01-01 00:00:00
ENDTIME|2014-12-31 00:00:00
FILENAME | PREC_SUM.tif
```

## 2. 运行

+ SEIMS_OMP版本调用格式为：

```
seims_omp <ModelPath> <threadsNum> [<layeringMethod> <IP> <port>]
```
其中：

<ModelPath> 为模型路径，必需

<threadsNum> 为OMP并行线程数，必需，大于1

<layeringMethod> 为栅格分层方法，0或1，分别代表UP_DOWN和DOWN_UP，可选，默认为0

<IP> 为MongoDB的IP地址，可选，默认为本机

<port> 为MongoDB端口号，可选，默认为27017

>注意：<ModelPath>的文件夹名称需与MongoDB中数据库名称对应，且文件夹内需包含`config.fig`、`file.in`、`file.out`。


+ SEIMS_MPI版本调用格式为：

```
mpiexec –n 6 seims <parallel.config file>
```

其中：

-n 6 指定并行计算的进程数

<parallel.config file> 为配置文件，包含

	ModelName，ProjectPath，ModulePath，DBHost，DBPort，TreadNumber，LayeringMethod


## 3. 率定

TO BE CONTINUED！