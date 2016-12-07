
------------------------
SEIMS（Spatially Explicit Integrated Modeling System）模型是由刘军志博士、于志强博士、吴辉博士等人开发的以栅格为基本模拟单元、能体现水流空间运动的全分布式水文模型。SEIMS在借鉴Wetspa、LISEM、CASC2D、DHSVM和流溪河模型等多个模型基础上开发的模块化水文模型，采用C++编写，实现了子流域-基本单元的双层并行，采用的并行策略为共享内存（OpenMP）和消息传递（MPI），并利用NoSql数据库MongoDB进行数据组织管理。模型可在Windows及Linux平台下运行。

+ 模型开发文档主页为：http://seims.github.io/SEIMS
+ 模型Wiki主页地址为：https://github.com/seims/SEIMS/wiki

目前，模型仍在持续开发中。。。

## 对于开发者

流域过程模型设计多个过程，各过程之间通常有发生的先后顺序，也有同时发生的现象。如何在程序中对这些地理过程进行模块化划分及组合，需要我们对模型的整体结构、地理过程有一个宏观的把握。

因此，希望参与模型开发的同学能够整体了解SEIMS的基本架构，在分析解决问题过程中，能够从
>数据准备——预处理——输入——运算——输出

整体流程的角度去思考。


### 1. 知识储备

+ 1.1. [Git简明操作](Git-guidance)
+ 1.2. [Python环境搭建](https://zhulj.net/python/2016/03/18/Python-Env-For-GISer.html)及语法学习
+ 1.3. C++语法规范，可学习[Google Code Style](https://github.com/google/styleguide)

### 2. 建议步骤

+ 2.1. 详细学习[Git简明操作教程](Git-guidance)
+ 2.2. [Windows下编译SEIMS程序（OMP版本）](Windows)
+ 2.3. 学习[SEIMS模型所需准备数据](Data-preparation)，[配置预处理程序](Construct-python-env)，并成功实现[数据预处理](Data-preprocess)（即顺利构建mongoDB数据库）
+ 2.4. 按照[示例数据](Dianbu)，[配置并运行SEIMS](Executation-and-calibration)，得到结果
+ 2.5. [配置Visual Studio开发环境](Develop-environment)
+ 2.6. 配合现有模块及[SEIMS开发文档](http://seims.github.io/SEIMS/modules.html)，学习[SEIMS结构及模块开发规范](Coding-protocol)
+ 2.7. 根据[模块开发Demo](Module-demo)，完整敲一遍代码，并编译、运行、调试
+ 2.8. 根据[现有模块开发状态](Module-in-development)，结合需要，进行现有模块测试、增加新模块等
+ 2.9. 查看[SEIMS更新日志](ChangeLOG)，并将自己所做修改按日期追加

### 3. 参考文献

#### 3.1 学位论文

+ 谢军, 2015. 福建红壤区流域水土流失治理措施的情景分析--以朱溪河小流域为例. 福州大学, 福州. (硕士)
+ 吴辉, 2014. 流域最佳管理措施空间配置优化研究, 地理科学与资源研究所. 中国科学院大学, 北京. (博士)
+ 刘军志, 2013. 分布式水文模型的子流域-基本单元双层并行计算方法, 地理科学与资源研究所. 中国科学院大学, 北京. (博士)

#### 3.2 期刊文章

+ Liu, J.-Z, Zhu, A.-X., Qin, C.-Z., Wu, H., Jiang, J.-C, 2016. A two-level parallelization method for distributed hydrological models. Environmental Modelling & Software 80, 175-184.
+ Liu, J.-Z, Zhu, A.-X., Liu, Y.-B, Zhu, T.-X, Qin, C.-Z., 2014. A layered approach to parallel computing for spatially distributed hydrological modeling. Environmental Modelling & Software 51, 221-227.
+ 吴辉, 刘永波, 刘军志, 朱阿兴, 2014. 农业最佳管理措施在全分布式水文模型中的表达--以罗玉沟流域为例. 资源与生态学报: 英文版, 179-184.
+ 刘军志, 朱阿兴, 刘永波, 秦承志, 陈腊娇, 吴辉, 杨琳, 2013. 基于栅格分层的逐栅格汇流算法并行化研究. 国防科技大学学报 35, 123-129.



>文档编写： 朱良君、高会然
>
>更新时间： 2016-5-30