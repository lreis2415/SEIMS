# first build google test (gmock, gtest)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest)
# add framework unittest
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/seims_framework)
