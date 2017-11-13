# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End



#check for clang 3.4 and the fact that CMAKE_CXX_STANDARD doesn't work yet for that compiler

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5)
    set(CMAKE_REQUIRED_FLAGS -std=c++1y)
	set(VERSION_OPTION -std=c++1y)
  else ()
    set(CMAKE_REQUIRED_FLAGS -std=c++1z)
    set(VERSION_OPTION -std=c++1z)
  endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  # c++14 becomes default in GCC 6.1
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.1)
    set(CMAKE_REQUIRED_FLAGS -std=c++1y)
    set(VERSION_OPTION -std=c++1y)
  else ()
    set(CMAKE_REQUIRED_FLAGS -std=c++1z)
    set(VERSION_OPTION -std=c++1z)
  endif()
endif()

#boost libraries don't compile under /std:c++latest flag 1.66 might solve this issue
#if (MSVC)
#set(VERSION_OPTION /std:c++latest)
#endif()


try_compile(OPTIONAL_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/config/test_compiler_cxx/check_optional.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})

try_compile(VARIANT_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/config/test_compiler_cxx/check_variant.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})

try_compile(STRING_VIEW_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/config/test_compiler_cxx/check_string_view.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})

try_compile(EXPERIMENTAL_STRING_VIEW_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/config/test_compiler_cxx/check_experimental_string_view.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})
try_compile(CLAMP_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/config/test_compiler_cxx/check_clamp.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})
try_compile(HYPOT3_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/config/test_compiler_cxx/check_hypot3.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION} )
try_compile(IFCONSTEXPR_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/config/test_compiler_cxx/check_constexpr_if.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION} )
try_compile(FALLTHROUGH_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/config/test_compiler_cxx/check_fallthrough.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})

try_compile(VARIABLE_TEMPLATE_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/config/test_compiler_cxx/check_variable_template.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})

try_compile(UNUSED_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/config/test_compiler_cxx/check_unused.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION} )


#message(STATUS ${RESULT})
if (OPTIONAL_AVAILABLE)
set(HAVE_OPTIONAL 1)
endif()

if (EXPERIMENTAL_STRING_VIEW_AVAILABLE)
set(HAVE_EXPERIMENTAL_STRING_VIEW 1)
endif()

if (VARIANT_AVAILABLE)
set(HAVE_VARIANT 1)
endif()

if (STRING_VIEW_AVAILABLE)
set(HAVE_STRING_VIEW 1)
endif()

if (CLAMP_AVAILABLE)
set(HAVE_CLAMP 1)
endif()

if (HYPOT3_AVAILABLE)
set(HAVE_HYPOT3 1)
endif()

if (IFCONSTEXPR_AVAILABLE)
set(HAVE_IF_CONSTEXPR 1)
endif()

if (FALLTHROUGH_AVAILABLE)
set(HAVE_FALLTHROUGH 1)
endif()

if (VARIABLE_TEMPLATE_AVAILABLE)
set(HAVE_VARIABLE_TEMPLATE 1)
endif()

if (UNUSED_AVAILABLE)
set(HAVE_UNUSED 1)
endif()

if (NOT NO_CONFIG_GENERATION)
if (CONFIGURE_TARGET_LOCATION)
CONFIGURE_FILE(compiler-config.h.in ${CONFIGURE_TARGET_LOCATION}/compiler-config.h)
else()
CONFIGURE_FILE(compiler-config.h.in ${PROJECT_BINARY_DIR}/compiler-config.h)
endif()

endif(NOT NO_CONFIG_GENERATION)