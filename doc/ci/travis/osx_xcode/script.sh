#!/bin/sh

set -e
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DUNITTEST=1 ..
make
./test/UnitTests_Mongo
cd ..
ls
