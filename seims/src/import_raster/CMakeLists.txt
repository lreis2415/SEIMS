# CMake build for Windows, Linux, and macOS
# Updated by Liang-Jun Zhu
# zlj@lreis.ac.cn
cmake_minimum_required(VERSION 3.1)
project(Import_Raster_prj)
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0015 NEW)
    SET(CMAKE_MACOSX_RPATH 1)
ENDIF (COMMAND CMAKE_POLICY)
SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

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
ELSE ()
    # 1. GDAL CMake will find GDAL and get GDAL_INCLUDE_DIR and GDAL_LIBRARY.
    find_package(GDAL REQUIRED)
    # 2. Bson and MongoC
    include(FindBson.cmake)
    include(FindMongoC.cmake)
ENDIF ()
# 4. Add support for OpenMP
FIND_PACKAGE(OpenMP)
IF (OPENMP_FOUND)
    add_definitions(-DSUPPORT_OMP)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ENDIF ()
### Active C++11 standard, cross-platform. This requires CMake 3.1+
# SET(CMAKE_CXX_STANDARD 11) CMake may make mistake in intel c/c++ compiler
# 12.1, so make some test when setting the C++ Standard.
# refers to https://github.com/biicode/client/issues/10
include(CheckCXXCompilerFlag)
set(ENABLE_CXXFLAGS_TO_CHECK
    -std=gnu++11 
    -std=c++11
    -std=gnu++0x 
    -std=c++0x)

foreach(flag ${ENABLE_CXXFLAGS_TO_CHECK})
    string(REPLACE "-std=" "_" flag_var ${flag})
    string(REPLACE "+" "x" flag_var ${flag_var})
    check_cxx_compiler_flag("${flag}" COMPILER_HAS_CXX_FLAG${flag_var})
    if(COMPILER_HAS_CXX_FLAG${flag_var})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
        break()
    endif()
endforeach()
##############  Set common libraries   ###############
set(UTILS_INC ${CMAKE_CURRENT_SOURCE_DIR}/../commonlibs/UtilsClass)
set(UTILS_FILES ${UTILS_INC}/utils.cpp ${UTILS_INC}/ModelException.cpp)
set(MONGO_INC ${CMAKE_CURRENT_SOURCE_DIR}/../commonlibs/MongoUtilClass)
set(MONGO_FILES ${MONGO_INC}/MongoUtil.cpp)
SET(RASTER_INC ${CMAKE_CURRENT_SOURCE_DIR}/../commonlibs/RasterClass)
SET(RASTER_FILES ${RASTER_INC}/clsRasterData.cpp)
################ Add executables #################
set(SOURCE_FILES ${UTILS_FILES} ${RASTER_FILES} ${MONGO_FILES} SubBasin.cpp main.cpp)
ADD_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB)
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR} ${BSON_INCLUDE_DIR} ${MONGOC_INCLUDE_DIR} ${UTILS_INC} ${RASTER_INC} ${MONGO_INC})
ADD_EXECUTABLE(import_raster ${SOURCE_FILES})
target_link_libraries(import_raster ${GDAL_LIBRARY} ${BSON_LIBRARIES} ${MONGOC_LIBRARIES})
install(TARGETS import_raster DESTINATION ${INSTALL_DIR})
### For CLion to implement the "make install" command
add_custom_target(install_${PROJECT_NAME}
        $(MAKE) install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")
IF (WIN32)
    SET_PROPERTY(TARGET import_raster PROPERTY FOLDER "RasterUtility")
ENDIF ()
MESSAGE(STATUS "    Compiling Import Raster done")
