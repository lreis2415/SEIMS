##
# crazyzlj/ccgl:alpine-test for test locally
#
# This script is written based on PROJ and GDAL. 
#   https://github.com/OSGeo/PROJ/blob/master/Dockerfile
#   https://github.com/OSGeo/gdal/tree/master/docker
#
# Usage: 
#   > cd CCGL
#   > docker build -t <tag> -f docker/test/apline/Dockerfile .
#
# Copyright 2022 Liang-Jun Zhu <zlj@lreis.ac.cn>

# Use alpine as the build and test container
ARG ALPINE_VERSION=3.15
FROM alpine:${ALPINE_VERSION} as builder

LABEL maintainer="Liang-Jun Zhu <zlj@lreis.ac.cn>"

# Replace alpine repository source cdn
RUN sed -i 's/dl-cdn.alpinelinux.org/mirrors.tuna.tsinghua.edu.cn/g' /etc/apk/repositories

# Setup build env
RUN apk add --no-cache cmake make g++

# Copy source directory
WORKDIR /CCGL
COPY CMakeLists.txt .
COPY cmake cmake
COPY src src
COPY apps apps
COPY test test
COPY data data

# Build with GoogleTest and run unittest
RUN cd /CCGL \
    && mkdir -p build_with_test \
    && cd build_with_test \
    && cmake -DCMAKE_BUILD_TYPE=Debug -DUNITTEST=1 -DFORCEDOWNGTEST=1 -DCMAKE_INSTALL_PREFIX=/usr/local .. \
    && make -j 2 \
    && ctest -C Debug --rerun-failed --output-on-failure \
    && make install \
    && cd ..

