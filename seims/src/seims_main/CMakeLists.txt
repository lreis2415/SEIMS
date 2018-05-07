# ----------------------------------------------------------------------------
#  Root CMake file for the main program of SEIMS.
#     Spatially Explicit Integrated Modeling System
#
#     Please from the off-tree build directory, invoke:
#       $ cmake <path/to/SEIMS/seims/seims_main>
#
#  Routine testing platforms and compilers include:
#     1. Windows 10 with Visual Studio 2010/2013, mongo-c-driver-1.5.5, GDAL-1.11.4
#     2. Windows 10 with mingw64 (GCC-4.9.3), mongo-c-driver-1.8.1, GDAL-1.11.5
#     3. CentOS 6.2 (cluster) with GCC-4.8.4, mongo-c-driver-1.5.5, GDAL-1.9.0
#     4. Red Hat Server 6.2 (cluster) with ICC-12.1.0, mongo-c-driver-1.5.5, GDAL-1.11.5 (Without UnitTest)
#     5. macOS 10.12.6 with Clang-8.0 (or GCC-4.9.3), mongo-c-driver-1.5.5, GDAL-1.11.4
#
#  Created and maintained by Liangjun Zhu (zlj@lreis.ac.cn)
#  Latest updated: Mar. 4, 2017
#
#  Copyright (C) 2013-2018 Lreis, IGSNRR, CAS
# ----------------------------------------------------------------------------
### Set coding language
PROJECT(seims_prj CXX C)
### SEIMS Version.
SET(SEIMSPRJ "seims")
### Check MPI which is optional
IF(MPI_FOUND)
  SET(WITH_MPI 1)
  SET(SEIMSPRJ ${SEIMSPRJ}_mpi)
  MESSAGE(STATUS "Compiling with MPI...")
ENDIF()
### Check OMP which is optional
IF(OPENMP_FOUND)
  SET(WITH_OMP 1)
  ADD_DEFINITIONS(-DSUPPORT_OMP)
  SET(SEIMSPRJ ${SEIMSPRJ}_omp)
  MESSAGE(STATUS "Compiling with OpenMP...")
ENDIF()
SET(SEIMSPRJ ${SEIMSPRJ}_prj)
PROJECT(${SEIMSPRJ})

### Compile with Visual leak detector for MSVC
IF (VLD EQUAL 1)
  ADD_DEFINITIONS(-DVLD)
ENDIF ()

### Build Postfix
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Default Debug postfix")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "Default Release postfix")
SET(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "Default RelWithDebInfo postfix")
SET(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "Default MinSizeRel postfix")
IF (CMAKE_BUILD_TYPE MATCHES "Release")
  SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
  SET(CMAKE_BUILD_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
  SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "Debug")
  SET(CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
ELSE()
  SET(CMAKE_BUILD_POSTFIX "")
ENDIF()

ADD_DEFINITIONS(-DMODULE_EXPORTS)
### Use MongoDB as database
IF (BSON_FOUND AND MONGOC_FOUND)
  ADD_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB)
ENDIF ()

########  Stand-alone libraries/programs   ###########
SET(BASE_HOME ${CMAKE_CURRENT_SOURCE_DIR}/base)
SET(BASE_INC ${BASE_HOME} ${BASE_HOME}/common_algorithm ${BASE_HOME}/data ${BASE_HOME}/module_setting ${BASE_HOME}/bmps)
SET(MODULE_HOME ${CMAKE_CURRENT_SOURCE_DIR}/modules)
SET(MAIN_HOME ${CMAKE_CURRENT_SOURCE_DIR}/main)
############  Set include directories    #############
geo_include_directories(${CCGL_INC}
                        ${BASE_INC}
                        ${GDAL_INCLUDE_DIR}
                        ${BSON_INCLUDE_DIR}
                        ${MONGOC_INCLUDE_DIR})
############  step into subdirectories   #############
ADD_SUBDIRECTORY(${BASE_HOME})
ADD_SUBDIRECTORY(${MODULE_HOME})
ADD_SUBDIRECTORY(${MAIN_HOME})
