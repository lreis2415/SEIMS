project(Mask_Raster_prj)
################ Add executables #################
file(GLOB MR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SOURCE_FILES ${CCGL_SRC} ${MR_SRC})
geo_include_directories(${GDAL_INCLUDE_DIR} ${CCGL_INC})
# Not need MongoDB support
REMOVE_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB)
ADD_EXECUTABLE(mask_raster ${SOURCE_FILES})
target_link_libraries(mask_raster ${GDAL_LIBRARIES})
install(TARGETS mask_raster DESTINATION ${INSTALL_DIR})

LIST(APPEND UTILS_APPS mask_raster)
SET(UTILS_APPS "${UTILS_APPS}" PARENT_SCOPE)

IF (MSVC OR XCODE)
    SET_PROPERTY(TARGET mask_raster PROPERTY FOLDER "UtilApps")
ENDIF ()
MESSAGE(STATUS "    Compiling Mask Raster done")
