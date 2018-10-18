#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. See LICENSE
# file and DISCLAIMER for more details.
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

  add_custom_target("helicsPYTHON" ALL)

  # Set directories to use as defaults in setup.py for the location of ZeroMQ
  set(ZMQ_INCLUDE_DIR ${ZeroMQ_INCLUDE_DIR})
  get_filename_component(ZMQ_LIB_DIR ${ZeroMQ_LIBRARY} DIRECTORY)
  set(PYTHON_ZMQ_LINK_FLAG "-lzmq")
  if(ZMQ_USE_STATIC_LIBRARY)
      set(PYTHON_ZMQ_LINK_FLAG "")
  endif()

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in ${CMAKE_CURRENT_BINARY_DIR}/setup.py
  )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/helics_wrap.c ${CMAKE_CURRENT_BINARY_DIR}/helics_wrap.c
    COPYONLY
  )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/helics.py ${CMAKE_CURRENT_BINARY_DIR}/helics.py COPYONLY
  )

  add_custom_target("helicsPYTHONEXT" ALL DEPENDS helicsSharedLib helicsPYTHON)

  add_custom_command(
    TARGET "helicsPYTHONEXT" POST_BUILD
    COMMAND
      ${PYTHON_EXECUTABLE}
      ARGS
        "-m"
        "pip"
        "install"
        "-e"
        "."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Building Python extension"
  )

endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.py DESTINATION python COMPONENT python)
install(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION python COMPONENT python)
install(FILES ${KEY_LIBRARY_FILES} DESTINATION python COMPONENT python)
