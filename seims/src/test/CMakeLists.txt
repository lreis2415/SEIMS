# first build google test (gmock, gtest)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest)
# add framework unittest
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/seims_framework/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/seims_framework)
endif()
