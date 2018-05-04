### GDAL
IF (WITH_GDAL)
  IF (NOT DEFINED GDAL_LIBRARIES)
    FIND_PACKAGE(GDAL REQUIRED)
  ENDIF ()
  IF(NOT GDAL_FOUND)
    MESSAGE(FATAL_ERROR "FATAL: Could not find GDAL!")
  ENDIF()
ENDIF()

# Fixup GDAL variable.
IF(WIN32 AND MINGW)
    # On Windows, find_package always selects the release dll named
    # libgdal.dll. This is a symbolic links which trips the linker. Also,
    # for debug builds, we want to link against the debug library.
    # Before fixing this, verify that the variable contains a single string
    # and that the name is libgdal.dll. The CMake module may be fixed at
    # some point.
    LIST(LENGTH GDAL_LIBRARIES NR_GDAL_LIBRARIES)
    IF(NR_GDAL_LIBRARIES EQUAL 1)
        GET_FILENAME_COMPONENT(GDAL_LIBRARY_NAME ${GDAL_LIBRARIES} NAME)
        GET_FILENAME_COMPONENT(GDAL_LIBRARY_PATH ${GDAL_LIBRARIES} DIRECTORY)
        FILE(GLOB GDAL_LIBWITH_VER "${GDAL_LIBRARY_PATH}/${GDAL_LIBRARY_NAME}.1.*.*")
        GET_FILENAME_COMPONENT(GDAL_VERSION ${GDAL_LIBWITH_VER} EXT)
        STRING(REPLACE ".dll.1" "1" GDAL_VERSION ${GDAL_VERSION})
        IF(GDAL_LIBRARY_NAME STREQUAL "libgdal.dll")
            SET(OPTIMIZED_GDAL_LIBRARY ${GDAL_LIBRARIES}.${GDAL_VERSION})
            SET(DEBUG_GDAL_LIBRARY ${GDAL_LIBRARIES}.${GDAL_VERSION})
            STRING(REPLACE gdal gdald DEBUG_GDAL_LIBRARY ${DEBUG_GDAL_LIBRARY})
            SET(GDAL_LIBRARIES
                optimized ${OPTIMIZED_GDAL_LIBRARY}
                debug ${DEBUG_GDAL_LIBRARY})
        ENDIF()
    ENDIF()
ENDIF()