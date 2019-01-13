# CCGL
Common Cross-platform Geographic-computing Library

## Build status

+ Windows_MSVC2013x64_GDAL-1.11.4-mongo-c-driver-1.5.5: [![Build status](https://ci.appveyor.com/api/projects/status/b239pv4qvvxxythk/branch/master?svg=true)](https://ci.appveyor.com/project/crazyzlj/ccgl/branch/master)
+ Linux(Ubuntu trusty)_GCC-4.8_GDAL-1.10.1-mongo-c-driver-1.6.1: [![Build Status](https://travis-ci.org/crazyzlj/CCGL.svg?branch=master)](https://travis-ci.org/crazyzlj/CCGL)
+ Code coverage: [![codecov](https://codecov.io/gh/crazyzlj/CCGL/branch/master/graph/badge.svg)](https://codecov.io/gh/crazyzlj/CCGL)

## UnitTest
+ [Google Test](https://github.com/google/googletest) Tag-[release-1.8.0](https://github.com/google/googletest/tree/release-1.8.0) is adopted as UnitTest framework. Note that other versions that higher than `release-1.8.0` will cause error C2220: warning treated as error - no 'object' file generated on MSVS 2013.
+ Unittest code should named like `test_xx.cpp` in the `test` directory.
+ To run UnitTest, use the common commands:
    ```shell
    cd <path-to-CCGL>
    mkdir build
    cd build
    cmake .. -DUNITTEST=1
    make -j4
    ./test/UnitTests_CCGL
    ```
## Subtree

CCGL use CMake for cross-platform build. The [cmake](https://github.com/lreis2415/cmake) repository is adopted as subtree.

+ Add subtree (for administrator only)

  ```
  git remote add -f cmake git@github.com:lreis2415/cmake.git
  git subtree add --prefix=cmake cmake master --squash
  ```

+ Pull updates from remote repository

  ```
  git fetch cmake master
  git subtree pull --prefix=cmake cmake master --squash
  ```

+ Push local changes to remote repository

  ```
  git subtree push --prefix=cmake cmake master
  ```
