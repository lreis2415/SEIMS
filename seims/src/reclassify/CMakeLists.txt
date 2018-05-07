project(ReClassify)
################ Add executables #################
file(GLOB RC_SRC ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)
set(SOURCE_FILES ${RC_SRC})
geo_include_directories(${GDAL_INCLUDE_DIR} ${CCGL_INC})
# Not need MongoDB support
REMOVE_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB)
ADD_EXECUTABLE(reclassify ${SOURCE_FILES})
target_link_libraries(reclassify ${CCGLNAME} ${GDAL_LIBRARIES})
install(TARGETS reclassify DESTINATION ${INSTALL_DIR})

LIST(APPEND UTILS_APPS reclassify)
SET(UTILS_APPS "${UTILS_APPS}" PARENT_SCOPE)

IF (MSVC OR XCODE)
    SET_PROPERTY(TARGET reclassify PROPERTY FOLDER "UtilApps")
ENDIF ()
MESSAGE(STATUS "    Compiling ReClassify Raster done")
