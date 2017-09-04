# NSGA-II for Scenario Analysis Using SEIMS

## 基于NSGA-II和SEIMS模型的情景优化

by Huiran Gao, Liangjun Zhu

Latest Updated: Aug.18, 2017

----
## 1. 代码结构
情景分析主要是根据应用需求，设计多种情景，通过模型评价和优化算法选优，得到最优解集。因此，情景分析是一个具体问题具体分析的过程。

基于SEIMS和NSAG-II的情景分析工具的设计考虑通用性和特定应用，将代码组织如下：

```text
+-- __init__.py
+-- config.py: 实现基类SAConfig，读取配置文件中通用设置
+-- scenario.py: 实现基类Scenario，是模型与遗传算法耦合的部分，包括环境、经济效益计算等
+-- userdef.py: 实现用户自定义的优化算法操作基类，包括优化目标，交叉、变异等
+-- visualization.py: 实现情景空间配置可视化，独立于情景优化过程
+-- utility.py: 其他通用类、函数等
+-- slpposunits (具体情景分析应用)
|   +-- __init__.py
|   +-- config.py: 继承于SAConfig类，读取配置文件中特定应用所需的参数
|   +-- scenario.py: 继承于Scenario类，实现具体应用中情景的定制
|   +-- userdef.py: 用户自定义算法操作的补充
|   +-- main.py: NSGA-II流程的实现，执行情景分析的主程序
+-- 其他应用案例等
```

## 2. 运行方式

### 2.1. 单机环境(Windows, Linux, etc)

+ Windows, CMD

```cmd
# D:\SEIMS
cd D:\SEIMS
set PYTHONPATH=D:\SEIMS;%PYTHONPATH%
python -c "import sys; print('\n'.join(sys.path))"
python -m scoop -n 4 ./seims/scenario_analysis/slpposunits/main.py -ini <path to your config file>
```

+ Linux/Unix, shell
```bash
# $HOME/SEIMS
cd $HOME/SEIMS
export PYTHONPATH=$HOME/SEIMS:$PYTHONPATH
python -c "import sys; print('\n'.join(sys.path))"
python -m scoop -n 4 ./seims/scenario_analysis/slpposunits/main.py -ini <path to your config file>
```

### 2.1. 集群环境
集群环境下推荐使用作业管理系统进行任务提交，以PBS/Torque为例：

在PBS系统中，用户使用`qsub`命令提交作业。

```bash
$ qsub <PBS_script.sh>
```

`PBS_script.sh`为运行程序的命令及PBS环境变量设置组成PBS作业脚本。

**PBS作业脚本包括：**
+ PBS指令,以“#PBS”开头
+ 注释,以“#”开头
+ shell命令

示例：

```shell
#PBS -N scenario_analysis	//指定作业名称
#PBS -l nodes=4:ppn=5		//指定程序运行的节点数和每个节点的线程数
#PBS -W depend=afterok:n	//等待序号为n的作业进行完后，再提交改作业，可选
#PBS -o $HOME/scenario_analysis.out	//指定作业输出文件的路径
#PBS -e $HOME/scenario_analysis.err	//指定作业生成错误报告的路径

# 下面是真正执行作业的指令，和在集群中执行的时候用的指令是一样的
seimsprj=$HOME/SEIMS
modelpath=$HOME/model_youwuzhen_10m_longterm
script=$seimsprj/seims/scenario_analysis/slpposunits/main.py
cfg=$modelpath/sa_ywz_10m_longterm_omp_zhulj_dgpm.ini
stdout=$modelpath/ywz10mSA.stdout.log
cd $seimsprj
export PYTHONPATH=$seimsprj:$PYTHONPATH
# workers is default to nodes * ppn, e.g., nodes=4:ppn=5 means '-n 20'
python -m scoop -vv $script -ini $cfg > $stdout
```

## 3. 注意事项


### 3.1. 并行线程数设置

优化程序设置中，每一代的个体数（population）、scoop的workers（即-n参数）、hostfile中每个节点可运行进程数（此进程等同于worker）、以及SEIMS共享内存并行进程数之间的设置关系：

+ 遗传算法种群数量（Population size），应设置为4的倍数；
+ scoop的并行数（Workers),应小于等于集群hostfile中设置的所有节点可运行进程总数；
+ SEIMS并行线程数与Workers的乘积，要小于集群总的可用进程数。