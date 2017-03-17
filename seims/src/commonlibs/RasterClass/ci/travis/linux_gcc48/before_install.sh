#!/bin/sh

set -e
# update gcc to version 4.8
sudo apt-get install gcc-4.8 g++-4.8
# download gdal from osgeo, and compile and install
GDAL_VERSION=1.11.5
wget -q http://download.osgeo.org/gdal/gdal-$GDAL_VERSION.tar.gz
tar xvzf gdal-$GDAL_VERSION.tar.gz
cd gdal-$GDAL_VERSION
./configure
make -j4
sudo make install
# download mongo-c-driver from github, and compile and install
MONGOC_VERSION=1.6.1
wget -q https://github.com/mongodb/mongo-c-driver/archive/$MONGOC_VERSION.tar.gz
tar xzf mongo-c-driver-$MONGOC_VERSION.tar.gz
cd mongo-c-driver-$MONGOC_VERSION
./configure --disable-automatic-init-and-cleanup
make -j4
sudo make install
sudo ldconfig
