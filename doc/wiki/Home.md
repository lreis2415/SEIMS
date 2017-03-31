## 1.基本信息
SEIMS（Spatially Explicit Integrated Modeling System）模型是以栅格为基本模拟单元、能体现水流空间运动的全分布式水文模型，在借鉴SWAT、Wetspa、LISEM、CASC2D、DHSVM和流溪河模型等多个模型基础上开发的模块化水文模型，采用C++编写，采用共享内存（OpenMP）和消息传递（MPI）实现了子流域-基本单元的双层并行计算，并利用NoSql数据库MongoDB进行数据组织管理。

SEIMS模型可通过常用C++编译器（如MSVC、GCC、Clang）编译成32位或64位程序，并运行在常见操作系统中（如Windows、Linux、macOS），master主分支在多个平台下的编译状态：

[![Build status](https://ci.appveyor.com/api/projects/status/i3mxjy0wjgphcyu1/branch/master?svg=true)](https://ci.appveyor.com/project/lreis-2415/seims/branch/master) [![Build Status](http://badges.herokuapp.com/travis/lreis2415/SEIMS?branch=master&env=BUILD_NAME=linux_gcc48&label=linux_gcc48)](https://travis-ci.org/lreis2415/SEIMS) [![Build Status](http://badges.herokuapp.com/travis/lreis2415/SEIMS?branch=master&env=BUILD_NAME=osx_xcode&label=osx_clang)](https://travis-ci.org/lreis2415/SEIMS)

目前，模型仍在持续开发、完善中，我们欢迎任何建议和反馈（通过issue、push request或email等方式）。

## 2.对于开发者

流域过程模型涉及多个子过程，各子过程之间通常有发生的先后顺序，也有同时发生的现象。如何在程序中对这些地理过程进行模块化划分及组合，需要我们对模型的整体结构、地理过程有一个宏观的把握。

因此，希望参与模型开发的同学能够整体了解SEIMS的基本架构，在分析解决问题过程中，能够从
>数据准备——预处理——输入——运算——输出

这个整体流程的角度去思考。


### 2.1. 知识储备

+ 2.1.1. [Git简明操作](Git-guidance)
+ 2.1.2. [Python环境搭建](https://zhulj.net/python/2016/03/18/Python-Env-For-GISer.html)及语法学习
+ 2.1.3. C++语法学习
+ 2.1.4. 代码规范，可学习[Google Code Style](https://github.com/google/styleguide)

### 2.2. 建议步骤

+ 2.2.1. 详细学习[Git简明操作教程](Git-guidance)，重点掌握**冲突管理**和**Git分支管理**；
+ 2.2.2. 根据实际情况选择对应操作系统对SEIMS进行编译、安装，只需编译OpenMP版本即可，[Windows](Windows)、[Linux](Linux)、[macOS](macOS)；
+ 2.2.3. 学习[SEIMS模型所需准备数据](Data-preparation)，[配置预处理程序](Construct-python-env)，并成功实现[数据预处理](Data-preprocess)（即顺利构建MongoDB数据库）；
+ 2.2.4. 按照[示例数据](Dianbu)，[配置并运行SEIMS](Executation-and-calibration)，得到结果；
+ 2.2.5. 接下来就可以自己搭建SEIMS开发环境，[以Visual Studio和PyCharm为例](Develop-environment)，并[阅读开发规范](Coding-protocol)；
+ 2.2.6. 配合现有模块及[SEIMS开发文档](https://lreis2415.github.io/SEIMS/)，学习SEIMS结构，尤其是数据组织方式、模型运行流程等；
+ 2.2.7. 根据[模块开发Demo](Module-demo)，熟悉模块、函数、变量的命名及[名词缩写规范](Global-abbreviation)，完整敲一遍代码，并编译、运行、调试；
+ 2.2.8. 根据[现有模块开发状态](Module-in-development)，结合需要，进行现有模块测试、增加新模块等，**请注意**这些工作都是在自定义分支下进行的，如`moduletest_musk`；
+ 2.2.9. 根据示例数据（或补充数据）进行模块测试，确定代码无误后，合并分支至`dev`，并新建`issue`供模型开发组讨论。

### 3. 参考文献

#### 3.1 学位论文

+ 谢军, 2015. 福建红壤区流域水土流失治理措施的情景分析--以朱溪河小流域为例. 福州大学, 福州. (硕士)
+ 吴辉, 2014. 流域最佳管理措施空间配置优化研究, 地理科学与资源研究所. 中国科学院大学, 北京. (博士)
+ 刘军志, 2013. 分布式水文模型的子流域-基本单元双层并行计算方法, 地理科学与资源研究所. 中国科学院大学, 北京. (博士)

#### 3.2 期刊文章

+ 吴辉, 刘永波, 秦承志, 刘军志, 江净超, 朱阿兴, 2016. [流域最佳管理措施情景优化算法的并行化](http://www.cnki.net/kcms/doi/10.13203/j.whugis20140048.html). 武汉大学学报(信息科学版). 41(2): 202–207.
+ Liu, J.-Z, Zhu, A.-X., Qin, C.-Z., Wu, H., Jiang, J.-C, 2016. [A two-level parallelization method for distributed hydrological models](http://dx.doi.org/10.1016/j.envsoft.2016.02.032). Environmental Modelling & Software 80, 175-184.
+ Liu, J.-Z, Zhu, A.-X., Liu, Y.-B, Zhu, T.-X, Qin, C.-Z., 2014. [A layered approach to parallel computing for spatially distributed hydrological modeling](http://dx.doi.org/10.1016/j.envsoft.2013.10.005). Environmental Modelling & Software 51, 221-227.
+ 吴辉, 刘永波, 刘军志, 朱阿兴, 2014. [农业最佳管理措施在全分布式水文模型中的表达--以罗玉沟流域为例](http://www.cnki.net/kcms/detail/detail.aspx?dbcode=CJFD&filename=JORE201402011&dbname=CJFD2014&uid=WEEvREcwSlJHSldRa1FhcEE0NXdnZ3lnczMwU2hwUHdrU0I0MkQzNUsyUT0=$9A4hF_YAuvQ5obgVAqNKPCYcEjKensW4ggI8Fm4gTkoUKaID8j8gFw!!). 资源与生态学报: 英文版, 179-184.
+ 刘军志, 朱阿兴, 刘永波, 秦承志, 陈腊娇, 吴辉, 杨琳, 2013. [基于栅格分层的逐栅格汇流算法并行化研究](http://www.cnki.net/kcms/detail/detail.aspx?dbcode=CJFD&filename=GFKJ201301023&dbname=CJFD2013&uid=WEEvREcwSlJHSldRa1FhcEE0NXdnZ3lnczMwU2hwUHdrU0I0MkQzNUsyUT0=$9A4hF_YAuvQ5obgVAqNKPCYcEjKensW4ggI8Fm4gTkoUKaID8j8gFw!!). 国防科技大学学报 35, 123-129.
