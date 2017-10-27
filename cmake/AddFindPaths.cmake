# ----------------------------------------------------------------------------
# Add standard paths or specified paths for Find libraries and headers.
# ----------------------------------------------------------------------------
if(UNIX)
  if(X86_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8)
    if(EXISTS /lib64)
      list(APPEND CMAKE_LIBRARY_PATH /lib64)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /lib)
    endif()
    if(EXISTS /usr/lib64)
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib64)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib)
    endif()
  elseif(X86 OR CMAKE_SIZEOF_VOID_P EQUAL 4)
    if(EXISTS /lib32)
      list(APPEND CMAKE_LIBRARY_PATH /lib32)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /lib)
    endif()
    if(EXISTS /usr/lib32)
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib32)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib)
    endif()
  endif()
endif()

# Add these standard paths to the search paths for FIND_PATH
# to find include files from these locations first
if(MINGW)
  if(EXISTS /mingw)
      list(APPEND CMAKE_INCLUDE_PATH /mingw)
  endif()
  if(EXISTS /mingw32)
      list(APPEND CMAKE_INCLUDE_PATH /mingw32)
  endif()
  if(EXISTS /mingw64)
      list(APPEND CMAKE_INCLUDE_PATH /mingw64)
      LIST(APPEND CMAKE_PREFIX_PATH /mingw64)
  endif()
endif()

IF(GEO_3RD_PARTY_ROOT)
  # GDAL.
  FILE(GLOB DEFAULT_PATH ${GEO_3RD_PARTY_ROOT}/gdal-*)
  IF(NOT DEFAULT_PATH)
    SET(DEFAULT_PATH ${GEO_3RD_PARTY_ROOT})
  ENDIF()
  SET(GEO_GDAL_ROOT
          ${DEFAULT_PATH}
          CACHE PATH "Path to root of GDAL software"
          )
  LIST(APPEND CMAKE_PREFIX_PATH ${GEO_GDAL_ROOT})
  # mongo-c-driver.
  FILE(GLOB DEFAULT_PATH ${GEO_3RD_PARTY_ROOT}/mongo-c-driver-*)
  IF(NOT DEFAULT_PATH)
    SET(DEFAULT_PATH ${GEO_3RD_PARTY_ROOT})
  ENDIF()
  SET(GEO_MONGOC_ROOT
          ${DEFAULT_PATH}
          CACHE PATH "Path to root of mongo-c-driver"
          )
  LIST(APPEND CMAKE_PREFIX_PATH ${GEO_MONGOC_ROOT})
ENDIF()