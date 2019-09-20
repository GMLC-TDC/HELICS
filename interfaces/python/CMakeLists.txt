#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details. 
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(SWIG_GENERATE_INTERFACE_FILES_ONLY)
 include (${CMAKE_CURRENT_SOURCE_DIR}/pythonSwigGenerateOnly.cmake)
else()
#at some point in the future (HELICS 3) switch everything to use find_package(Python3 ...)
	if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.12)
		option(HELICS_USE_NEW_PYTHON_FIND "Use the FindPython Routine only available from cmake 3.12 and later" OFF)
		mark_as_advanced(HELICS_USE_NEW_PYTHON_FIND)
	else()
		set(HELICS_USE_NEW_PYTHON_FIND OFF CACHE BOOL "" FORCE)
		hide_variable(HELICS_USE_NEW_PYTHON_FIND)
	endif()
 
 if (HELICS_USE_NEW_PYTHON_FIND)
	FindPackage(Python3 Interpreter Development)
	if(ENABLE_SWIG AND SWIG_EXECUTABLE)
    
  # https://stackoverflow.com/a/3818084/5451769
  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(
      CMAKE_C_FLAGS
      "${CMAKE_C_FLAGS} -Wno-long-long -Wno-overlength-strings -Wno-ignored-attributes"
    )
  endif()

  set(CMAKE_SWIG_FLAGS "-py3")

  set_property(SOURCE ../helics.i PROPERTY SWIG_MODULE_NAME helics)

  
    swig_add_library(helics TYPE MODULE LANGUAGE python SOURCES helicsPython.i)

  swig_link_libraries(helics helicsSharedLib Python3::Module)
  if (MSVC)
	target_compile_options(_helics PRIVATE "/wd4100")
  else(MSVC)
   target_compile_options(_helics PRIVATE "-Wno-long-long")
  endif(MSVC)

  set_target_properties(_helics PROPERTIES FOLDER interfaces)

  install(TARGETS _helics DESTINATION python COMPONENT python)

  copy_key_files_to_target_location(_helics)
  copy_shared_target(_helics)
 
else()
  message("Building without swig cmake ${CMAKE_VERSION}")
    python3_add_library(helicsPYTHON MODULE helicsPython.c)
	
	copy_key_files_to_target_location(helicsPYTHON PUBLIC)
    copy_shared_target(helicsPYTHON PUBLIC)


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
 else()

    set(Python_ADDITIONAL_VERSIONS 3.4 3.5 3.6 3.7 3.8)
	if (NOT PYTHON_EXECUTABLE)
       find_package(PythonInterp 3 REQUIRED)
	endif()
    find_package(PythonLibs 3 REQUIRED)

if(ENABLE_SWIG AND SWIG_EXECUTABLE)

  # https://stackoverflow.com/a/3818084/5451769
  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(
      CMAKE_C_FLAGS
      "${CMAKE_C_FLAGS} -Wno-long-long -Wno-overlength-strings -Wno-ignored-attributes"
    )
  endif()

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
  else(MSVC)
   target_compile_options(_helics PRIVATE "-Wno-long-long")
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

  copy_key_files_to_target_location(_helics)
  copy_shared_target(_helics)
 
else()
  message("Building without swig cmake ${CMAKE_VERSION}")

#convert to use the python3_add_library in HELICS 3 
    
    add_library(helicsPYTHON MODULE helicsPython.c)
    # Add path to Python include directory
    target_include_directories(helicsPYTHON PUBLIC ${PYTHON_INCLUDE_DIRS})

    # Get list of libraries to link with for Python module
	if (NOT MSVC)
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
	else()
	  set(pymodule_libs "")
	endif()
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # https://groups.google.com/a/continuum.io/d/msg/anaconda/057P4uNWyCU/Iem6Ot jBCQAJ
    set_target_properties(helicsPYTHON PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
  else()
        target_link_libraries(helicsPYTHON PUBLIC ${pymodule_libs} ${PYTHON_LIBRARIES})
  endif()

if (NOT MSVC)
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
	
	copy_key_files_to_target_location(helicsPYTHON PUBLIC)
    copy_shared_target(helicsPYTHON PUBLIC)


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
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.py DESTINATION python COMPONENT python)
install_key_files_with_comp(python)

endif(SWIG_GENERATE_INTERFACE_FILES_ONLY)
