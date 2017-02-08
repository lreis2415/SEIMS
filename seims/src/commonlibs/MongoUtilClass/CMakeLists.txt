# CMake build for Windows, Linux, and macOS
# Updated by Liang-Jun Zhu
# zlj@lreis.ac.cn
cmake_minimum_required(VERSION 2.8)
project(MongoUtil)
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
# 1. Bson and MongoC are required
include(FindBson.cmake)
include(FindMongoC.cmake)
# 2. Add support for OpenMP (if supported)
find_package(OpenMP)
IF (OPENMP_FOUND)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ENDIF ()
################ Add executables #################
set(UTILS_INC ${CMAKE_CURRENT_SOURCE_DIR}/../UtilsClass)
set(UTILS_FILES ${UTILS_INC}/utils.cpp  ${UTILS_INC}/ModelException.cpp)
set(SOURCE_FILES main.cpp MongoUtil.cpp)
include_directories(${UTILS_INC} ${BSON_INCLUDE_DIR} ${MONGOC_INCLUDE_DIR})
add_executable(MongoUtil ${SOURCE_FILES} ${UTILS_FILES})
target_link_libraries(MongoUtil ${BSON_LIBRARIES} ${MONGOC_LIBRARIES})
install(TARGETS MongoUtil DESTINATION bin)
################# Windows Only  ##################
# Since mongoc_lib_dir is stored in Environment path, there is no need to
# copy dynamic libraries to the bin directory.
# Just remain for memory. -- LJ
# IF(WIN32)
#   # for compile executables
#   install(FILES ${BSON_DLL} ${MONGOC_DLL} DESTINATION bin)
#   # for build vs project
#   IF(MSVC)
#     SET(EXTERNAL_DLLS ${BSON_DLL} ${MONGOC_DLL})
#     FILE(COPY ${EXTERNAL_DLLS} DESTINATION Debug)
#     FILE(COPY ${EXTERNAL_DLLS} DESTINATION Release)
#   ENDIF()
# ENDIF()
