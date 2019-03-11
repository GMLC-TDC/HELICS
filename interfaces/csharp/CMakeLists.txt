#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details. 
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#


if(ENABLE_SWIG AND SWIG_EXECUTABLE)
   # Enable generation using swig

  set_property(SOURCE helicsCsharp.i PROPERTY SWIG_MODULE_NAME csharp)
  #set_source_files_properties(helicsCsharp.i PROPERTIES SWIG_FLAGS "-outfile;helics.cs")
  set(CMAKE_SWIG_FLAGS "-outfile;helics.cs;-namespace;gmlc")

  swig_add_library(CShelics TYPE MODULE LANGUAGE csharp SOURCES helicsCsharp.i)

  swig_link_libraries(CShelics helicsSharedLib)

  set_target_properties(CShelics PROPERTIES FOLDER interfaces)

else()
 file(
    COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/
    DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long")
  endif()
  #add_library(JNIhelics SHARED interface/helicsJavaJAVA_wrap.c)
  #target_link_libraries(JNIhelics helicsSharedLib ${JAVA_LIBRARIES})
  #set_target_properties(JNIhelics PROPERTIES FOLDER interfaces)
endif() # ENABLE_SWIG

# Set the output library name to _helics
set_target_properties(CShelics PROPERTIES DEBUG_POSTFIX "")

if(WIN32)
  foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
    add_custom_command(
      TARGET
      CShelics
      POST_BUILD # Adds a post-build event to api tests
      COMMAND
        ${CMAKE_COMMAND}
        -E
        copy_if_different # which executes "cmake - E copy_if_different..."
        "${keyfile}" # <--this is in-file
        "$<TARGET_FILE_DIR:CShelics>/"
    ) # <--this is out-file path
  endforeach(keyfile)

  add_custom_command(
    TARGET
    CShelics
    POST_BUILD # Adds a post-build event to api tests
    COMMAND
      ${CMAKE_COMMAND}
      -E
      copy_if_different # which executes "cmake - E copy_if_different..."
      "$<TARGET_FILE:helicsSharedLib>" # <--this is in- file
      "$<TARGET_FILE_DIR:CShelics>/"
  ) # <--this is out- file path

endif(WIN32)

install(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION csharp COMPONENT csharp)
install(TARGETS CShelics DESTINATION csharp COMPONENT csharp)
install(FILES ${KEY_LIBRARY_FILES} DESTINATION csharp COMPONENT csharp)
