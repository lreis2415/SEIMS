#!/bin/sh

set -e
# create directories to be install dirs
mkdir -p seims_longterm_osx_xcode/seims/bin
mkdir -p seims_storm_osx_xcode/seims/bin
cd seims
# longterm version
mkdir buildlongterm
cd buildlongterm
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNITTEST=1 -DINSTALL_PREFIX=/Users/travis/build/lreis2415/SEIMS/seims_longterm_osx_xcode/seims/bin
make -j4
make install
cd ..
# storm version
mkdir buildstorm
cd buildstorm
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNITTEST=1 -DSTORM=1 -DINSTALL_PREFIX=/Users/travis/build/lreis2415/SEIMS/seims_storm_osx_xcode/seims/bin
make -j4
make install
cd ../..
ls
# copy files to releases
# 1. longterm
cp *.md seims_longterm_osx_xcode
cp -R data seims_longterm_osx_xcode/data
mkdir -p seims_longterm_osx_xcode/doc
cp -R doc/theory seims_longterm_osx_xcode/doc/theory
cp -R doc/wiki seims_longterm_osx_xcode/doc/wiki
cp seims/*.* seims_longterm_osx_xcode/seims
cp -R seims/pygeoc seims_longterm_osx_xcode/seims/pygeoc
cp -R seims/preprocess seims_longterm_osx_xcode/seims/preprocess
cp -R seims/postprocess seims_longterm_osx_xcode/seims/postprocess
cp -R seims/scenario_analysis seims_longterm_osx_xcode/seims/scenario_analysis
cp -R seims/calibration seims_longterm_osx_xcode/seims/calibration
# 2. storm
cp *.md seims_storm_osx_xcode
cp -R data seims_storm_osx_xcode/data
mkdir -p seims_storm_osx_xcode/doc
cp -R doc/theory seims_storm_osx_xcode/doc/theory
cp -R doc/wiki seims_storm_osx_xcode/doc/wiki
cp seims/*.* seims_storm_osx_xcode/seims
cp -R seims/pygeoc seims_storm_osx_xcode/seims/pygeoc
cp -R seims/preprocess seims_storm_osx_xcode/seims/preprocess
cp -R seims/postprocess seims_storm_osx_xcode/seims/postprocess
cp -R seims/scenario_analysis seims_storm_osx_xcode/seims/scenario_analysis
cp -R seims/calibration seims_storm_osx_xcode/seims/calibration
# 3. zip
zip -r seims_longterm_osx_xcode.zip seims_longterm_osx_xcode
zip -r seims_storm_osx_xcode.zip seims_storm_osx_xcode
ls
