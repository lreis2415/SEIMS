About {#mainpage}
=====================

Copyright (C) 2018 [Lreis](http://www.lreis.ac.cn), [IGSNRR](http://english.igsnrr.cas.cn), [CAS](http://english.cas.cn), and [NJNU](http://en.njnu.edu.cn). All rights reserved.

* [SEIMS GitHub](https://github.com/lreis2415/SEIMS)
* SEIMS Documentations (**Not finished yet!**)
  * Online versions
    * [English (powered by Doxygen)](https://lreis2415.github.io/SEIMS/), [English without API references (powered by Gitbook)](https://crazyzlj.gitbooks.io/seims-documentation-version-2018/content/en/)
    * [简体中文（由Doxygen驱动）](https://lreis2415.github.io/SEIMS/zh-cn/), [简体中文，不含 API 参考手册（由Gitbook驱动）](https://crazyzlj.gitbooks.io/seims-documentation-version-2018/content/zh-cn/)
  * Download PDF versions without API references (powered by [Gitbook](https://www.gitbook.com/))
    * [English](https://legacy.gitbook.com/download/pdf/book/crazyzlj/seims-documentation-version-2018?lang=en)
    * [简体中文](https://legacy.gitbook.com/download/pdf/book/crazyzlj/seims-documentation-version-2018?lang=zh-cn)

# Brief introduction {#BriefIntroduction}

The **Spatially Explicit Integrated Modeling System** (**SEIMS**), is an integrated, modular, parallelized, fully-distributed watershed modeling and scenario analysis system.

SEIMS is mainly written by C++ with support of [GDAL](https://github.com/OSGeo/gdal), [Mongo-C-Driver](https://github.com/mongodb/mongo-c-driver), [OpenMP](https://en.wikipedia.org/wiki/OpenMP) and/or [MPI](https://en.wikipedia.org/wiki/Message_Passing_Interface), while Python is used for organizing the utility tools such as preprocessing, postprocessing, auto-calibration, and scenario analysis, etc. SEIMS is designed to be an open-source, cross-platform, and high performaced integrated watershed modeling system. Theoretically, SEIMS could be compiled by common used compiler (e.g. MSVC 2010+, GCC 4.6+, and Intel C++ 12.0+) as 32-bit or 64-bit programs and run on any mainstream OS (e.g. Windows, Linux, and macOS).

SEIMS contains several module catogories, include **Hydrology, Erosion, Nutrient, Plant Growth, BMP Management**, etc. Algorithms are integrated from SWAT, LISEM, WetSpa Extension, DHSVM, CASC2D, etc.

SEIMS is still being developing and any constructive feedback (issues or push requests) will be welcome and appreciated.

# Get Started {#GetStarted}
## Get Source Code {#GetSourceCode}

+ Download or clone from [Github](https://github.com/lreis2415/SEIMS). A useful Github guidiance can be found [here](https://github.com/lreis2415/SEIMS/wiki/Git-guidance).
+ Read the [basic code structure](https://github.com/lreis2415/SEIMS/blob/master/seims/README.md).

## Compile & Install {#CompileInstall}

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

## Config MongoDB {#ConfigMongoDB}

+ [Install MongoDB client and start mongo service automatically.](https://github.com/lreis2415/SEIMS2017/wiki/MongoDB-install-and-config)
+ You may need a MongoDB IDE to view and edit data. MongoVUE, Robo 3T (formerly Robomongo), or Mongo Explorer for JetBrains (e.g. PyCharm, CLion) are recommended.

## Run Demo Data {#RunDemoData}
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

## Build Your Own Model {#BuildYourOwnModel}
Now, you can build you own SEIMS model!

SEIMS workflow can be summarized as five part.

+ a) [Data preparation](https://github.com/lreis2415/SEIMS2017/wiki/Data-preparation) and [Preprocessing for SEIMS](https://github.com/lreis2415/SEIMS2017/wiki/Data-preprocess)
+ b) [Run SEIMS model](https://github.com/lreis2415/SEIMS2017/wiki/Executation-and-calibration)
+ c) [Postprocess, e.g. export hydrograph](https://github.com/lreis2415/SEIMS2017/wiki/result-postprocess)
+ d) Model calibration
+ e) Scenario analysis


# Contact Us {#ContactUs}
+ Dr. Liang-Jun Zhu (zlj@lreis.ac.cn)
+ Asso. Prof. Junzhi Liu (liujunzhi@njnu.edu.cn)


