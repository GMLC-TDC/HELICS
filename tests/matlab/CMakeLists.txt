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
    add_test(NAME matlab_tests COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nosplash -minimize -r
                                       "\"run('${CMAKE_CURRENT_BINARY_DIR}/runTests.m');quit;\""
    )
elseif(APPLE)
    configure_file(runTestsUNIX.m.in runTests.m)
    add_test(NAME matlab_tests COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nodisplay -nosplash -r
                                       "\"run('${CMAKE_CURRENT_BINARY_DIR}/runTests.m');quit;\""
    )
else()
    configure_file(runTestsUNIX.m.in runTests.m)
    add_test(NAME matlab_tests COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nodisplay -nosplash -r
                                       "\"run('${CMAKE_CURRENT_BINARY_DIR}/runTests.m');quit;\""
    )
endif()

set_property(TEST matlab_tests PROPERTY LABELS Matlab Continuous Daily)
