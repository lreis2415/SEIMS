### Set each program separately.
# IO of mask raster in single or multiple subset, support asc, tif, and mongodb's gridfs
SET(MASKFILES mask_rasterio.cpp)
# reclassify raster
SET(RECLASSIFYFILES reclassify_raster.cpp)

IF (MONGOC_FOUND)
    geo_include_directories(${BSON_INCLUDE_DIR} ${MONGOC_INCLUDE_DIR})
ENDIF ()
IF (GDAL_FOUND)
    INCLUDE(SetFlagGDAL)
    geo_include_directories(${GDAL_INCLUDE_DIR})
ENDIF ()
geo_include_directories(${CCGL_DIR})

ADD_EXECUTABLE(mask_rasterio ${MASKFILES})
ADD_EXECUTABLE(reclassify_raster ${RECLASSIFYFILES})

SET(APPS_TARGETS mask_rasterio
                 reclassify_raster
                )

foreach (c_target ${APPS_TARGETS})
    SET_TARGET_PROPERTIES(${c_target} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_link_libraries(${c_target} ${TARGET_VISIBILITY} ${CCGLNAME})
    IF (MONGOC_FOUND)
        TARGET_LINK_LIBRARIES(${c_target} ${TARGET_VISIBILITY} ${BSON_LIBRARIES} ${MONGOC_LIBRARIES})
    ENDIF ()
    IF (GDAL_FOUND)
        TARGET_LINK_LIBRARIES(${c_target} ${TARGET_VISIBILITY} ${GDAL_LIBRARIES})
        SET_TARGET_PROPERTIES(${c_target} PROPERTIES LINKER_LANGUAGE CXX)
    ENDIF ()
    ### For LLVM-Clang installed by brew, add link library of OpenMP explicitly.
    IF(CV_CLANG AND LLVM_VERSION_MAJOR)
        TARGET_LINK_LIBRARIES(${c_target} ${TARGET_VISIBILITY} ${OpenMP_LIBRARY})
    ENDIF()
    install(TARGETS ${c_target} DESTINATION ${INSTALL_DIR}/bin)
    LIST(APPEND APP_TARGETS ${c_target})
    IF (MSVC OR XCODE)
        SET_PROPERTY(TARGET ${c_target} PROPERTY FOLDER "apps")
    ENDIF ()
endforeach (c_target ${APPS_TARGETS})

SET(APP_TARGETS "${APP_TARGETS}" PARENT_SCOPE)
