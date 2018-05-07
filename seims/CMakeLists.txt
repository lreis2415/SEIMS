### Use MongoDB as database if available
IF (BSON_FOUND AND MONGOC_FOUND)
  ADD_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB)
ENDIF ()

##############  Set CCGL               ###############
SET(CCGL_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/ccgl)
SET(CCGL_INC ${CCGL_LIB}/src)
FILE(GLOB CCGL_SRC ${CCGL_INC} *.h *.cpp)
##############  Stand-alone programs   ###############
SET(FIELD_PARTITION ${CMAKE_CURRENT_SOURCE_DIR}/src/field_partition)
SET(GRID_LAYERING ${CMAKE_CURRENT_SOURCE_DIR}/src/grid_layering)
SET(IMPORT_RASTER ${CMAKE_CURRENT_SOURCE_DIR}/src/import_raster)
SET(MASK_RASTER ${CMAKE_CURRENT_SOURCE_DIR}/src/mask_rasters)
SET(RECLASSIFY ${CMAKE_CURRENT_SOURCE_DIR}/src/reclassify)
SET(COMBINE_RASTER ${CMAKE_CURRENT_SOURCE_DIR}/src/combine_raster)
SET(IUH ${CMAKE_CURRENT_SOURCE_DIR}/src/iuh)
SET(METIS ${CMAKE_CURRENT_SOURCE_DIR}/src/metis)
SET(TAUDEM ${CMAKE_CURRENT_SOURCE_DIR}/src/taudem)
SET(SEIMS_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/src/seims_main)
### unit test is an optional configuration.
IF (UNITTEST STREQUAL 1)
    SET(SEIMS_TEST ${CMAKE_CURRENT_SOURCE_DIR}/src/test)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
    ADD_SUBDIRECTORY(${SEIMS_TEST} bin/test)
ENDIF ()

############  step into subdirectories   #############
ADD_SUBDIRECTORY(${CCGL_LIB} bin/ccgl)
ADD_SUBDIRECTORY(${FIELD_PARTITION} bin/field_partition)
ADD_SUBDIRECTORY(${GRID_LAYERING} bin/grid_layering)
ADD_SUBDIRECTORY(${IMPORT_RASTER} bin/import_raster)
ADD_SUBDIRECTORY(${MASK_RASTER} bin/mask_raster)
ADD_SUBDIRECTORY(${RECLASSIFY} bin/reclassify)
ADD_SUBDIRECTORY(${COMBINE_RASTER} bin/combine_raster)
ADD_SUBDIRECTORY(${IUH} bin/iuh)
ADD_SUBDIRECTORY(${METIS} bin/metis)
ADD_SUBDIRECTORY(${TAUDEM} bin/taudem)
ADD_SUBDIRECTORY(${SEIMS_MAIN} bin/seims_project)

### For CLion to implement the "make install" command
add_custom_target(install_${PROJECT_NAME}
        $(MAKE) install
        DEPENDS ${UTILS_APPS} ${TAUDEM_EXT_APP} ${TAUDEM_APP} ${SEIMSPRJ}
        COMMENT "Installing ${PROJECT_NAME}")
MESSAGE(STATUS "All SEIMS related compiled done!")
