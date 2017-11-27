# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

if ( MSVC )
    set(WERROR_FLAG "/W4 /WX")
else( MSVC )
set(WERROR_FLAG "-Werror")
endif ( MSVC )

try_compile(OPTIONAL_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/test_compiler_cxx/check_optional.cpp )

try_compile(VARIANT_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/test_compiler_cxx/check_variant.cpp )

try_compile(STRING_VIEW_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/test_compiler_cxx/check_string_view.cpp )

try_compile(EXPERIMENTAL_STRING_VIEW_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/test_compiler_cxx/check_experimental_string_view.cpp )
try_compile(CLAMP_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/test_compiler_cxx/check_clamp.cpp  )
try_compile(HYPOT3_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/test_compiler_cxx/check_hypot3.cpp  )
try_compile(IFCONSTEXPR_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/test_compiler_cxx/check_constexpr_if.cpp  )
try_compile(FALLTHROUGH_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/test_compiler_cxx/check_fallthrough.cpp  COMPILE_DEFINITIONS ${WERROR_FLAG})

try_compile(VARIABLE_TEMPLATE_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/test_compiler_cxx/check_variable_template.cpp )

try_compile(UNUSED_AVAILABLE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/test_compiler_cxx/check_unused.cpp  COMPILE_DEFINITIONS ${WERROR_FLAG} )


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
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/compiler-config.h.in ${CONFIGURE_TARGET_LOCATION}/compiler-config.h)
else()
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/compiler-config.h.in ${PROJECT_BINARY_DIR}/compiler-config.h)
endif()

endif(NOT NO_CONFIG_GENERATION)
