# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

add_executable(interopFed1 InterOpFed1.cpp)

target_link_libraries(interopFed1 PUBLIC helics_application_api)
target_link_libraries(interopFed1 PRIVATE compile_flags_target)
set_target_properties(interopFed1 PROPERTIES FOLDER examples)

add_executable(interopFed2 InterOpFed2.cpp)

target_link_libraries(interopFed2 PUBLIC helics_application_api)
target_link_libraries(interopFed2 PRIVATE compile_flags_target)
set_target_properties(interopFed2 PROPERTIES FOLDER examples)
