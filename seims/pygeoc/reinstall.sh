#!/usr/bin/env bash
#
#    This file is aimed for reCompile and reInstall PyGeoC for debugging on Linu/Unix platform.
#
cd $PWD
rm -r dist
pip install tox
python setup.py bdist_wheel --python-tag py2
cd dist
for i in `find . -name *.whl`; do pip install $i  --upgrade; done