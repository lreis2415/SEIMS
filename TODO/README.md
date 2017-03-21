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

> 凡是模块间的传递参数，以及模拟中动态改变的参数，依旧以DT_Raster1D形式组织。
