# ----------------------------------------------------------------------------
#  Root CMake file for UtilsClass.
#     A common used cross-platform utility class for C++.
#
#     Please build from the off-tree build directory, e.g.:
#       $ mkdir -p build
#       $ cd build
#       $ cmake ..
#     Optional cmake options:
#       -DUNITTEST=1  means run unittest based gtest/gmock
#       -DRUNCOV=1    means run code coverage based gcov and lcov
#
#  Routine testing platforms and compilers include:
#     1. Windows 10 with Visual Studio 2013
#     2. Windows 10 with mingw64 (GCC-4.9.3)
#     3. CentOS 6.2 (cluster) with GCC-4.8.4
#     4. Red Hat Server 6.2 (cluster) with ICC-12.1.0
#     5. macOS 10.12.6 with Clang-8.0 (or GCC-4.9.3)
#
#  Created and maintained by Liangjun Zhu (zlj@lreis.ac.cn)
#  Latest updated: Nov. 30, 2017
#
#  Contributors: Liangjun Zhu, Junzhi Liu, Hui Wu, Huiran Gao, etc.
#  Copyright (C) 2013-2017 Lreis, IGSNRR, CAS
# ----------------------------------------------------------------------------
### Disable in-source builds to prevent source tree corruption.
IF(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
    MESSAGE(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.")
ENDIF()
CMAKE_MINIMUM_REQUIRED(VERSION 3.1 FATAL_ERROR)
PROJECT(UtilsClass CXX C)
if(POLICY CMP0015)
    cmake_policy(SET CMP0015 NEW)
endif()
if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()
### Set cmake find path
set(CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        ${CMAKE_MODULE_PATH}
        )

### Set default or specified installation directory.
IF (INSTALL_PREFIX)
    SET(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX} CACHE PATH "Set specified install path" FORCE)
ELSEIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin CACHE PATH "Set default install path" FORCE)
ENDIF ()
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
### In case of Makefiles if the user does not setup CMAKE_BUILD_TYPE, assume it's Release.
IF(CMAKE_GENERATOR MATCHES "Makefiles|Ninja" AND "${CMAKE_BUILD_TYPE}" STREQUAL "")
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()
### Allow project folders in MSVC.
IF(MSVC)
    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF()

### Break in case of popular CMake configuration mistakes.
IF(NOT CMAKE_SIZEOF_VOID_P GREATER 0)
    MESSAGE(FATAL_ERROR "CMake fails to determine the bitness of the target platform.
  Please check your CMake and compiler installation."
            )
ENDIF()

# Global flag to cause add_library to create shared libraries if on, otherwise static library.
option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)
# libraries built with hunter link against the dynamic CRT on windows
option(BUILD_WITH_STATIC_CRT "Build against dynamic CRT on windows." ON)

### Detect compiler and target platform architecture.
SET(WITH_OPENMP 1)
SET(ENABLE_CXX11 1)
INCLUDE(Utils)
INCLUDE(DetectCXXCompiler)
### Add standard paths or specified paths for Find libraries and headers.
INCLUDE(AddFindPaths)
### Use statically or dynamically linked CRT? Default: dynamic
IF(MSVC)
    INCLUDE(CRTLinkage)
ENDIF(MSVC)

### Find packages.
INCLUDE(FindPackages)

### OS platform.
if (WIN32)
    add_definitions(-Dwindows)
    if (MSVC)
        add_definitions(-DMSVC)
    endif ()
elseif (APPLE)
    # -- Determine the version of OSX
    # -- 8 and less are OSX 10.0 - 10.4
    # -- 9 is 10.5 (LEOPARD)
    EXEC_PROGRAM(uname ARGS -v OUTPUT_VARIABLE DARWIN_VERSION)
    STRING(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
    MESSAGE(STATUS "DARWIN_VERSION=${DARWIN_VERSION}")
    if (DARWIN_VERSION GREATER 8)
        add_definitions(-Dmacos)
    else ()
        add_definitions(-Dmacosold)
    endif ()
else ()
    add_definitions(-Dlinux)
endif ()

### UtilsClass library
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(utilsclass ${SOURCE_FILES})
if (RUNCOV STREQUAL 1)
    set_target_properties(utilsclass PROPERTIES COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    set_target_properties(utilsclass PROPERTIES LINK_FLAGS "-coverage -lgcov")
endif()
### unittest based on gtest/gmock, which is optional.
IF (UNITTEST STREQUAL 1)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    ### Download and compile googletest
    include(DownloadProject)
    download_project(PROJ                googletest
            GIT_REPOSITORY      https://github.com/google/googletest.git
            GIT_TAG             master
            ${UPDATE_DISCONNECTED_IF_AVAILABLE}
            )
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
    include_directories("${gtest_SOURCE_DIR}/include" "${gmock_SOURCE_DIR}/include")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
ENDIF ()

### Copy test data to build directory.
if (MSVC)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test)
else()
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif ()