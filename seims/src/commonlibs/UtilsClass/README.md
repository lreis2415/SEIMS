# UtilsClass
------------
Utilities class, including operations of string, datetime, etc.

Selected build environments:

+ Windows-MSVC 2013-64bit: [![Build status](https://ci.appveyor.com/api/projects/status/b3eu2hfca1mte3ta?svg=true)](https://ci.appveyor.com/project/lreis-2415/utilsclass)
+ Linux(Ubuntu trusty)-GCC-4.8: [![Build Status](http://badges.herokuapp.com/travis/lreis2415/UtilsClass?branch=master&env=BUILD_NAME=linux_gcc48&label=linux_gcc48)](https://travis-ci.org/lreis2415/UtilsClass)
+ macOS-Clang-7.3: [![Build Status](http://badges.herokuapp.com/travis/lreis2415/UtilsClass?branch=master&env=BUILD_NAME=osx_xcode&label=osx_clang)](https://travis-ci.org/lreis2415/UtilsClass)

Code coverage: [![codecov](https://codecov.io/gh/lreis2415/UtilsClass/branch/master/graph/badge.svg)](https://codecov.io/gh/lreis2415/UtilsClass)

## 单元测试
+ UtilsClass采用CMake进行跨平台编译。
+ UtilsClass采用[Google Test](https://github.com/google/googletest)单元测试框架。
+ 所有单元测试代码统一存放在`test`文件夹下，并以`Test_XX.cpp`格式命名。
+ 通用编译命令
    ```shell
    cd <path-to-UtilsClass>
    mkdir build
    cd build
    cmake .. -DUNITTEST=1
    make
    ./test/UnitTests_Utils
    ```
+ 强烈推荐CLion，直接打开UtilsClass目录并在CMake Options中添加`-DUNITTEST=1`，
即可自动构建工程，方便且跨平台。