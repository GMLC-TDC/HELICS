# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Set a default build type if none was specified
set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(
        STATUS "Setting build type to '${default_build_type}' as none was specified."
    )
    set(
        CMAKE_BUILD_TYPE
        "${default_build_type}"
        CACHE STRING "Choose the type of build." FORCE
    )
    # Set the possible values of build type for cmake-gui
    set_property(
        CACHE CMAKE_BUILD_TYPE
        PROPERTY
            STRINGS
            "Debug"
            "Release"
            "MinSizeRel"
            "RelWithDebInfo"
    )
endif()
