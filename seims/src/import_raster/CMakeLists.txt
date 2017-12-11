PROJECT(Import_raster2mongo)
################ Add executables #################
set(SOURCE_FILES ${UTILS_FILES} ${MONGO_FILES} SubBasin.cpp main.cpp)
ADD_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB)
geo_include_directories(${GDAL_INCLUDE_DIR} ${BSON_INCLUDE_DIR} ${MONGOC_INCLUDE_DIR} ${UTILS_INC} ${RASTER_INC} ${MONGO_INC})
ADD_EXECUTABLE(import_raster ${SOURCE_FILES})
target_link_libraries(import_raster ${GDAL_LIBRARIES} ${BSON_LIBRARIES} ${MONGOC_LIBRARIES})
install(TARGETS import_raster DESTINATION ${INSTALL_DIR})

LIST(APPEND UTILS_APPS import_raster)
SET(UTILS_APPS "${UTILS_APPS}" PARENT_SCOPE)

IF (WIN32)
    SET_PROPERTY(TARGET import_raster PROPERTY FOLDER "UtilApps")
ENDIF ()
MESSAGE(STATUS "    Compiling Import Raster done")
