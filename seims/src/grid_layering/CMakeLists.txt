PROJECT(Grid_Layering)
################ Add executables #################
file(GLOB GL_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)
set(SOURCE_FILES ${GL_SRC_FILES})
geo_include_directories(${GDAL_INCLUDE_DIR} ${BSON_INCLUDE_DIR} ${MONGOC_INCLUDE_DIR} ${CCGL_INC})
ADD_EXECUTABLE(grid_layering ${SOURCE_FILES})
target_link_libraries(grid_layering ${CCGLNAME} ${GDAL_LIBRARIES} ${BSON_LIBRARIES} ${MONGOC_LIBRARIES})
install(TARGETS grid_layering DESTINATION ${INSTALL_DIR})

LIST(APPEND UTILS_APPS grid_layering)
SET(UTILS_APPS "${UTILS_APPS}" PARENT_SCOPE)

IF (MSVC OR XCODE)
    SET_PROPERTY(TARGET grid_layering PROPERTY FOLDER "UtilApps")
ENDIF ()
MESSAGE(STATUS "    Compiling Grid Layering done")
