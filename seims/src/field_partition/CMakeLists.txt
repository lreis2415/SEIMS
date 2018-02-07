PROJECT(Field_Partition)
################ Add executables #################
file(GLOB FP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SOURCE_FILES ${UTILS_FILES} ${FP_SRC})
geo_include_directories(${GDAL_INCLUDE_DIR} ${UTILS_INC} ${RASTER_INC})
ADD_EXECUTABLE(fieldpartition ${SOURCE_FILES})
target_link_libraries(fieldpartition ${GDAL_LIBRARIES})
install(TARGETS fieldpartition DESTINATION ${INSTALL_DIR})

LIST(APPEND UTILS_APPS fieldpartition)
SET(UTILS_APPS "${UTILS_APPS}" PARENT_SCOPE)

IF (MSVC)
    SET_PROPERTY(TARGET fieldpartition PROPERTY FOLDER "UtilApps")
ENDIF ()
MESSAGE(STATUS "    Compiling FieldPartition done")
