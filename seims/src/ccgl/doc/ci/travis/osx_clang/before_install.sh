#!/bin/sh

set -e
brew update
# In order to use OpenMP, the LLVM-clang is highly recommended on macOS
# brew install llvm
# echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.bash_profile

# Check if GDAL is already installed
brew list gdal &>/dev/null || brew install gdal

# MongoDB database service
echo "Installing and starting mongodb"
# The follow dependencies will be automatically installed by mongodb
# brew install automake autoconf libtool openssl
brew install mongodb
# create a folder for mongodb to prevent an error on mac osx
sudo mkdir -p /data/db
brew services start mongodb

# Install mongo-c-driver via brew
brew install mongo-c-driver
# cd ..
# MONGOC_VERSION=1.6.1
# wget -q https://github.com/mongodb/mongo-c-driver/releases/download/$MONGOC_VERSION/mongo-c-driver-$MONGOC_VERSION.tar.gz
# tar xzf mongo-c-driver-$MONGOC_VERSION.tar.gz
# cd mongo-c-driver-$MONGOC_VERSION
# export LDFLAGS="-L/usr/local/opt/openssl/lib"
# export CPPFLAGS="-I/usr/local/opt/openssl/include"
# ./configure --disable-automatic-init-and-cleanup
# make -j4
# sudo make install
