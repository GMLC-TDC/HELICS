# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
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
    ringMessageBenchmarks
    messageSendBenchmarks
    pholdBenchmarks
    timingBenchmarks
)

set(HELICS_MULTINODE_BENCHMARKS
    PholdFederate
    MessageExchangeFederate
    RingTransmitFederate
    EchoHubFederate
    EchoLeafFederate
    EchoMessageHubFederate
    EchoMessageLeafFederate
    TimingHubFederate
    TimingLeafFederate
)

# Only affects current directory, so safe
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach(T ${HELICS_BENCHMARKS})

    add_executable(${T} ${T}.cpp)
    target_link_libraries(${T} PUBLIC helics_application_api)
    add_benchmark(${T})
    set_target_properties(${T} PROPERTIES FOLDER benchmarks)

endforeach()

add_executable(helics_benchmarks BenchmarkMain.cpp)
target_link_libraries(helics_benchmarks PUBLIC helics_application_api)
set_target_properties(helics_benchmarks PROPERTIES FOLDER benchmarks_multimachine)
foreach(T ${HELICS_MULTINODE_BENCHMARKS})
    target_sources(helics_benchmarks PUBLIC ${T}.hpp)
endforeach()

if (NOT HELICS_DISABLE_C_SHARED_LIB)
   add_executable(echoBenchmarks_c echoBenchmarks_c.cpp)
   target_link_libraries(echoBenchmarks_c PUBLIC helicsSharedLib)
   add_benchmark(echoBenchmarks_c)
   set_target_properties(echoBenchmarks_c PROPERTIES FOLDER benchmarks)
   target_include_directories(
      echoBenchmarks_c
      PUBLIC ${HELICS_BINARY_DIR}/helics_generated_includes/
   )
   copy_shared_target(echoBenchmarks_c helicsSharedLib)
endif()

copy_key_files_to_target_location(ActionMessageBenchmarks)

string(TIMESTAMP current_date "%Y-%m-%d")
string(RANDOM rname)

set(BM_RESULT_DIR)
set(BM_FORMAT --benchmark_format=json)
if (NOT HELICS_DISABLE_C_SHARED_LIB)
    set(HELICS_ECHO_C_COMMANDS COMMAND echoBenchmarks_c ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_echo_cResults${current_date}_${rname}.txt")
endif()
# add a custom target to run all the benchmarks in a consistent fashion
add_custom_target(
    RUN_ALL_BENCHMARKS
    COMMAND ${CMAKE_COMMAND} -E echo " running ActionMessageBenchmarks"
    COMMAND ActionMessageBenchmarks ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_ActionMessageResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running conversionBenchmarks"
    COMMAND conversionBenchmarks ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_conversionResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running echoBenchmarks"
    COMMAND echoBenchmarks ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_echoResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running echoBenchmarks_c"
    ${HELICS_ECHO_C_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E echo " running ringBenchmarks"
    COMMAND ringBenchmarks ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_ringResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running ringMessageBenchmarks"
    COMMAND ringMessageBenchmarks ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_ringMessageResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running messageLookupBenchmarks"
    COMMAND messageLookupBenchmarks ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_messageLookupResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running echoMessageBenchmarks"
    COMMAND echoMessageBenchmarks ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_echoMessageResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running timingBenchmarks"
    COMMAND timingBenchmarks ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_timingResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running filterBenchmarks"
    COMMAND filterBenchmarks ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_filterResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running pHoldBenchmarks"
    COMMAND pholdBenchmarks ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_pholdResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running messageSendBenchmarks"
    COMMAND messageSendBenchmarks ${BM_FORMAT}
            ">${BM_RESULT_DIR}bm_messageSendResults${current_date}_${rname}.txt"
)

foreach(T ${HELICS_BENCHMARKS})
    add_dependencies(RUN_ALL_BENCHMARKS ${T})
endforeach()

if (NOT HELICS_DISABLE_C_SHARED_LIB)
    add_dependencies(RUN_ALL_BENCHMARKS echoBenchmarks_c)
endif()

set_target_properties(RUN_ALL_BENCHMARKS PROPERTIES FOLDER benchmarks)

add_custom_target(
    RUN_KEY_BENCHMARKS
    COMMAND ${CMAKE_COMMAND} -E echo " running conversionBenchmarks"
    COMMAND conversionBenchmarks --benchmark_format=json
            ">${BM_RESULT_DIR}bmk_conversionResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running echoBenchmarks with filter=/4"
    COMMAND echoBenchmarks --benchmark_format=json --benchmark_filter=/4
            ">${BM_RESULT_DIR}bmk_echoResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running timingoBenchmarks with filter=/4"
    COMMAND timingBenchmarks --benchmark_format=json --benchmark_filter=/4
            ">${BM_RESULT_DIR}bmk_timingResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo
            " running messageLookupBenchmarks with filter=/[0-9][0-9][0-9][0-9]/"
    COMMAND messageLookupBenchmarks --benchmark_format=json
            --benchmark_filter=/[0-9][0-9][0-9][0-9]/
            ">${BM_RESULT_DIR}bmk_messageLookupResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running echoMessageBenchmarks with filter=/4"
    COMMAND echoMessageBenchmarks --benchmark_format=json --benchmark_filter=/4
            ">${BM_RESULT_DIR}bmk_echoMessageResults${current_date}_${rname}.txt"
)

foreach(T ${HELICS_BENCHMARKS})
    add_dependencies(RUN_KEY_BENCHMARKS ${T})
endforeach()

set_target_properties(RUN_KEY_BENCHMARKS PROPERTIES FOLDER benchmarks)
