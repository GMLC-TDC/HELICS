#
# Copyright © 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. 
# 
# SPDX-License-Identifier: BSD-3-Clause
#

if(NOT ENABLE_SWIG OR NOT SWIG_EXECUTABLE)

  message(ERROR "Python 2 build requires swig")

endif()

# https://stackoverflow.com/a/3818084/5451769
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(
    CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -Wno-long-long -Wno-overlength-strings -Wno-ignored-attributes"
  )
endif()

find_package(PythonLibs 2 REQUIRED)

include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${PYTHON_INCLUDE_DIRS})

set(CMAKE_SWIG_FLAGS "")

set_property(SOURCE ../helics.i PROPERTY SWIG_MODULE_NAME helics)

if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.7)
  swig_add_library(helics TYPE MODULE LANGUAGE python SOURCES helicsPython2.i)
else()
  swig_add_module(helics python helicsPython2.i)
endif()

swig_link_libraries(helics helicsSharedLib)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # https://groups.google.com/a/continuum.io/d/msg/anaconda/057P4uNWyCU/Iem6OtjB CQAJ
  set_target_properties(_helics PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
else()
  swig_link_libraries(helics ${PYTHON_LIBRARIES})
endif()

set_target_properties(_helics PROPERTIES FOLDER interfaces)

install(TARGETS _helics DESTINATION python COMPONENT python)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
  add_custom_command(
    TARGET
    _helics
    POST_BUILD # Adds a post-build event to api tests
    COMMAND
      ${CMAKE_COMMAND}
      -E
      copy_if_different # which executes "cmake - E copy_if_different..."
      "${keyfile}" # <--this is in-file
      "$<TARGET_FILE_DIR:_helics>/"
  ) # <--this is out- file path
endforeach(keyfile)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.py DESTINATION python COMPONENT python)
install(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION python COMPONENT python)
install(FILES ${KEY_LIBRARY_FILES} DESTINATION python COMPONENT python)
