project(Combine_Raster_prj)
################ Add executables #################
file(GLOB CR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SOURCE_FILES ${UTILS_FILES} ${MONGO_FILES} ${CR_SRC})
geo_include_directories(${GDAL_INCLUDE_DIR} ${BSON_INCLUDE_DIR} ${MONGOC_INCLUDE_DIR} ${UTILS_INC} ${RASTER_INC} ${MONGO_INC})
ADD_EXECUTABLE(combine_raster ${SOURCE_FILES})
target_link_libraries(combine_raster ${GDAL_LIBRARIES} ${BSON_LIBRARIES} ${MONGOC_LIBRARIES})
install(TARGETS combine_raster DESTINATION ${INSTALL_DIR})

LIST(APPEND UTILS_APPS combine_raster)
SET(UTILS_APPS "${UTILS_APPS}" PARENT_SCOPE)

IF (WIN32)
    SET_PROPERTY(TARGET combine_raster PROPERTY FOLDER "UtilApps")
ENDIF ()
MESSAGE(STATUS "    Compiling Combine Raster done")
