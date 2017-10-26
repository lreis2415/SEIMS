MESSAGE(STATUS "        Compiling base_subdir: util...")
SET(SRC_LIST MetadataInfo.cpp SimulationModule.cpp
        MetadataInfoConst.h seims.h text.h
        ${UTILS_FILES})
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
ADD_LIBRARY(util STATIC ${SRC_LIST})
INSTALL(TARGETS util DESTINATION ${INSTALL_DIR})
IF (MSVC)
    SET_PROPERTY(TARGET util PROPERTY FOLDER "base")
ENDIF ()
