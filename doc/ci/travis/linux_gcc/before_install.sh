#!/bin/sh

set -e

# Changelog:
#   - 1. 2019-6-18: No need to update GCC and build GDAL from source. Use OpenMPI instead of MPICH2.

# update gcc to version 4.8
#sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
#sudo apt-get update -qq
#sudo apt-get install -qq gcc-4.8 g++-4.8
#export CXX="g++-4.8" CC="gcc-4.8"
#sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 90

# download gdal from osgeo, and compile and install
#GDAL_VERSION=1.11.5
#cd ..
#wget -q http://download.osgeo.org/gdal/$GDAL_VERSION/gdal-$GDAL_VERSION.tar.gz
#tar xvzf gdal-$GDAL_VERSION.tar.gz
#cd gdal-$GDAL_VERSION
#./configure
#make -j4
#sudo make install

# Adding Ubuntu GIS repository to system's software sources
sudo apt-get install python-software-properties
sudo add-apt-repository -y ppa:ubuntugis/ppa
sudo apt-get update

# install gdal from apt-get
sudo apt-get install -qq gdal-bin libgdal-dev

# install mpich2
#sudo apt-get install -q libc-dev mpich2 libmpich2-dev

# install OpenMPI
sudo apt-get install -q openmpi-bin openmpi-doc libopenmpi-dev
mpiexec --version

# download mongo-c-driver from github, and compile and install
cd ..
MONGOC_VERSION=1.6.3
wget -q https://github.com/mongodb/mongo-c-driver/releases/download/$MONGOC_VERSION/mongo-c-driver-$MONGOC_VERSION.tar.gz
tar xzf mongo-c-driver-$MONGOC_VERSION.tar.gz
cd mongo-c-driver-$MONGOC_VERSION
./configure --disable-automatic-init-and-cleanup
make -j4
sudo make install
sudo ldconfig

