# These programs use internal metis data structures.
include_directories(../libmetis)

# Build program.
add_executable(gpmetis gpmetis.c cmdline_gpmetis.c io.c stat.c)
add_executable(ndmetis ndmetis.c cmdline_ndmetis.c io.c smbfactor.c)
add_executable(mpmetis mpmetis.c cmdline_mpmetis.c io.c stat.c)
add_executable(m2gmetis m2gmetis.c cmdline_m2gmetis.c io.c)
add_executable(graphchk graphchk.c io.c)
add_executable(cmpfillin cmpfillin.c io.c smbfactor.c)
foreach (prog gpmetis ndmetis mpmetis m2gmetis graphchk cmpfillin)
    SET_TARGET_PROPERTIES(${prog} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_link_libraries(${prog} metis)
    IF (MSVC OR XCODE)
        SET_PROPERTY(TARGET ${prog} PROPERTY FOLDER "METIS_PACKAGE")
    ENDIF ()
    #  target_link_libraries(${prog} metis profiler)
    ### For Xcode generator, specific LLVM-Clang installed by brew, and add link library of OpenMP explicitly.
    IF(XCODE AND LLVM_FOUND)
        TARGET_LINK_LIBRARIES(${prog} ${OpenMP_LIBRARY})
        SET_TARGET_PROPERTIES(${prog} PROPERTIES XCODE_ATTRIBUTE_CC ${LLVM_ROOT_DIR}/bin/clang)
        SET_TARGET_PROPERTIES(${prog} PROPERTIES XCODE_ATTRIBUTE_CXX ${LLVM_ROOT_DIR}/bin/clang++)
        SET_TARGET_PROPERTIES(${prog} PROPERTIES XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE "No")
    ENDIF()
endforeach (prog)

if (METIS_INSTALL)
    install(TARGETS gpmetis ndmetis mpmetis m2gmetis graphchk cmpfillin
            RUNTIME DESTINATION ${INSTALL_DIR})
endif ()

# Try to find subversion revision.
set(SVNREV "")
file(TO_NATIVE_PATH ${PROJECT_SOURCE_DIR}/.svn svn_dir)
if (IS_DIRECTORY ${svn_dir})
    include(FindSubversion)
    if (Subversion_FOUND)
        Subversion_WC_INFO(${PROJECT_SOURCE_DIR} metis)
        set(SVNREV ${metis_WC_REVISION})
    endif (Subversion_FOUND)
endif ()
add_definitions(-DSVNINFO="${SVNREV}")
