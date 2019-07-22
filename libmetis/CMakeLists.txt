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
### For Xcode generator, specific LLVM-Clang installed by brew, and add link library of OpenMP explicitly.
IF(XCODE AND LLVM_FOUND)
    TARGET_LINK_LIBRARIES(metis ${OpenMP_LIBRARY})
    SET_TARGET_PROPERTIES(metis PROPERTIES XCODE_ATTRIBUTE_CC ${LLVM_ROOT_DIR}/bin/clang)
    SET_TARGET_PROPERTIES(metis PROPERTIES XCODE_ATTRIBUTE_CXX ${LLVM_ROOT_DIR}/bin/clang++)
    SET_TARGET_PROPERTIES(metis PROPERTIES XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE "No")
ENDIF()
# refers to https://github.com/Alexpux/MINGW-packages/tree/master/mingw-w64-metis
if(WIN32)
  set(RT_DEST bin)
else()
  set(RT_DEST lib)
endif()
if (METIS_INSTALL)
    install(TARGETS metis
            LIBRARY DESTINATION lib
            RUNTIME DESTINATION ${RT_DEST}
            ARCHIVE DESTINATION lib)
endif ()
