#!/bin/sh

set -e
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DUNITTEST=1 ..
make -j4
./test/UnitTests_Utils
cd ..
ls
