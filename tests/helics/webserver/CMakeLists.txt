# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

add_executable(
    helics_webserver-tests helics_webserver_tests.cpp webServerHttpTests.cpp
                           webServerWebSocketTests.cpp
)

set_target_properties(helics_webserver-tests PROPERTIES FOLDER tests)

target_link_libraries(helics_webserver-tests helics_apps helics_test_base)

target_include_directories(helics_webserver-tests PRIVATE ${PROJECT_SOURCE_DIR}/src)

add_test(NAME helics_webserver-tests COMMAND helics_webserver-tests)

set_property(TEST helics_webserver-tests PROPERTY LABELS webserver Continuous Coverage)
