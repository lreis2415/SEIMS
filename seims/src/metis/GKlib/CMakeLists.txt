cmake_minimum_required(VERSION 3.1)
project(GKlib)

get_filename_component(abs "." ABSOLUTE)
set(GKLIB_PATH ${abs})
unset(abs)
include(GKlibSystem.cmake)

include_directories(".")
add_library(GKlib STATIC ${GKlib_sources})
if (UNIX)
    target_link_libraries(GKlib m)
endif (UNIX)
### For Xcode generator, specific LLVM-Clang installed by brew, and add link library of OpenMP explicitly.
IF(XCODE AND LLVM_FOUND)
    TARGET_LINK_LIBRARIES(GKlib ${OpenMP_LIBRARY})
    SET_TARGET_PROPERTIES(GKlib PROPERTIES XCODE_ATTRIBUTE_CC ${LLVM_ROOT_DIR}/bin/clang)
    SET_TARGET_PROPERTIES(GKlib PROPERTIES XCODE_ATTRIBUTE_CXX ${LLVM_ROOT_DIR}/bin/clang++)
    SET_TARGET_PROPERTIES(GKlib PROPERTIES XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE "No")
ENDIF()

include_directories("test")
add_subdirectory("test")

install(TARGETS GKlib
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
install(FILES ${GKlib_includes} DESTINATION include)
