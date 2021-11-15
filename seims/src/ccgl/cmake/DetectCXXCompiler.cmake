# Update according to OpenCV 4.x accessed on 2021-11-14
#   https://github.com/opencv/opencv/tree/4.x

# ----------------------------------------------------------------------------
# Compilers:
# - CV_GCC - GNU compiler (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
# - CV_CLANG - Clang-compatible compiler (CMAKE_CXX_COMPILER_ID MATCHES "Clang" - Clang or AppleClang, see CMP0025)
# - CV_ICC - Intel compiler
# - MSVC - Microsoft Visual Compiler (CMake variable)
# - MINGW / CYGWIN / CMAKE_COMPILER_IS_MINGW / CMAKE_COMPILER_IS_CYGWIN (CMake original variables)
#
# CPU Platforms:
# - X86 / X86_64
# - ARM - ARM CPU, not defined for AArch64      [NOT TESTED]
# - AARCH64 - ARMv8+ (64-bit)                   [NOT TESTED]
# - PPC64 / PPC64LE - PowerPC                   [NOT USED]
# - MIPS                                        [NOT USED]
#
# OS:
# - WIN32 - Windows | MINGW
# - UNIX - Linux | MacOSX | ANDROID
# - ANDROID                                     [NOT USED]
# - IOS                                         [NOT USED]
# - APPLE - MacOSX | iOS                        [NOT TESTED]
# ----------------------------------------------------------------------------


# CMAKE_CL is discouraged by offical CMake, use CMAKE_SIZEOF_VOID_P instead.
# https://cmake.org/cmake/help/latest/variable/CMAKE_CL_64.html
#IF(CMAKE_CL_64)
#  SET(MSVC64 1)
#ENDIF()
geo_declare_removed_variables(MINGW64 MSVC64)
# do not use (CMake variables): CMAKE_CL_64

if(NOT DEFINED CV_GCC AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CV_GCC 1)
endif()
if(NOT DEFINED CV_CLANG AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")  # Clang or AppleClang (see CMP0025)
  set(CV_CLANG 1)
  set(CMAKE_COMPILER_IS_CLANGCXX 1)  # TODO next release: remove this
  set(CMAKE_COMPILER_IS_CLANGCC 1)   # TODO next release: remove this
endif()

function(access_CMAKE_COMPILER_IS_CLANGCXX)
  if(NOT GEO_SUPPRESS_DEPRECATIONS)
    message(WARNING "DEPRECATED: CMAKE_COMPILER_IS_CLANGCXX support is deprecated.
    Consider using:
    - CV_GCC    # GCC
    - CV_CLANG  # Clang or AppleClang (see CMP0025)
")
  endif()
endfunction()
variable_watch(CMAKE_COMPILER_IS_CLANGCXX access_CMAKE_COMPILER_IS_CLANGCXX)
variable_watch(CMAKE_COMPILER_IS_CLANGCC access_CMAKE_COMPILER_IS_CLANGCXX)


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

# ----------------------------------------------------------------------------
# Detect Intel ICC compiler
# ----------------------------------------------------------------------------
if(UNIX)
  if(__ICL)
    set(CV_ICC   __ICL)
  elseif(__ICC)
    set(CV_ICC   __ICC)
  elseif(__ECL)
    set(CV_ICC   __ECL)
  elseif(__ECC)
    set(CV_ICC   __ECC)
  elseif(__INTEL_COMPILER)
    set(CV_ICC   __INTEL_COMPILER)
  elseif(CMAKE_C_COMPILER MATCHES "icc")
    set(CV_ICC   icc_matches_c_compiler)
  endif()
endif()

if(MSVC AND CMAKE_C_COMPILER MATCHES "icc|icl")
  set(CV_ICC   __INTEL_COMPILER_FOR_WINDOWS)
endif()

if(NOT DEFINED CMAKE_CXX_COMPILER_VERSION)
  message(WARNING "Compiler version is not available: CMAKE_CXX_COMPILER_VERSION is not set")
endif()
if(NOT DEFINED CMAKE_SYSTEM_PROCESSOR OR CMAKE_SYSTEM_PROCESSOR STREQUAL "")
  message(WARNING "CMAKE_SYSTEM_PROCESSOR is not defined. Perhaps CMake toolchain is broken")
endif()
if(NOT DEFINED CMAKE_SIZEOF_VOID_P)
  message(WARNING "CMAKE_SIZEOF_VOID_P is not defined. Perhaps CMake toolchain is broken")
endif()

message(STATUS "Detected processor: ${CMAKE_SYSTEM_PROCESSOR}")
if(SKIP_SYSTEM_PROCESSOR_DETECTION)
  # custom setup: required variables are passed through cache / CMake's command-line
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
  set(X86_64 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686.*|i386.*|x86.*")
  set(X86 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64.*|AARCH64.*|arm64.*|ARM64.*)")
  set(AARCH64 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*)")
  set(ARM 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)64le")
  set(PPC64LE 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)64")
  set(PPC64 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(mips.*|MIPS.*)")
  set(MIPS 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(riscv.*|RISCV.*)")
  set(RISCV 1)
else()
  message(WARNING "Unrecognized target processor configuration")
endif()

# Workaround for 32-bit operating systems on x86_64
if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND X86_64
    AND NOT FORCE_X86_64  # deprecated (2019-12)
)
  message(STATUS "sizeof(void) = 4 on 64 bit processor. Assume 32-bit compilation mode")
  if(X86_64)
    unset(X86_64)
    set(X86 1)
  endif()
endif()

# Workaround for 32-bit operating systems on aarch64 processor
if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND AARCH64)
  message(STATUS "sizeof(void) = 4 on 64 bit processor. Assume 32-bit compilation mode")
  if(AARCH64)
    unset(AARCH64)
    set(ARM 1)
  endif()
endif()


if(DEFINED GEO_ARCH AND DEFINED GEO_RUNTIME)
  # custom overridden values
elseif(MSVC)
  if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
    set(GEO_ARCH "x64")
  elseif("${CMAKE_GENERATOR_PLATFORM}" MATCHES "ARM64")
    set(GEO_ARCH "ARM64")
  elseif("${CMAKE_GENERATOR}" MATCHES "ARM")
    set(GEO_ARCH "ARM")
  elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(GEO_ARCH "x64")
  else()
    set(GEO_ARCH x86)
  endif()

  if(MSVC_VERSION EQUAL 1400)
    set(GEO_RUNTIME vc8)
  elseif(MSVC_VERSION EQUAL 1500)
    set(GEO_RUNTIME vc9)
  elseif(MSVC_VERSION EQUAL 1600)
    set(GEO_RUNTIME vc10)
  elseif(MSVC_VERSION EQUAL 1700)
    set(GEO_RUNTIME vc11)
  elseif(MSVC_VERSION EQUAL 1800)
    set(GEO_RUNTIME vc12)
  elseif(MSVC_VERSION EQUAL 1900)
    set(GEO_RUNTIME vc14)
  elseif(MSVC_VERSION MATCHES "^191[0-9]$")
    set(GEO_RUNTIME vc15)
  elseif(MSVC_VERSION MATCHES "^192[0-9]$")
    set(GEO_RUNTIME vc16)
  elseif(MSVC_VERSION MATCHES "^193[0-9]$")
    set(GEO_RUNTIME vc17)
  else()
    message(WARNING "Do not recognize MSVC_VERSION \"${MSVC_VERSION}\". Cannot set GEO_RUNTIME")
  endif()
elseif(MINGW)
  set(GEO_RUNTIME mingw)

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
    set(GEO_ARCH x64)
  else()
    set(GEO_ARCH x86)
  endif()
endif()

# Fix handling of duplicated files in the same static library:
# https://public.kitware.com/Bug/view.php?id=14874
if(CMAKE_VERSION VERSION_LESS "3.1")
  foreach(var CMAKE_C_ARCHIVE_APPEND CMAKE_CXX_ARCHIVE_APPEND)
    if(${var} MATCHES "^<CMAKE_AR> r")
      string(REPLACE "<CMAKE_AR> r" "<CMAKE_AR> q" ${var} "${${var}}")
    endif()
  endforeach()
endif()


# Disables the use of the global variable errno
# for math functions that represent a single floating-point instruction.
# Not sure if this is safe and needed. ljzhu
#IF(CMAKE_COMPILER_IS_GNUCXX)
#  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
#ENDIF()


IF(MSVC AND ENABLE_CXX11)
 SET(CMAKE_CXX_STANDARD 11)
 SET(CMAKE_CXX_STANDARD_REQUIRED TRUE)
 SET(CMAKE_CXX_EXTENSIONS OFF)
 IF(CMAKE_CXX11_COMPILE_FEATURES)
   SET(HAVE_CXX11 ON)
 ENDIF()
ENDIF()

# refers to https://github.com/biicode/client/issues/10
include(CheckCXXCompilerFlag)
IF(NOT MSVC AND ENABLE_CXX11)
  SET(ENABLE_CXXFLAGS_TO_CHECK
                              -std=c++11
                              -std=c++0x
                              -std=gnu++0x
                              -std=gnu++11)
  foreach(flag ${ENABLE_CXXFLAGS_TO_CHECK})
    string(REPLACE "-std=" "_" flag_var ${flag})
    string(REPLACE "+" "x" flag_var ${flag_var})
    check_cxx_compiler_flag("${flag}" COMPILER_HAS_CXX_FLAG${flag_var})
    if(COMPILER_HAS_CXX_FLAG${flag_var})
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
      geo_list_unique(CMAKE_CXX_FLAGS)
      SET(HAVE_CXX11 1)
      break()
    endif()
  endforeach()
ENDIF()
