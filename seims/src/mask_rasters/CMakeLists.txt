project(Mask_Raster_prj)
################ Add executables #################
set(SOURCE_FILES ${UTILS_FILES} mask_rasters.cpp)
geo_include_directories(${GDAL_INCLUDE_DIR} ${UTILS_INC} ${RASTER_INC})
ADD_EXECUTABLE(mask_raster ${SOURCE_FILES})
target_link_libraries(mask_raster ${GDAL_LIBRARIES})
install(TARGETS mask_raster DESTINATION ${INSTALL_DIR})

LIST(APPEND UTILS_APPS mask_raster)
SET(UTILS_APPS "${UTILS_APPS}" PARENT_SCOPE)

IF (WIN32)
    SET_PROPERTY(TARGET mask_raster PROPERTY FOLDER "UtilApps")
ENDIF ()
MESSAGE(STATUS "    Compiling Mask Raster done")
