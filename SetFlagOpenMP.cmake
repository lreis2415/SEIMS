### OpenMP, FIND_PACKAGE(OpenMP) MUST be invoked before.
IF(WITH_OPENMP)
  IF(OPENMP_FOUND)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    geo_list_unique(CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
  ELSE ()
    MESSAGE(WARNING "CMake fails to determine OpenMP.
Please check your compiler to make sure the support of OpenMP.")
  ENDIF()
ENDIF()