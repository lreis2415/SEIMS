#!/bin/sh

set -e

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
sudo make install
cd ..
