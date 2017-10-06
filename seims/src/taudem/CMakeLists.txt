# CMake build for Windows, Linux, and macOS
# Updated by Liang-Jun Zhu
# Description: Separated the CMakeLists.txt into two
# zlj@lreis.ac.cn
cmake_minimum_required(VERSION 2.8)
project(TauDEM)
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0015 NEW)
ENDIF (COMMAND CMAKE_POLICY)
################# Install directory ##################
IF (INSTALL_PREFIX)
    SET(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX})
    SET(INSTALL_DIR ${INSTALL_PREFIX})
ELSE ()
    SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
    SET(INSTALL_DIR ${CMAKE_SOURCE_DIR}/bin)
ENDIF ()
################# Find packages ##################
# MPI and GDAL are required
IF (WIN32)
    # 1. GDAL
    SET(GDAL_DIR $ENV{GDAL_DIR})
    SET(GDAL_DIR $ENV{GDAL_DIR})
    SET(GDAL_INCLUDE_DIR ${GDAL_DIR}\\include)
    SET(GDAL_LIBRARIES ${GDAL_DIR}\\lib)
    SET(GDAL_BIN_DIR ${GDAL_DIR}\\bin)
    SET(GDAL_LIBRARY ${GDAL_LIBRARIES}\\gdal_i.lib)
    # 2. MPI
    IF (ARCH EQUAL 64)
        SET(MPI_LIB $ENV{MSMPI_LIB64})
    ELSE ()
        SET(MPI_LIB $ENV{MSMPI_LIB32})
    ENDIF ()
    SET(MPI_HOME $ENV{MSMPI_BIN})
    SET(MPI_INCLUDE_PATH $ENV{MSMPI_INC})
    SET(MPI_LIBRARIES ${MPI_LIB}\\msmpi.lib)
ELSE ()
    # for Linux and macOS
    # 1. GDAL, CMake will find GDAL and get GDAL_INCLUDE_DIR and GDAL_LIBRARY.
    find_package(GDAL REQUIRED)
    # 2. MPI
    find_package(MPI REQUIRED)
    set(CMAKE_CXX_FLAG ${CMAKE_CXX_FLAG} ${MPI_COMPILE_FLAGS})
    set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
    IF (APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "-Wl -headerpad_max_install_names")
    ENDIF ()
ENDIF ()
# Set a default build type for single-configuration
# CMake generators if no build type is set.
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF()

# Select flags.
# refers to https://software.intel.com/en-us/articles/intel-cluster-toolkit-for-linux-error-when-compiling-c-aps-using-intel-mpi-library-compilation-driver-mpiicpc
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")

INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR} ${MPI_INCLUDE_PATH})
################# Packages found ##################
SET(TAUDEM_SRC ./src)
ADD_SUBDIRECTORY(${TAUDEM_SRC})
### For CLion to implement the "make install" command
add_custom_target(install_${PROJECT_NAME}
        $(MAKE) install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")
MESSAGE(STATUS "    Compiling TauDEM done")
