# Code from RapidJSON
# https://github.com/Tencent/rapidjson/blob/master/doc/CMakeLists.txt
# commit/bbdf5d1d4b40891c82e5c1946d32dfc841926066 Sep 6, 2017
#
find_package(Doxygen)

IF(NOT DOXYGEN_FOUND)
    MESSAGE(STATUS "No Doxygen found. Documentation won't be built")
ELSE()
    file(GLOB SOURCES ${CMAKE_CURRENT_LIST_DIR}/../src/*)
    file(GLOB MARKDOWN_DOC ${CMAKE_CURRENT_LIST_DIR}/../doc/*.md)
    list(APPEND MARKDOWN_DOC ${CMAKE_CURRENT_LIST_DIR}/../README.md
                             ${CMAKE_CURRENT_LIST_DIR}/../README-zh-cn.md)

    CONFIGURE_FILE(Doxyfile.in Doxyfile @ONLY)
    CONFIGURE_FILE(Doxyfile.zh-cn.in Doxyfile.zh-cn @ONLY)

    file(GLOB DOXYFILES ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile*)

    add_custom_command(OUTPUT html
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.zh-cn
        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/html
        DEPENDS ${MARKDOWN_DOC} ${SOURCES} ${DOXYFILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../
        )

    add_custom_target(doc ALL DEPENDS html)
#    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
#        DESTINATION ${DOC_INSTALL_DIR}
#        COMPONENT doc)
ENDIF()
