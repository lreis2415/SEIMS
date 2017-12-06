##############  Stand-alone programs   ###############
SET(FIELD_PARTITION ${CMAKE_CURRENT_SOURCE_DIR}/src/field_partition)
SET(GRID_LAYERING ${CMAKE_CURRENT_SOURCE_DIR}/src/grid_layering)
SET(IMPORT_RASTER ${CMAKE_CURRENT_SOURCE_DIR}/src/import_raster)
SET(MASK_RASTER ${CMAKE_CURRENT_SOURCE_DIR}/src/mask_rasters)
SET(RECLASSIFY ${CMAKE_CURRENT_SOURCE_DIR}/src/reclassify)
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
##############  Set common libraries   ###############
set(COMMONLIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/commonlibs)
set(UTILS_INC ${COMMONLIBS_PATH}/UtilsClass)
set(UTILS_FILES ${UTILS_INC}/utils.cpp ${UTILS_INC}/ModelException.cpp)
set(MONGO_INC ${COMMONLIBS_PATH}/MongoUtilClass)
set(MONGO_FILES ${MONGO_INC}/MongoUtil.cpp)
SET(RASTER_INC ${COMMONLIBS_PATH}/RasterClass)
############  step into subdirectories   #############
ADD_SUBDIRECTORY(${FIELD_PARTITION} bin/field_partition)
ADD_SUBDIRECTORY(${GRID_LAYERING} bin/grid_layering)
ADD_SUBDIRECTORY(${IMPORT_RASTER} bin/import_raster)
ADD_SUBDIRECTORY(${MASK_RASTER} bin/mask_raster)
ADD_SUBDIRECTORY(${RECLASSIFY} bin/reclassify)
ADD_SUBDIRECTORY(${IUH} bin/iuh)
ADD_SUBDIRECTORY(${METIS} bin/metis)
ADD_SUBDIRECTORY(${TAUDEM} bin/taudem)
IF (PARALLEL STREQUAL MPI)
  ADD_SUBDIRECTORY(${SEIMS_MAIN} bin/seims_mpi_project)
ELSE()
  ADD_SUBDIRECTORY(${SEIMS_MAIN} bin/seims_omp_project)
ENDIF()

### For CLion to implement the "make install" command
add_custom_target(install_${PROJECT_NAME}
        $(MAKE) install
        DEPENDS ${UTILS_APPS} ${TAUDEM_EXT_APP} ${TAUDEM_APP} ${SEIMSPRJ}
        COMMENT "Installing ${PROJECT_NAME}")
MESSAGE(STATUS "All SEIMS related compiled done!")
