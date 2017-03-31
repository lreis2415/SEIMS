#!/bin/sh

set -e
cd seims
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNITTEST=1
make -j4
sudo make install
cd ..
cd bin
ls
