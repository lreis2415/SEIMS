#!/bin/sh

set -e
# create directories to be install dirs
mkdir -p seims_linux_gcc48/bin
mkdir -p seims_linux_gcc48/seims
# Release version
mkdir buildRel
cd buildRel
# Do not compile UnitTest: -DUNITTEST=1
cmake .. -DCMAKE_BUILD_TYPE=Release -DINSTALL_PREFIX=/home/travis/build/lreis2415/SEIMS/seims_linux_gcc48/bin
make -j4
make install
cd ..
ls
# copy files to releases
cp README.md seims_linux_gcc48
cp -R data seims_linux_gcc48/data
# mkdir -p seims_linux_gcc48/doc # uncomment when PDF version of user guide, theory, technical mannual, etc are available.
cp seims/*.* seims_linux_gcc48/seims
cp -R seims/preprocess seims_linux_gcc48/seims/preprocess
cp -R seims/postprocess seims_linux_gcc48/seims/postprocess
cp -R seims/parameters_sensitivity seims_linux_gcc48/seims/parameters_sensitivity
cp -R seims/scenario_analysis seims_linux_gcc48/seims/scenario_analysis
cp -R seims/calibration seims_linux_gcc48/seims/calibration
# 2. zip
zip -r seims_linux_gcc48.zip seims_linux_gcc48
ls
