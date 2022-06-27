# Add this directory for internal users.
include_directories(.)
# Find sources.
file(GLOB metis_sources *.c)
# Build libmetis.
add_library(metis ${METIS_LIBRARY_TYPE} ${GKlib_sources} ${metis_sources})
IF (MSVC OR XCODE)
    SET_PROPERTY(TARGET metis PROPERTY FOLDER "METIS_PACKAGE")
ENDIF ()
if (UNIX)
    target_link_libraries(metis m)
endif ()
### For LLVM-Clang installed by brew, add link library of OpenMP explicitly.
IF(NOT CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
IF(CV_CLANG AND LLVM_VERSION_MAJOR)
    TARGET_LINK_LIBRARIES(metis ${OpenMP_LIBRARY})
ENDIF()
ENDIF()
# refers to https://github.com/Alexpux/MINGW-packages/tree/master/mingw-w64-metis
if(WIN32)
  set(RT_DEST "${INSTALL_DIR}/bin")
else()
  set(RT_DEST "${INSTALL_DIR}/lib")
endif()
if (METIS_INSTALL)
    install(TARGETS metis
            LIBRARY DESTINATION "${INSTALL_DIR}/lib"
            RUNTIME DESTINATION ${RT_DEST}
            ARCHIVE DESTINATION "${INSTALL_DIR}/lib")
endif ()
