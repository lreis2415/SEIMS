set(UT_NAME_STR seims_ut)
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
    include(FindBson)
    include(FindMongoC)
    IF (PARALLEL STREQUAL MPI)
        # 3. MPI
        IF (ARCH EQUAL 64)
            SET(MPI_LIB $ENV{MSMPI_LIB64})
        ELSE ()
            SET(MPI_LIB $ENV{MSMPI_LIB32})
        ENDIF ()
        SET(MPI_HOME $ENV{MSMPI_BIN})
        SET(MPI_INCLUDE_PATH $ENV{MSMPI_INC})
        SET(MPI_LIBRARIES ${MPI_LIB}\\msmpi.lib)
    ENDIF ()
ELSE ()
    # 1. GDAL CMake will find GDAL and get GDAL_INCLUDE_DIR and GDAL_LIBRARY.
    find_package(GDAL REQUIRED)
    # 2. Bson and MongoC
    include(FindBson)
    include(FindMongoC)
    IF (PARVERSION STREQUAL MPI)
        # 3. MPI
        find_package(MPI REQUIRED)
        set(CMAKE_CXX_FLAG "${CMAKE_CXX_FLAG} ${MPI_COMPILE_FLAGS}")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS}")
    ENDIF ()
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
MESSAGE(STATUS "CHECK CXX STANDARD FLAGS...")
foreach(flag ${ENABLE_CXXFLAGS_TO_CHECK})
    string(REPLACE "-std=" "_" flag_var ${flag})
    string(REPLACE "+" "x" flag_var ${flag_var})
    check_cxx_compiler_flag("${flag}" COMPILER_HAS_CXX_FLAG${flag_var})
    if(COMPILER_HAS_CXX_FLAG${flag_var})
        MESSAGE(STATUS "${flag}...yes")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
        break()
    endif()
endforeach()
MESSAGE(STATUS "USED CXX FLAGS: ${CMAKE_CXX_FLAGS}")
include(SetThreadFlagMSVC)
IF (MSVC)
    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF ()


###########################################################
######              unittest executable        ############
###########################################################
enable_testing()
set(PROJECT_TEST_NAME ${UT_NAME_STR}_exec)
file(GLOB TEST_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(${PROJECT_TEST_NAME} ${TEST_SRC_FILES})
target_link_libraries(${PROJECT_TEST_NAME} ${PROJECT_LIB_NAME} gtest gmock_main)
## here is the template for adding another unittest of a module
# 1. Create a unittest_MODULEID.cpp
# 2. Add target_link_libraries(${PROJECT_TEST_NAME} MODULEID) in this file
target_link_libraries(${PROJECT_TEST_NAME} util)
add_test(${PROJECT_TEST_NAME} ${PROJECT_TEST_NAME})
install(TARGETS ${PROJECT_TEST_NAME} DESTINATION ${INSTALL_DIR})
IF (MSVC)
    SET_PROPERTY(TARGET ${PROJECT_TEST_NAME} PROPERTY FOLDER "unittest")
ENDIF ()
