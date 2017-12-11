##############
# Unit Tests
##############
enable_testing()

file(GLOB TEST_SRC_FILES *.cpp)
add_executable(UnitTests_Utils ${TEST_SRC_FILES})
target_link_libraries(UnitTests_Utils gtest gtest_main utilsclass)

INSTALL(TARGETS UnitTests_Utils DESTINATION ${INSTALL_DIR})
###
## This is so you can do 'make test' to see all your tests run, instead of
## manually running the executable runUnitTests to see those specific tests.
add_test(NAME UnitTests_Utils
         COMMAND UnitTests_Utils)

if (RUNCOV STREQUAL 1)
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
    set(COVERAGE_EXCLUDES 'test/*' '*test/*')
    setup_target_for_coverage(NAME UnitTestCoverage
                              EXECUTABLE UnitTests_Utils
                              DEPENDENCIES UnitTests_Utils)
endif ()
