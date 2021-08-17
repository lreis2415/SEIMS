# ----------------------------------------------------------------------------
#  Root CMake file for the main program of SEIMS.
#     Spatially Explicit Integrated Modeling System
#
#     Please from the off-tree build directory, invoke:
#       $ cmake <path/to/SEIMS/seims/seims_main>
#
#  Routine testing platforms and compilers include:
#     1. Windows 10 with Visual Studio 2010/2013/2015, MSMPI-v8.1, GDAL-1.11.4
#     2. Windows 10 with msys2/mingw64 (GCC-9.1.0), MSMPI-v8.1, GDAL-3.0
#     3. CentOS 6.2 (cluster) with GCC-4.8.4, mongo-c-driver-1.5.5, GDAL-1.9.0
#     4. Red Hat Server 6.2 (cluster) with ICC-12.1.0, mongo-c-driver-1.6.1, GDAL-1.11.5 (Without UnitTest)
#     5. macOS 10.14.5 with Clang-10.0 with Xcode, OpenMPI-4.0.1, GDAL-2.4.2 (brew installed)
#
#  Created and maintained by Liangjun Zhu (zlj@lreis.ac.cn)
#  Latest updated: Jul. 21, 2019
#
#  Copyright (C) 2013-2019 Lreis, IGSNRR, CAS
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
IF(XCODE)
  # To use OpenMP in Xcode built by CMake, the recommended way is to
  #   install llvm by 'brew install llvm', which has builtin 'libomp'
  IF (NOT LLVM_ROOT_DIR)
    SET(LLVM_ROOT_DIR "/usr/local/opt/llvm")
  ENDIF ()
  FIND_PACKAGE(LLVM)
  IF(LLVM_FOUND)
    geo_include_directories(${LLVM_LIBRARY_DIRS}/clang/${LLVM_VERSION_BASE_STRING}/include)
	SET(OpenMP_CXX_FLAGS "-fopenmp")
    SET(OpenMP_C_FLAGS "-fopenmp")
	FIND_LIBRARY(OpenMP_LIBRARY
		NAMES "omp"
		HINTS ${LLVM_LIBRARY_DIRS})
    SET(OPENMP_FOUND 1)
  ENDIF ()
ELSE ()
  FIND_PACKAGE(OpenMP)
ENDIF ()
IF(OPENMP_FOUND)
  SET(WITH_OPENMP 1)
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

ADD_DEFINITIONS(-DMODULE_EXPORTS)
### Use MongoDB as database
IF (BSON_FOUND AND MONGOC_FOUND)
  ADD_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB)
ENDIF ()

########  Stand-alone libraries/programs   ###########
SET(BASE_HOME ${CMAKE_CURRENT_SOURCE_DIR}/base)
SET(BASE_INC ${BASE_HOME} ${BASE_HOME}/util ${BASE_HOME}/common_algorithm
             ${BASE_HOME}/data ${BASE_HOME}/module_setting ${BASE_HOME}/bmps)
SET(MODULE_HOME ${CMAKE_CURRENT_SOURCE_DIR}/modules)
SET(MAIN_HOME ${CMAKE_CURRENT_SOURCE_DIR}/main)
############  Set include directories    #############
geo_include_directories(${CCGL_INC}
                        ${BASE_INC}
                        ${GDAL_INCLUDE_DIR}
                        ${BSON_INCLUDE_DIR}
                        ${MONGOC_INCLUDE_DIR})
############  step into subdirectories   #############
SET(SEIMS_BINARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
ADD_SUBDIRECTORY(${BASE_HOME})
ADD_SUBDIRECTORY(${MODULE_HOME})
ADD_SUBDIRECTORY(${MAIN_HOME})
