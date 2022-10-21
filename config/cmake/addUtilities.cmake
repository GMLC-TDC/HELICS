# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(NOT TARGET gmlc::utilities)
    set(GMLC_UTILITIES_INSTALL OFF CACHE INTERNAL "")

    add_subdirectory(ThirdParty/utilities)

    hide_variable(GMLC_UTILITIES_GENERATE_DOXYGEN_DOC)
    hide_variable(GMLC_UTILITIES_INCLUDE_BOOST)
    hide_variable(GMLC_UTILITIES_USE_BOOST_SPIRIT)
    hide_variable(GMLC_UTILITIES_WITH_CMAKE_PACKAGE)
    hide_variable(GMLC_UTILITIES_OBJECT_LIB)
    hide_variable(GMLC_UTILITIES_STATIC_LIB)
    hide_variable(GMLC_UTILITIES_CLANG_TIDY_OPTIONS)

    target_compile_definitions(gmlc_utilities PUBLIC USE_STD_STRING_VIEW=1)

    set_target_properties(gmlc_utilities PROPERTIES FOLDER Extern)
    if(NOT ${PROJECT_NAME}_DISABLE_BOOST)
        target_include_directories(
            gmlc_utilities SYSTEM PRIVATE $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>
        )
    endif()
endif()
