#!/bin/sh

set -e
brew update
brew install gdal
# download mongo-c-driver from github, and compile and install
echo "Installing and starting mongodb"
brew install automake autoconf libtool pkgconfig openssl
brew install mongodb --with-openssl
# create a folder for mongodb to prevent an error on mac osx
sudo mkdir -p /data/db
brew services start mongodb
cd ..
MONGOC_VERSION=1.6.1
wget -q https://github.com/mongodb/mongo-c-driver/releases/download/$MONGOC_VERSION/mongo-c-driver-$MONGOC_VERSION.tar.gz
tar xzf mongo-c-driver-$MONGOC_VERSION.tar.gz
cd mongo-c-driver-$MONGOC_VERSION
export LDFLAGS="-L/usr/local/opt/openssl/lib"
export CPPFLAGS="-I/usr/local/opt/openssl/include"
./configure --disable-automatic-init-and-cleanup
make -j4
sudo make install
# git clone MongoUtilClass and UtilClass
cd ..
git clone --depth=50 --branch=master https://github.com/lreis2415/UtilsClass.git
git clone --depth=50 --branch=master https://github.com/lreis2415/MongoUtilClass.git
cd RasterClass
