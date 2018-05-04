### MPI
# currently, for mingw64 on Windows, you must specify MPI_LIBRARIES and DMPI_INCLUDE_PATH through CMake command.
IF (WITH_MPI)
  IF (NOT DEFINED MPI_LIBRARIES)
    FIND_PACKAGE(MPI REQUIRED)
  ENDIF ()
  IF (MPI_FOUND)
    SET(CMAKE_CXX_FLAG "${CMAKE_CXX_FLAG} ${MPI_COMPILE_FLAGS}")
    SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS}")
    IF (CMAKE_COMPILER_IS_CLANGCXX OR CMAKE_COMPILER_IS_CLANGCC) # using regular Clang or AppleClang
    SET(CMAKE_EXE_LINKER_FLAGS "-Wl -headerpad_max_install_names")
    SET(CMAKE_EXE_LINKER_FLAGS "-F/Library/Frameworks") # fix: Xcode ld error: framework not found gdal.
    ELSEIF (MPI_LIBRARIES MATCHES "impi")
    # Select flags for ICC, refers to https://software.intel.com/en-us/articles/intel-cluster-toolkit-for-linux-error-when-compiling-c-aps-using-intel-mpi-library-compilation-driver-mpiicpc
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX")
    geo_list_unique(CMAKE_CXX_FLAGS)
    ENDIF ()
  ELSE ()
    MESSAGE(FATAL_ERROR "CMake fails to determine MPI. Please check your installation of MPI.")
  ENDIF ()
ENDIF ()
