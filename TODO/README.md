# Github项目管理方面的TODO List

+ 1.跨平台一键安装脚本 （可参照程序自动测试脚本，即`.travis.yml`和`appveyor.yml`）
	+ 1.1.检查环境变量，是否满足模型依赖，不满足，则自动下载、安装，包括GDAL、mongo-c-driver以及MongoDB服务；
	+ 1.2.编译并安装SEIMS模型；
	+ 1.3.运行测试脚本检查模型功能是否正常（参照[RHESSys](https://github.com/RHESSys/RHESSys)），并加入CI自动构建脚本中。

+ 2.~~程序自动测试（持续集成服务）~~**DONE By ZhuLJ on 2017-3-20**

	~~完成一键安装脚本及测试脚本之后，可利用[Travis CI](https://travis-ci.org/)及[AppVeyor](https://www.appveyor.com/)构建多平台下的代码测试，这样做的最大好处就是：~~

	~~**多人协同开发过程中，不必每次提交总库都检查一遍代码，交由Travis CI和AppVeyor去完成吧！**~~

+ 3.~~Doxygen代码文档自动更新~~ **DONE By ZhuLJ on 2017-3-20**

	~~同样是采用Travis CI的持续集成服务，doxygen配置文件位于`doc/doxygen`，配置模型根目录下的`.travis.yml`实现自动更新基于gh-pages的代码文档，详见[旧版本的SEIMS示例](http://seims.github.io/SEIMS/)，还有很大的完善空间。~~

+ 4.输入数据去冗余

目前，土地利用、植被覆盖等查找表属性数据均以DT_Raster1D形式准备和存储，运行时会占据较多内存，应该修改为使用查找表方式。

> 凡是模块间的传递参数，以及模拟中动态改变的参数，依旧以`DT_Raster1D`或`DT_Raster2D`形式组织。

+ 5.流域离散化时，自动合并面积非常小（如几个栅格）的子流域至下游或平级相邻子流域中，代码补充至`sd_delineation.py`，参考QSWAT（同样为开源python代码）。

+ 6.模块智能组合调整，目前SEIMS模块组织顺序依赖于输入文件`config.fig`，而一旦模块顺序出错，或者模块缺失，都会导致模拟失真或者模型出错。因此需要在`ModuleFactory`组织模块时进行智能调整。

+ 7.地块划分程序field_partition，目前存在内存泄漏问题没有解决，主要是Field和Cell两个类的问题。

+ 8.增加单元测试代码 **UPDATE By ZhuLJ on 2017-3-31**
~~利用[DownloadProject](https://github.com/Crascit/DownloadProject)在CMake构建工程的时候自动克隆[GoogleTest](https://github.com/google/googletest)，并在项目编译之前自动编译，从而可供UnitTest自动构建、测试。在CMake命令后追加`-DUNITTEST=1`开启单元测试。参考[这篇博客](https://crascit.com/2015/07/25/cmake-gtest/)~~。
目前，基于gtest/gmock的单元测试框架已经搭建好，后续写模块的过程中应当同步写单元测试代码！
+ 9.增加flood模拟方法。如openLISEM
+ 10.增加泥沙汇流模块，采用水流功率模型计算，目前需将SWAT中的4种模型集成进来：
    + ~~Simplified Bagnold model (DONE)~~
    + Kodatie model (for streams with bed material size ranging from silt to gravel)
    + Molinas and Wu model (for primarily sand size particles)
    + Yang sand and gravel model (for primarily sand and gravel size particles)


