MESSAGE(STATUS "      Compiling SEIMS_subdir: main...")
ADD_SUBDIRECTORY(./main_omp)
ADD_SUBDIRECTORY(./main_mpi)
# ADD_SUBDIRECTORY(./main_mpi_deprecated)
