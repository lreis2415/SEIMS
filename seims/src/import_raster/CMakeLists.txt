PROJECT(Import_raster2mongo)
################ Add executables #################
file(GLOB IR_SRC ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)
set(SOURCE_FILES ${IR_SRC})
geo_include_directories(${GDAL_INCLUDE_DIR} ${BSON_INCLUDE_DIR} ${MONGOC_INCLUDE_DIR} ${CCGL_INC})
ADD_EXECUTABLE(import_raster ${SOURCE_FILES})
target_link_libraries(import_raster ${CCGLNAME} ${GDAL_LIBRARIES} ${BSON_LIBRARIES} ${MONGOC_LIBRARIES})
install(TARGETS import_raster DESTINATION ${INSTALL_DIR})

LIST(APPEND UTILS_APPS import_raster)
SET(UTILS_APPS "${UTILS_APPS}" PARENT_SCOPE)

IF (MSVC OR XCODE)
    SET_PROPERTY(TARGET import_raster PROPERTY FOLDER "UtilApps")
ENDIF ()
MESSAGE(STATUS "    Compiling Import Raster done")
