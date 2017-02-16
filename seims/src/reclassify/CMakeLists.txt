# CMake build for Windows, Linux, and macOS
# Updated by Liang-Jun Zhu
# zlj@lreis.ac.cn
cmake_minimum_required(VERSION 3.1)
project(ReClassify_prj)
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0015 NEW)
    SET(CMAKE_MACOSX_RPATH 1)
ENDIF (COMMAND CMAKE_POLICY)
SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
### Active C++11 standard, cross-platform. This requires CMake 3.1+
SET (CMAKE_CXX_STANDARD 11)
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
ELSE ()
    # 1. GDAL CMake will find GDAL and get GDAL_INCLUDE_DIR and GDAL_LIBRARY.
    find_package(GDAL REQUIRED)
ENDIF ()
# 4. Add support for OpenMP
FIND_PACKAGE(OpenMP)
IF (OPENMP_FOUND)
    add_definitions(-DSUPPORT_OMP)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ENDIF ()
##############  Set common libraries   ###############
set(UTILS_INC ${CMAKE_CURRENT_SOURCE_DIR}/../commonlibs/UtilsClass)
set(UTILS_FILES ${UTILS_INC}/utils.cpp ${UTILS_INC}/ModelException.cpp)
SET(RASTER_INC ${CMAKE_CURRENT_SOURCE_DIR}/../commonlibs/RasterClass)
SET(RASTER_FILES ${RASTER_INC}/clsRasterData.cpp)
################ Add executables #################
set(SOURCE_FILES ${UTILS_FILES} ${RASTER_FILES} reclassify_raster.cpp)
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR} ${UTILS_INC} ${RASTER_INC})
ADD_EXECUTABLE(reclassify ${SOURCE_FILES})
target_link_libraries(reclassify ${GDAL_LIBRARY})
install(TARGETS reclassify DESTINATION bin)
### For CLion to implement the "make install" command
add_custom_target(install_${PROJECT_NAME}
        $(MAKE) install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")
IF(WIN32)
	SET_PROPERTY(TARGET reclassify PROPERTY FOLDER "ReClassify")
ENDIF()
MESSAGE(STATUS "    Compiling ReClassify Raster done")
