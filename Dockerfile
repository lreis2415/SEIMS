##
#
# Usage:
#   > cd SEIMS
#   > docker build -t <tag>  .
#
# Copyright 2024 Liang-Jun Zhu <zlj@lreis.ac.cn>
#

# Use continuumio/miniconda3 as the build and test container, which is based on debian:12.5-slim
# https://github.com/ContinuumIO/docker-images/blob/main/miniconda3/debian/Dockerfile
FROM continuumio/miniconda3 as builder

LABEL maintainer="Liang-Jun Zhu <zlj@lreis.ac.cn>"

# RUN apt-get update -q && \
#     apt-get install -q -y --no-install-recommends \
#     cmake make g++ gdal-bin openmpi-bin libgdal-dev libopenmpi-dev \
#     && apt-get clean \
#     && rm -rf /var/lib/apt/lists/*

# Copy source directory
WORKDIR /seims
COPY CMakeLists.txt .
COPY cmake cmake
COPY seims seims

# Refers to https://pythonspeed.com/articles/activate-conda-dockerfile/
RUN conda env create -f ./seims/pyseims_env.yml

# Make RUN commands use the new environment:
SHELL ["conda", "run", "--no-capture-output", "-n", "pyseims", "/bin/bash", "-c"]
