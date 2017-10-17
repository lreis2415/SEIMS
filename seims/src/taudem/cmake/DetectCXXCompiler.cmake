# ----------------------------------------------------------------------------
# Detect Microsoft compiler:
# ----------------------------------------------------------------------------
IF(CMAKE_CL_64)
  SET(MSVC64 1)
ENDIF()

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  SET(CMAKE_COMPILER_IS_GNUCXX 1)
  SET(CMAKE_COMPILER_IS_CLANGCXX 1)
ENDIF()
IF(CMAKE_C_COMPILER_ID MATCHES "Clang")
  SET(CMAKE_COMPILER_IS_GNUCC 1)
  SET(CMAKE_COMPILER_IS_CLANGCC 1)
ENDIF()
IF("${CMAKE_CXX_COMPILER};${CMAKE_C_COMPILER}" MATCHES "ccache")
  SET(CMAKE_COMPILER_IS_CCACHE 1)
ENDIF()

# ----------------------------------------------------------------------------
# Detect Intel ICC compiler -- for -fPIC in 3rdparty ( UNIX ONLY ):
# NOTE: The system needs to determine if the '-fPIC' option needs to be added
#  for the 3rdparty static libs being compiled.  The CMakeLists.txt files
#  in 3rdparty use the GEO_ICC definition being set here to determine if
#  the -fPIC flag should be used.
# ----------------------------------------------------------------------------
IF(UNIX)
  IF  (__ICL)
    SET(GEO_ICC   __ICL)
  ELSEIF(__ICC)
    SET(GEO_ICC   __ICC)
  ELSEIF(__ECL)
    SET(GEO_ICC   __ECL)
  ELSEIF(__ECC)
    SET(GEO_ICC   __ECC)
  ELSEIF(__INTEL_COMPILER)
    SET(GEO_ICC   __INTEL_COMPILER)
  ELSEIF(CMAKE_C_COMPILER MATCHES "icc")
    SET(GEO_ICC   icc_matches_c_compiler)
  ENDIF()
ENDIF()

IF(MSVC AND CMAKE_C_COMPILER MATCHES "icc|icl")
  SET(GEO_ICC   __INTEL_COMPILER_FOR_WINDOWS)
ENDIF()

IF(NOT DEFINED CMAKE_CXX_COMPILER_VERSION)
  MESSAGE(WARNING "Compiler version is not available: CMAKE_CXX_COMPILER_VERSION is not set")
ENDIF()

# ----------------------------------------------------------------------------
# Detect and set architecture:
#
#   ${CMAKE_CXX_COMPILER} -dumpmachine examples:
#     in Windows 10, ming64 bash: x86_64-w64-mingw32
#     in CentOS 6.2: x86_64-redhat-linux
#     in macOS 10.12.6: x86_64-apple-darwin16.7.0
# ----------------------------------------------------------------------------
IF(CMAKE_COMPILER_IS_GNUCXX)
  IF(WIN32)
    EXECUTE_PROCESS(COMMAND ${CMAKE_CXX_COMPILER} -dumpmachine
              OUTPUT_VARIABLE GCC_TARGET_MACHINE
              OUTPUT_STRIP_TRAILING_WHITESPACE)
    IF(GCC_TARGET_MACHINE MATCHES "amd64|x86_64|AMD64")
      SET(MINGW64 1)
    ENDIF()
  ENDIF()
ENDIF()

IF(MSVC64 OR MINGW64)
  SET(X86_64 1)
ELSEIF(MINGW OR (MSVC AND NOT CMAKE_CROSSCOMPILING))
  SET(X86 1)
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
  SET(X86_64 1)
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "i686.*|i386.*|x86.*|amd64.*|AMD64.*")
  SET(X86 1)
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*)")
  SET(ARM 1)
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64.*|AARCH64.*)")
  SET(AARCH64 1)
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^ppc64le.*|PPC64LE.*")
  SET(PPC64LE 1)
ENDIF()

# Workaround for 32-bit operating systems on 64-bit x86_64 processor
IF(X86_64 AND CMAKE_SIZEOF_VOID_P EQUAL 4 AND NOT FORCE_X86_64)
  MESSAGE(STATUS "sizeof(void) = 4 on x86 / x86_64 processor. Assume 32-bit compilation mode (X86=1)")
  UNSET(X86_64)
  SET(X86 1)
ENDIF()

IF(MSVC)
  IF(CMAKE_CL_64)
    SET(GEO_ARCH x64)
  ELSEIF((CMAKE_GENERATOR MATCHES "ARM") OR ("${arch_hint}" STREQUAL "ARM") OR (CMAKE_VS_EFFECTIVE_PLATFORMS MATCHES "ARM|arm"))
    SET(GEO_ARCH ARM)
  ELSE()
    SET(GEO_ARCH x86)
  ENDIF()
  IF(MSVC_VERSION EQUAL 1400)
    SET(GEO_RUNTIME vc8)
  ELSEIF(MSVC_VERSION EQUAL 1500)
    SET(GEO_RUNTIME vc9)
  ELSEIF(MSVC_VERSION EQUAL 1600)
    SET(GEO_RUNTIME vc10)
  ELSEIF(MSVC_VERSION EQUAL 1700)
    SET(GEO_RUNTIME vc11)
  ELSEIF(MSVC_VERSION EQUAL 1800)
    SET(GEO_RUNTIME vc12)
  ELSEIF(MSVC_VERSION EQUAL 1900)
    SET(GEO_RUNTIME vc14)
  ELSEIF(MSVC_VERSION EQUAL 1910 OR MSVC_VERSION EQUAL 1911)
    SET(GEO_RUNTIME vc15)
  ELSE()
    message(WARNING "Does not recognize MSVC_VERSION \"${MSVC_VERSION}\". Cannot set GEO_RUNTIME")
  ENDIF()
ELSEIF(MINGW)
  SET(GEO_RUNTIME mingw)
  IF(MINGW64)
    SET(GEO_ARCH x64)
  ELSE()
    SET(GEO_ARCH x86)
  ENDIF()
ENDIF()

# Fix handling of duplicated files in the same static library:
# https://public.kitware.com/Bug/view.php?id=14874
IF(CMAKE_VERSION VERSION_LESS "3.1")
  foreach(var CMAKE_C_ARCHIVE_APPEND CMAKE_CXX_ARCHIVE_APPEND)
    IF(${var} MATCHES "^<CMAKE_AR> r")
      string(REPLACE "<CMAKE_AR> r" "<CMAKE_AR> q" ${var} "${${var}}")
    ENDIF()
  endforeach()
ENDIF()

IF(ENABLE_CXX11)
  SET(CMAKE_CXX_STANDARD 11)
  SET(CMAKE_CXX_STANDARD_REQUIRED TRUE)
  SET(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++11 instead of -std=gnu++11
  IF(CMAKE_CXX11_COMPILE_FEATURES)
    SET(HAVE_CXX11 ON)
  ENDIF()
ENDIF()
IF(NOT HAVE_CXX11)
  geo_check_compiler_flag(CXX "" HAVE_CXX11 "${CMAKE_SOURCE_DIR}/cmake/checks/cxx11.cpp")
  IF(NOT HAVE_CXX11 AND ENABLE_CXX11)
    geo_check_compiler_flag(CXX "-std=c++11" HAVE_STD_CXX11 "${CMAKE_SOURCE_DIR}/cmake/checks/cxx11.cpp")
    IF(HAVE_STD_CXX11)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
      SET(HAVE_CXX11 ON)
    ENDIF()
  ENDIF()
ENDIF()
