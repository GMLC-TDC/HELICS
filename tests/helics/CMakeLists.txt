##############################################################################
#Copyright © 2017-2018,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################


#add a baseline library for underlying dependencies and flags for test executables
add_library(helics_test_base INTERFACE)
target_link_libraries(helics_test_base INTERFACE Boostlibs::test)
target_link_libraries(helics_test_base INTERFACE helics_base_includes)

add_subdirectory(common)

add_subdirectory (core)
add_subdirectory(application_api)
add_subdirectory(system_tests)
if (BUILD_C_SHARED_LIB OR INTERFACE_BUILD)
    add_subdirectory(shared_library)
endif()

add_subdirectory(apps)

