project(google_test)
###########################################################
#########  Use the vendored googletest release  ############
###########################################################
# Keep the framework tests reproducible and available without network access.
include(FetchContent)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    googletest
    URL "file://${CCGL_LIB}/src/googletest_tags_zip/release-1.8.1.zip"
)
FetchContent_MakeAvailable(googletest)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
# solve the bug of error LNK2038
# refers to https://stackoverflow.com/questions/14172856/cmake-compile-with-mt-instead-of-md
set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()
geo_include_directories(${gtest_SOURCE_DIR}/include)
geo_include_directories(${gmock_SOURCE_DIR}/include)
