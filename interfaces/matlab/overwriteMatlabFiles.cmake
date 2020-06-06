# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

configure_file(helicsMEX.cpp ${TARGET_DIR}/helicsMEX.cpp COPYONLY)

 FILE(GLOB MATLAB_FILES *.m)
 message(STATUS ${MATLAB_FILES})
 list(REMOVE_ITEM MATLAB_FILES mkhelicsMEXFile.m generatehelicsMEXFile.m)
 FILE(COPY ${MATLAB_FILES} DESTINATION ${TARGET_DIR})

 # remove all the existing files
 FILE(REMOVE_RECURSE ${TARGET_DIR}/+helics)
 #copy the new ones
 FILE(COPY +helics DESTINATION ${TARGET_DIR})

 message(STATUS "overwriting matlab files in source directory")
