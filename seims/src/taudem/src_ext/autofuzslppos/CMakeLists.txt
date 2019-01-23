set(shape_srcs ${TAUDEM_SRC}/ReadOutlets.cpp)
set(common_srcs ${TAUDEM_SRC}/commonLib.cpp ${TAUDEM_SRC}/tiffIO.cpp)
# Adds include directories in such a way that directories from the TauDEM source tree go first.
geo_include_directories(${TAUDEM_SRC})

set(RIDGEEXTRACTION stats.cpp RidgeExtraction.cpp RidgeExtractionmn.cpp ${common_srcs})
set(TYPLOCSEL stats.cpp SelectTypLocSlpPos.cpp SelectTypLocSlpPosmn.cpp ${common_srcs})
set(FUZSLPPOSINF FuzzySlpPosInference.cpp FuzzySlpPosInferencemn.cpp ${common_srcs})
set(HARDENSLPPOS HardenSlpPos.cpp HardenSlpPosmn.cpp ${common_srcs})

add_executable(ridgeextraction ${RIDGEEXTRACTION})
add_executable(selecttyplocslppos ${TYPLOCSEL})
add_executable(fuzzyslpposinference ${FUZSLPPOSINF})
add_executable(hardenslppos ${HARDENSLPPOS})

set(TAUDEM_EXT_APP
        ridgeextraction
        selecttyplocslppos
        fuzzyslpposinference
        hardenslppos
        )

# third-party programs based on TauDEM framework
foreach (c_target ${TAUDEM_EXT_APP})
    target_link_libraries(${c_target} ${MPI_LIBRARIES} ${GDAL_LIBRARIES})
    install(TARGETS ${c_target} DESTINATION ${INSTALL_DIR})
    IF (MSVC OR XCODE)
        SET_PROPERTY(TARGET ${c_target} PROPERTY FOLDER "TauDEM_EXT/AutoFuzSlpPos")
    ENDIF ()
endforeach (c_target ${TAUDEM_EXT_APP})
