# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
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
    wattsStrogatzBenchmarks
)

set(HELICS_MULTINODE_BENCHMARKS
    PholdFederate
    MessageExchangeFederate
    RingTransmitFederate
    RingTransmitMessageFederate
    EchoHubFederate
    EchoLeafFederate
    EchoMessageHubFederate
    EchoMessageLeafFederate
    TimingHubFederate
    TimingLeafFederate
    WattsStrogatzFederate
)

if(NOT DEFINED HELICS_BENCHMARK_SHIFT_FACTOR)
    include(ProcessorCount)
    processorcount(N)
    if(N LESS 5)
        set(HELICS_BENCHMARK_SHIFT_FACTOR -1)
    else()
        set(HELICS_BENCHMARK_SHIFT_FACTOR 0)
    endif()
endif()

if(NOT HELICS_BENCHMARK_SHIFT_FACTOR GREATER -4)
    message(FATAL_ERROR "defined shift factor must be a number and greater than -4")
endif()

# Only affects current directory, so safe
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach(T ${HELICS_BENCHMARKS})

    add_executable(${T} ${T}.cpp helics_benchmark_main.h helics_benchmark_util.h)
    target_link_libraries(${T} PUBLIC helics_application_api)
    add_benchmark(${T})
    set_target_properties(${T} PROPERTIES FOLDER benchmarks)
    target_compile_definitions(
        ${T} PRIVATE "HELICS_BENCHMARK_SHIFT_FACTOR=(${HELICS_BENCHMARK_SHIFT_FACTOR})"
    )
    install(TARGETS ${T} ${HELICS_EXPORT_COMMAND} DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT benchmarks
    )
endforeach()

add_executable(helics_benchmarks BenchmarkMain.cpp BenchmarkFederate.hpp)
target_link_libraries(helics_benchmarks PUBLIC helics_application_api)
set_target_properties(helics_benchmarks PROPERTIES FOLDER benchmarks_multimachine)
foreach(T ${HELICS_MULTINODE_BENCHMARKS})
    target_sources(helics_benchmarks PRIVATE ${T}.hpp)
endforeach()
install(TARGETS helics_benchmarks ${HELICS_EXPORT_COMMAND} DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT benchmarks
)

if(NOT HELICS_DISABLE_C_SHARED_LIB)
    add_executable(echoBenchmarks_c echoBenchmarks_c.cpp)
    target_link_libraries(echoBenchmarks_c PUBLIC helicsSharedLib)
    add_benchmark(echoBenchmarks_c)
    set_target_properties(echoBenchmarks_c PROPERTIES FOLDER benchmarks)
    target_compile_definitions(
        echoBenchmarks_c PRIVATE "HELICS_BENCHMARK_SHIFT_FACTOR=(${HELICS_BENCHMARK_SHIFT_FACTOR})"
    )
    target_include_directories(
        echoBenchmarks_c PRIVATE ${HELICS_BINARY_DIR}/helics_generated_includes/
    )
    install(TARGETS echoBenchmarks_c ${HELICS_EXPORT_COMMAND} DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT benchmarks
    )
endif()

string(TIMESTAMP current_date "%Y-%m-%d")
string(RANDOM rname)

set(BM_RESULT_DIR)
set(BM_FORMAT --benchmark_format=json)
if(NOT HELICS_DISABLE_C_SHARED_LIB)
    set(HELICS_ECHO_C_COMMANDS COMMAND echoBenchmarks_c ${BM_FORMAT}
                               ">${BM_RESULT_DIR}bm_echo_cResults${current_date}_${rname}.txt"
    )
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
    COMMAND ${CMAKE_COMMAND} -E echo " running echoBenchmarks_c" ${HELICS_ECHO_C_COMMANDS}
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

if(NOT HELICS_DISABLE_C_SHARED_LIB)
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
    COMMAND
        messageLookupBenchmarks --benchmark_format=json --benchmark_filter=/[0-9][0-9][0-9][0-9]/
        ">${BM_RESULT_DIR}bmk_messageLookupResults${current_date}_${rname}.txt"
    COMMAND ${CMAKE_COMMAND} -E echo " running echoMessageBenchmarks with filter=/4"
    COMMAND echoMessageBenchmarks --benchmark_format=json --benchmark_filter=/4
            ">${BM_RESULT_DIR}bmk_echoMessageResults${current_date}_${rname}.txt"
)

foreach(T ${HELICS_BENCHMARKS})
    add_dependencies(RUN_KEY_BENCHMARKS ${T})
endforeach()

set_target_properties(RUN_KEY_BENCHMARKS PROPERTIES FOLDER benchmarks)
