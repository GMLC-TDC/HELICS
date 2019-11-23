# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(HELICS_BENCHMARKS
    ActionMessageBenchmarks
    filterBenchmarks
    echoBenchmarks
    ringBenchmarks
    messageLookupBenchmarks
    conversionBenchmarks
    echoMessageBenchmarks
    messageSendBenchmarks
    pholdBenchmarks
)

# Only affects current directory, so safe
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach(T ${HELICS_BENCHMARKS})

    add_executable(${T} ${T}.cpp)
    target_link_libraries(${T} PUBLIC helics_application_api)
    add_benchmark(${T})
    set_target_properties(${T} PROPERTIES FOLDER benchmarks)

endforeach()

target_sources(filterBenchmarks PUBLIC EchoMessage.hpp)
target_sources(echoMessageBenchmarks PUBLIC EchoMessage.hpp)
target_sources(pholdBenchmarks PUBLIC PholdFederate.hpp)

copy_key_files_to_target_location(ActionMessageBenchmarks)
