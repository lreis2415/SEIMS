##############  Stand-alone programs   ###############
SET(FIELD_PARTITION ./src/field_partition)
SET(GRID_LAYERING ./src/grid_layering)
SET(IMPORT_RASTER ./src/import_raster)
SET(MASK_RASTER ./src/mask_rasters)
SET(RECLASSIFY ./src/reclassify)
SET(IUH ./src/iuh)
SET(METIS ./src/metis)
SET(TAUDEM ./src/taudem)
SET(SEIMS_MAIN ./src/seims_main)
### unit test is an optional configuration.
IF (UNITTEST STREQUAL 1)
    SET(SEIMS_TEST ./src/test)
    INCLUDE_DIRECTORIES(.)
    ADD_SUBDIRECTORY(${SEIMS_TEST} bin/test)
ENDIF ()
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
        DEPENDS ${EXT_TARGETS} ${MY_TARGETS}
        COMMENT "Installing ${PROJECT_NAME}")
MESSAGE(STATUS "All SEIMS related compiled done!")
