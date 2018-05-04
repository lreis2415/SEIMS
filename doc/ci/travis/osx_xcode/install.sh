#!/bin/sh

set -e
# create directories to be install dirs
mkdir -p seims_osx_xcode/bin
mkdir -p seims_osx_xcode/seims
# Release version
mkdir buildRel
cd buildRel
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNITTEST=1 -DINSTALL_PREFIX=/Users/travis/build/lreis2415/SEIMS/seims_osx_xcode/bin
make -j4
make install
cd ..
ls
# copy files to releases
cp README.md seims_osx_xcode
cp -R data seims_osx_xcode/data
cp seims/*.* seims_osx_xcode/seims
cp -R seims/preprocess seims_osx_xcode/seims/preprocess
cp -R seims/postprocess seims_osx_xcode/seims/postprocess
cp -R seims/parameters_sensitivity seims_osx_xcode/seims/parameters_sensitivity
cp -R seims/scenario_analysis seims_osx_xcode/seims/scenario_analysis
cp -R seims/calibration seims_osx_xcode/seims/calibration
# 2. zip
zip -r seims_osx_xcode.zip seims_osx_xcode
ls
