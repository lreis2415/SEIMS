PROJECT(Field_Partition)
################ Add executables #################
file(GLOB FP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SOURCE_FILES ${CCGL_SRC} ${FP_SRC})
geo_include_directories(${GDAL_INCLUDE_DIR} ${CCGL_INC})
# Not need MongoDB support
REMOVE_DEFINITIONS(-DMONGO_HAVE_STDINT -DUSE_MONGODB)
ADD_EXECUTABLE(fieldpartition ${SOURCE_FILES})
target_link_libraries(fieldpartition ${GDAL_LIBRARIES})
install(TARGETS fieldpartition DESTINATION ${INSTALL_DIR})

LIST(APPEND UTILS_APPS fieldpartition)
SET(UTILS_APPS "${UTILS_APPS}" PARENT_SCOPE)

IF (MSVC OR XCODE)
    SET_PROPERTY(TARGET fieldpartition PROPERTY FOLDER "UtilApps")
ENDIF ()
MESSAGE(STATUS "    Compiling FieldPartition done")
