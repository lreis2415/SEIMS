##############
# Unit Tests
##############
enable_testing()

file(GLOB TEST_SRC_FILES *.cpp)
add_executable(UnitTests_Raster ${TEST_SRC_FILES})
target_link_libraries(UnitTests_Raster gtest gtest_main RasterClass)

INSTALL(TARGETS UnitTests_Raster DESTINATION ${INSTALL_DIR})
###
## This is so you can do 'make test' to see all your tests run, instead of
## manually running the executable runUnitTests to see those specific tests.
add_test(NAME UnitTests_Raster
         COMMAND UnitTests_Raster)

if (RUNCOV STREQUAL 1)
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
    set(COVERAGE_EXCLUDES 'test/*' '*test/*')
    setup_target_for_coverage(NAME UnitTestCoverage
                              EXECUTABLE UnitTests_Raster
                              DEPENDENCIES UnitTests_Raster)
endif ()
