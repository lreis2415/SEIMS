# CMake build for Windows, Linux, and macOS
# Author         : LiangJun Zhu, JunZhi Liu
# Contact        : zlj@lreis.ac.cn
# Latest updated : Dec. 20, 2016
#
cmake_minimum_required(VERSION 2.8)
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0015 NEW)
    SET(CMAKE_MACOSX_RPATH 1)
ENDIF (COMMAND CMAKE_POLICY)
################# SEIMS Version ##################
IF (PARALLEL STREQUAL MPI)
    PROJECT(SEIMS_MPI_ALL)
    MESSAGE(STATUS "SEIMS-MPI version...")
ELSE ()
    PROJECT(SEIMS_OMP_ALL)
    MESSAGE(STATUS "SEIMS-OpenMP version...")
ENDIF ()
################# Install directory ##################
SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
##############  Stand-alone programs   ###############
SET(FIELD_PARTITION ./src/field_partition)
SET(GRID_LAYERING ./src/grid_layering)
SET(IMPORT_RASTER ./src/import_raster)
SET(IUH ./src/iuh)
SET(METIS ./src/metis)
SET(TAUDEM ./src/taudem)
SET(SEIMS_MAIN ./src/seims_main)
############  step into subdirectories   #############
ADD_SUBDIRECTORY(${FIELD_PARTITION} bin/field_partition)
ADD_SUBDIRECTORY(${GRID_LAYERING} bin/grid_layering)
ADD_SUBDIRECTORY(${IMPORT_RASTER} bin/import_raster)
ADD_SUBDIRECTORY(${IUH} bin/iuh)
ADD_SUBDIRECTORY(${METIS} bin/metis)
ADD_SUBDIRECTORY(${TAUDEM} bin/taudem)
IF (PARALLEL STREQUAL MPI)
  ADD_SUBDIRECTORY(${SEIMS_MAIN} bin/seims_mpi_project)
ELSE()
  ADD_SUBDIRECTORY(${SEIMS_MAIN} bin/seims_omp_project)
ENDIF()
IF(MSVC)
  SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF()
MESSAGE(STATUS "All SEIMS related compiled done!")
