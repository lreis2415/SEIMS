project(Mask_Raster_prj)
################ Add executables #################
file(GLOB MR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SOURCE_FILES ${UTILS_FILES} ${MR_SRC})
geo_include_directories(${GDAL_INCLUDE_DIR} ${UTILS_INC} ${RASTER_INC})
ADD_EXECUTABLE(mask_raster ${SOURCE_FILES})
target_link_libraries(mask_raster ${GDAL_LIBRARIES})
install(TARGETS mask_raster DESTINATION ${INSTALL_DIR})

LIST(APPEND UTILS_APPS mask_raster)
SET(UTILS_APPS "${UTILS_APPS}" PARENT_SCOPE)

IF (MSVC OR XCODE)
    SET_PROPERTY(TARGET mask_raster PROPERTY FOLDER "UtilApps")
ENDIF ()
MESSAGE(STATUS "    Compiling Mask Raster done")
