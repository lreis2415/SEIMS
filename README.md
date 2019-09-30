## SEIMS: A modular and parallelized watershed modeling framework

Copyright (C) 2013-2019 [Lreis](http://www.lreis.ac.cn), [IGSNRR](http://english.igsnrr.cas.cn), [CAS](http://english.cas.cn), and [NJNU](http://en.njnu.edu.cn). All rights reserved.

* [SEIMS GitHub](https://github.com/lreis2415/SEIMS)
* SEIMS Documentations
  * [PDF version](SEIMS-UserManual.pdf)
  * Online versions (**Will coming soon!**)
    * [English (powered by Doxygen)](https://lreis2415.github.io/SEIMS/), [English without API references (powered by Gitbook)](https://crazyzlj.gitbooks.io/seims-documentation-version-2018/content/en/), and [Gitbook PDF](https://legacy.gitbook.com/download/pdf/book/crazyzlj/seims-documentation-version-2018?lang=en)
    * [简体中文（由Doxygen驱动）](https://lreis2415.github.io/SEIMS/zh-cn/), [简体中文，不含 API 参考手册（由Gitbook驱动）](https://crazyzlj.gitbooks.io/seims-documentation-version-2018/content/zh-cn/), and [Gitbook PDF](https://legacy.gitbook.com/download/pdf/book/crazyzlj/seims-documentation-version-2018?lang=zh-cn)

## Build Status

+ Windows-MSVC 2013-64bit with MSMPI-v8: [![Build status](https://ci.appveyor.com/api/projects/status/i3mxjy0wjgphcyu1/branch/master?svg=true)](https://ci.appveyor.com/project/lreis-2415/seims/branch/master)
+ Linux(Ubuntu xenial)-GCC-5.4.0 with OpenMPI-1.10.2: [![Build Status](http://badges.herokuapp.com/travis/lreis2415/SEIMS?branch=master&env=BUILD_NAME=linux_gcc&label=linux_gcc)](https://travis-ci.org/lreis2415/SEIMS)
+ macOS(10.13.3)_AppleClang-10.0 with GDAL-2.3.1, mongo-c-driver-1.14.0, and OpenMPI-4.0.1: [![Build Status](http://badges.herokuapp.com/travis/lreis2415/SEIMS?branch=master&env=BUILD_NAME=osx_clang&label=osx_clang)](https://travis-ci.org/crazyzlj/CCGL)

## Brief Introduction

The **Spatially Explicit Integrated Modeling System** (**SEIMS**), is a lightweight, modular, and parallelized watershed modeling framework for watershed modeling and scenario analysis.

SEIMS is mainly written by C++ with support of [GDAL](https://github.com/OSGeo/gdal), [Mongo-C-Driver](https://github.com/mongodb/mongo-c-driver), [OpenMP](https://en.wikipedia.org/wiki/OpenMP) and [MPI](https://en.wikipedia.org/wiki/Message_Passing_Interface), while Python is used for organizing the preprocessing, postprocessing, scenario analysis, etc. workflows. Theoretically, SEIMS could be compiled by common used compiler (e.g. MSVC 2010+, GCC 4.6+, and Intel C++ 12.0+) as 32-bit or 64-bit programs and run on any mainstream OS (e.g. Windows, Linux, and macOS).

SEIMS contains several module categories, include **Hydrology, Erosion, Nutrient, Plant Growth, BMP Management**, etc. Algorithms are integrated from SWAT, LISEM, WetSpa Extension, DHSVM, CASC2D, etc.

SEIMS is still being developing and any constructive feedback (issues or push requests) will be welcome and appreciated.

## Selected peer-reviewed papers

### Watershed modeling framework

+ Zhu, L.-J., Liu, J., Qin, C.-Z., Zhu, A-X., 2019. [A modular and parallelized watershed modeling framework](http://www.sciencedirect.com/science/article/pii/S1364815218309241). Environmental Modelling & Software 122, 104526. doi:10.1016/j.envsoft.2019.104526
+ Liu, J., Zhu, A-X., Qin, C.-Z., Wu, H., Jiang, J., 2016. [A two-level parallelization method for distributed hydrological models](http://dx.doi.org/10.1016/j.envsoft.2016.02.032). Environmental Modelling & Software 80, 175-184.
+ Liu, J., Zhu, A-X., Liu, Y., Zhu, T., Qin, C.-Z., 2014. [A layered approach to parallel computing for spatially distributed hydrological modeling](http://dx.doi.org/10.1016/j.envsoft.2013.10.005). Environmental Modelling & Software 51, 221-227.
+ Liu, J., Zhu, A-X., Qin, C.-Z., 2013. [Estimation of theoretical maximum speedup ratio for parallel computing of grid-based distributed hydrological models](https://doi.org/10.1016/j.cageo.2013.04.030). Computers & Geosciences 60, 58–62.

### BMPs scenario analysis

+ Zhu, L.-J., Qin, C.-Z., Zhu, A-X., Liu, J., Wu, H., 2019. [Effects of different spatial configuration units for the spatial optimization of watershed best management practice scenarios](https://doi.org/10.3390/w11020262). Water, 11(2), 262.
+ Qin, C.-Z., Gao, H.-R., Zhu, L.-J., Zhu, A-X., Liu, J.-Z., Wu, H., 2018. [Spatial optimization of watershed best management practices based on slope position units](https://doi.org/10.2489/jswc.73.5.504). Journal of Soil and Water Conservation 73(5), 504-517.
+ Wu, H., Zhu, A-X., Liu, J., Liu, Y., Jiang, J., 2018. [Best Management Practices Optimization at Watershed Scale: Incorporating Spatial Topology among Fields](https://doi.org/10.1007/s11269-017-1801-8). Water Resource Management 32, 155–177.

## Support

SEIMS is an open source software. Support is provided through the Github issues and Email of present developers.

+ SEIMS issues: https://github.com/lreis2415/SEIMS/issues
+ Emails of present developers:
  + Dr. Liang-Jun Zhu (zlj@lreis.ac.cn)
  + Asso. Prof. Junzhi Liu (liujunzhi@njnu.edu.cn)

