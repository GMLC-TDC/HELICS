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


set(TEST_CXX_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/test_compiler_cxx)

if (ENABLE_CXX_17)
try_compile(OPTIONAL_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_optional.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})

try_compile(VARIANT_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_variant.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})

try_compile(STRING_VIEW_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_string_view.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})

#try_compile(CLAMP_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_clamp.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})
#try_compile(HYPOT3_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_hypot3.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})

try_compile(IFCONSTEXPR_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_constexpr_if.cpp  COMPILE_DEFINITIONS ${WERROR_FLAG} )

try_compile(FALLTHROUGH_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_fallthrough.cpp  COMPILE_DEFINITIONS ${WERROR_FLAG} ${VERSION_OPTION})

try_compile(UNUSED_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_unused.cpp  COMPILE_DEFINITIONS ${WERROR_FLAG} ${VERSION_OPTION})

endif(ENABLE_CXX_17)

try_compile(VARIABLE_TEMPLATE_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_variable_template.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})
# this is normally a C++17 thing but clang <3.5 had it available before the standard switched to shared_timed_mutex
try_compile(SHARED_MUTEX_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_shared_mutex.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})

try_compile(SHARED_TIMED_MUTEX_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_shared_timed_mutex.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})

try_compile(EXPERIMENTAL_STRING_VIEW_AVAILABLE ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_experimental_string_view.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})

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
set(HAVE_VARIABLE_TEMPLATES 1)
endif()

if (UNUSED_AVAILABLE)
set(HAVE_UNUSED 1)
endif()

if (SHARED_TIMED_MUTEX_AVAILABLE)
set(HAVE_SHARED_TIMED_MUTEX 1)
endif()

if (SHARED_MUTEX_AVAILABLE)
set(HAVE_SHARED_MUTEX 1)
endif()



if (NOT NO_CONFIG_GENERATION)
if (CONFIGURE_TARGET_LOCATION)
CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/compiler-config.h.in ${CONFIGURE_TARGET_LOCATION}/compiler-config.h)
else()
CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/compiler-config.h.in ${PROJECT_BINARY_DIR}/compiler-config.h)
endif()

endif(NOT NO_CONFIG_GENERATION)

