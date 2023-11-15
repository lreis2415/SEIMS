MESSAGE(STATUS "      Compiling SEIMS_subdir: conceptual_model...")
FOREACH(subdir ${SUBDIRS})
  ADD_SUBDIRECTORY(${subdir})
ENDFOREACH()