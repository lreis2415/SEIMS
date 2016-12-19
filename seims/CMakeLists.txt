# CMake build for Windows, Linux, and macOS
# Author         : LiangJun Zhu, JunZhi Liu
# Contact        : zlj@lreis.ac.cn
# Latest updated : Dec. 18, 2016
#

cmake_minimum_required(VERSION 2.6)
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0015 NEW)
ENDIF (COMMAND CMAKE_POLICY)
################# SEIMS Version ##################
IF (SEIMSVERSION STREQUAL MPI)
    PROJECT(SEIMS_MPI)
    MESSAGE(STATUS "SEIMS-MPI version...")
    ADD_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB -DMODULE_EXPORTS)
ELSE ()
    PROJECT(SEIMS_OMP)
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
    # 3. MPI
    SET (MPI_LIB $ENV{MSMPI_LIB32})
    SET (MPI_HOME $ENV{MSMPI_BIN})
    SET (MPI_INCLUDE_PATH $ENV{MSMPI_INC})
    SET (MPI_LIBRARIES ${MPI_LIB}\\msmpi.lib)
ELSE ()
    # 1. GDAL CMake will find GDAL and get GDAL_INCLUDE_DIR and GDAL_LIBRARY.
    find_package(GDAL REQUIRED)
    # 2. Bson and MongoC
    include(FindBson.cmake)
    include(FindMongoC.cmake)
    # 3. MPI
    find_package(MPI REQUIRED)
    set(CMAKE_CXX_FLAG ${CMAKE_CXX_FLAG} ${MPI_COMPILE_FLAGS})
    set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
ENDIF ()
# 4. Add support for OpenMP
FIND_PACKAGE(OpenMP)
IF (OPENMP_FOUND)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ENDIF ()
################# Install directory ##################
SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
#################   Common library  ##################
set(UTILS_INC ${CMAKE_INSTALL_PREFIX}/commonlibs/UtilsClass)
set(UTILS_FILES ${UTILS_INC}/utils.cpp ${UTILS_INC}/util.cpp ${UTILS_INC}/StringList.cpp ${UTILS_INC}/ModelException.cpp)
set(MONGO_INC ${CMAKE_INSTALL_PREFIX}/commonlibs/MongoUtilClass)
set(MONGO_FILES ${MONGO_INC}/MongoUtil.cpp)
set(RASTER_INC ${CMAKE_INSTALL_PREFIX}/commonlibs/RasterClass)
set(RASTER_FILES ${RASTER_INC}/clsRasterData.cpp)
##############  Stand-alone programs   ###############
SET(FIELD_PARTITION ./src/field_partition)
SET(GRID_LAYERING ./src/grid_layering)
SET(IMPORT_RASTER ./src/import_raster)
SET(IUH ./src/iuh)
SET(METIS ./src/metis)
SET(TAUDEM ./src/taudem)
SET(SEIMS_MAIN ./src/seims_main)

ADD_SUBDIRECTORY(${TAUDEM} bin/taudem)
ADD_SUBDIRECTORY(${METIS} bin/metis)

IF(MSVC)
  SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF()
#SET(MODULE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/base/util/SimulationModule.cpp)
#INCLUDE_DIRECTORIES(${BSON_INC} ${MONGO_INC} ${GDAL_HOME} ${GDAL_HOME}/include ${BASE_UTIL} ${BASE_DATA} ${BASE_MODS} ${BASE_BMPS})
#LINK_DIRECTORIES(${GDAL_HOME}/lib ${LIB_DIR} ${PROJECT_BINARY_DIR})
#IF (SEIMSVERSION STREQUAL MPI)
#    INCLUDE_DIRECTORIES(./src/main/main_omp ${MPI_INCLUDE})
#    LINK_DIRECTORIES(${MPI_LIBPATH})
#    LINK_LIBRARIES(${MPI_LIB})
#ENDIF()
#SET(BASE_HOME ./src/base)
#SET(MODULE_HOME ./src/modules)
#SET(MAIN_HOME ./src/main)
#
#IF (WIN32)## copy external dll libraries to Debug and Release folder
#    SET(DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/dlls)
#    SET(EXTERNAL_DLLS "${DLL_DIR}/gdal15-vc9.dll" "${DLL_DIR}/libpng13-vc9.dll" "${DLL_DIR}/libexpat.dll" "${DLL_DIR}/libmongoc-1.0.dll" "${DLL_DIR}/zlib1-vc9.dll" "${DLL_DIR}/libbson-1.0.dll")
#    FILE(COPY ${EXTERNAL_DLLS} DESTINATION ${PROJECT_BINARY_DIR}/Debug)
#    FILE(COPY ${EXTERNAL_DLLS} DESTINATION ${PROJECT_BINARY_DIR}/Release)
#    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
#ENDIF ()
#
#ADD_SUBDIRECTORY(${BASE_HOME})
#ADD_SUBDIRECTORY(${MODULE_HOME})
#ADD_SUBDIRECTORY(${MAIN_HOME})

MESSAGE(STATUS "SEIMS is Ready for you!")
