#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details. 
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(BUILD_PYTHON_INTERFACE)
  set(Python_ADDITIONAL_VERSIONS 3.4 3.5 3.6 3.7 3.8)
  find_package(PythonInterp 3)
  find_package(PythonLibs 3 REQUIRED)
endif(BUILD_PYTHON_INTERFACE)

if(ENABLE_SWIG AND SWIG_EXECUTABLE)

  # https://stackoverflow.com/a/3818084/5451769
  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(
      CMAKE_C_FLAGS
      "${CMAKE_C_FLAGS} -Wno-long-long -Wno-overlength-strings -Wno-ignored-attributes"
    )
  endif()

  include_directories(${PYTHON_INCLUDE_PATH})
  include_directories(${PYTHON_INCLUDE_DIRS})

  set(CMAKE_SWIG_FLAGS "-py3")

  set_property(SOURCE ../helics.i PROPERTY SWIG_MODULE_NAME helics)

  if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.7)
    swig_add_library(helics TYPE MODULE LANGUAGE python SOURCES helicsPython.i)
  else()
    swig_add_module(helics python helicsPython.i)
  endif()

  swig_link_libraries(helics helicsSharedLib)
  if (MSVC)
	target_compile_options(_helics PRIVATE "/wd4100")
  endif(MSVC)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # https://groups.google.com/a/continuum.io/d/msg/anaconda/057P4uNWyCU/Iem6Ot jBCQAJ
    set_target_properties(_helics PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
  else()
    swig_link_libraries(helics ${PYTHON_LIBRARIES})
  endif()

  set_target_properties(_helics PROPERTIES FOLDER interfaces)

  # execute_process(COMMAND python -c "from distutils.sysconfig import get_python_lib;
  # print get_python_lib()" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
  # OUTPUT_STRIP_TRAILING_WHITESPACE)

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

  add_custom_command(
    TARGET
    _helics
    POST_BUILD # Adds a post-build event to api tests
    COMMAND
      ${CMAKE_COMMAND}
      -E
      copy_if_different # which executes "cmake - E copy_if_different..."
      "$<TARGET_FILE:helicsSharedLib>" # <--this is in- file
      "$<TARGET_FILE_DIR:_helics>/"
  ) # <--this is out- file path

else()
  message("Building without swig")

  if(NOT ${CMAKE_VERSION} VERSION_LESS "3.12.0")
    # Use the Python_add_library function added in 3.12.0
    find_package(Python COMPONENTS Development)
    Python_add_library(helicsPYTHON MODULE helics_wrap.c)
  else()
    add_library(helicsPYTHON MODULE helics_wrap.c)
    # Add path to Python include directory
    target_include_directories(helicsPYTHON PUBLIC ${PYTHON_INCLUDE_DIRS})

    # Get list of libraries to link with for Python module
    execute_process(
      COMMAND
          ${PYTHON_EXECUTABLE}
          "-c"
          "from distutils import sysconfig; print(sysconfig.get_config_var('BLDSHARED').split(' ', 1)[1])"
      OUTPUT_VARIABLE
          pymodule_libs
    )
    # Clean-up leading and trailing whitespace
    string(STRIP ${pymodule_libs} pymodule_libs)
    target_link_libraries(helicsPYTHON PUBLIC ${pymodule_libs})

    # Get list of compiler flags for compiling Python module
    execute_process(
      COMMAND
          ${PYTHON_EXECUTABLE}
          "-c"
          "from distutils import sysconfig; print(sysconfig.get_config_var('CFLAGS').split(' ', 1)[1])"
      OUTPUT_VARIABLE
          pymodule_includes
    )
    # Clean-up leading and trailing whitespace, convert into a CMake ;-separated list
    string(STRIP ${pymodule_includes} pymodule_includes)
    string(REPLACE " " ";" pymodule_includes "${pymodule_includes}")
    target_compile_options(helicsPYTHON PUBLIC ${pymodule_includes})
  endif()

  # Set the output library name to _helics
  set_target_properties(helicsPYTHON
      PROPERTIES PREFIX "_"
                 OUTPUT_NAME "helics"
  )
  target_link_libraries(helicsPYTHON PUBLIC helicsSharedLib)

  install(TARGETS helicsPYTHON DESTINATION python COMPONENT python)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in ${CMAKE_CURRENT_BINARY_DIR}/setup.py
  )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/helics.py ${CMAKE_CURRENT_BINARY_DIR}/helics.py
    COPYONLY
  )

endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.py DESTINATION python COMPONENT python)
install(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION python COMPONENT python)
install(FILES ${KEY_LIBRARY_FILES} DESTINATION python COMPONENT python)
