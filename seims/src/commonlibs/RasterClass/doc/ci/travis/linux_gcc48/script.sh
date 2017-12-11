#!/bin/sh

set -e
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DUNITTEST=1 -DRUNCOV=1 ..
make -j4
make UnitTestCoverage
./test/UnitTests_Raster
cd ..
ls
codecov -t 0537586e-469d-4215-a244-8fb35a93028f -X gcov
