# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# addLoadLibraryCommand.cmake
#adds a load library function to the helicsJava.java

if (NOT LIBRARY_FILE)
set(LIBRARY_FILE helicsJava)
else()
get_filename_component(LIBRARY_FILE ${LIBRARY_FILE} NAME_WE)
STRING(REGEX REPLACE "^lib" "" LIBRARY_FILE ${LIBRARY_FILE})
endif()
file(READ helicsJavaJNI.java HELICS_JNI_SOURCE)
string(FIND "${HELICS_JNI_SOURCE}" "System.loadLibrary" ALREADY_LOADED)
if (${ALREADY_LOADED} LESS 0)
string(REPLACE "public class helicsJavaJNI {"
       "public class helicsJavaJNI {\n  static {\n    System.loadLibrary\(\"${LIBRARY_FILE}\"\);\n  }" HELICS_JNI_SOURCE
       "${HELICS_JNI_SOURCE}")
   
file(WRITE helicsJavaJNI.java "${HELICS_JNI_SOURCE}")
endif()