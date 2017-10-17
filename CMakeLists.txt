# ----------------------------------------------------------------------------
#  Root CMake file for SEIMS.
#     Spatially Explicit Integrated Modeling System
#
#     Please from the off-tree build directory, invoke:
#       $ cmake <path/to/SEIMS>
#
#  Routine testing platforms and compilers include:
#     1. Windows 10 with Visual Studio 2013, mongo-c-driver-1.5.5
#     2. Windows 10 with mingw64 (GCC-4.9.3), mongo-c-driver-1.8.1
#     3. CentOS 6.2 (cluster) with GCC-4.8.4, mongo-c-driver-1.5.5
#     4. Red Hat Server 6.2 (cluster) with ICC-12.1.0, mongo-c-driver-1.5.5
#     5. macOS 10.12.6 with Clang-8.0 (or GCC-4.9.3), mongo-c-driver-1.5.5
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
CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0015 NEW)
    SET(CMAKE_MACOSX_RPATH 1)
ENDIF (COMMAND CMAKE_POLICY)

### SEIMS Version.
SET(PRJNAME "SEIMS")
IF (PARALLEL STREQUAL MPI)
    SET(PRJNAME ${PRJNAME}_MPI_OMP)
    SET(WITH_MPI 1)
    SET(WITH_OMP 1)
    MESSAGE(STATUS "MPI&OpenMP version...")
ELSE ()
    SET(PRJNAME ${PRJNAME}_OMP)
    SET(WITH_OMP 1)
    MESSAGE(STATUS "OpenMP version...")
ENDIF ()
SET(PRJNAME ${PRJNAME}_ALL)
PROJECT(${PRJNAME})

### Set default or specified installation directory.
IF (INSTALL_PREFIX)
    SET(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX})
ELSE ()
    SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin)
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

### Detect compiler and target platform architecture.
INCLUDE(cmake/Utils.cmake)
SET(ENABLE_CXX11 1)
INCLUDE(cmake/DetectCXXCompiler.cmake)
### Use statically or dynamically linked CRT? Default: dynamic
IF(MSVC)
    INCLUDE(cmake/CRTLinkage.cmake)
ENDIF(MSVC)

### Add standard paths or specified paths for Find libraries and headers.
INCLUDE(cmake/AddFindPaths.cmake)
# MESSAGE(STATUS ${CMAKE_PREFIX_PATH})

### Find packages.
SET(WITH_GDAL 1)
SET(WITH_MONGOC 1)
INCLUDE(cmake/FindPackages.cmake)

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
STATUS("    GDAL: ${MPI_LIBRARIES} ${MPI_INCLUDE_PATH}")
STATUS("    BSON: ${BSON_LIBRARIES}")
STATUS("    MongoC: ${MONGOC_LIBRARIES}")

### Auxiliary.
STATUS("")
STATUS("  Install path:" ${CMAKE_INSTALL_PREFIX})
STATUS("")
