#!/bin/sh
set -e
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DUNITTEST=1 -DRUNCOV=1 -DBUILD_DOC=1 ..
# Automatic documentation
make travis_doc
make -j4
make install
# Run Unittest, Coverage and upload results to codecov.io
make UnitTestCoverage
cd ..
ls
codecov -t 133bbd22-b103-4f41-aa26-06c879c54bb9 -X gcov
