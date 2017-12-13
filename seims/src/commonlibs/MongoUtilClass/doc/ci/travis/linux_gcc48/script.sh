#!/bin/sh

set -e
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DUNITTEST=1 -DRUNCOV=1 ..
make -j4
make UnitTestCoverage
./test/UnitTests_Mongo
cd ..
ls
codecov -t 0a50a17e-e869-4b1c-b9ff-7eb1bc45a043 -X gcov
