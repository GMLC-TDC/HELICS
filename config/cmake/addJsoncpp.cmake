# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

option(${PROJECT_NAME}_USE_EXTERNAL_JSONCPP "Use external copy of JsonCpp" OFF)
mark_as_advanced(${PROJECT_NAME}_USE_EXTERNAL_JSONCPP)

if(${PROJECT_NAME}_USE_EXTERNAL_JSONCPP)
    # NOTE: static jsoncpp should be built with CMAKE_POSITION_INDEPENDENT_CODE
    find_package(jsoncpp REQUIRED)
else()
    set(JSONCPP_WITH_TESTS OFF CACHE INTERNAL "")
    set(JSONCPP_WITH_CMAKE_PACKAGE OFF CACHE INTERNAL "")
    set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE INTERNAL "")
    set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE INTERNAL "")
    set(JSONCPP_WITH_WARNING_AS_ERROR OFF CACHE INTERNAL "")
    set(DEBUG_LIBNAME_SUFFIX ${CMAKE_DEBUG_POSTFIX} CACHE INTERNAL "")

    if(MSVC)
        if(STATIC_STANDARD_LIB STREQUAL "default")
            set(JSONCPP_STATIC_WINDOWS_RUNTIME OFF CACHE INTERNAL "")
        elseif(STATIC_STANDARD_LIB STREQUAL "static")
            set(JSONCPP_STATIC_WINDOWS_RUNTIME OFF CACHE INTERNAL "")
        elseif(STATIC_STANDARD_LIB STREQUAL "dynamic")
            set(JSONCPP_STATIC_WINDOWS_RUNTIME OFF CACHE INTERNAL "")
        endif()
    endif()

    # so json cpp exports to the correct target export
    set(INSTALL_EXPORT "" CACHE INTERNAL "")

    set(JSONCPP_DISABLE_CCACHE ON CACHE INTERNAL "")

    if(NOT CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 17) # Supported values are ``17``, ``20``, and ``23``.
    endif()

    if(BUILD_SHARED_LIBS)
        set(OLD_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
        set(BUILD_SHARED_LIBS OFF)
    endif()

    if(BUILD_STATIC_LIBS)
        set(OLD_BUILD_STATIC_LIBS ${BUILD_STATIC_LIBS})
        set(BUILD_STATIC_LIBS ON)
    endif()

    if(BUILD_OBJECT_LIBS)
        set(OLD_BUILD_OBJECT_LIBS ${BUILD_OBJECT_LIBS})
        set(BUILD_OBJECT_LIBS OFF)
    endif()

    # these are internal variables used in JSONCPP that we know to be true based on the
    # requirements in HELICS for newer compilers than JSONCPP supports
    set(HAVE_CLOCALE ON)
    set(HAVE_LOCALECONV ON)
    set(COMPILER_HAS_DEPRECATED ON)
    set(HAVE_STDINT_H ON)
    set(HAVE_DECIMAL_POINT ON)
    add_subdirectory("${PROJECT_SOURCE_DIR}/ThirdParty/json_cpp"
                     "${PROJECT_BINARY_DIR}/ThirdParty/json_cpp" EXCLUDE_FROM_ALL)


    set_target_properties(jsoncpp_static PROPERTIES FOLDER Extern)

    if(OLD_BUILD_SHARED_LIBS)
        set(BUILD_SHARED_LIBS ${OLD_BUILD_SHARED_LIBS})
    endif()

    if(OLD_BUILD_STATIC_LIBS)
        set(BUILD_STATIC_LIBS ${OLD_BUILD_STATIC_LIBS})
    endif()

    if(OLD_BUILD_OBJECT_LIBS)
        set(BUILD_OBJECT_LIBS ${OLD_BUILD_OBJECT_LIBS})
    endif()

    hide_variable(JSONCPP_WITH_STRICT_ISO)
    hide_variable(DEBUG_LIBNAME_SUFFIX)
    hide_variable(JSONCPP_USE_SECURE_MEMORY)
    hide_variable(JSONCPP_WITH_EXAMPLE)
endif()
