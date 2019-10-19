# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# so units cpp exports to the correct target export
set(UNITS_LIBRARY_EXPORT_COMMAND ${HELICS_EXPORT_COMMAND} CACHE INTERNAL "")

set(UNITS_BINARY_ONLY_INSTALL ${HELICS_BINARY_ONLY_INSTALL} CACHE INTERNAL "")
if( DISABLE_STATIC_LIB_INSTALL)
    set(UNITS_BINARY_ONLY_INSTALL ON CACHE INTERNAL "")
endif()

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14) # Supported values are ``11``, ``14``, and ``17``.
endif()

if(CMAKE_INSTALL_INCLUDEDIR)
    set(OLD_CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
    set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/helics/external/optional)
endif()


set(BUILD_UNITS_OBJECT_LIBRARY OFF CACHE INTERNAL "")
set(BUILD_UNITS_STATIC_LIBRARY ON CACHE INTERNAL "")
set(BUILD_UNITS_SHARED_LIBRARY OFF CACHE INTERNAL "")

add_subdirectory("${HELICS_SOURCE_DIR}/ThirdParty/units"
                 "${PROJECT_BINARY_DIR}/ThirdParty/units")

if(OLD_CMAKE_INSTALL_INCLUDEDIR)
    set(CMAKE_INSTALL_INCLUDEDIR ${OLD_CMAKE_INSTALL_INCLUDEDIR})
endif()

set_target_properties(units-static PROPERTIES FOLDER Extern)
add_library(HELICS::units-static ALIAS units-static)


hide_variable(BUILD_UNITS_FUZZ_TARGETS)
hide_variable(UNITS_ENABLE_TESTS)
hide_variable(UNITS_HEADER_ONLY)
hide_variable(UNITS_WITH_CMAKE_PACKAGE)
