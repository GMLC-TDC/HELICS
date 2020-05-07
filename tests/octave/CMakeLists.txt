# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(WIN32)
    configure_file(runTestsWIN32.m.in runTests.m)
    add_test(NAME octave_tests COMMAND ${OCTAVE_EXECUTABLE} "-q"
                                       "run(\"${CMAKE_CURRENT_BINARY_DIR}/runTests.m\")"
    )

else()
    configure_file(runTestsUNIX.m.in runTests.m)
    add_test(NAME octave_tests COMMAND ${OCTAVE_EXECUTABLE} "-q"
                                       "run(\"${CMAKE_CURRENT_BINARY_DIR}/runTests.m\")"
    )
endif()

set_property(TEST octave_tests PROPERTY LABELS Octave Continuous Daily)
