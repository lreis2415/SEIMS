project(ReClassify)
################ Add executables #################
set(SOURCE_FILES ${UTILS_FILES} reclassify_raster.cpp)
geo_include_directories(${GDAL_INCLUDE_DIR} ${UTILS_INC} ${RASTER_INC})
ADD_EXECUTABLE(reclassify ${SOURCE_FILES})
target_link_libraries(reclassify ${GDAL_LIBRARIES})
install(TARGETS reclassify DESTINATION ${INSTALL_DIR})

LIST(APPEND UTILS_APPS reclassify)
SET(UTILS_APPS "${UTILS_APPS}" PARENT_SCOPE)

IF (MSVC)
    SET_PROPERTY(TARGET reclassify PROPERTY FOLDER "UtilApps")
ENDIF ()
MESSAGE(STATUS "    Compiling ReClassify Raster done")
