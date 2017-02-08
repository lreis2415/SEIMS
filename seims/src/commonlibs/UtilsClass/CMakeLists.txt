# CMake build for Windows, Linux, and macOS
# Updated by Liang-Jun Zhu
# zlj@lreis.ac.cn
cmake_minimum_required(VERSION 2.8)
project(UtilsClass)
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0015 NEW)
ENDIF (COMMAND CMAKE_POLICY)
SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
################# OS platform   ##################
if (WIN32)
    add_definitions(-Dwindows)
    if (MSVC)
        add_definitions(-DMSVC)
    endif ()
elseif (APPLE)
	# -- Determine the version of OSX
	# -- 8 and less are OSX 10.0 - 10.4
	# -- 9 is 10.5 (LEOPARD)
	EXEC_PROGRAM(uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
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
set(SOURCE_FILES main.cpp utils.cpp ModelException.cpp)
add_executable(UtilsClass ${SOURCE_FILES})
install(TARGETS ${UtilsClass} DESTINATION bin)
