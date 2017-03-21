Python环境配置
--------------------------

by Huiran Gao, Liang-Jun Zhu

Latest Updated：May.31, 2016 

--------------------------

# 目录

[**1. GDAL（独立于Python）安装**](#1-gdal（独立于python）安装)

[**2. Python脚本依赖库安装**](#2-python脚本依赖库安装)

1. [pip安装](#i-pip安装)

2. [使用pip安装whl依赖库](#ii-使用pip安装whl依赖库)

[**3. 最好用的Python编辑器之一--PyCharm介绍**](#3-最好用的python编辑器之一--pycharm介绍)

# 1. GDAL（独立于Python）安装

下载并安装gdal-111-1600-x64-core.msi

+ 链接一: [http://pan.baidu.com/s/1c0saqQW](http://pan.baidu.com/s/1c0saqQW) 提取码: ggbj 
+ 链接二：[http://download.gisinternals.com/sdk/downloads/release-1600-x64-gdal-1-11-1-mapserver-6-4-1/gdal-111-1600-x64-core.msi](http://download.gisinternals.com/sdk/downloads/release-1600-x64-gdal-1-11-1-mapserver-6-4-1/gdal-111-1600-x64-core.msi) 

下载完成后，安装GDAL，一路 Next 就可以，安装路径设为`C:\Program Files\GDAL`，完成后，在环境变量里新建GDAL_DATA项，值设置为`C:\Program Files\GDAL\gdal-data`

[返回目录](#目录)

# 2. Python脚本依赖库安装

SEIMS预处理程序Python (python 2.7版本)依赖包集合下载：

+ Python x64, [百度云盘](http://pan.baidu.com/s/1dF9GiYl), 提取码：z2ia
+ Python x32, [百度云盘](http://pan.baidu.com/s/1qY7XNw4), 提取码：6cap

> 注意，这里的64位或者32位指的是Python的版本，而非Windows版本

## i. pip安装

+ 离线安装pip

	离线安装pip需要下载setuptools和pip并解压

	随后，管理员方式运行cmd，输入以下命令：

	```shell
	cd <setuptools path>
	python setup.py install
	cd <pip path>
	python setup.py install
	```

+ 在线安装pip，从[这里](https://bootstrap.pypa.io/get-pip.py)下载`get-pip.py`

	```python
	python get-pip.py
	```

安装好pip，就可以离线安装whl格式的python第三方库了，whl库的下载地址为：[http://www.lfd.uci.edu/~gohlke/pythonlibs/](http://www.lfd.uci.edu/~gohlke/pythonlibs/)。

## ii. 使用pip安装whl依赖库

+ GDAL（依赖于Python）安装
  通过上面下载whl库的链接，可以下载完整的GDAL打包文件 `GDAL-1.11.3-cp27-none-win_amd64.whl`(64位，注意32位和64位的区分)，管理员方式运行cmd，输入安装命令：

  ```shell
  cd <GDAL path>
  pip install GDAL-1.11.3-cp27-none-win_amd64.whl
  ```

  提示安装成功即可，输入以下命令验证是否安装成功：

  ```python
  python
  from osgeo import gdal
  from osgeo import ogr
  from osgeo import osr
  from osgeo import gdal_array
  from osgeo import gdalconst
  ```
+ whl依赖库的批量安装
  将下载的whl库文件(比如scipy，numexpr，PyQt4，wxPython（需要wxPython_common），PyGTK，python_dateutil，pytz，matplotlib，pywin32…)放到某一英文目录下，然后新建`install.bat`文件，输入如下内容：

> 注意修改python路径：`PYPATH`
> install.bat文件应与whl库文件放在同一文件夹。

```shell
@echo off
@echo off
echo "Liangjun Zhu, zlj@lreis.ac.cn"
echo "-- Install Python Packages Using Pip..."
echo "-- WHL files downloaded from http://www.lfd.uci.edu/~gohlke/pythonlibs/"
pushd %~dp0
cd %~dp0
:: python path, x86 or x64 versioned python
set PYPATH=C:/python27x86/
set PIPPYTH=%PYPATH%Scripts/
:: COMMENT-1: Install easy_install and pip without network
::%PYPATH%python %~dp0\setuptools-18.2\setup.py install
::%PYPATH%python %~dp0\pip-7.1.2\setup.py install
:: COMMENT-2: Install easy_install and pip without network
::%PYPATH%python get-pip.py
for /f "delims=" %%i in ('dir /s/b "*.whl"') do ( 
echo installing %%~ni ...
%PIPPYTH%pip install %%i 
)
echo "-- All packages installed Succeed!"
pause
```

这样便会自动安装该文件夹所有的packages...


[返回目录](#目录)

# 3. 最好用的Python编辑器之一--PyCharm介绍

PyCharm是一种Python IDE，带有一整套可以帮助用户在使用Python语言开发时提高其效率的工具，比如调试、语法高亮、Project管理、代码跳转、智能提示、自动完成、单元测试、版本控制。


特点：

+ 编码协助
  
	提供了一个带编码补全，代码片段，支持代码折叠和分割窗口的智能、可配置的编辑器;
+ 项目代码导航

	帮助用户即时从一个文件导航至另一个，从一个方法至其申明或者用法甚至可以穿过类的层次;
+ 代码分析
	用户可使用其编码语法，错误高亮，智能检测以及一键式代码快速补全建议;
+ Python重构
	在项目范围内轻松进行重命名，提取方法/超类，导入域/变量/常量，移动和前推/后退重构。
+ ......

下载链接：[百度云盘](http://pan.baidu.com/s/1i5nnvNJ)，提取码：gyy2

[返回目录](#目录)







 