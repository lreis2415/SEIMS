#!/bin/sh

set -e
ls
mkdir utils_linux_gcc48
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DINSTALL_PREFIX=../utils_linux_gcc48
make -j4
sudo make install
cd ..
# zip the compiled binary
zip -r utils_linux_gcc48.zip utils_linux_gcc48
# list all executable file
ls
cd utils_linux_gcc48
ls
