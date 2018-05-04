##############
# Unit Tests
##############
enable_testing()

file(GLOB_RECURSE TEST_SRC_FILES *.cpp)
set(APPNAME UnitTests_CCGL)
add_executable(${APPNAME} ${TEST_SRC_FILES})
target_link_libraries(${APPNAME} gtest gtest_main ${CCGLNAME})

install(TARGETS ${APPNAME} DESTINATION ${INSTALL_DIR})
###
## This is so you can do 'make test' to see all your tests run, instead of
## manually running the executable runUnitTests to see those specific tests.
add_test(NAME test
         COMMAND ${APPNAME})

if (RUNCOV STREQUAL 1)
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
    set(COVERAGE_EXCLUDES 'test/*' '*test/*')
    setup_target_for_coverage(
        NAME UnitTestCoverage               # New target name
        EXECUTABLE ${APPNAME}               # Executable in PROJECT_BINARY_DIR
        DEPENDENCIES ${APPNAME}             # Dependencies to build first
        )
endif ()
