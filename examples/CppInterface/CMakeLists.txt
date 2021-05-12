# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

add_executable(nonlings_fed1_cpp nonlings_fed1.cpp)
add_executable(nonlings_fed2_cpp nonlings_fed2.cpp)

target_link_libraries(nonlings_fed1_cpp PUBLIC helicsCpp98)
target_link_libraries(nonlings_fed2_cpp PUBLIC helicsCpp98)
target_link_libraries(nonlings_fed1_cpp PRIVATE compile_flags_target)
target_link_libraries(nonlings_fed2_cpp PRIVATE compile_flags_target)

set_property(TARGET nonlings_fed1_cpp PROPERTY CXX_STANDARD 98)
set_property(TARGET nonlings_fed2_cpp PROPERTY CXX_STANDARD 98)

if(!MSVC)
    target_compile_options(nonlings_fed1_cpp PRIVATE -pedantic -Werror)
    target_compile_options(nonlings_fed2_cpp PRIVATE -pedantic -Werror)
endif()

set_target_properties(nonlings_fed1_cpp nonlings_fed2_cpp PROPERTIES FOLDER examples)

if(HELICS_BUILD_TESTS AND NOT MSVC)
    add_test(NAME nonlings_example
             COMMAND ${PROJECT_SOURCE_DIR}/scripts/test_example.sh 10
                     $<TARGET_FILE:nonlings_fed1_cpp> $<TARGET_FILE:nonlings_fed2_cpp>
    )
    set_property(TEST nonlings_example PROPERTY LABELS Continuous)
endif()
