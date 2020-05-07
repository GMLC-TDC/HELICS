# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(HELICS_EXAMPLES_WARNINGS_AS_ERROR)
    # the examples had better compile without warnings otherwise they should not be
    add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/WX>> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>>)

endif()

if(HELICS_BUILD_APP_LIBRARY)
    add_subdirectory(comboFederates1)
    add_subdirectory(BrokerServerTestCase)
endif()

if(NOT HELICS_DISABLE_C_SHARED_LIB)
    add_subdirectory(CppInterface)
endif()

if(HELICS_BUILD_CXX_SHARED_LIB)
    add_subdirectory(comboFederates_cpp_shared)
endif()
