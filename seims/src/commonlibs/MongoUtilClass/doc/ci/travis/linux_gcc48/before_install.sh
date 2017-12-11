#!/bin/sh

set -e
# update gcc to version 4.8
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update -qq
sudo apt-get install -qq gcc-4.8 g++-4.8
export CXX="g++-4.8" CC="gcc-4.8"
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 90

# download mongo-c-driver from github, and compile and install
cd ..
MONGOC_VERSION=1.6.1
wget -q https://github.com/mongodb/mongo-c-driver/releases/download/$MONGOC_VERSION/mongo-c-driver-$MONGOC_VERSION.tar.gz
tar xzf mongo-c-driver-$MONGOC_VERSION.tar.gz
cd mongo-c-driver-$MONGOC_VERSION
./configure --disable-automatic-init-and-cleanup
make -j4
sudo make install
sudo ldconfig
# git clone MongoUtilClass and UtilClass
cd ..
git clone --depth=50 --branch=master https://github.com/lreis2415/UtilsClass.git
cd MongoUtilClass
