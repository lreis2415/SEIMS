# PyGeoC
***Py***thon for ***GeoC***omputation --- easy, convenient, and efficient.

[![Build Status](https://travis-ci.org/crazyzlj/PyGeoC.svg?branch=master)](https://travis-ci.org/crazyzlj/PyGeoC)

## 1.Introduction

本科快毕业的时候，接触并开始学习Python和arcpy（当时ArcGIS 9时代还是arcgisscripting）已有五六年时间了，当时写了一篇文章《[[初学入门]ArcGIS中Python脚本的使用](http://www.docin.com/p-164282884.html)》，在各个网站、空间被转载多次。

随着学习的深入，逐渐接触了Numpy, GDAL, Matplotlib等一系列优秀Python包。在日常学习工作中，Python早已成为我进行简单、复杂科学计算/地学计算的编程工具，也积累了一些常用的地学计算代码。

很久以来，我就有一个念头，就是构建一个适用于地学计算的基础Python库，这个库可以包含一些现有的开源代码，但一定要保证具有以下特点：

+ 配置方便
+ 简单易学
+ 高效易用

## 2.Download and Installation

PyGeoC采用纯Python开发（仅在Python2.x下测试），利用[Wheels](http://pythonwheels.com/)打包[发布](http://python-packaging-user-guide.readthedocs.org/en/latest/distributing/#working-in-development-mode)，可从PyPI项目主页下载。
也可从[PyGeoC](https://github.com/crazyzlj/PyGeoC)库的dist目录中下载。

    ```python
    1.source distribution:
    python setup.py sdist
    2.wheel
    python setup.py bdist_wheel
    3.wheel(py27, I just test in py27 currently)
    python setup.py bdist_wheel --python-tag py2
    ```

安装：
+ 确认本机已经安装python2.6+
+ 安装pip: please refer to the [installation page
](https://pip.pypa.io/en/latest/installing/)
+ `pip install <path to PyGeoC .whl>/PyGeoC<version>.whl`

## 3.Structure