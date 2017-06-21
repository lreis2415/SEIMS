# CMake build for Windows, Linux, and macOS
# Author         : LiangJun Zhu, JunZhi Liu
# Contact        : zlj@lreis.ac.cn
# Latest updated : Mar. 31, 2017
#
cmake_minimum_required(VERSION 3.1)
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0015 NEW)
    SET(CMAKE_MACOSX_RPATH 1)
ENDIF (COMMAND CMAKE_POLICY)
### Active C++11 standard, cross-platform. This requires CMake 3.1+
SET (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH}
)
include(SetThreadFlagMSVC)
################# SEIMS Version ##################
SET(PRJNAME "SEIMS")
IF (STORM EQUAL 1)
    SET(PRJNAME ${PRJNAME}_STORM)
    MESSAGE(STATUS "Building STORM_MODE model...")
ELSE ()
    SET(PRJNAME ${PRJNAME}_LONGTERM)
    MESSAGE(STATUS "Building LONGTERM_MODE model...")
ENDIF ()
IF (PARALLEL STREQUAL MPI)
    SET(PRJNAME ${PRJNAME}_MPI)
    MESSAGE(STATUS "MPI&OpenMP version...")
ELSE ()
    SET(PRJNAME ${PRJNAME}_OMP)
    MESSAGE(STATUS "OpenMP version...")
ENDIF ()
IF (ARCH EQUAL 64)
    SET(PRJNAME ${PRJNAME}_x64)
    MESSAGE(STATUS "64-bit version...")
ELSE ()
    SET(PRJNAME ${PRJNAME}_x32)
    MESSAGE(STATUS "32-bit version...")
ENDIF ()
SET(PRJNAME ${PRJNAME}_ALL)
PROJECT(${PRJNAME})
################# Install directory ##################
IF (INSTALL_PREFIX)
    SET(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX})
    SET(INSTALL_DIR ${INSTALL_PREFIX})
ELSE ()
    SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
    SET(INSTALL_DIR ${CMAKE_SOURCE_DIR}/bin)
ENDIF ()
##############  Stand-alone programs   ###############
SET(FIELD_PARTITION ./src/field_partition)
SET(GRID_LAYERING ./src/grid_layering)
SET(IMPORT_RASTER ./src/import_raster)
SET(MASK_RASTER ./src/mask_rasters)
SET(RECLASSIFY ./src/reclassify)
SET(IUH ./src/iuh)
SET(METIS ./src/metis)
SET(TAUDEM ./src/taudem)
SET(SEIMS_MAIN ./src/seims_main)
# unit test is an optional configuration
IF (UNITTEST STREQUAL 1)
    SET(SEIMS_TEST ./src/test)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
    ADD_SUBDIRECTORY(${SEIMS_TEST} bin/test)
ENDIF ()
############  step into subdirectories   #############
ADD_SUBDIRECTORY(${FIELD_PARTITION} bin/field_partition)
ADD_SUBDIRECTORY(${GRID_LAYERING} bin/grid_layering)
ADD_SUBDIRECTORY(${IMPORT_RASTER} bin/import_raster)
ADD_SUBDIRECTORY(${MASK_RASTER} bin/mask_raster)
ADD_SUBDIRECTORY(${RECLASSIFY} bin/reclassify)
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
### For CLion to implement the "make install" command
add_custom_target(install_${PROJECT_NAME}
        $(MAKE) install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")
MESSAGE(STATUS "All SEIMS related compiled done!")
