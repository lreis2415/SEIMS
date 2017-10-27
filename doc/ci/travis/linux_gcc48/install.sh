#!/bin/sh

set -e
# create directories to be install dirs
mkdir -p seims_linux_gcc48/bin
mkdir -p seims_linux_gcc48/seims
# Release version
mkdir buildRel
cd buildRel
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNITTEST=1 -DINSTALL_PREFIX=/home/travis/build/lreis2415/SEIMS/seims_linux_gcc48/bin
make -j4
make install
cd ..
ls
# copy files to releases
cp README.md seims_linux_gcc48
cp -R data seims_linux_gcc48/data
mkdir -p seims_linux_gcc48/doc
cp -R doc/theory seims_linux_gcc48/doc/theory
cp seims/*.* seims_linux_gcc48/seims
cp -R seims/preprocess seims_linux_gcc48/seims/preprocess
cp -R seims/postprocess seims_linux_gcc48/seims/postprocess
cp -R seims/scenario_analysis seims_linux_gcc48/seims/scenario_analysis
cp -R seims/calibration seims_linux_gcc48/seims/calibration
# 2. zip
zip -r seims_linux_gcc48.zip seims_linux_gcc48
ls
