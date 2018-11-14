set(shape_srcs ${TAUDEM_SRC}/ReadOutlets.cpp)
set(common_srcs ${TAUDEM_SRC}/commonLib.cpp ${TAUDEM_SRC}/tiffIO.cpp)
# Adds include directories in such a way that directories from the TauDEM source tree go first.
geo_include_directories(${TAUDEM_SRC})

# Added by Liangjun Zhu, 2015/5/21
set(CURVATURE Curvature.cpp Curvaturemn.cpp ${common_srcs})
set(LOGOP demoLogOperatormn.cpp demoLogOperator.cpp ${common_srcs})
set(D8DISTDOWN D8DistDownToStream.cpp D8DistDownToStreammn.cpp ${common_srcs})

add_executable(curvature ${CURVATURE})
add_executable(logop ${LOGOP})
add_executable(d8distdowntostream ${D8DISTDOWN})

set(TAUDEM_EXT_APP curvature
                   logop
                   d8distdowntostream
        )
# third-party programs based on TauDEM framework
foreach (c_target ${TAUDEM_EXT_APP})
    target_link_libraries(${c_target} ${MPI_LIBRARIES} ${GDAL_LIBRARIES})
    install(TARGETS ${c_target} DESTINATION ${INSTALL_DIR})
    IF (MSVC)
        SET_PROPERTY(TARGET ${c_target} PROPERTY FOLDER "TauDEM_EXT")
    ENDIF ()
endforeach (c_target ${TAUDEM_EXT_APP})
