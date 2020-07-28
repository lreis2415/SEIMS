#!/bin/sh

set -e
sudo pip install codecov
echo ${PATH}
echo ${CXX}
${CXX} --version
${CXX} -v
cmake --version
ls
