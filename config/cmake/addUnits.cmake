# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(NOT TARGET units::units)

    option(${PROJECT_NAME}_USE_EXTERNAL_UNITS "Use external copy of LLNL Units" OFF)
    mark_as_advanced(${PROJECT_NAME}_USE_EXTERNAL_UNITS)

    if(${PROJECT_NAME}_USE_EXTERNAL_UNITS)
        # NOTE: static units should be built with CMAKE_POSITION_INDEPENDENT_CODE
        find_package(units REQUIRED)
    else()
        # so units cpp exports to the correct target export
        set(UNITS_INSTALL OFF CACHE INTERNAL "")

        if(NOT CMAKE_CXX_STANDARD)
            set(CMAKE_CXX_STANDARD 17) # Supported values are ``11``, ``14``, and ``17``.
        endif()

        set(UNITS_BUILD_OBJECT_LIBRARY OFF CACHE INTERNAL "")
        set(UNITS_BUILD_STATIC_LIBRARY ON CACHE INTERNAL "")
        set(UNITS_BUILD_SHARED_LIBRARY OFF CACHE INTERNAL "")
        set(UNITS_BUILD_CONVERTER_APP OFF CACHE INTERNAL "")
        set(UNITS_BUILD_WEBSERVER OFF CACHE INTERNAL "")
        set(UNITS_CLANG_TIDY_OPTIONS "" CACHE INTERNAL "")
        set(UNITS_BUILD_FUZZ_TARGETS OFF CACHE INTERNAL "")

        add_subdirectory(
            "${PROJECT_SOURCE_DIR}/ThirdParty/units" "${PROJECT_BINARY_DIR}/ThirdParty/units"
        )

        set_target_properties(units PROPERTIES FOLDER Extern)

        hide_variable(UNITS_HEADER_ONLY)
        hide_variable(UNITS_BUILD_OBJECT_LIBRARY)
        hide_variable(UNITS_NAMESPACE)
    endif()
endif()
