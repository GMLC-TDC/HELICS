# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# so units cpp exports to the correct target export
set(UNITS_INSTALL OFF CACHE INTERNAL "")

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14) # Supported values are ``11``, ``14``, and ``17``.
endif()

set(BUILD_UNITS_OBJECT_LIBRARY OFF CACHE INTERNAL "")
set(BUILD_UNITS_STATIC_LIBRARY ON CACHE INTERNAL "")
set(BUILD_UNITS_SHARED_LIBRARY OFF CACHE INTERNAL "")

add_subdirectory("${HELICS_SOURCE_DIR}/ThirdParty/units"
                 "${PROJECT_BINARY_DIR}/ThirdParty/units")

set_target_properties(units-static PROPERTIES FOLDER Extern)
add_library(HELICS::units ALIAS units-static)


hide_variable(BUILD_UNITS_FUZZ_TARGETS)
hide_variable(UNITS_ENABLE_TESTS)
hide_variable(UNITS_HEADER_ONLY)
hide_variable(UNITS_WITH_CMAKE_PACKAGE)
