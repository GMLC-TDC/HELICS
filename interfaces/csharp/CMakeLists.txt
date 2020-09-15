# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(SWIG_EXECUTABLE)
    # Enable generation using swig

    set_property(SOURCE helicsCsharp.i PROPERTY SWIG_MODULE_NAME helics)
    # set_source_files_properties(helicsCsharp.i PROPERTIES SWIG_FLAGS "-outfile;helics.cs")
    set(CMAKE_SWIG_FLAGS "-outfile;helics.cs;-namespace;gmlc")

    swig_add_library(
        CShelics
        TYPE MODULE
        LANGUAGE csharp
        SOURCES helicsCsharp.i
    )

    swig_link_libraries(CShelics helics)

    set_target_properties(${SWIG_MODULE_CShelics_REAL_NAME} PROPERTIES FOLDER interfaces)

else()
    message(SEND_ERROR "C# interface build requires swig")
endif() # SWIG_EXECUTABLE

# Set the output library name to _helics
set_target_properties(${SWIG_MODULE_CShelics_REAL_NAME} PROPERTIES DEBUG_POSTFIX "")

install_key_files_with_comp(csharp)

install(TARGETS ${SWIG_MODULE_CShelics_REAL_NAME} DESTINATION csharp COMPONENT csharp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.cs DESTINATION csharp COMPONENT csharp)
