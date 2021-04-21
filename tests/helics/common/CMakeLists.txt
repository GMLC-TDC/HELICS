# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# -----------------------------------------------------------------------------
# Common library tests using Boost
# -----------------------------------------------------------------------------

set(common_test_headers)

set(common_test_sources TimeTests.cpp)

add_executable(common-tests ${common_test_sources} ${common_test_headers})
target_link_libraries(common-tests PRIVATE helics_core helics_test_base)
set_target_properties(common-tests PROPERTIES FOLDER tests)

add_test(NAME common-tests COMMAND common-tests)
set_property(TEST common-tests PROPERTY LABELS Common Daily Coverage Continuous)
