#!/usr/bin/env bash
#
#    This file is aimed for Packages checking and installing for SEIMS on Linu/Unix platform.
#
#    SEIMS is distributed for Research and/or Education only,
#    any commercial purpose will be FORBIDDEN.
#    SEIMS is an open-source project,
#    but WITHOUT ANY WARRANTY; WITHOUT even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#    See the GNU General Public License for more details.
#
cd $PWD
cd pygeoc
rm -r dist
# Compile and install PyGeoC through pip
pip install tox
python setup.py bdist_wheel --python-tag py2
cd dist
for i in `find . -name *.whl`; do pip install $i --upgrade; done

cd ../..
# Install all dependent packages requied by SEIMS
for i in `find . -name requirements.txt`; do pip install -r $i; done