# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

configure_file(helicsPython.c ${TARGET_DIR}/helicsPython.c COPYONLY)

 FILE(GLOB PYTHONI_FILES *.py)

 FILE(COPY ${PYTHONI_FILES} DESTINATION ${TARGET_DIR})

 message(STATUS "overwriting python interface files to source directory")
