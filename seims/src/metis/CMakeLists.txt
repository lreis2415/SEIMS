cmake_minimum_required(VERSION 3.1)
project(METIS)

set(GKLIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GKlib" CACHE PATH "path to GKlib")
set(SHARED FALSE CACHE BOOL "build a shared library")

set(METIS_INSTALL TRUE)
# Configure libmetis library.
if (SHARED)
    set(METIS_LIBRARY_TYPE SHARED)
else ()
    set(METIS_LIBRARY_TYPE STATIC)
endif (SHARED)

include(${GKLIB_PATH}/GKlibSystem.cmake)
# Add include directories.
include_directories(${GKLIB_PATH})
include_directories(include)
# Recursively look for CMakeLists.txt in subdirs.
add_subdirectory("include")
add_subdirectory("libmetis")
add_subdirectory("programs")
### For CLion to implement the "make install" command
add_custom_target(install_${PROJECT_NAME}
        $(MAKE) install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")
MESSAGE(STATUS "    Compiling METIS done")
