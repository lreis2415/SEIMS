# Update according to OpenCV 4.x accessed on 2021-11-14
#   https://github.com/opencv/opencv/tree/4.x

if(COMMAND geo_cmake_dump_vars)  # include guard
  return()
endif()

INCLUDE(CMakeParseArguments)

# Debugging function
function(geo_cmake_dump_vars)
  set(GEO_SUPPRESS_DEPRECATIONS 1)  # suppress deprecation warnings from variable_watch() guards
  get_cmake_property(__variableNames VARIABLES)
  cmake_parse_arguments(DUMP "FORCE" "TOFILE" "" ${ARGN})

  # avoid generation of excessive logs with "--trace" or "--trace-expand" parameters
  # Note: `-DCMAKE_TRACE_MODE=1` should be passed to CMake through command line.
  #       It is not a CMake buildin variable for now (2020-12)
  #       Use `cmake . -UCMAKE_TRACE_MODE` to remove this variable from cache
  if(CMAKE_TRACE_MODE AND NOT DUMP_FORCE)
    if(DUMP_TOFILE)
      file(WRITE ${CMAKE_BINARY_DIR}/${DUMP_TOFILE} "Skipped due to enabled CMAKE_TRACE_MODE")
    else()
      message(AUTHOR_WARNING "geo_cmake_dump_vars() is skipped due to enabled CMAKE_TRACE_MODE")
    endif()
    return()
  endif()

  set(regex "${DUMP_UNPARSED_ARGUMENTS}")
  string(TOLOWER "${regex}" regex_lower)
  set(__VARS "")
  foreach(__variableName ${__variableNames})
    string(TOLOWER "${__variableName}" __variableName_lower)
    if((__variableName MATCHES "${regex}" OR __variableName_lower MATCHES "${regex_lower}")
        AND NOT __variableName_lower MATCHES "^__")
      get_property(__value VARIABLE PROPERTY "${__variableName}")
      set(__VARS "${__VARS}${__variableName}=${__value}\n")
    endif()
  endforeach()
  if(DUMP_TOFILE)
    file(WRITE ${CMAKE_BINARY_DIR}/${DUMP_TOFILE} "${__VARS}")
  else()
    message(AUTHOR_WARNING "${__VARS}")
  endif()
endfunction()

#
# CMake script hooks support
#
# Not how to use cmake_hook related functions, just not included!

function(geo_cmake_eval var_name)
  if(DEFINED ${var_name})
    file(WRITE "${CMAKE_BINARY_DIR}/CMakeCommand-${var_name}.cmake" ${${var_name}})
    include("${CMAKE_BINARY_DIR}/CMakeCommand-${var_name}.cmake")
  endif()
  if(";${ARGN};" MATCHES ";ONCE;")
    unset(${var_name} CACHE)
  endif()
endfunction()

macro(geo_cmake_configure file_name var_name)
  file(READ "${file_name}" __config)
  string(CONFIGURE "${__config}" ${var_name} ${ARGN})
endmacro()

macro(geo_update VAR)
  if(NOT DEFINED ${VAR})
    if("x${ARGN}" STREQUAL "x")
      set(${VAR} "")
    else()
      set(${VAR} ${ARGN})
    endif()
  else()
    #geo_debug_message("Preserve old value for ${VAR}: ${${VAR}}")
  endif()
endmacro()


function(_geo_access_removed_variable VAR ACCESS)
  if(ACCESS STREQUAL "MODIFIED_ACCESS")
    set(GEO_SUPPRESS_MESSAGE_REMOVED_VARIABLE_${VAR} 1 PARENT_SCOPE)
    return()
  endif()
  if(ACCESS MATCHES "UNKNOWN_.*"
      AND NOT GEO_SUPPRESS_MESSAGE_REMOVED_VARIABLE
      AND NOT GEO_SUPPRESS_MESSAGE_REMOVED_VARIABLE_${VAR}
  )
    message(WARNING "Variable has been removed from CMake scripts: ${VAR}")
    set(GEO_SUPPRESS_MESSAGE_REMOVED_VARIABLE_${VAR} 1 PARENT_SCOPE)  # suppress similar messages
  endif()
endfunction()
macro(geo_declare_removed_variable VAR)
  if(NOT DEFINED ${VAR})  # don't hit external variables
    variable_watch(${VAR} _geo_access_removed_variable)
  endif()
endmacro()
macro(geo_declare_removed_variables)
  foreach(_var ${ARGN})
    geo_declare_removed_variable(${_var})
  endforeach()
endmacro()


# Search packages for the host system instead of packages for the target system
# in case of cross compilation these macros should be defined by the toolchain file
if(NOT COMMAND find_host_package)
  macro(find_host_package)
    find_package(${ARGN})
  endmacro()
endif()
if(NOT COMMAND find_host_program)
  macro(find_host_program)
    find_program(${ARGN})
  endmacro()
endif()

# assert macro
# Note: it doesn't support lists in arguments
# Usage samples:
#   geo_assert(MyLib_FOUND)
#   geo_assert(DEFINED MyLib_INCLUDE_DIRS)
macro(geo_assert)
  if(NOT (${ARGN}))
    string(REPLACE ";" " " __assert_msg "${ARGN}")
    message(AUTHOR_WARNING "Assertion failed: ${__assert_msg}")
  endif()
endmacro()

macro(geo_debug_message)
  if(GEO_CMAKE_DEBUG_MESSAGES)
    string(REPLACE ";" " " __msg "${ARGN}")
    message(STATUS "${__msg}")
  endif()
endmacro()

macro(geo_check_environment_variables)
  foreach(_var ${ARGN})
    if(" ${${_var}}" STREQUAL " " AND DEFINED ENV{${_var}})
      set(__value "$ENV{${_var}}")
      file(TO_CMAKE_PATH "${__value}" __value) # Assume that we receive paths
      set(${_var} "${__value}")
      message(STATUS "Update variable ${_var} from environment: ${${_var}}")
    endif()
  endforeach()
endmacro()

macro(geo_path_join result_var P1 P2_)
  string(REGEX REPLACE "^[/]+" "" P2 "${P2_}")
  if("${P1}" STREQUAL "" OR "${P1}" STREQUAL ".")
    set(${result_var} "${P2}")
  elseif("${P1}" STREQUAL "/")
    set(${result_var} "/${P2}")
  elseif("${P2}" STREQUAL "")
    set(${result_var} "${P1}")
  else()
    set(${result_var} "${P1}/${P2}")
  endif()
  string(REPLACE "\\\\" "\\" ${result_var} "${${result_var}}")
  string(REPLACE "//" "/" ${result_var} "${${result_var}}")
  string(REGEX REPLACE "(^|[/\\])[\\.][/\\]" "\\1" ${result_var} "${${result_var}}")
  if("${${result_var}}" STREQUAL "")
    set(${result_var} ".")
  endif()
  #message(STATUS "'${P1}' '${P2_}' => '${${result_var}}'")
endmacro()


# check if "sub" (file or dir) is below "dir"
function(geo_is_subdir res dir sub )
  get_filename_component(dir "${dir}" ABSOLUTE)
  get_filename_component(sub "${sub}" ABSOLUTE)
  file(TO_CMAKE_PATH "${dir}" dir)
  file(TO_CMAKE_PATH "${sub}" sub)
  set(dir "${dir}/")
  string(LENGTH "${dir}" len)
  string(LENGTH "${sub}" len_sub)
  if(NOT len GREATER len_sub)
    string(SUBSTRING "${sub}" 0 ${len} prefix)
  endif()
  if(prefix AND prefix STREQUAL dir)
    set(${res} TRUE PARENT_SCOPE)
  else()
    set(${res} FALSE PARENT_SCOPE)
  endif()
endfunction()


function(geo_is_repo_directory result_var dir)
  get_filename_component(__abs_dir "${dir}" ABSOLUTE)
  if("${__abs_dir}" MATCHES "^${CMAKE_SOURCE_DIR}"
      OR "${__abs_dir}" MATCHES "^${CMAKE_BINARY_DIR}")
    set(${result_var} 1 PARENT_SCOPE)
  else()
    set(${result_var} 0 PARENT_SCOPE)
  endif()
endfunction()


# adds include directories in such a way that directories from the repository source tree go first
function(geo_include_directories)
  geo_debug_message("geo_include_directories( ${ARGN} )")
  set(__add_before "")
  foreach(dir ${ARGN})
    geo_is_repo_directory(__is_repo_dir "${dir}")
    if(__is_repo_dir)
      list(APPEND __add_before "${dir}")
    elseif(CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0" AND
           dir MATCHES "/usr/include$")
      # workaround for GCC 6.x bug
    else()
      include_directories(AFTER SYSTEM "${dir}")
    endif()
  endforeach()
  include_directories(BEFORE ${__add_before})
endfunction()

function(geo_append_target_property target prop)
  get_target_property(val ${target} ${prop})
  if(val)
    set(val "${val} ${ARGN}")
    set_target_properties(${target} PROPERTIES ${prop} "${val}")
  else()
    set_target_properties(${target} PROPERTIES ${prop} "${ARGN}")
  endif()
endfunction()


# clears all passed variables
MACRO(clear_vars)
  foreach(_var ${ARGN})
    unset(${_var})
    unset(${_var} CACHE)
  endforeach()
ENDMACRO()

# Clears passed variables with INTERNAL type from CMake cache
macro(geo_clear_internal_cache_vars)
  foreach(_var ${ARGN})
    get_property(_propertySet CACHE ${_var} PROPERTY TYPE SET)
    if(_propertySet)
      get_property(_type CACHE ${_var} PROPERTY TYPE)
      if(_type STREQUAL "INTERNAL")
        message("Cleaning INTERNAL cached variable: ${_var}")
        unset(${_var} CACHE)
      endif()
    endif()
  endforeach()
  unset(_propertySet)
  unset(_type)
endmacro()

set(GEO_COMPILER_FAIL_REGEX
    "argument .* is not valid"                  # GCC 9+ (including support of unicode quotes)
    "command[- ]line option .* is valid for .* but not for C\\+\\+" # GNU
    "command[- ]line option .* is valid for .* but not for C" # GNU
    "unrecognized .*option"                     # GNU
    "unknown .*option"                          # Clang
    "ignoring unknown option"                   # MSVC
    "warning D9002"                             # MSVC, any lang
    "option .*not supported"                    # Intel
    "[Uu]nknown option"                         # HP
    "[Ww]arning: [Oo]ption"                     # SunPro
    "command option .* is not recognized"       # XL
    "not supported in this configuration, ignored"       # AIX (';' is replaced with ',')
    "File with unknown suffix passed to linker" # PGI
    "WARNING: unknown flag:"                    # Open64
  )


MACRO(geo_check_compiler_flag LANG FLAG RESULT)
  set(_fname "${ARGN}")
  if(NOT DEFINED ${RESULT})
    if(_fname)
      # nothing
    elseif("_${LANG}_" MATCHES "_CXX_")
      set(_fname "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx")
      if("${CMAKE_CXX_FLAGS} ${FLAG} " MATCHES "-Werror " OR "${CMAKE_CXX_FLAGS} ${FLAG} " MATCHES "-Werror=unknown-pragmas ")
        FILE(WRITE "${_fname}" "int main() { return 0; }\n")
      else()
        FILE(WRITE "${_fname}" "#pragma\nint main() { return 0; }\n")
      endif()
    elseif("_${LANG}_" MATCHES "_C_")
      set(_fname "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.c")
      if("${CMAKE_C_FLAGS} ${FLAG} " MATCHES "-Werror " OR "${CMAKE_C_FLAGS} ${FLAG} " MATCHES "-Werror=unknown-pragmas ")
        FILE(WRITE "${_fname}" "int main(void) { return 0; }\n")
      else()
        FILE(WRITE "${_fname}" "#pragma\nint main(void) { return 0; }\n")
      endif()
    elseif("_${LANG}_" MATCHES "_OBJCXX_")
      set(_fname "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.mm")
      if("${CMAKE_CXX_FLAGS} ${FLAG} " MATCHES "-Werror " OR "${CMAKE_CXX_FLAGS} ${FLAG} " MATCHES "-Werror=unknown-pragmas ")
        FILE(WRITE "${_fname}" "int main() { return 0; }\n")
      else()
        FILE(WRITE "${_fname}" "#pragma\nint main() { return 0; }\n")
      endif()
    else()
      unset(_fname)
    endif()
    if(_fname)
      if(NOT "x${ARGN}" STREQUAL "x")
        file(RELATIVE_PATH __msg "${CMAKE_SOURCE_DIR}" "${ARGN}")
        set(__msg " (check file: ${__msg})")
      else()
        set(__msg "")
      endif()
      if(CMAKE_REQUIRED_LIBRARIES)
        set(__link_libs LINK_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
      else()
        set(__link_libs)
      endif()
      set(__cmake_flags "")
      if(CMAKE_EXE_LINKER_FLAGS)  # CMP0056 do this on new CMake
        list(APPEND __cmake_flags "-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}")
      endif()

      # CMP0067 do this on new CMake
      if(DEFINED CMAKE_CXX_STANDARD)
        list(APPEND __cmake_flags "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}")
      endif()
      if(DEFINED CMAKE_CXX_STANDARD_REQUIRED)
        list(APPEND __cmake_flags "-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}")
      endif()
      if(DEFINED CMAKE_CXX_EXTENSIONS)
        list(APPEND __cmake_flags "-DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}")
      endif()

      MESSAGE(STATUS "Performing Test ${RESULT}${__msg}")
      TRY_COMPILE(${RESULT}
        "${CMAKE_BINARY_DIR}"
        "${_fname}"
        CMAKE_FLAGS ${__cmake_flags}
        COMPILE_DEFINITIONS "${FLAG}"
        ${__link_libs}
        OUTPUT_VARIABLE OUTPUT)

      if(${RESULT})
        string(REPLACE ";" "," OUTPUT_LINES "${OUTPUT}")
        string(REPLACE "\n" ";" OUTPUT_LINES "${OUTPUT_LINES}")
        foreach(_regex ${GEO_COMPILER_FAIL_REGEX})
          if(NOT ${RESULT})
            break()
          endif()
          foreach(_line ${OUTPUT_LINES})
            if("${_line}" MATCHES "${_regex}")
              file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
                  "Build output check failed:\n"
                  "    Regex: '${_regex}'\n"
                  "    Output line: '${_line}'\n")
              set(${RESULT} 0)
              break()
            endif()
          endforeach()
        endforeach()
      endif()

      IF(${RESULT})
        SET(${RESULT} 1 CACHE INTERNAL "Test ${RESULT}")
        MESSAGE(STATUS "Performing Test ${RESULT} - Success")
      ELSE(${RESULT})
        MESSAGE(STATUS "Performing Test ${RESULT} - Failed")
        SET(${RESULT} "" CACHE INTERNAL "Test ${RESULT}")
        file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
            "Compilation failed:\n"
            "    source file: '${_fname}'\n"
            "    check option: '${FLAG}'\n"
            "===== BUILD LOG =====\n"
            "${OUTPUT}\n"
            "===== END =====\n\n")
      ENDIF(${RESULT})
    else()
      SET(${RESULT} 0)
    endif()
  endif()
ENDMACRO()

macro(geo_check_flag_support lang flag varname base_options)
  if(CMAKE_BUILD_TYPE)
    set(CMAKE_TRY_COMPILE_CONFIGURATION ${CMAKE_BUILD_TYPE})
  endif()

  if("_${lang}_" MATCHES "_CXX_")
    set(_lang CXX)
  elseif("_${lang}_" MATCHES "_C_")
    set(_lang C)
  elseif("_${lang}_" MATCHES "_OBJCXX_")
    if(DEFINED CMAKE_OBJCXX_COMPILER)  # CMake 3.16+ and enable_language(OBJCXX) call are required
      set(_lang OBJCXX)
    else()
      set(_lang CXX)
    endif()
  else()
    set(_lang ${lang})
  endif()

  string(TOUPPER "${flag}" ${varname})
  string(REGEX REPLACE "^(/|-)" "HAVE_${_lang}_" ${varname} "${${varname}}")
  string(REGEX REPLACE " -|-|=| |\\.|," "_" ${varname} "${${varname}}")

  if(DEFINED CMAKE_${_lang}_COMPILER)
    geo_check_compiler_flag("${_lang}" "${base_options} ${flag}" ${${varname}} ${ARGN})
  endif()
endmacro()

macro(geo_check_runtime_flag flag result)
  set(_fname "${ARGN}")
  if(NOT DEFINED ${result})
    file(RELATIVE_PATH _rname "${CMAKE_SOURCE_DIR}" "${_fname}")
    message(STATUS "Performing Runtime Test ${result} (check file: ${_rname})")
    try_run(exec_return compile_result
      "${CMAKE_BINARY_DIR}"
      "${_fname}"
      CMAKE_FLAGS "-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}" # CMP0056 do this on new CMake
      COMPILE_DEFINITIONS "${flag}"
      OUTPUT_VARIABLE OUTPUT)

    if(${compile_result})
      if(exec_return EQUAL 0)
        set(${result} 1 CACHE INTERNAL "Runtime Test ${result}")
        message(STATUS "Performing Runtime Test ${result} - Success")
      else()
        message(STATUS "Performing Runtime Test ${result} - Failed(${exec_return})")
        set(${result} 0 CACHE INTERNAL "Runtime Test ${result}")
      endif()
    else()
      set(${result} 0 CACHE INTERNAL "Runtime Test ${result}")
      message(STATUS "Performing Runtime Test ${result} - Compiling Failed")
    endif()

    if(NOT ${result})
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Runtime Test failed:\n"
        "    source file: '${_fname}'\n"
        "    check option: '${flag}'\n"
        "    exec return: ${exec_return}\n"
        "===== BUILD AND RUNTIME LOG =====\n"
        "${OUTPUT}\n"
        "===== END =====\n\n")
    endif()
  endif()
endmacro()

# turns off warnings
macro(geo_warnings_disable)
  if(NOT ENABLE_NOISY_WARNINGS)
    set(_flag_vars "")
    set(_msvc_warnings "")
    set(_gxx_warnings "")
    set(_icc_warnings "")
    foreach(arg ${ARGN})
      if(arg MATCHES "^CMAKE_")
        list(APPEND _flag_vars ${arg})
      elseif(arg MATCHES "^/wd")
        list(APPEND _msvc_warnings ${arg})
      elseif(arg MATCHES "^-W")
        list(APPEND _gxx_warnings ${arg})
      elseif(arg MATCHES "^-wd" OR arg MATCHES "^-Qwd" OR arg MATCHES "^/Qwd")
        list(APPEND _icc_warnings ${arg})
      endif()
    endforeach()
    if(MSVC AND _msvc_warnings AND _flag_vars)
      foreach(var ${_flag_vars})
        foreach(warning ${_msvc_warnings})
          set(${var} "${${var}} ${warning}")
        endforeach()
      endforeach()
    elseif(((CV_GCC OR CV_CLANG) OR (UNIX AND CV_ICC)) AND _gxx_warnings AND _flag_vars)
      foreach(var ${_flag_vars})
        foreach(warning ${_gxx_warnings})
          if(NOT warning MATCHES "^-Wno-")
            string(REGEX REPLACE "(^|[ ]+)${warning}(=[^ ]*)?([ ]+|$)" " " ${var} "${${var}}")
            string(REPLACE "-W" "-Wno-" warning "${warning}")
          endif()
          geo_check_flag_support(${var} "${warning}" _varname "")
          if(${_varname})
            set(${var} "${${var}} ${warning}")
          endif()
        endforeach()
      endforeach()
    endif()
    if(CV_ICC AND _icc_warnings AND _flag_vars)
      foreach(var ${_flag_vars})
        foreach(warning ${_icc_warnings})
          if(UNIX)
            string(REPLACE "-Qwd" "-wd" warning "${warning}")
          else()
            string(REPLACE "-wd" "-Qwd" warning "${warning}")
          endif()
          geo_check_flag_support(${var} "${warning}" _varname "")
          if(${_varname})
            set(${var} "${${var}} ${warning}")
          endif()
        endforeach()
      endforeach()
    endif()
    unset(_flag_vars)
    unset(_msvc_warnings)
    unset(_gxx_warnings)
    unset(_icc_warnings)
  endif(NOT ENABLE_NOISY_WARNINGS)
endmacro()

macro(geo_append_source_file_compile_definitions source)
  get_source_file_property(_value "${source}" COMPILE_DEFINITIONS)
  if(_value)
    set(_value ${_value} ${ARGN})
  else()
    set(_value ${ARGN})
  endif()
  set_source_files_properties("${source}" PROPERTIES COMPILE_DEFINITIONS "${_value}")
endmacro()

macro(add_apple_compiler_options srcs)
  geo_check_flag_support(OBJCXX "-fobjc-exceptions" HAVE_OBJC_EXCEPTIONS "")
  if(HAVE_OBJC_EXCEPTIONS)
    foreach(source ${srcs})
      if("${source}" MATCHES "\\.mm$")
        get_source_file_property(flags "${source}" COMPILE_FLAGS)
        if(flags)
          set(flags "${_flags} -fobjc-exceptions")
        else()
          set(flags "-fobjc-exceptions")
        endif()

        set_source_files_properties("${source}" PROPERTIES COMPILE_FLAGS "${flags}")
      endif()
    endforeach()
  endif()
endmacro()


# Provides an option that the user can optionally select.
# Can accept condition to control when option is available for user.
# Usage:
#   option(<option_variable>
#          "help string describing the option"
#          <initial value or boolean expression>
#          [VISIBLE_IF <condition>]
#          [VERIFY <condition>])
macro(GEO_OPTION variable description value)
  set(__value ${value})
  set(__condition "")
  set(__verification)
  set(__varname "__value")
  foreach(arg ${ARGN})
    if(arg STREQUAL "IF" OR arg STREQUAL "if" OR arg STREQUAL "VISIBLE_IF")
      set(__varname "__condition")
    elseif(arg STREQUAL "VERIFY")
      set(__varname "__verification")
    else()
      list(APPEND ${__varname} ${arg})
    endif()
  endforeach()
  unset(__varname)
  if(__condition STREQUAL "")
    set(__condition 2 GREATER 1)
  endif()

  if(${__condition})
    if(__value MATCHES ";")
      if(${__value})
        option(${variable} "${description}" ON)
      else()
        option(${variable} "${description}" OFF)
      endif()
    elseif(DEFINED ${__value})
      if(${__value})
        option(${variable} "${description}" ON)
      else()
        option(${variable} "${description}" OFF)
      endif()
    else()
      option(${variable} "${description}" ${__value})
    endif()
  else()
    if(DEFINED ${variable} AND "${${variable}}")  # emit warnings about turned ON options only.
      message(WARNING "Unexpected option: ${variable} (=${${variable}})\nCondition: IF (${__condition})")
    endif()
    if(GEO_UNSET_UNSUPPORTED_OPTION)
      unset(${variable} CACHE)
    endif()
  endif()
  if(__verification)
    set(GEO_VERIFY_${variable} "${__verification}") # variable containing condition to verify
    list(APPEND GEO_VERIFICATIONS "${variable}") # list of variable names (WITH_XXX;WITH_YYY;...)
  endif()
  unset(__condition)
  unset(__value)
endmacro()


# Check that each variable stored in GEO_VERIFICATIONS list
# is consistent with actual detection result (stored as condition in GEO_VERIFY_...) variables
function(geo_verify_config)
  set(broken_options)
  foreach(var ${GEO_VERIFICATIONS})
    set(evaluated FALSE)
    if(${GEO_VERIFY_${var}})
      set(evaluated TRUE)
    endif()
    status("Verifying ${var}=${${var}} => '${GEO_VERIFY_${var}}'=${evaluated}")
    if (${var} AND NOT evaluated)
      list(APPEND broken_options ${var})
      message(WARNING
        "Option ${var} is enabled but corresponding dependency "
        "have not been found: \"${GEO_VERIFY_${var}}\" is FALSE")
    elseif(NOT ${var} AND evaluated)
      list(APPEND broken_options ${var})
      message(WARNING
        "Option ${var} is disabled or unset but corresponding dependency "
        "have been explicitly turned on: \"${GEO_VERIFY_${var}}\" is TRUE")
    endif()
  endforeach()
  if(broken_options)
    string(REPLACE ";" "\n" broken_options "${broken_options}")
    message(FATAL_ERROR
      "Some dependencies have not been found or have been forced, "
      "unset ENABLE_CONFIG_VERIFICATION option to ignore these failures "
      "or change following options:\n${broken_options}")
  endif()
endfunction()


function(geo_append_source_files_cxx_compiler_options files_var)
  set(__flags "${ARGN}")
  geo_check_flag_support(CXX "${__flags}" __HAVE_COMPILER_OPTIONS_VAR "")
  if(${__HAVE_COMPILER_OPTIONS_VAR})
    foreach(source ${${files_var}})
      if("${source}" MATCHES "\\.(cpp|cc|cxx)$")
        get_source_file_property(flags "${source}" COMPILE_FLAGS)
        if(flags)
          set(flags "${flags} ${__flags}")
        else()
          set(flags "${__flags}")
        endif()
        set_source_files_properties("${source}" PROPERTIES COMPILE_FLAGS "${flags}")
      endif()
    endforeach()
  endif()
endfunction()

# Usage is similar to CMake 'pkg_check_modules' command
# It additionally controls HAVE_${define} and ${define}_${modname}_FOUND variables
macro(geo_check_modules define)
  unset(HAVE_${define})
  foreach(m ${ARGN})
    if (m MATCHES "(.*[^><])(>=|=|<=)(.*)")
      set(__modname "${CMAKE_MATCH_1}")
    else()
      set(__modname "${m}")
    endif()
    unset(${define}_${__modname}_FOUND)
  endforeach()
  if(PKG_CONFIG_FOUND OR PkgConfig_FOUND)
    pkg_check_modules(${define} ${ARGN})
  endif()
  if(${define}_FOUND)
    set(HAVE_${define} 1)
  endif()
  foreach(m ${ARGN})
    if (m MATCHES "(.*[^><])(>=|=|<=)(.*)")
      set(__modname "${CMAKE_MATCH_1}")
    else()
      set(__modname "${m}")
    endif()
    if(NOT DEFINED ${define}_${__modname}_FOUND AND ${define}_FOUND)
      set(${define}_${__modname}_FOUND 1)
    endif()
  endforeach()
  if(${define}_FOUND AND ${define}_LIBRARIES)
    if(${define}_LINK_LIBRARIES_XXXXX)  # CMake 3.12+: https://gitlab.kitware.com/cmake/cmake/merge_requests/2068
      set(${define}_LIBRARIES "${${define}_LINK_LIBRARIES}" CACHE INTERNAL "")
    else()
      unset(_libs)          # absolute paths
      unset(_libs_paths)  # -L args
      foreach(flag ${${define}_LDFLAGS})
        if(flag MATCHES "^-L(.*)")
          list(APPEND _libs_paths ${CMAKE_MATCH_1})
        elseif(IS_ABSOLUTE "${flag}"
            OR flag STREQUAL "-lstdc++"
        )
          list(APPEND _libs "${flag}")
        elseif(flag MATCHES "^-l(.*)")
          set(_lib "${CMAKE_MATCH_1}")
          if(_libs_paths)
            find_library(pkgcfg_lib_${define}_${_lib} NAMES ${_lib}
                         HINTS ${_libs_paths} NO_DEFAULT_PATH)
          endif()
          find_library(pkgcfg_lib_${define}_${_lib} NAMES ${_lib})
          mark_as_advanced(pkgcfg_lib_${define}_${_lib})
          if(pkgcfg_lib_${define}_${_lib})
            list(APPEND _libs "${pkgcfg_lib_${define}_${_lib}}")
          else()
            message(WARNING "geo_check_modules(${define}): can't find library '${_lib}'. Specify 'pkgcfg_lib_${define}_${_lib}' manually")
            list(APPEND _libs "${_lib}")
          endif()
        else()
          # -pthread
          #message(WARNING "geo_check_modules(${define}): unknown LDFLAG '${flag}'")
        endif()
      endforeach()
      set(${define}_LINK_LIBRARIES "${_libs}")
      set(${define}_LIBRARIES "${_libs}" CACHE INTERNAL "")
      unset(_lib)
      unset(_libs)
      unset(_libs_paths)
    endif()
  endif()
endmacro()


if(NOT DEFINED CMAKE_ARGC) # Guard CMake standalone invocations

# Use this option carefully, CMake's install() will install symlinks instead of real files
# It is fine for development, but should not be used by real installations
set(__symlink_default OFF)  # preprocessing is required for old CMake like 2.8.12
if(DEFINED ENV{BUILD_USE_SYMLINKS})
  set(__symlink_default $ENV{BUILD_USE_SYMLINKS})
endif()
GEO_OPTION(BUILD_USE_SYMLINKS "Use symlinks instead of files copying during build (and !!INSTALL!!)" (${__symlink_default}) IF (UNIX OR DEFINED __symlink_default))

if(CMAKE_VERSION VERSION_LESS "3.2")
  macro(geo_cmake_byproducts var_name)
    set(${var_name}) # nothing
  endmacro()
else()
  macro(geo_cmake_byproducts var_name)
    set(${var_name} BYPRODUCTS ${ARGN})
  endmacro()
endif()

if(BUILD_USE_SYMLINKS)
  set(__file0 "${CMAKE_CURRENT_LIST_FILE}")
  set(__file1 "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/symlink_test")
  if(NOT IS_SYMLINK "${__file1}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${__file0}" "${__file1}"
        RESULT_VARIABLE SYMLINK_RESULT)
    if(NOT SYMLINK_RESULT EQUAL 0)
      file(REMOVE "${__file1}")
    endif()
    if(NOT IS_SYMLINK "${__file1}")
      set(BUILD_USE_SYMLINKS 0 CACHE INTERNAL "")
    endif()
  endif()
  if(NOT BUILD_USE_SYMLINKS)
    message(STATUS "Build symlinks are not available (disabled)")
  endif()
endif()

function(geo_output_status msg)
  message(STATUS "${msg}")
  string(REPLACE "\\" "\\\\" msg "${msg}")
  string(REPLACE "\"" "\\\"" msg "${msg}")
  string(REGEX REPLACE "^\n+|\n+$" "" msg "${msg}")
  if(msg MATCHES "\n")
    message(WARNING "String to be inserted to version_string.inc has an unexpected line break: '${msg}'")
    string(REPLACE "\n" "\\n" msg "${msg}")
  endif()
endfunction()

# Status report function.
# Automatically align right column and selects text based on condition.
# Usage:
#   status(<text>)
#   status(<heading> <value1> [<value2> ...])
#   status(<heading> <condition> THEN <text for TRUE> ELSE <text for FALSE> )
function(status text)
  set(status_cond)
  set(status_then)
  set(status_else)

  set(status_current_name "cond")
  foreach(arg ${ARGN})
    if(arg STREQUAL "THEN")
      set(status_current_name "then")
    elseif(arg STREQUAL "ELSE")
      set(status_current_name "else")
    else()
      list(APPEND status_${status_current_name} ${arg})
    endif()
  endforeach()

  if(DEFINED status_cond)
    set(status_placeholder_length 32)
    string(RANDOM LENGTH ${status_placeholder_length} ALPHABET " " status_placeholder)
    string(LENGTH "${text}" status_text_length)
    if(status_text_length LESS status_placeholder_length)
      string(SUBSTRING "${text}${status_placeholder}" 0 ${status_placeholder_length} status_text)
    elseif(DEFINED status_then OR DEFINED status_else)
      geo_output_status("${text}")
      set(status_text "${status_placeholder}")
    else()
      set(status_text "${text}")
    endif()

    if(DEFINED status_then OR DEFINED status_else)
      if(${status_cond})
        string(REPLACE ";" " " status_then "${status_then}")
        string(REGEX REPLACE "^[ \t]+" "" status_then "${status_then}")
        geo_output_status("${status_text} ${status_then}")
      else()
        string(REPLACE ";" " " status_else "${status_else}")
        string(REGEX REPLACE "^[ \t]+" "" status_else "${status_else}")
        geo_output_status("${status_text} ${status_else}")
      endif()
    else()
      string(REPLACE ";" " " status_cond "${status_cond}")
      string(REGEX REPLACE "^[ \t]+" "" status_cond "${status_cond}")
      geo_output_status("${status_text} ${status_cond}")
    endif()
  else()
    geo_output_status("${text}")
  endif()
endfunction()

endif() # NOT DEFINED CMAKE_ARGC


macro(geo_check_flag_support lang flag varname base_options)
  if(CMAKE_BUILD_TYPE)
    set(CMAKE_TRY_COMPILE_CONFIGURATION ${CMAKE_BUILD_TYPE})
  endif()

  if("_${lang}_" MATCHES "_CXX_")
    set(_lang CXX)
  elseif("_${lang}_" MATCHES "_C_")
    set(_lang C)
  elseif("_${lang}_" MATCHES "_OBJCXX_")
    set(_lang OBJCXX)
  else()
    set(_lang ${lang})
  endif()

  string(TOUPPER "${flag}" ${varname})
  string(REGEX REPLACE "^(/|-)" "HAVE_${_lang}_" ${varname} "${${varname}}")
  string(REGEX REPLACE " -|-|=| |\\." "_" ${varname} "${${varname}}")

  geo_check_compiler_flag("${_lang}" "${base_options} ${flag}" ${${varname}} ${ARGN})
endmacro()


# Macro that checks if module has been installed.
# After it adds module to build and define
# constants passed as second arg
macro(CHECK_MODULE module_name define geo_module)
  set(${define} 0)
  if(PKG_CONFIG_FOUND)
    set(ALIAS               ALIASOF_${module_name})
    set(ALIAS_FOUND                 ${ALIAS}_FOUND)
    set(ALIAS_INCLUDE_DIRS   ${ALIAS}_INCLUDE_DIRS)
    set(ALIAS_LIBRARY_DIRS   ${ALIAS}_LIBRARY_DIRS)
    set(ALIAS_LIBRARIES         ${ALIAS}_LIBRARIES)

    PKG_CHECK_MODULES(${ALIAS} ${module_name})
    if(${ALIAS_FOUND})
      set(${define} 1)
      geo_append_build_options(${geo_module} ${ALIAS})
    endif()
  endif()
endmacro()

# remove all matching elements from the list
macro(geo_list_filterout lst regex)
  foreach(item ${${lst}})
    if(item MATCHES "${regex}")
      list(REMOVE_ITEM ${lst} "${item}")
    endif()
  endforeach()
endmacro()


# filter matching elements from the list
macro(geo_list_filter lst regex)
  set(dst ${ARGN})
  if(NOT dst)
    set(dst ${lst})
  endif()
  set(__result ${${lst}})
  foreach(item ${__result})
    if(NOT item MATCHES "${regex}")
      list(REMOVE_ITEM __result "${item}")
    endif()
  endforeach()
  set(${dst} ${__result})
endmacro()


# stable & safe duplicates removal macro
macro(geo_list_unique __lst)
  if(${__lst})
    list(REMOVE_DUPLICATES ${__lst})
  endif()
endmacro()


# safe list reversal macro
macro(geo_list_reverse __lst)
  if(${__lst})
    list(REVERSE ${__lst})
  endif()
endmacro()


# safe list sorting macro
macro(geo_list_sort __lst)
  if(${__lst})
    list(SORT ${__lst})
  endif()
endmacro()


# add prefix to each item in the list
macro(geo_list_add_prefix LST PREFIX)
  set(__tmp "")
  foreach(item ${${LST}})
    list(APPEND __tmp "${PREFIX}${item}")
  endforeach()
  set(${LST} ${__tmp})
  unset(__tmp)
endmacro()


# add suffix to each item in the list
macro(geo_list_add_suffix LST SUFFIX)
  set(__tmp "")
  foreach(item ${${LST}})
    list(APPEND __tmp "${item}${SUFFIX}")
  endforeach()
  set(${LST} ${__tmp})
  unset(__tmp)
endmacro()


# gets and removes the first element from the list
macro(geo_list_pop_front LST VAR)
  if(${LST})
    list(GET ${LST} 0 ${VAR})
    list(REMOVE_AT ${LST} 0)
  else()
    set(${VAR} "")
  endif()
endmacro()


# Get list of duplicates in the list of input items.
# geo_get_duplicates(<output list> <element> [<element> ...])
function(geo_get_duplicates res)
  if(ARGC LESS 2)
    message(FATAL_ERROR "Invalid call to geo_get_duplicates")
  endif()
  set(lst ${ARGN})
  list(SORT lst)
  set(prev_item)
  foreach(item ${lst})
    if(item STREQUAL prev_item)
      list(APPEND dups ${item})
    endif()
    set(prev_item ${item})
  endforeach()
  set(${res} ${dups} PARENT_SCOPE)
endfunction()

# simple regex escaping routine (does not cover all cases!!!)
macro(geo_regex_escape var regex)
  string(REGEX REPLACE "([+.*^$])" "\\\\1" ${var} "${regex}")
endmacro()


# convert list of paths to full paths
macro(geo_convert_to_full_paths VAR)
  if(${VAR})
    set(__tmp "")
    foreach(path ${${VAR}})
      get_filename_component(${VAR} "${path}" ABSOLUTE)
      list(APPEND __tmp "${${VAR}}")
    endforeach()
    set(${VAR} ${__tmp})
    unset(__tmp)
  endif()
endmacro()


# convert list of paths to libraries names without lib prefix
function(geo_convert_to_lib_name var)
  set(tmp "")
  foreach(path ${ARGN})
    get_filename_component(tmp_name "${path}" NAME)
    geo_get_libname(tmp_name "${tmp_name}")
    list(APPEND tmp "${tmp_name}")
  endforeach()
  set(${var} ${tmp} PARENT_SCOPE)
endfunction()

# read set of version defines from the header file
macro(geo_parse_header FILENAME FILE_VAR)
  set(vars_regex "")
  set(__parnet_scope OFF)
  set(__add_cache OFF)
  foreach(name ${ARGN})
    if(${name} STREQUAL "PARENT_SCOPE")
      set(__parnet_scope ON)
    elseif(${name} STREQUAL "CACHE")
      set(__add_cache ON)
    elseif(vars_regex)
      set(vars_regex "${vars_regex}|${name}")
    else()
      set(vars_regex "${name}")
    endif()
  endforeach()
  if(EXISTS "${FILENAME}")
    file(STRINGS "${FILENAME}" ${FILE_VAR} REGEX "#define[ \t]+(${vars_regex})[ \t]+[0-9]+" )
  else()
    unset(${FILE_VAR})
  endif()
  foreach(name ${ARGN})
    if(NOT ${name} STREQUAL "PARENT_SCOPE" AND NOT ${name} STREQUAL "CACHE")
      if(${FILE_VAR})
        if(${FILE_VAR} MATCHES ".+[ \t]${name}[ \t]+([0-9]+).*")
          string(REGEX REPLACE ".+[ \t]${name}[ \t]+([0-9]+).*" "\\1" ${name} "${${FILE_VAR}}")
        else()
          set(${name} "")
        endif()
        if(__add_cache)
          set(${name} ${${name}} CACHE INTERNAL "${name} parsed from ${FILENAME}" FORCE)
        elseif(__parnet_scope)
          set(${name} "${${name}}" PARENT_SCOPE)
        endif()
      else()
        unset(${name} CACHE)
      endif()
    endif()
  endforeach()
endmacro()

# read single version define from the header file
macro(geo_parse_header2 LIBNAME HDR_PATH VARNAME)
  geo_clear_vars(${LIBNAME}_VERSION_MAJOR
                 ${LIBNAME}_VERSION_MAJOR
                 ${LIBNAME}_VERSION_MINOR
                 ${LIBNAME}_VERSION_PATCH
                 ${LIBNAME}_VERSION_TWEAK
                 ${LIBNAME}_VERSION_STRING)
  set(${LIBNAME}_H "")
  if(EXISTS "${HDR_PATH}")
    file(STRINGS "${HDR_PATH}" ${LIBNAME}_H REGEX "^#define[ \t]+${VARNAME}[ \t]+\"[^\"]*\".*$" LIMIT_COUNT 1)
  endif()

  if(${LIBNAME}_H)
    string(REGEX REPLACE "^.*[ \t]${VARNAME}[ \t]+\"([0-9]+).*$" "\\1" ${LIBNAME}_VERSION_MAJOR "${${LIBNAME}_H}")
    string(REGEX REPLACE "^.*[ \t]${VARNAME}[ \t]+\"[0-9]+\\.([0-9]+).*$" "\\1" ${LIBNAME}_VERSION_MINOR  "${${LIBNAME}_H}")
    string(REGEX REPLACE "^.*[ \t]${VARNAME}[ \t]+\"[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" ${LIBNAME}_VERSION_PATCH "${${LIBNAME}_H}")
    set(${LIBNAME}_VERSION_MAJOR ${${LIBNAME}_VERSION_MAJOR} ${ARGN})
    set(${LIBNAME}_VERSION_MINOR ${${LIBNAME}_VERSION_MINOR} ${ARGN})
    set(${LIBNAME}_VERSION_PATCH ${${LIBNAME}_VERSION_PATCH} ${ARGN})
    set(${LIBNAME}_VERSION_STRING "${${LIBNAME}_VERSION_MAJOR}.${${LIBNAME}_VERSION_MINOR}.${${LIBNAME}_VERSION_PATCH}")

    # append a TWEAK version if it exists:
    set(${LIBNAME}_VERSION_TWEAK "")
    if("${${LIBNAME}_H}" MATCHES "^.*[ \t]${VARNAME}[ \t]+\"[0-9]+\\.[0-9]+\\.[0-9]+\\.([0-9]+).*$")
      set(${LIBNAME}_VERSION_TWEAK "${CMAKE_MATCH_1}" ${ARGN})
    endif()
    if(${LIBNAME}_VERSION_TWEAK)
      set(${LIBNAME}_VERSION_STRING "${${LIBNAME}_VERSION_STRING}.${${LIBNAME}_VERSION_TWEAK}" ${ARGN})
    else()
      set(${LIBNAME}_VERSION_STRING "${${LIBNAME}_VERSION_STRING}" ${ARGN})
    endif()
  endif()
endmacro()

# read single version info from the pkg file
macro(geo_parse_pkg LIBNAME PKG_PATH SCOPE)
  if(EXISTS "${PKG_PATH}/${LIBNAME}.pc")
    file(STRINGS "${PKG_PATH}/${LIBNAME}.pc" line_to_parse REGEX "^Version:[ \t]+[0-9.]*.*$" LIMIT_COUNT 1)
    STRING(REGEX REPLACE ".*Version: ([^ ]+).*" "\\1" ALIASOF_${LIBNAME}_VERSION "${line_to_parse}" )
  endif()
endmacro()

macro(geo_get_libname var_name)
  get_filename_component(__libname "${ARGN}" NAME)
  string(REGEX REPLACE "^lib(.+).(a|so)(.[.0-9]+)?$" "\\1" __libname "${__libname}")
  set(${var_name} "${__libname}")
endmacro()

macro(geo_cmake_script_append_var content_var)
  foreach(var_name ${ARGN})
    set(${content_var} "${${content_var}}
set(${var_name} \"${${var_name}}\")
")
  endforeach()
endmacro()
