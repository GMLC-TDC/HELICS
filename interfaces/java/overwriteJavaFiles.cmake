# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

configure_file(helicsJava.c ${TARGET_DIR}/helicsJava.c COPYONLY)

 FILE(GLOB JAVA_FILES *.java)

 FILE(COPY ${JAVA_FILES} DESTINATION ${TARGET_DIR})

 message(STATUS "overwriting java files in source directory")
