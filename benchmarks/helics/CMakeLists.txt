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
    timingBenchmarks
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

string(TIMESTAMP current_date "%Y-%m-%d") 
string(RANDOM rname)



set(BM_RESULT_DIR )
# add a custom target to run all the benchmarks in a consistent fashion
add_custom_target(RUN_ALL_BENCHMARKS  
                  COMMAND ActionMessageBenchmarks --benchmark_format=json ">${BM_RESULT_DIR}bm_ActionMessageResults${current_date}_${rname}.txt"
				  COMMAND conversionBenchmarks --benchmark_format=json ">${BM_RESULT_DIR}bm_conversionResults${current_date}_${rname}.txt"
				  COMMAND echoBenchmarks --benchmark_format=json ">${BM_RESULT_DIR}bm_echoResults${current_date}_${rname}.txt"
				  COMMAND ringBenchmarks --benchmark_format=json ">${BM_RESULT_DIR}bm_ringResults${current_date}_${rname}.txt"
				  COMMAND messageLookupBenchmarks --benchmark_format=json ">${BM_RESULT_DIR}bm_messageLookupResults${current_date}_${rname}.txt"
				  COMMAND echoMessageBenchmarks --benchmark_format=json ">${BM_RESULT_DIR}bm_echoMessageResults${current_date}_${rname}.txt"
                  COMMAND timingBenchmarks --benchmark_format=json ">${BM_RESULT_DIR}bm_timingResults${current_date}_${rname}.txt"
				  COMMAND filterBenchmarks --benchmark_format=json ">${BM_RESULT_DIR}bm_filterResults${current_date}_${rname}.txt"
				  COMMAND pholdBenchmarks --benchmark_format=json ">${BM_RESULT_DIR}bm_pholdResults${current_date}_${rname}.txt"
				  COMMAND messageSendBenchmarks --benchmark_format=json ">${BM_RESULT_DIR}bm_messageSendResults${current_date}_${rname}.txt"
				  ) 

foreach(T ${HELICS_BENCHMARKS})
	add_dependencies(RUN_ALL_BENCHMARKS ${T})
endforeach()

set_target_properties(RUN_ALL_BENCHMARKS PROPERTIES FOLDER benchmarks)

add_custom_target(RUN_KEY_BENCHMARKS  
                  COMMAND ${CMAKE_COMMAND} -E echo " running conversionBenchmarks"  
                  COMMAND conversionBenchmarks --benchmark_format=json ">${BM_RESULT_DIR}bmk_conversionResults${current_date}_${rname}.txt"
                  COMMAND ${CMAKE_COMMAND} -E echo " running echoBenchmarks with filter=/4"
				  COMMAND echoBenchmarks --benchmark_format=json --benchmark_filter=/4 ">${BM_RESULT_DIR}bmk_echoResults${current_date}_${rname}.txt"
                  COMMAND timingBenchmarks --benchmark_format=json --benchmark_filter=/4 ">${BM_RESULT_DIR}bmk_timingResults${current_date}_${rname}.txt"
				  COMMAND ${CMAKE_COMMAND} -E echo " running messageLookupBenchmarks with filter=/[0-9][0-9][0-9][0-9]/"
                  COMMAND messageLookupBenchmarks --benchmark_format=json --benchmark_filter=/[0-9][0-9][0-9][0-9]/ ">${BM_RESULT_DIR}bmk_messageLookupResults${current_date}_${rname}.txt"
                  COMMAND ${CMAKE_COMMAND} -E echo " running echoMessageBenchmarks with filter=/4"
				  COMMAND echoMessageBenchmarks --benchmark_format=json --benchmark_filter=/4 ">${BM_RESULT_DIR}bmk_echoMessageResults${current_date}_${rname}.txt"
				  )

foreach(T ${HELICS_BENCHMARKS})
	add_dependencies(RUN_KEY_BENCHMARKS ${T})
endforeach()

set_target_properties(RUN_KEY_BENCHMARKS PROPERTIES FOLDER benchmarks)
