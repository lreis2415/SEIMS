#!/bin/sh

set -e
# create directories to be install dirs
mkdir -p seims_longterm_linux_gcc48/seims/bin
mkdir -p seims_storm_linux_gcc48/seims/bin
cd seims
# longterm version
mkdir buildlongterm
cd buildlongterm
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNITTEST=1 -DINSTALL_PREFIX=../../seims_longterm_linux_gcc48/seims/bin
make -j4
make install
cd ..
# storm version
mkdir buildstorm
cd buildstorm
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNITTEST=1 -DSTORM=1 -DINSTALL_PREFIX=../../seims_storm_linux_gcc48/seims/bin
make -j4
make install
cd ../..
ls
# copy files to releases
# 1. longterm
cp *.md seims_longterm_linux_gcc48
cp -R data seims_longterm_linux_gcc48/data
cp -R doc/theory seims_longterm_linux_gcc48/doc/theory
cp -R doc/wiki seims_longterm_linux_gcc48/doc/wiki
cp seims/*.* seims_longterm_linux_gcc48/seims
cp -R seims/pygeoc seims_longterm_linux_gcc48/seims/pygeoc
cp -R seims/preprocess seims_longterm_linux_gcc48/seims/preprocess
cp -R seims/postprocess seims_longterm_linux_gcc48/seims/postprocess
cp -R seims/scenario_analysis seims_longterm_linux_gcc48/seims/scenario_analysis
cp -R seims/calibration seims_longterm_linux_gcc48/seims/calibration
# 2. storm
cp *.md seims_storm_linux_gcc48
cp -R data seims_storm_linux_gcc48/data
cp -R doc/theory seims_storm_linux_gcc48/doc/theory
cp -R doc/wiki seims_storm_linux_gcc48/doc/wiki
cp seims/*.* seims_storm_linux_gcc48/seims
cp -R seims/pygeoc seims_storm_linux_gcc48/seims/pygeoc
cp -R seims/preprocess seims_storm_linux_gcc48/seims/preprocess
cp -R seims/postprocess seims_storm_linux_gcc48/seims/postprocess
cp -R seims/scenario_analysis seims_storm_linux_gcc48/seims/scenario_analysis
cp -R seims/calibration seims_storm_linux_gcc48/seims/calibration
# 3. zip
zip -r seims_longterm_linux_gcc48.zip seims_longterm_linux_gcc48
zip -r seims_storm_linux_gcc48.zip seims_storm_linux_gcc48
ls
