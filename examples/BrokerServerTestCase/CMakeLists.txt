# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

add_executable(brokerServerTestCase brokerServerTest.cpp)

target_link_libraries(brokerServerTestCase PUBLIC HELICS::apps)

set_target_properties(brokerServerTestCase PROPERTIES FOLDER examples)

if(HELICS_BUILD_TESTS AND NOT MSVC)
   # add_test(
    #    NAME brokerServerTestCase
    #    COMMAND
    #        ${PROJECT_SOURCE_DIR}/scripts/test_example.sh 10
    #        $<TARGET_FILE:nonlings_fed1_cpp> $<TARGET_FILE:nonlings_fed2_cpp>
    #)
    #set_property(TEST nonlings_example PROPERTY LABELS Continuous)
endif()

copy_key_files_to_target_location(brokerServerTestCase)
