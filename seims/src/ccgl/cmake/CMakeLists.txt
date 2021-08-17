# ----------------------------------------------------------------------------
#  Root CMake file for CCGL.
#     A simple Common Cross-platform Geographic Library.
#        + Basic operations on array, file system, string, and time, etc.
#        + Raster data class with support of GDAL
#        + Data management with support of MongoDB
#
#     Please build from the off-tree build directory, e.g.:
#       $ mkdir -p build
#       $ cd build
#       $ cmake ..
#     Optional cmake options:
#       -DUNITTEST=1  means run unittest based gtest/gmock
#       -DFORCEDOWNGTEST=1 means download and compile gtest no matter it has existed or not
#       -DRUNCOV=1    means run code coverage based gcov and lcov
#       -DLLVM_ROOT_DIR Specific the root directory of brew installed LLVM, e.g., /usr/local/opt/llvm
#       -DBUILD_DOC=1 means build CCGL documentation based on doxygen
#
#  Routine testing platforms and compilers include:
#     1. Windows 10 with Visual Studio 2010/2015, mongo-c-driver-1.5.5, GDAL-1.11.4/2.4.1
#     2. Windows 10 with msys2/mingw64 (GCC-8.2.0), mongo-c-driver-1.6.1, GDAL-2.3.2
#     3. CentOS 6.2 (cluster) with GCC-4.8.4, mongo-c-driver-1.5.5, GDAL-1.9.0
#     4. Red Hat Server 6.2 (cluster) with ICC-12.1.0, mongo-c-driver-1.6.1, GDAL-1.11.5 (Without UnitTest)
#     5. macOS 10.14.5 with Clang-10.0 with Xcode, mongo-c-driver-1.14.0, GDAL-2.4.2 (brew installed)
#     6. Windows 10 with Visual Studio 2013+Intel C++ Compiler 17.0, mongo-c-driver-1.5.5, GDAL-1.11.4
#
#  Created and maintained by Liangjun Zhu (zlj@lreis.ac.cn)
#  Latest updated: Aug. 18, 2019
#  Copyright (C) 2017-2019 Lreis, IGSNRR, CAS
# ----------------------------------------------------------------------------
### Disable in-source builds to prevent source tree corruption.
IF (" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
    MESSAGE(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.")
ENDIF ()
CMAKE_MINIMUM_REQUIRED(VERSION 3.1 FATAL_ERROR)

if(POLICY CMP0015)
    cmake_policy(SET CMP0015 NEW)
endif()
if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

### Set coding language
PROJECT(CCGL CXX C)

### Project name and output library name
PROJECT(CCGL)
SET(CCGLNAME ccgl)

### Set cmake find path
set(CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        ${CMAKE_MODULE_PATH}
        )

### Set default or specified installation directory.
IF (INSTALL_PREFIX)
    SET(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX} CACHE PATH "Set specified install path" FORCE)
ELSE()
    SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin CACHE PATH "Set default install path" FORCE)
ENDIF ()
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
SET(DOC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/doc" CACHE PATH "Path to the documentation")

### In case of Makefiles if the user does not setup CMAKE_BUILD_TYPE, assume it's Release.
IF(CMAKE_GENERATOR MATCHES "Makefiles|Ninja" AND "${CMAKE_BUILD_TYPE}" STREQUAL "")
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

### Allow project folders in MSVC.
IF(MSVC OR XCODE)
    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF()

### OS platform.
if (WIN32)
    add_definitions(-DWINDOWS)
    if (MSVC)
        add_definitions(-DMSVC)
    endif ()
elseif (APPLE)
    add_definitions(-DMACOSX)
    if (XCODE)
        add_definitions(-DXCODE)
    endif ()
else ()
    add_definitions(-DLINUX)
endif ()

### Assume built-in pthreads on macOS X
IF(APPLE)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
ENDIF()

### Break in case of popular CMake configuration mistakes.
IF(NOT CMAKE_SIZEOF_VOID_P GREATER 0)
    MESSAGE(FATAL_ERROR "CMake fails to determine the bitness of the target platform.
  Please check your CMake and compiler installation.")
ENDIF()

# Global flag to cause add_library to create shared libraries if on, otherwise static library.
option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)
# libraries built with hunter link against the dynamic CRT on windows
option(BUILD_WITH_STATIC_CRT "Build against dynamic CRT on windows." ON)
# Build documatation by doxygen
option(BUILD_DOC "Build CCGL documentation." OFF)

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

### Use GNU standard installation directories
INCLUDE(GNUInstallDirs)

### CCGL library
FILE(GLOB INC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
FILE(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
IF(UNIX)
    IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_ICC)
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
        geo_list_unique(CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    ENDIF()
ENDIF()

ADD_LIBRARY(${CCGLNAME} ${SRC_FILES})
SET_TARGET_PROPERTIES(${CCGLNAME} PROPERTIES PUBLIC_HEADER "${INC_FILES}")
GET_DIRECTORY_PROPERTY(hasParent PARENT_DIRECTORY)
IF (hasParent)
    SET(CCGLNAME "${CCGLNAME}" PARENT_SCOPE)
ENDIF ()

### unittest based on gtest/gmock, which is optional.
SET(GTestTAG release-1.8.1)
IF(MSVC_VERSION LESS 1900)
    SET(GTestTAG release-1.8.0)
ENDIF()
### Find or build Googletest if UNITTEST is set as True or 1
SET(DOWNLOAD_GTEST 0)
IF (UNITTEST STREQUAL 1)
    ADD_DEFINITIONS(-DUNITTEST)
    IF (FORCEDOWNGTEST STREQUAL 1)
        SET(DOWNLOAD_GTEST 1)
    ELSE ()
        find_package(GTest)
        IF (GTEST_FOUND)
            geo_include_directories(${GTEST_INCLUDE_DIRS})
        ELSE ()
            SET(DOWNLOAD_GTEST 1)
        ENDIF ()
    ENDIF ()
ENDIF()
IF (DOWNLOAD_GTEST)
    MESSAGE(STATUS "GoogleTest Version: ${GTestTAG}")
    geo_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    ### Download and compile googletest
    INCLUDE(DownloadProject)
    download_project(PROJ                googletest
            GIT_REPOSITORY      https://github.com/google/googletest.git
            GIT_TAG             ${GTestTAG}
            ${UPDATE_DISCONNECTED_IF_AVAILABLE}
            )
    ADD_SUBDIRECTORY(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
    geo_include_directories(${gtest_SOURCE_DIR}/include)
    geo_include_directories(${gmock_SOURCE_DIR}/include)
ENDIF ()

### Find OMP which is optional
IF(XCODE)
  # To use OpenMP in Xcode built by CMake, the recommended way is to
  #   install llvm by 'brew install llvm', which has builtin 'libomp'
  IF (NOT LLVM_ROOT_DIR)
    SET(LLVM_ROOT_DIR "/usr/local/opt/llvm")
  ENDIF ()
  FIND_PACKAGE(LLVM)
  IF(LLVM_FOUND)
    geo_include_directories(${LLVM_LIBRARY_DIRS}/clang/${LLVM_VERSION_BASE_STRING}/include)
	SET(OpenMP_CXX_FLAGS "-fopenmp")
    SET(OpenMP_C_FLAGS "-fopenmp")
	FIND_LIBRARY(OpenMP_LIBRARY
		NAMES "omp"
		HINTS ${LLVM_LIBRARY_DIRS})
    SET(OPENMP_FOUND 1)
  ENDIF ()
ELSE ()
  FIND_PACKAGE(OpenMP)
ENDIF ()
IF(OPENMP_FOUND)
  SET(WITH_OPENMP 1)
  ADD_DEFINITIONS(-DSUPPORT_OMP)
  INCLUDE(SetFlagOpenMP)
  MESSAGE(STATUS "Compiling with OpenMP...")
ENDIF()

### For Xcode generator, specific LLVM-Clang installed by brew, and add link library of OpenMP explicitly.
IF(XCODE AND LLVM_FOUND)
    TARGET_LINK_LIBRARIES(${CCGLNAME} ${OpenMP_LIBRARY})
	SET_TARGET_PROPERTIES(${CCGLNAME} PROPERTIES XCODE_ATTRIBUTE_CC ${LLVM_ROOT_DIR}/bin/clang)
	SET_TARGET_PROPERTIES(${CCGLNAME} PROPERTIES XCODE_ATTRIBUTE_CXX ${LLVM_ROOT_DIR}/bin/clang++)
	SET_TARGET_PROPERTIES(${CCGLNAME} PROPERTIES XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE "No")
ENDIF()

### Find GDAL which is optional.
FIND_PACKAGE(GDAL)
IF (GDAL_FOUND)
  SET(WITH_GDAL 1)
  ADD_DEFINITIONS(-DUSE_GDAL)
  INCLUDE(SetFlagGDAL)
  geo_include_directories(${GDAL_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(${CCGLNAME} ${GDAL_LIBRARIES})
  SET_TARGET_PROPERTIES(${CCGLNAME} PROPERTIES LINKER_LANGUAGE CXX)
  MESSAGE(STATUS "Compiling with GDAL...")
ENDIF ()

### Find mongo-c-driver which is optional
IF(BSON_ROOT)
  SET(ENV{BSON_ROOT} ${BSON_ROOT})
  SET(ENV{BSON_DIR} ${BSON_ROOT})
  SET(ENV{BSON_ROOT_DIR} ${BSON_ROOT})
ENDIF()
IF(MONGOC_ROOT)
  SET(ENV{MONGOC_ROOT} ${MONGOC_ROOT})
  SET(ENV{MONGOC_DIR} ${MONGOC_ROOT})
  SET(ENV{MONGOC_ROOT_DIR} ${MONGOC_ROOT})
ENDIF()
INCLUDE(FindMongoC)
INCLUDE(FindBson)
IF (BSON_FOUND AND MONGOC_FOUND)
  SET(WITH_MONGOC 1)
  ADD_DEFINITIONS(-DUSE_MONGODB)
  geo_include_directories(${BSON_INCLUDE_DIR} ${MONGOC_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(${CCGLNAME} ${BSON_LIBRARIES} ${MONGOC_LIBRARIES})
  MESSAGE(STATUS "Compiling with mongo-c-driver...")
ENDIF ()

### Set code coverage linkage.
IF (RUNCOV STREQUAL 1)
    SET(UNITTEST 1) # Code coverage needs the UnitTest
    SET_TARGET_PROPERTIES(${CCGLNAME} PROPERTIES COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    SET_TARGET_PROPERTIES(${CCGLNAME} PROPERTIES LINK_FLAGS "-coverage -lgcov")
ENDIF ()

IF (UNITTEST STREQUAL 1)
    ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/test)
    ### Copy test data to build directory for UnitTest Only!
    MAKE_DIRECTORY(${INSTALL_DIR})
    FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${INSTALL_DIR})
    # For the convenient of debugging in MSVC and CLion
    IF (MSVC OR XCODE)
        FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test/Debug)
        FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test/Release)
    ELSE ()
        FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test)
    ENDIF ()
ENDIF ()

# Install CCGL, including include files and library file
INSTALL(TARGETS ${CCGLNAME}
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
### For CLion to implement the "make install" command
ADD_CUSTOM_TARGET(install_${PROJECT_NAME}
        $(MAKE) install
        DEPENDS ${CCGLNAME}
        COMMENT "Installing ${PROJECT_NAME}")

### Build doc based on doxygen
IF(BUILD_DOC)
  ADD_SUBDIRECTORY(./doc)
  ADD_CUSTOM_TARGET(travis_doc)
  ADD_CUSTOM_COMMAND(TARGET travis_doc
                     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/doc/travis-doxygen.sh)
ENDIF()

### Build platform.
STATUS("")
STATUS("  Platform:")
IF(NOT CMAKE_VERSION VERSION_LESS 2.8.11 AND NOT BUILD_INFO_SKIP_TIMESTAMP)
    STRING(TIMESTAMP TIMESTAMP "" UTC)
    IF(TIMESTAMP)
        STATUS("    Timestamp:"  ${TIMESTAMP})
    ENDIF()
ENDIF()
STATUS("    Host:"               ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION} ${CMAKE_HOST_SYSTEM_PROCESSOR})
IF(CMAKE_CROSSCOMPILING)
    STATUS("    Target:"         ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION} ${CMAKE_SYSTEM_PROCESSOR})
ENDIF()
STATUS("    CMake:"              ${CMAKE_VERSION})
STATUS("    CMake generator:"    ${CMAKE_GENERATOR})
STATUS("    CMake build tool:"   ${CMAKE_BUILD_TOOL})
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
STATUS("    Use OpenMP:"  OPENMP_FOUND   THEN "YES" ELSE "NO")
STATUS("    Use GDAL:"    GDAL_FOUND     THEN "YES (LIB: ${GDAL_LIBRARIES}, INC: ${GDAL_INCLUDE_DIR})" ELSE "NO")
STATUS("    Use BSON:"    BSON_FOUND     THEN "YES (LIB: ${BSON_LIBRARIES}, INC: ${BSON_INCLUDE_DIR})" ELSE "NO")
STATUS("    Use MongoC:"  MONGOC_FOUND   THEN "YES (LIB: ${MONGOC_LIBRARIES}, INC: ${MONGOC_INCLUDE_DIR})" ELSE "NO")

### Auxiliary.
STATUS("")
STATUS("  Install path:" ${INSTALL_DIR})
STATUS("")
