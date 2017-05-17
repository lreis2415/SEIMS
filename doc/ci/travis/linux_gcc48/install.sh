#!/bin/sh

set -e
cd seims
# longterm version
mkdir buildlongterm
cd buildlongterm
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNITTEST=1
make -j4
sudo make install
cd ..
# storm version
mkdir buildstorm
cd buildstorm
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNITTEST=1 -DSTORM=1
make -j4
sudo make install
cd ..
# list all modules and executable file
cd bin
ls
