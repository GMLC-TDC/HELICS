# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(JSONCPP_WITH_TESTS OFF CACHE INTERNAL "")
set(JSONCPP_WITH_CMAKE_PACKAGE OFF CACHE INTERNAL "")
set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE INTERNAL "")
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE INTERNAL "")
set(DEBUG_LIBNAME_SUFFIX ${CMAKE_DEBUG_POSTFIX} CACHE INTERNAL "")
# so json cpp exports to the correct target export

set(INSTALL_EXPORT ${HELICS_EXPORT_COMMAND} CACHE INTERNAL "")
set(JSONCPP_BINARY_ONLY_INSTALL ${HELICS_BINARY_ONLY_INSTALL} CACHE INTERNAL "")
if(DISABLE_STATIC_LIB_INSTALL AND NOT HELICS_BUILD_CXX_SHARED_LIB)
    set(JSONCPP_BINARY_ONLY_INSTALL ON CACHE INTERNAL "")
endif()

set(JSONCPP_DISABLE_CCACHE ON CACHE INTERNAL "")

cmake_conditional_option(JSONCPP_OBJLIB
           "use jsoncpp objlib for linking object files instead of the normal target"
           "NOT MSVC")

mark_as_advanced(JSONCPP_OBJLIB)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14) # Supported values are ``11``, ``14``, and ``17``.
endif()

if(BUILD_SHARED_LIBS)
    set(OLD_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF)
endif()

if(CMAKE_INSTALL_INCLUDEDIR)
    set(OLD_CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
    set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/helics/external/optional)
endif()

# these are internal variables used in JSONCPP that we know to be true based on the
# requirements in HELICS for newer compilers than JSONCPP supports
set(HAVE_CLOCALE ON)
set(HAVE_LOCALECONV ON)
set(COMPILER_HAS_DEPRECATED ON)
set(HAVE_STDINT_H ON)
set(HAVE_DECIMAL_POINT ON)
add_subdirectory("${HELICS_SOURCE_DIR}/ThirdParty/jsoncpp"
                 "${PROJECT_BINARY_DIR}/ThirdParty/jsoncpp")

if(NOT JSONCPP_OBJLIB)
    add_library(HELICS::jsoncpp_lib ALIAS jsoncpp_lib)
endif()

if(OLD_CMAKE_INSTALL_INCLUDEDIR)
    set(CMAKE_INSTALL_INCLUDEDIR ${OLD_CMAKE_INSTALL_INCLUDEDIR})
endif()

if(OLD_BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ${OLD_BUILD_SHARED_LIBS})
endif()

hide_variable(JSONCPP_WITH_STRICT_ISO)
hide_variable(JSONCPP_WITH_WARNING_AS_ERROR)
hide_variable(DEBUG_LIBNAME_SUFFIX)
hide_variable(JSONCPP_USE_SECURE_MEMORY)

# don't want build_shared_libs to show up since it interacts oddly with the submodules
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    hide_variable(BUILD_SHARED_LIBS)
endif()
