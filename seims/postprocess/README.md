seims.postprocess package {#intro_postprocess_pkg}
==================================================

TODO.


后处理程序采用与预处理程序一致的调用方式，即通过配置文件（*.ini）指定程序运行参数，通过如下形式命令调用任意一个功能脚本：

```bash
cd <path to>/SEIMS
python -m seims.postprocess.main -ini XXX.ini
```
以下分别介绍后处理程序功能及参数设置。

## 模拟结果出图
+ 相关参数设置：
	+ [PATH]下的MODEL_DIR，该目录中需有OUTPUT文件夹
	+ [MONGODB]所有参数
	+ [PARAMETERS]下的PLOT_VARIABLES，用于定于输出图标的变量，无需加引号，需与OUTPUT目录下TXT文件名一致，如Q, SED, CH_TN, 等，多个名称用空格分割
	+ [OPTIONAL_PARAMETERS]：Time_start 与Time_end 设定作图起止日期，格式为`yyyy-mm-dd`
