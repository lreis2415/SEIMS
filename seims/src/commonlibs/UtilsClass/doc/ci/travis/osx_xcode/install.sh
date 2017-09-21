#!/bin/sh

set -e
ls
mkdir utils_osx_xcode
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DINSTALL_PREFIX=../utils_osx_xcode
make -j4
sudo make install
cd ..
# zip the compiled binary
zip -r utils_osx_xcode.zip utils_osx_xcode
# list all executable file
ls
cd utils_osx_xcode
ls
