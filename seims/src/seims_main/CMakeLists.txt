# ----------------------------------------------------------------------------
#  Root CMake file for the main program of SEIMS.
#     Spatially Explicit Integrated Modeling System
#
#     Please from the off-tree build directory, invoke:
#       $ cmake <path/to/SEIMS/seims/seims_main>
#
#  Routine testing platforms and compilers include:
#     1. Windows 10 with Visual Studio 2013, mongo-c-driver-1.5.5, GDAL-1.11.4
#     2. Windows 10 with mingw64 (GCC-4.9.3), mongo-c-driver-1.8.1, GDAL-1.11.5
#     3. CentOS 6.2 (cluster) with GCC-4.8.4, mongo-c-driver-1.5.5, GDAL-1.9.0
#     4. Red Hat Server 6.2 (cluster) with ICC-12.1.0, mongo-c-driver-1.5.5, GDAL-1.11.5
#     5. macOS 10.12.6 with Clang-8.0 (or GCC-4.9.3), mongo-c-driver-1.5.5, GDAL-1.11.4
#
#  Created and maintained by Liangjun Zhu (zlj@lreis.ac.cn)
#  Latest updated: Oct. 17, 2017
#
#  Contributors: Junzhi Liu, Hui Wu, Liangjun Zhu, Huiran Gao, etc.
#  Copyright (C) 2013-2017 Lreis, IGSNRR, CAS
# ----------------------------------------------------------------------------
### Disable in-source builds to prevent source tree corruption.
#IF(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
#    MESSAGE(FATAL_ERROR "
#FATAL: In-source builds are not allowed.
#       You should create a separate directory for build files.")
#ENDIF()
#CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)
#IF (COMMAND CMAKE_POLICY)
#    CMAKE_POLICY(SET CMP0015 NEW)
#    SET(CMAKE_MACOSX_RPATH 1)
#ENDIF (COMMAND CMAKE_POLICY)

### SEIMS Version.
SET(SEIMSPRJ "seims")
IF (PARALLEL STREQUAL MPI)
    SET(SEIMSPRJ ${SEIMSPRJ}_mpi_omp)
    MESSAGE(STATUS "MPI&OpenMP version...")
ELSE ()
    SET(SEIMSPRJ ${SEIMSPRJ}_omp)
    MESSAGE(STATUS "OpenMP version...")
    ADD_DEFINITIONS(-DMULTIPLY_REACHES)
ENDIF ()
ADD_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB -DMODULE_EXPORTS)
SET(SEIMSPRJ ${SEIMSPRJ}_prj)
PROJECT(${SEIMSPRJ})
IF (VLD EQUAL 1)
  ADD_DEFINITIONS(-DVLD)
ENDIF ()
########  Stand-alone libraries/programs   ###########
SET(BASE_HOME ${CMAKE_CURRENT_SOURCE_DIR}/base)
SET(BASE_INC ${BASE_HOME}/util ${BASE_HOME}/data ${BASE_HOME}/module_setting ${BASE_HOME}/bmps)
SET(MODULE_HOME ${CMAKE_CURRENT_SOURCE_DIR}/modules)
SET(MAIN_HOME ${CMAKE_CURRENT_SOURCE_DIR}/main)
############  Set include directories    #############
SET(MODULE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/base/util/SimulationModule.cpp)
geo_include_directories(${BASE_INC}
                        ${UTILS_INC}
                        ${MONGO_INC}
                        ${RASTER_INC}
                        ${GDAL_INCLUDE_DIR}
                        ${BSON_INCLUDE_DIR}
                        ${MONGOC_INCLUDE_DIR})
IF (PARALLEL STREQUAL MPI)
    geo_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/main/main_omp
                            ${MPI_INCLUDE_PATH})
    LINK_LIBRARIES(${MPI_LIBRARIES})
ENDIF ()
############  step into subdirectories   #############
ADD_SUBDIRECTORY(${BASE_HOME})
ADD_SUBDIRECTORY(${MODULE_HOME})
ADD_SUBDIRECTORY(${MAIN_HOME})
