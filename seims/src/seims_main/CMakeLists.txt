# CMake build for Windows, Linux, and macOS
# Author         : LiangJun Zhu, JunZhi Liu
# Contact        : zlj@lreis.ac.cn
# Latest updated : Dec. 18, 2016
#

cmake_minimum_required(VERSION 3.1)
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0015 NEW)
    SET(CMAKE_MACOSX_RPATH 1)
ENDIF (COMMAND CMAKE_POLICY)
### Active C++11 standard, cross-platform. This requires CMake 3.1+
SET (CMAKE_CXX_STANDARD 11)
################# SEIMS Version ##################
IF (PARALLEL STREQUAL MPI)
    PROJECT(SEIMS_MPI_prj)
    MESSAGE(STATUS "SEIMS-MPI version...")
    ADD_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB -DMODULE_EXPORTS)
ELSE ()
    PROJECT(SEIMS_OMP_prj)
    MESSAGE(STATUS "SEIMS-OpenMP version...")
    ADD_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB -DMODULE_EXPORTS -DMULTIPLY_REACHES)
ENDIF ()
################# OS platform   ##################
if (WIN32)
    add_definitions(-Dwindows)
    if (MSVC)
        add_definitions(-DMSVC)
    endif ()
elseif (APPLE)
    add_definitions(-Dmacos)
else ()
    add_definitions(-Dlinux)
endif ()
################# Find packages ##################
# GDAL, Bson, Mongoc, MPI, and OpenMP are required
IF (WIN32)
    # 1. GDAL
    SET(GDAL_DIR $ENV{GDAL_DIR})
    SET(GDAL_DIR $ENV{GDAL_DIR})
    SET(GDAL_INCLUDE_DIR ${GDAL_DIR}\\include)
    SET(GDAL_LIBRARIES ${GDAL_DIR}\\lib)
    SET(GDAL_BIN_DIR ${GDAL_DIR}\\bin)
    SET(GDAL_LIBRARY ${GDAL_LIBRARIES}\\gdal_i.lib)
    # 2. Bson and MongoC
    include(FindBson.cmake)
    include(FindMongoC.cmake)
    IF (PARALLEL STREQUAL MPI)
      # 3. MPI
      SET (MPI_LIB $ENV{MSMPI_LIB32})
      SET (MPI_HOME $ENV{MSMPI_BIN})
      SET (MPI_INCLUDE_PATH $ENV{MSMPI_INC})
      SET (MPI_LIBRARIES ${MPI_LIB}\\msmpi.lib)
    ENDIF()
ELSE ()
    # 1. GDAL CMake will find GDAL and get GDAL_INCLUDE_DIR and GDAL_LIBRARY.
    find_package(GDAL REQUIRED)
    # 2. Bson and MongoC
    include(FindBson.cmake)
    include(FindMongoC.cmake)
    IF (PARVERSION STREQUAL MPI)
      # 3. MPI
      find_package(MPI REQUIRED)
      set(CMAKE_CXX_FLAG ${CMAKE_CXX_FLAG} ${MPI_COMPILE_FLAGS})
      set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
    ENDIF()
ENDIF ()
# 4. Add support for OpenMP
FIND_PACKAGE(OpenMP)
IF (OPENMP_FOUND)
    add_definitions(-DSUPPORT_OMP)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ENDIF ()
################# Install directory ##################
SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
##############  Set common libraries   ###############
set(UTILS_INC ${CMAKE_CURRENT_SOURCE_DIR}/../commonlibs/UtilsClass)
set(UTILS_FILES ${UTILS_INC}/utils.cpp ${UTILS_INC}/ModelException.cpp)
set(MONGO_INC ${CMAKE_CURRENT_SOURCE_DIR}/../commonlibs/MongoUtilClass)
set(MONGO_FILES ${MONGO_INC}/MongoUtil.cpp)
SET(RASTER_INC ${CMAKE_CURRENT_SOURCE_DIR}/../commonlibs/RasterClass)
SET(RASTER_FILES ${RASTER_INC}/clsRasterData.cpp)
SET(FIND_BASELIB ${CMAKE_CURRENT_SOURCE_DIR}/FindBase.cmake)
########  Stand-alone libraries/programs   ###########
SET(BASE_HOME ${CMAKE_CURRENT_SOURCE_DIR}/base)
SET(BASE_INC ${BASE_HOME}/util ${BASE_HOME}/data ${BASE_HOME}/module_setting ${BASE_HOME}/bmps)
SET(MODULE_HOME ${CMAKE_CURRENT_SOURCE_DIR}/modules)
SET(MAIN_HOME ${CMAKE_CURRENT_SOURCE_DIR}/main)
############  Set include directories    #############
SET(MODULE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/base/util/SimulationModule.cpp)
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR} ${BSON_INCLUDE_DIR} ${MONGOC_INCLUDE_DIR} ${UTILS_INC} ${MONGO_INC} ${RASTER_INC} ${BASE_INC})
IF (PARALLEL STREQUAL MPI)
   INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/main/main_omp ${MPI_INCLUDE_PATH})
   LINK_LIBRARIES(${MPI_LIBRARIES})
ENDIF()
IF(MSVC)
  SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF()
############  step into subdirectories   #############
ADD_SUBDIRECTORY(${BASE_HOME})
ADD_SUBDIRECTORY(${MODULE_HOME})
ADD_SUBDIRECTORY(${MAIN_HOME})
### For CLion to implement the "make install" command
add_custom_target(install_${PROJECT_NAME}
      $(MAKE) install
      DEPENDS ${PROJECT_NAME}
      COMMENT "Installing ${PROJECT_NAME}")
IF (PARALLEL STREQUAL MPI)
  MESSAGE(STATUS "    Compiling SEIMS_MPI done")
ELSE()
  MESSAGE(STATUS "    Compiling SEIMS_OMP done")
ENDIF()
