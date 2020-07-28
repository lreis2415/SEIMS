#!/bin/sh

set -e
# create directories to be install dirs
mkdir -p seims_linux_gcc/bin
#mkdir -p seims_linux_gcc/seims
# Release version
mkdir build
# Update relative paths in Markdown files for Doxygen
python ./doc/update_relative_paths_for_doxygen.py
cd build
# Do not compile UnitTest: -DUNITTEST=1
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_DOC=ON -DINSTALL_PREFIX=/home/travis/build/lreis2415/SEIMS/seims_linux_gcc/bin
# Automatic documentation
make travis_doc
make -j4
#make install
cd ..
ls
# copy files to releases for deployment
#cp README.md seims_linux_gcc
#cp -R data seims_linux_gcc/data
# mkdir -p seims_linux_gcc/doc # uncomment when PDF version of user guide, theory, technical mannual, etc are available.
#cp seims/*.* seims_linux_gcc/seims
#cp -R seims/preprocess seims_linux_gcc/seims/preprocess
#cp -R seims/postprocess seims_linux_gcc/seims/postprocess
#cp -R seims/parameters_sensitivity seims_linux_gcc/seims/parameters_sensitivity
#cp -R seims/scenario_analysis seims_linux_gcc/seims/scenario_analysis
#cp -R seims/calibration seims_linux_gcc/seims/calibration
# 2. zip
#zip -r seims_linux_gcc.zip seims_linux_gcc
#ls
