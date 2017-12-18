# SEIMS
-------

Selected build environments：

+ Windows-MSVC 2013-64bit with MSMPI-v8: [![Build status](https://ci.appveyor.com/api/projects/status/i3mxjy0wjgphcyu1/branch/master?svg=true)](https://ci.appveyor.com/project/lreis-2415/seims/branch/master) 
+ Linux(Ubuntu trusty)-GCC-4.8 with MPICH2-3.0.4: [![Build Status](http://badges.herokuapp.com/travis/lreis2415/SEIMS?branch=master&env=BUILD_NAME=linux_gcc48&label=linux_gcc48)](https://travis-ci.org/lreis2415/SEIMS)

## 1.Brief introduction

The **Spatially Explicit Integrated Modeling System** (**SEIMS**), is an integrated, modular, parallellized, fully-distributed, and continuous Watershed modeling and scenario analysis system.

SEIMS is mainly written by **C++** with support of [GDAL](https://github.com/OSGeo/gdal), [Mongo-C-Driver](https://github.com/mongodb/mongo-c-driver), [OpenMP](https://en.wikipedia.org/wiki/OpenMP) and/or [MPI](https://en.wikipedia.org/wiki/Message_Passing_Interface), while **Python** is used for organizing the preprocessing, postprocessing, scenario analysis, etc. workflows. SEIMS is intented to be an open-source, cross-platform, and high performaced integrated watershed modeling system. Theoretically, SEIMS could be compiled by common used compiler (e.g. MSVC, GCC, and Clang) as 32-bit or 64-bit programs and run on any mainstream OS (e.g. Windows, Linux, and macOS).

SEIMS contains several module catogories, include **Hydrology, Erosion, Nutrient, Plant Growth, BMP Management**, etc. Algorithms are integrated from SWAT, LISEM, WetSpa Extension, DHSVM, CASC2D, etc.

SEIMS is still being developing and any constructive feedback (issues or push requests) will be welcome and appreciated.

## 2.Wiki
Currently, only Chinese-version wiki is provided and hosted on Github, English-version will be available in the near future. More information on [SEIMS Wiki](https://github.com/lreis2415/SEIMS/wiki).

## 3.Get Started
### 3.1.Get source code

+ Download or clone from [Github](https://github.com/lreis2415/SEIMS). A useful Github guidiance can be found [here](https://github.com/lreis2415/SEIMS/wiki/Git-guidance).
+ Read the [basic code structure](https://github.com/lreis2415/SEIMS/blob/master/seims/README.md).

### 3.2.Compile and Install

+ [Windows](https://github.com/lreis2415/SEIMS2017/wiki/Windows)
+ [Linux](https://github.com/lreis2415/SEIMS2017/wiki/Linux)
+ [macOS](https://github.com/lreis2415/SEIMS2017/wiki/macOS)

Generally, with the required dependencies (i.e., cmake, C++ compiler, GDAL, mongo-c-driver), the compilation and installation should follows:

```shell
cd <path-to-SEIMS>
mkdir build
cd build
cmake ..
make -j4
make install
```

Besides, Python 2.7+ with Numpy, GDAL, pymongo, matplotlib, etc. is also required.

### 3.3.Config MongoDB database

+ [Install MongoDB client and start mongo service automatically.](https://github.com/lreis2415/SEIMS2017/wiki/MongoDB-install-and-config)
+ You may need a MongoDB IDE to view and edit data. MongoVUE, Robo 3T (formerly Robomongo), or Mongo Explorer for JetBrains (e.g. PyCharm, CLion) are recommended.

### 3.4.Run the Demo data
Demo data is provided in `~/data/dianbu2`. The common steps for testing SEIMS with the demo data are as follows:

```shell
cd SEIMS
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
make install
cd ..
# Run demo data, e.g., dianbu2 watershed
python seims/test/demo_dianbu2_preprocess.py
python seims/test/demo_dianbu2_runmodel.py
python seims/test/demo_dianbu2_postprocess.py
# TO BE CONTINUED...
```

### 3.5.Build your own model
Now, you can build you own SEIMS model!

SEIMS workflow can be summarized as five part.

+ a) [Data preparation](https://github.com/lreis2415/SEIMS2017/wiki/Data-preparation) and [Preprocessing for SEIMS](https://github.com/lreis2415/SEIMS2017/wiki/Data-preprocess)
+ b) [Run SEIMS model](https://github.com/lreis2415/SEIMS2017/wiki/Executation-and-calibration)
+ c) [Postprocess, e.g. export hydrograph](https://github.com/lreis2415/SEIMS2017/wiki/result-postprocess)
+ d) Model calibration
+ e) Scenario analysis


Contact Us
----------
Dr.Junzhi Liu (liujunzhi@njnu.edu.cn)

Liangjun Zhu (zlj@lreis.ac.cn)

*Updated: 2017-12-7*
