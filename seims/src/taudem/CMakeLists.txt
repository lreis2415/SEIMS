# ----------------------------------------------------------------------------
#  Root CMake file for extended TauDEM.
#
#     Please from the off-tree build directory, invoke:
#       $ cmake <path/to/TauDEM_ext>
#
#  Routine testing platforms and compilers include:
#     1. Windows 10 with Visual Studio 2013, MSMPI-v8, GDAL-1.11.4
#     2. Windows 10 with mingw64 (GCC-4.9.3), MSMPI-v8, GDAL-1.11.5
#     3. CentOS 6.2 (cluster) with GCC-4.8.4, MPICH-3.1.4, GDAL-1.9.0
#     4. Red Hat Server 6.2 (cluster) with ICC-12.1.0, Intel MPI 4.0, GDAL-1.11.5
#     5. macOS 10.12.6 with Clang-8.0 (or GCC-4.9.3), OpenMPI-1.10.4, GDAL-1.11.4 (Framework)
#
#  Created and maintained by Liang-Jun Zhu (zlj@lreis.ac.cn)
#  Latest updated: Mar. 4, 2018
# ----------------------------------------------------------------------------
### Disable in-source builds to prevent source tree corruption.
IF(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  MESSAGE(FATAL_ERROR "
FATAL: In-source builds are not allowed. 
       You should create a separate directory for build files."
)
ENDIF()
CMAKE_MINIMUM_REQUIRED(VERSION 3.1 FATAL_ERROR)
PROJECT(TauDEM_ext CXX C)

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
IF(MSVC OR XCODE)
  SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF()

### Break in case of popular CMake configuration mistakes.
IF(NOT CMAKE_SIZEOF_VOID_P GREATER 0)
  MESSAGE(FATAL_ERROR "CMake fails to determine the bitness of the target platform.
  Please check your CMake and compiler installation."
)
ENDIF()

### Detect compiler and target platform architecture.
INCLUDE(Utils)
SET(ENABLE_CXX11 1)
INCLUDE(DetectCXXCompiler)

### Add standard paths or specified paths for Find libraries and headers.
INCLUDE(AddFindPaths)

### Find packages.

### Find MPI which is required
FIND_PACKAGE(MPI REQUIRED)
IF(MPI_FOUND)
  SET(WITH_MPI 1)
  MESSAGE(STATUS "Compiling with MPI...")
ELSE()
  MESSAGE(FATAL_ERROR "FATAL: Could not find MPI, which is required for TauDEM and SEIMS!")
ENDIF()

### Find OMP which is optional
FIND_PACKAGE(OpenMP)
IF(OPENMP_FOUND)
  SET(WITH_OPENMP 1)
  ADD_DEFINITIONS(-DSUPPORT_OMP)
  MESSAGE(STATUS "Compiling with OpenMP...")
ENDIF()

### Find GDAL which is required.
FIND_PACKAGE(GDAL REQUIRED)
IF (GDAL_FOUND)
  SET(WITH_GDAL 1)
ELSE ()
  MESSAGE(FATAL_ERROR "FATAL: Could not find GDAL!")
ENDIF ()

### Set specific packages.
INCLUDE(SetFlagMPI)
INCLUDE(SetFlagOpenMP)
INCLUDE(SetFlagGDAL)

### Use statically or dynamically linked CRT? Default: dynamic
IF(MSVC)
  INCLUDE(CRTLinkage)
ENDIF(MSVC)
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

SET(TAUDEM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(TAUDEM_EXT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src_ext)

geo_include_directories(${GDAL_INCLUDE_DIR} ${MPI_INCLUDE_PATH})
ADD_SUBDIRECTORY(${TAUDEM_SRC})
ADD_SUBDIRECTORY(${TAUDEM_EXT_SRC})
### For CLion to implement the "make install" command
ADD_CUSTOM_TARGET(install_${PROJECT_NAME}
        $(MAKE) install
        DEPENDS ${TAUDEM_EXT_APP} ${TAUDEM_APP}
        COMMENT "Installing ${PROJECT_NAME}")

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
STATUS("")
STATUS("    Use MPI:"     MPI_FOUND      THEN "YES (LIB: ${MPI_LIBRARIES}, INC: ${MPI_INCLUDE_PATH})" ELSE "NO")
STATUS("    Use OpenMP:"  OPENMP_FOUND   THEN "YES" ELSE "NO")
STATUS("    Use GDAL:"    GDAL_FOUND     THEN "YES (LIB: ${GDAL_LIBRARIES}, INC: ${GDAL_INCLUDE_DIR})" ELSE "NO")

### Auxiliary.
STATUS("")
STATUS("  Install path:" ${INSTALL_DIR})
STATUS("")