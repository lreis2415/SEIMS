# CCGL - Common Cross-platform Geographic-computing Library (简易通用地学计算库)


[![Build on native C++](https://github.com/crazyzlj/CCGL/actions/workflows/cmake_builds.yml/badge.svg)](https://github.com/crazyzlj/CCGL/actions/workflows/cmake_builds.yml)
[![Build with GDAL](https://github.com/crazyzlj/CCGL/actions/workflows/cmake_builds_with-gdal.yml/badge.svg)](https://github.com/crazyzlj/CCGL/actions/workflows/cmake_builds_with-gdal.yml)
[![Build with MongoDB](https://github.com/crazyzlj/CCGL/actions/workflows/cmake_builds_with-mongodb.yml/badge.svg)](https://github.com/crazyzlj/CCGL/actions/workflows/cmake_builds_with-mongodb.yml)
[![Build with GDAL and MongoDB](https://github.com/crazyzlj/CCGL/actions/workflows/cmake_builds_with-gdal-mongodb.yml/badge.svg)](https://github.com/crazyzlj/CCGL/actions/workflows/cmake_builds_with-gdal-mongodb.yml)

## 单元测试
+ [Google Test](https://github.com/google/googletest) Tag-[release-1.8.0](https://github.com/google/googletest/tree/release-1.8.0) is adopted as UnitTest framework. Note that other versions that higher than `release-1.8.0` will cause error C2220: warning treated as error - no 'object' file generated on MSVS 2013.
+ Unittest code should named like `test_xx.cpp` in the `test` directory.
+ To run UnitTest, use the common commands:

  ```
  cd <path-to-CCGL>
  mkdir build
  cd build
  cmake .. -DUNITTEST=1
  make -j4
  make install
  cd ../bin
  ./UnitTests_CCGL
   ```

## Subtree

CCGL use CMake for cross-platform build. The [cmake](https://github.com/lreis2415/cmake) repository is adopted as subtree.

+ Add subtree
Open `.git/CONFIG` and append the following remote repository
information at the end of this file.

  ```
  [remote "cmake"]
      url = git@github.com:lreis2415/cmake.git
      fetch = +refs/heads/*:refs/remotes/cmake/*
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
