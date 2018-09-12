#
# Copyright (c) 2017-2018, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. See LICENSE
# file and DISCLAIMER for more details.
#


if(DISABLE_SWIG OR NOT SWIG_EXECUTABLE)
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
else(DISABLE_SWIG)
  # Enable generation using swig

  set_property(SOURCE helicsCsharp.i PROPERTY SWIG_MODULE_NAME csharp)

  set(CMAKE_SWIG_FLAGS "-namespace gmlc.helics")
  swig_add_library(CShelics TYPE MODULE LANGUAGE csharp SOURCES helicsCsharp.i)

  swig_link_libraries(CShelics helicsSharedLib)

  set_target_properties(CShelics PROPERTIES FOLDER interfaces)

endif() # DISABLE_SWIG

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

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/buildjar/helics-${HELICS_VERSION}.jar
  DESTINATION csharp
  COMPONENT csharp
)

install(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION csharp COMPONENT csharp)
install(TARGETS JNIhelics DESTINATION csharp COMPONENT csharp)
install(FILES ${KEY_LIBRARY_FILES} DESTINATION csharp COMPONENT csharp)
