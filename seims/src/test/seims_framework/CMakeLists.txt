set(UT_NAME_STR seims_ut)

###########################################################
######              unittest executable        ############
###########################################################
enable_testing()
set(PROJECT_TEST_NAME ${UT_NAME_STR}_exec)
file(GLOB TEST_SRC_FILES *.cpp)
add_executable(${PROJECT_TEST_NAME} ${TEST_SRC_FILES})
target_link_libraries(${PROJECT_TEST_NAME} ${PROJECT_LIB_NAME} gtest gmock_main)
## here is the template for adding another unittest of a module
# 1. Create a unittest_MODULEID.cpp, e.g., unittest_utilsclass.cpp
# 2. Add target_link_libraries(${PROJECT_TEST_NAME} MODULEID) in this file
target_link_libraries(${PROJECT_TEST_NAME} util)
add_test(${PROJECT_TEST_NAME} ${PROJECT_TEST_NAME})
install(TARGETS ${PROJECT_TEST_NAME} DESTINATION ${INSTALL_DIR})

IF (MSVC)
    SET_PROPERTY(TARGET ${PROJECT_TEST_NAME} PROPERTY FOLDER "UnitTest")
ENDIF ()
