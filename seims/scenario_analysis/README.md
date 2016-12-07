# NSGA-II for Scenario Analysis Using SEIMS

## 基于NSGA-II和SEIMS模型的情景优化

by Huiran Gao

Latest Updated：Oct.16, 2016 

----
# 目录
[**1. NSGA-II原理**](#1-NSGA-II原理)

1. [遗传算法（GA）](#i-遗传算法（GA）])

2. [非支配排序遗传算法（NSGA）](#ii-非支配排序遗传算法（NSGA）)

3. [带精英策略的非支配排序的遗传算法（NSGA-II）](#ii-带精英策略的非支配排序的遗传算法（NSGA-II）)

[**2. NSGA-II基本概念**](#2-NSGA-II基本概念)

1. [Paerot支配关系与Paerot解](#i-Paerot支配关系与Paerot解])

[**3. NSGA-II主要流程**](#3-NSGA-II主要流程)

[**4. NSGA-II for SEIMS的Python实现**](#4-NSGA-II-for-SEIMS的Python实现)

1. [文件组织结构](#i-文件组织结构])

2. [运行方式](#i-运行方式])


# 1. NSGA-II原理
## i. 遗传算法（GA）
**遗传算法(Genecit Algorihtnis, GA)**是一类借鉴生物界自然选择和自然遗传机制的随机的搜索算法,由Hollnad教授于1975年提出。其基本思想来源于达尔文(Darum)的进化论和孟德尔(Mendel)的遗传学说。达尔文的进化论认为:每个物种都是朝着越来越适应环境的方向进化的;物种的每个个体都继承了其父代的基本特征,但又不完全与之相同;更能适应环境的个体在进化过程中生存下来,其特征也被保留下来,体现了适者生存的原理。与此相应,孟德尔的遗传学认为:遗传是以基因的形式包含在染色体中的;每个基因有自己特定的位置并控制着个体的某个特性,而由这些基因控制的特性使个体对环境具有一定的适应性;通过基因杂交和基因突变可能产生对环境适应性更强的后代,通过优胜劣汰的自然选择,适应值高的基因结构被保存下来。
遗传算法正是受这两种学说的启发而发展起来的，通过计算机编程,将待求问题表示成二进制码串或数码串(也称作个体),并将产生的一群串(称作种群)置于问题的求解环境中,根据适者生存的原则,从种群中选出适应环境的串进行复制,且通过交叉!变异两种基因操作产生新的一代更适应环境的种群,经过这样一代一代地不断进化,收敛到一个最适应环境的串上,便求得了问题的最优解。

## i. 非支配排序遗传算法（NSGA）
1995年,Srinivas和Deb提出了**非支配排序遗传算法(Non一dominated Sorting Genetic Algorihtms, NSGA)**。这是一种基于Pareto最优概念的遗传算法,是众多的多目标优化遗传算法中体现Goldberg思想最直接的方法。
NSGA与简单的遗传算法的主要区别在于:该算法在选择算子执行之前根据个体之间的支配关系进行了分层。其选择算子、交叉算子和变异算子与简单遗传算法没有区别。NSGA采用的非支配分层方法,可以使好的个体有更大的机会遗传到下一代;适应度共享策略则使得准Pareto面上的个体均匀分布,保持了群体多样性,克服了超级个体的过度繁殖,防止了早熟收敛。

## i. 带精英策略的非支配排序的遗传算法（NSGA-II）
非支配排序遗传算法(NSGA)在许多问题上得到了应用,但NSGA仍存在一些问题：

1. 计算复杂度较高,为o(mn3)(m为目标函数个数,N为种群大小),所以当种群较大时,计算相当耗时；
2. 没有精英策略;精英策略可以加速算法的执行速度,在一定程度上确保己经找到的满意解不被丢失；
3. 需要指定共享半径。

**带精英策略的非支配排序的遗传算法（NSGA-II）**针对以上的缺陷通过以下三个方面进行了改进：

1. 提出了快速非支配排序法,降低了算法的计算复杂度,由原来的o(mn3)降到o(mn2)；
2. 提出了拥挤度和拥挤度比较算子,代替了需要指定共享半径的适应度共享策略,并在快速排序后的同级比较中作为胜出标准,使准Pareto域中的个体能扩展到整个Pareto域,并均匀分布,保持了种群的多样性；
3. 引入精英策略,扩大采样空间。


# 2. NSGA-II基本概念

## i. Paerot支配关系与Paerot解
多目标规划中，由于存在目标之间的冲突和无法比较的现象，一个解在某个目标上是最好的，在其他的目标上可能比较差。Pareto 在1986 年提出多目标的解不受支配解(Non-dominated set)的概念。其定义为：假设任何二解S1 及S2 对所有目标而言，S1均优于S2，则我们称S1支配S2，若S1的解没有被其他解所支配，则S1称为**非支配解（不受支配解）**，也称Pareto解。这些非支配解的集合即所谓的Pareto front。所有座落在Pareto front 中的所有解皆不受Pareto Front 之外的解（以及Pareto Front 曲线以内的其它解）所支配，因此这些非支配解较其他解而言拥有最少的目标冲突，可提供决策者一个较佳的选择空间。在某个非支配解的基础上改进任何目标函数的同时，必然会削弱至少一个其他目标函数。


# 3. NSGA-II主要流程

首先,随机初始化一个父代种群`P0`,并将所有个体按非支配关系排序且指定一个适应度值,如:可以指定适应度值等于其非支配序i,则1是最佳适应度值。然后,采用选择、交叉、变异算子产生下一代种群`Q0`,大小为N。

如图，

1. 将第t代产生的新种群`Qt`与父代`Pt`合并组成`Rt`,种群大小为2N；
2. 然后`Rt`进行非支配排序,产生一系列非支配集`Fi`并计算拥挤度；
3. 由于子代和父代个体都包含在`Rt`中,则经过非支配排序以后的非支配集名中包含的个体是`Rt`中最好的,所以先将`F1`放入新的父代种群`Pt+1`中；
4. 如果`F1`的大小小于N,则继续向`Pt+1`中填充下一级非支配集`F2`,直到添加`F4`时,种群的大小超出N,对`F4`中的个体进行拥挤度排序,取前`N-|Pt+1|`个个体,使`Pt+1`个体数量达到N；
5. 然后通过遗传算子(选择、交叉、变异)产生新的子代种群`Qt+1`。

![](http://i.imgur.com/9Scwo1B.png)

# 4. NSGA-II for SEIMS的Python实现
## i. 文件组织结构
文件结构：
![文件组织结构](http://i.imgur.com/deDus2h.jpg)

其中

`nsga2.py`是NSGA-II流程的实现代码；

`config.py`读取配置文件，主要是本地相关文件路径，相关变量的内容、取值等；

`readTextInfo.py`是读取地块划分、点源分布及相关BMPs信息等Txt文件的代码；

`userdef.py`是用户自定义的函数，包括优化目标，交叉、变异等，相关过程如交叉、变异等，添加了限制条件，符合情景优化；

`scenario.py`是情景与NSGA-II耦合的代码，将单个情景视为一个对象，具体设计实现如下表：

|属性|意义|
|---|---|
|id|scenario编号（根据MongoDB已存在的情景生成）|
|attributes|scenario的染色体编码，长度为1（农田视为一个整体）+牛场、猪场数目+污水处理点个数|
|*_Num|各点、面类型基因片段的长度|
|sce_list|染色体解码后的情景列表（单个情景的列表）|
|cost_eco|实施该情景的费用|
|benefit_env|治理效益|

|方法|功能|
|---|---|
|getIdfromMongo(self)|根据MongoDB生成该情景的编号|
|create(self)|染色体编码，生成情景编码字符串|
|decoding(self)|染色体解码，生成情景列表|
|importoMongo(self)|将情景列表导入数据库（其实可以与decoding()合并）|
|cost(self)|计算费用|
|benefit(self)|调用SEIMS模型，计算环境效益|

## ii. 运行方式

### （1）Windows下的scoop并行方式

```shell
python -m scoop -n 4 *\nsga2.py -ini *\nsgaii_dianbu2_30m_longterm_omp_gaohr_win.ini
```

### （2）Linux下提交PBS作业方式
在PBS系统中，用户使用qsub 命令提交用户程序。用户运行程序的命令及PBS环境变量设置组成PBS作业脚本，作业脚本使用如下格式提交到PBS系统运行：

```shell
[zhulj@dgpm-cluster ~]$ qsub <PBS作业脚本.sh>
```

**PBS作业脚本**
+ 注释,以“#”开头
+ PBS指令,以“#PBS”开头
+ SHELL命令

示例：

```shell
#PBS -N scenario_analysis	//指定作业名称
#PBS -l nodes=3:ppn=5		//指定程序运行的节点数
#PBS -M gaohrgao@163.com	//指定作业运行的一些状况信息发送到哪个邮箱
#PBS -o /home/zhulj/SEIMS/models/dianbu/scenario_analysis.out	//指定作业输出文件的路径
#PBS -e /home/zhulj/SEIMS/models/dianbu/scenario_analysis.err	//指定作业生成错误报告的路径

# 非必须
echo Running on hosts : $HOSTNAME  
echo The date is :  
date  
echo The directory is : $PWD
echo This job will run on the following nodes : $PBS_NODEFILE

# 下面是真正执行作业的指令，和在集群中执行的时候用的指令是一样的
python -m scoop --hostfile /home/zhulj/SEIMS/models/dianbu/dgpm_hosts_SCOOP -n 16 /home/zhulj/SEIMS/seims_omp_rel_x86-201611/scenario_analysis/nsga2.py -ini /home/zhulj/SEIMS/models/dianbu/nsgaii_dianbu2_30m_longterm_omp_dgpm.ini	
```

**优化程序参数设置**
每一代的pop数、scoop的workers、hostfile中每个节点可运行进程数（此进程等同于worker）、以及SEIMS共享内存并行进程数之间的设置关系

+ 遗传算法种群数量（Population size），应设置为4的倍数；
+ scoop的并行数（Workers),应小于等于集群hostfile中设置的所有节点可运行进程总数；
+ SEIMS并行核数与Workers的乘积，要小于集群总的可用进程数。