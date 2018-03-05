MESSAGE(STATUS "        Compiling main_subdir: seims_mpi...")
# Add ModelMain class
set(OMP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../main_omp)
# Add combine raster function
set(COMBINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../combine_raster)
geo_include_directories(${COMBINE_DIR} ${OMP_DIR} ${MPI_INCLUDE_PATH})
FILE(GLOB SRC_LIST *.cpp *.h ${OMP_DIR}/ModelMain.cpp ${COMBINE_DIR}/CombineRaster.cpp)
ADD_EXECUTABLE(seims_mpi ${SRC_LIST})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
TARGET_LINK_LIBRARIES(seims_mpi util data bmps module_setting ${GDAL_LIBRARIES} ${BSON_LIBRARIES} ${MONGOC_LIBRARIES} ${MPI_LIBRARIES} ${CMAKE_DL_LIBS})
INSTALL(TARGETS seims_mpi DESTINATION ${INSTALL_DIR})
IF (MSVC)
    SET_PROPERTY(TARGET seims_mpi PROPERTY FOLDER "main")
ENDIF ()
