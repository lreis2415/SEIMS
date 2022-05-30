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
    ### For LLVM-Clang installed by brew, add link library of OpenMP explicitly.
    IF(CV_CLANG AND LLVM_VERSION_MAJOR)
        TARGET_LINK_LIBRARIES(${EXECNAME} ${OpenMP_LIBRARY})
    ENDIF()
endforeach (prog)

if (METIS_INSTALL)
    install(TARGETS gpmetis ndmetis mpmetis m2gmetis graphchk cmpfillin
            RUNTIME DESTINATION "${INSTALL_DIR}/bin")
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
