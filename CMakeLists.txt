# ----------------------------------------------------------------------------
#  Root CMake file for SEIMS.
#     Spatially Explicit Integrated Modeling System
#
#     Please from the off-tree build directory, invoke:
#       $ cmake <path/to/SEIMS>
#
#  Routine testing platforms and compilers include:
#     1. Windows 10 with Visual Studio 2013, mongo-c-driver-1.5.5, GDAL-1.11.4
#     2. Windows 10 with mingw64 (GCC-4.9.3), mongo-c-driver-1.8.1, GDAL-1.11.5
#     3. CentOS 6.2 (cluster) with GCC-4.8.4, mongo-c-driver-1.5.5, GDAL-1.9.0
#     4. Red Hat Server 6.2 (cluster) with ICC-12.1.0, mongo-c-driver-1.5.5, GDAL-1.11.5
#     5. macOS 10.12.6 with Clang-8.0 (or GCC-4.9.3), mongo-c-driver-1.5.5, GDAL-1.11.4
#
#  Created and maintained by Liangjun Zhu (zlj@lreis.ac.cn)
#  Latest updated: Oct. 17, 2017
#
#  Contributors: Junzhi Liu, Hui Wu, Liangjun Zhu, Huiran Gao, etc.
#  Copyright (C) 2013-2017 Lreis, IGSNRR, CAS
# ----------------------------------------------------------------------------
### Disable in-source builds to prevent source tree corruption.
IF(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
    MESSAGE(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.")
ENDIF()
CMAKE_MINIMUM_REQUIRED(VERSION 3.1 FATAL_ERROR)

if(POLICY CMP0015)
    cmake_policy(SET CMP0015 NEW)
endif()
if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()
### SEIMS Version.
SET(PRJNAME "SEIMS")
IF (PARALLEL STREQUAL MPI)
    SET(PRJNAME ${PRJNAME}_MPI_OMP)
    SET(WITH_MPI 1)
    MESSAGE(STATUS "MPI&OpenMP version...")
ELSE ()
    SET(PRJNAME ${PRJNAME}_OMP)
    MESSAGE(STATUS "OpenMP version...")
ENDIF ()
SET(WITH_OMP 1)
SET(PRJNAME ${PRJNAME}_ALL)
PROJECT(${PRJNAME})

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

### OS platform.
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

### Break in case of popular CMake configuration mistakes.
IF(NOT CMAKE_SIZEOF_VOID_P GREATER 0)
    MESSAGE(FATAL_ERROR "CMake fails to determine the bitness of the target platform.
  Please check your CMake and compiler installation.")
ENDIF()

# Global flag to cause add_library to create shared libraries if on, otherwise static library.
option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)
# libraries built with hunter link against the dynamic CRT on windows
option(BUILD_WITH_STATIC_CRT "Build against dynamic CRT on windows." ON)

### Detect compiler and target platform architecture.
INCLUDE(Utils)
SET(ENABLE_CXX11 1)
INCLUDE(DetectCXXCompiler)
### Use statically or dynamically linked CRT? Default: dynamic
IF(MSVC)
    INCLUDE(CRTLinkage)
ENDIF(MSVC)

### Add standard paths or specified paths for Find libraries and headers.
INCLUDE(AddFindPaths)

### Find packages.
SET(WITH_GDAL 1)
SET(WITH_MONGOC 1)
SET(WITH_OPENMP 1)
INCLUDE(FindPackages)

### Add subdirectories.
ADD_SUBDIRECTORY(./seims)
# ADD_SUBDIRECTORY(./gui)  # TODO

### Build platform.
STATUS("")
STATUS("  Platform:")
IF(NOT CMAKE_VERSION VERSION_LESS 2.8.11 AND NOT BUILD_INFO_SKIP_TIMESTAMP)
    STRING(TIMESTAMP TIMESTAMP "" UTC)
    IF(TIMESTAMP)
        STATUS("    Timestamp:"    ${TIMESTAMP})
    ENDIF()
ENDIF()
STATUS("    Host:"             ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION} ${CMAKE_HOST_SYSTEM_PROCESSOR})
IF(CMAKE_CROSSCOMPILING)
    STATUS("    Target:"         ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION} ${CMAKE_SYSTEM_PROCESSOR})
ENDIF()
STATUS("    CMake:"            ${CMAKE_VERSION})
STATUS("    CMake generator:"  ${CMAKE_GENERATOR})
STATUS("    CMake build tool:" ${CMAKE_BUILD_TOOL})
IF(MSVC)
    STATUS("    MSVC:"           ${MSVC_VERSION})
ENDIF()
IF(CMAKE_GENERATOR MATCHES Xcode)
    STATUS("    Xcode:"          ${XCODE_VERSION})
ENDIF()
IF(NOT CMAKE_GENERATOR MATCHES "Xcode|Visual Studio")
    STATUS("    Configuration:"  ${CMAKE_BUILD_TYPE})
ENDIF()

### C/C++ options.
IF(CMAKE_CXX_COMPILER_VERSION)
    SET(GEO_COMPILER_STR "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} (ver ${CMAKE_CXX_COMPILER_VERSION})")
ELSE()
    SET(GEO_COMPILER_STR "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}")
ENDIF()
STRING(STRIP "${GEO_COMPILER_STR}" GEO_COMPILER_STR)

STATUS("")
STATUS("  C/C++:")
IF(ENABLE_CXX11 OR HAVE_CXX11)
    STATUS("    C++11:" HAVE_CXX11 THEN YES ELSE NO)
ENDIF()
STATUS("    C++ Compiler:"           ${GEO_COMPILER_STR})
STATUS("    C++ flags (Release):"    ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE})
STATUS("    C++ flags (Debug):"      ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG})
STATUS("    C Compiler:"             ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1})
STATUS("    C flags (Release):"      ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE})
STATUS("    C flags (Debug):"        ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG})
IF(WIN32)
    STATUS("    Linker flags (Release):" ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_RELEASE})
    STATUS("    Linker flags (Debug):"   ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
ELSE()
    STATUS("    Linker flags (Release):" ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
    STATUS("    Linker flags (Debug):"   ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
ENDIF()

### Dependencies.
IF(WITH_MPI)
    STATUS("    MPI: ${MPI_LIBRARIES} ${MPI_INCLUDE_PATH}")
ENDIF()
STATUS("    GDAL: ${GDAL_LIBRARIES} ${GDAL_INCLUDE_DIR}")
STATUS("    BSON: ${BSON_LIBRARIES} ${BSON_INCLUDE_DIR}")
STATUS("    MongoC: ${MONGOC_LIBRARIES} ${MONGOC_INCLUDE_DIR}")

### Auxiliary.
STATUS("")
STATUS("  Install path:" ${CMAKE_INSTALL_PREFIX})
STATUS("")
