#!/bin/sh

set -e
cd seims
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
export LDFLAGS="${LDFLAGS} -headerpad_max_install_names"
make -j4
sudo make install
cd ..
cd bin
ls

