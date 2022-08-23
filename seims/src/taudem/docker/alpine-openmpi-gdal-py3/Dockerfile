##
# crazyzlj/taudem_ext:alpine-openmpi-gdal
#
# Usage: 
#   > cd TauDEM_ext
#   > docker build -t <tag> -f docker/alpine-openmpi-gdal-py3/Dockerfile .
#
# Copyright 2022 Liang-Jun Zhu <zlj@lreis.ac.cn>
#

# Use GDAL image with the tag of alpine-small-latest as the build and test container
#ARG GDAL_VERSION=latest
FROM osgeo/gdal:alpine-normal-latest as builder

LABEL maintainer="Liang-Jun Zhu <zlj@lreis.ac.cn>"

# Replace alpine repository source cdn
RUN sed -i 's/dl-cdn.alpinelinux.org/mirrors.tuna.tsinghua.edu.cn/g' /etc/apk/repositories

# Setup build env
RUN apk add --no-cache cmake make g++ openmpi-dev

# Copy source directory
WORKDIR /taudem_ext
COPY CMakeLists.txt .
COPY cmake cmake
COPY src src
COPY src_ext src_ext
COPY data data

# # Build for release
ARG INSTALL_DIR=/taudem_ext/dist
RUN cd /taudem_ext \
    && mkdir build \
    && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
    && make -j 2 \
    && make install \
    && cd ..

# # Build final image
FROM osgeo/gdal:alpine-normal-latest as runner

# Replace alpine repository source cdn
RUN sed -i 's/dl-cdn.alpinelinux.org/mirrors.tuna.tsinghua.edu.cn/g' /etc/apk/repositories

# Add OpenMPI related tools/libraries for running TauDEM
RUN apk add --no-cache openmpi tini openssh

# Order layers starting with less frequently varying ones
ARG INSTALL_DIR=/taudem_ext/dist
COPY --from=builder ${INSTALL_DIR}/bin/ /usr/bin/

ENV OMPI_ALLOW_RUN_AS_ROOT=1
ENV OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1

COPY docker/docker-entrypoint.sh /
ENTRYPOINT ["/sbin/tini", "--", "/docker-entrypoint.sh"]
