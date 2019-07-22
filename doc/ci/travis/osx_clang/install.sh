#!/bin/sh

set -e
# create directories to be install dirs
mkdir -p seims_osx_clang/bin
mkdir -p seims_osx_clang/seims
# Release version
mkdir buildRel
cd buildRel
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
make install
cd ..
ls
# # copy files to releases
# cp README.md seims_osx_clang
# cp -R data seims_osx_clang/data
# cp seims/*.* seims_osx_clang/seims
# cp -R seims/preprocess seims_osx_clang/seims/preprocess
# cp -R seims/postprocess seims_osx_clang/seims/postprocess
# cp -R seims/parameters_sensitivity seims_osx_clang/seims/parameters_sensitivity
# cp -R seims/scenario_analysis seims_osx_clang/seims/scenario_analysis
# cp -R seims/calibration seims_osx_clang/seims/calibration
# # 2. zip
# zip -r seims_osx_clang.zip seims_osx_clang
# ls
