# Read-Only variables:
#  MONGOC_FOUND - system has the MONGOC library
#  MONGOC_INCLUDE_DIR - the MONGOC include directory
#  MONGOC_LIBRARIES - The libraries needed to use MONGOC
#  MONGOC_VERSION - This is set to $major.$minor.$revision$path (eg. 0.4.1)

if (UNIX)
    find_package(PkgConfig QUIET)
    pkg_check_modules(_MONGOC QUIET libmongoc-1.0)
endif ()

find_path(MONGOC_INCLUDE_DIR
        NAMES
        libmongoc-1.0/mongoc.h
        HINTS
        ${MONGOC_ROOT_DIR}
        ${_MONGOC_INCLUDEDIR}
        PATH_SUFFIXES
        include
        )

set(MONGOC_INCLUDE_DIR "${MONGOC_INCLUDE_DIR}/libmongoc-1.0")

if (WIN32 AND NOT CYGWIN)
    if (MSVC)
        find_library(MONGOC
                NAMES
                "mongoc-1.0"
                HINTS
                ${MONGOC_ROOT_DIR}
                PATH_SUFFIXES
                bin
                lib
                )

        mark_as_advanced(MONGOC)
        set(MONGOC_LIBRARIES ${MONGOC} ws2_32)
    else ()
        # bother supporting this?
    endif ()
else ()

    find_library(MONGOC_LIBRARY
            NAMES
            mongoc-1.0
            HINTS
            ${_MONGOC_LIBDIR}
            PATH_SUFFIXES
            lib
            )

    mark_as_advanced(MONGOC_LIBRARY)

    find_package(Threads REQUIRED)

    set(MONGOC_LIBRARIES ${MONGOC_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

endif ()

if (MONGOC_INCLUDE_DIR)
    if (_MONGOC_VERSION)
        set(MONGOC_VERSION "${_MONGOC_VERSION}")
    elseif (MONGOC_INCLUDE_DIR AND EXISTS "${MONGOC_INCLUDE_DIR}/mongoc-version.h")
        file(STRINGS "${MONGOC_INCLUDE_DIR}/mongoc-version.h" mongoc_version_str
                REGEX "^#define[\t ]+MONGOC_VERSION[\t ]+\([0-9.]+\)[\t ]+$")

        string(REGEX REPLACE "^.*MONGOC_VERSION[\t ]+\([0-9.]+\)[\t ]+$"
                "\\1" MONGOC_VERSION "${mongoc_version_str}")
    endif ()
endif ()

include(FindPackageHandleStandardArgs)

if (MONGOC_VERSION)
    find_package_handle_standard_args(MONGOC
            REQUIRED_VARS
            MONGOC_LIBRARIES
            MONGOC_INCLUDE_DIR
            VERSION_VAR
            MONGOC_VERSION
            FAIL_MESSAGE
            "Could NOT find MONGOC version"
            )
else ()
    find_package_handle_standard_args(MONGOC "Could NOT find MONGOC"
            MONGOC_LIBRARIES
            MONGOC_INCLUDE_DIR
            )
endif ()

mark_as_advanced(MONGOC_INCLUDE_DIR MONGOC_LIBRARIES)
