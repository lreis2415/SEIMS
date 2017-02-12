# CMake build for Windows, Linux, and macOS
# Updated by Liang-Jun Zhu
# Description: Separated the CMakeLists.txt into two
# zlj@lreis.ac.cn
cmake_minimum_required(VERSION 2.8)
project(TauDEM)
IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0015 NEW)
ENDIF(COMMAND CMAKE_POLICY)
SET (CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

################# Find packages ##################
# MPI and GDAL are required
IF(WIN32)
    # 1. GDAL
    SET (GDAL_DIR $ENV{GDAL_DIR})
    SET (GDAL_DIR $ENV{GDAL_DIR})
    SET (GDAL_INCLUDE_DIR ${GDAL_DIR}\\include)
    SET (GDAL_LIBRARIES ${GDAL_DIR}\\lib)
    SET (GDAL_BIN_DIR ${GDAL_DIR}\\bin)
    SET (GDAL_LIBRARY ${GDAL_LIBRARIES}\\gdal_i.lib)
    # 2. MPI
    SET (MPI_LIB $ENV{MSMPI_LIB32})
    SET (MPI_HOME $ENV{MSMPI_BIN})
    SET (MPI_INCLUDE_PATH $ENV{MSMPI_INC})
    SET (MPI_LIBRARIES ${MPI_LIB}\\msmpi.lib)
ELSE()
    # for Linux and macOS
    # 1. GDAL, CMake will find GDAL and get GDAL_INCLUDE_DIR and GDAL_LIBRARY.
    find_package(GDAL REQUIRED)
    # 2. MPI
    find_package(MPI REQUIRED)
    set(CMAKE_CXX_FLAG ${CMAKE_CXX_FLAG} ${MPI_COMPILE_FLAGS})
    set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
ENDIF()
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR} ${MPI_INCLUDE_PATH})
################# Packages found ##################
SET (TAUDEM_SRC ./src)
ADD_SUBDIRECTORY(${TAUDEM_SRC})
### For CLion to implement the "make install" command
add_custom_target(install_${PROJECT_NAME}
        $(MAKE) install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")
MESSAGE(STATUS "    Compiling TauDEM done")
