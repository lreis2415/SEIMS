set(shape_srcs ${TAUDEM_SRC}/ReadOutlets.cpp)
set(common_srcs ${TAUDEM_SRC}/commonLib.cpp ${TAUDEM_SRC}/tiffIO.cpp)
# Adds include directories in such a way that directories from the TauDEM source tree go first.
geo_include_directories(${TAUDEM_SRC})

# Added by Liangjun Zhu, 2015/5/21
set(CURVATURE Curvature.cpp Curvaturemn.cpp ${common_srcs})
set(LOGOP demoLogOperatormn.cpp demoLogOperator.cpp ${common_srcs})
set(D8DISTDOWN D8DistDownToStream.cpp D8DistDownToStreammn.cpp ${common_srcs})
# Integrated from AutoFuzSlpPos project by Liangjun Zhu, 2019/01/23
set(D8DISTUP D8DistUpToRidge.cpp D8DistUpToRidgemn.cpp ${common_srcs})
## TODO, TauDEM also implemented a similar function named dinfdistup, we need to find if they can be merged!
set(DINFDISTUPRDG DinfDistUpToRidge.cpp DinfDistUpToRidgemn.cpp ${common_srcs})
set(CALCULATOR SimpleCalculator.cpp SimpleCalculatormn.cpp ${common_srcs})
set(RPISKIDMORE RPISkidmore.cpp RPISkidmoremn.cpp ${common_srcs})
set(FLOWMFDMD MultiFlowDirMaxDown.cpp MultiFlowDirMaxDownmn.cpp ${common_srcs})

add_executable(curvature ${CURVATURE})
add_executable(logop ${LOGOP})
add_executable(d8distdowntostream ${D8DISTDOWN})
add_executable(d8distuptoridge ${D8DISTUP})
add_executable(dinfdistuptoridge ${DINFDISTUPRDG})
add_executable(simplecalculator ${CALCULATOR})
add_executable(rpiskidmore ${RPISKIDMORE})
add_executable(flowmfdmd ${FLOWMFDMD})

set(TAUDEM_EXT_APP curvature
                   logop
                   d8distdowntostream
                   d8distuptoridge
                   dinfdistuptoridge
                   simplecalculator
                   rpiskidmore
                   flowmfdmd
        )
# third-party programs based on TauDEM framework
foreach (c_target ${TAUDEM_EXT_APP})
    target_link_libraries(${c_target} ${MPI_LIBRARIES} ${GDAL_LIBRARIES})
    ### For Xcode generator, specific LLVM-Clang installed by brew, and add link library of OpenMP explicitly.
    IF(XCODE AND LLVM_FOUND)
        TARGET_LINK_LIBRARIES(${c_target} ${OpenMP_LIBRARY})
        SET_TARGET_PROPERTIES(${c_target} PROPERTIES XCODE_ATTRIBUTE_CC ${LLVM_ROOT_DIR}/bin/clang)
        SET_TARGET_PROPERTIES(${c_target} PROPERTIES XCODE_ATTRIBUTE_CXX ${LLVM_ROOT_DIR}/bin/clang++)
        SET_TARGET_PROPERTIES(${c_target} PROPERTIES XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE "No")
    ENDIF()
    install(TARGETS ${c_target} DESTINATION ${INSTALL_DIR})
    IF (MSVC OR XCODE)
        SET_PROPERTY(TARGET ${c_target} PROPERTY FOLDER "TauDEM_EXT")
    ENDIF ()
endforeach (c_target ${TAUDEM_EXT_APP})

# add other directories
ADD_SUBDIRECTORY(${TAUDEM_EXT_SRC}/autofuzslppos)
