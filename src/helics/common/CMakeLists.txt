# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(common_headers
    JsonProcessingFunctions.hpp
    JsonBuilder.hpp
    TomlProcessingFunctions.hpp
    GuardedTypes.hpp
    fmt_format.h
    fmt_ostream.h
    addTargets.hpp
    configFileHelpers.hpp
)

set(common_sources JsonProcessingFunctions.cpp JsonBuilder.cpp TomlProcessingFunctions.cpp
                   configFileHelpers.cpp addTargets.cpp
)

# headers that are part of the public interface
set(helics_public_common JsonProcessingFunctions.hpp AsioContextManager.h GuardedTypes.hpp)

if(NOT HELICS_DISABLE_ASIO)
    list(APPEND common_headers AsioContextManager.h)
    list(APPEND common_sources AsioContextManager.cpp)
endif()

add_library(helics_common STATIC ${common_sources} ${common_headers})
set_target_properties(helics_common PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(
    helics_common PUBLIC helics_base build_flags_target PRIVATE compile_flags_target
)
target_link_libraries(helics_common PUBLIC HELICS::jsoncpp_lib HELICS::utilities HELICS::units)

if(TARGET Boost::boost AND NOT HELICS_DISABLE_BOOST)
    target_link_libraries(helics_common PRIVATE Boost::boost)
endif()

if(APPLE)
    target_compile_definitions(helics_common PUBLIC "-DSTX_NO_STD_ANY=1")
endif(APPLE)

add_library(HELICS::common ALIAS helics_common)
