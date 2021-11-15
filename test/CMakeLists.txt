##############
# Unit Tests
##############
CMAKE_MINIMUM_REQUIRED(VERSION 3.10 FATAL_ERROR)
#enable_testing()
include(GoogleTest)

file(GLOB_RECURSE TEST_SRC_FILES *.cpp)
set(APPNAME UnitTests_CCGL)
add_executable(${APPNAME} ${TEST_SRC_FILES})

IF (GTEST_FOUND)
    target_link_libraries(${APPNAME} GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main ${CCGLNAME})
ELSE ()
    target_link_libraries(${APPNAME} gtest gtest_main gmock gmock_main ${CCGLNAME})
ENDIF ()

gtest_discover_tests(${APPNAME})

IF(XCODE AND LLVM_FOUND)
    TARGET_LINK_LIBRARIES(${APPNAME} ${OpenMP_LIBRARY})
	SET_TARGET_PROPERTIES(${APPNAME} PROPERTIES XCODE_ATTRIBUTE_CC ${LLVM_ROOT_DIR}/bin/clang)
	SET_TARGET_PROPERTIES(${APPNAME} PROPERTIES XCODE_ATTRIBUTE_CXX ${LLVM_ROOT_DIR}/bin/clang++)
	SET_TARGET_PROPERTIES(${APPNAME} PROPERTIES XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE "No")
ENDIF ()

install(TARGETS ${APPNAME} DESTINATION ${INSTALL_DIR})

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
