#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details. 
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(NOT SWIG_EXECUTABLE)

  message(SEND_ERROR "Python 2 build requires swig")

endif()

# https://stackoverflow.com/a/3818084/5451769
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(
    CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -Wno-long-long -Wno-overlength-strings -Wno-ignored-attributes"
  )
endif()

 if(NOT ${CMAKE_VERSION} VERSION_LESS "3.12.0")
	if (PYTHON_ROOT_DIR)
		set(Python3_ROOT_DIR ${PYTHON_ROOT_DIR})
	endif()
    # Use the Python_add_library function added in 3.12.0
    find_package(Python2 COMPONENTS Development)
	set(HELICS_PYTHON_INCLUDES ${Python2_INCLUDE_DIRS})
	set(HELICS_PYTHON_LIBS ${Python2_LIBRARIES})

  else()
    set(Python_ADDITIONAL_VERSIONS 3.4 3.5 3.6 3.7 3.8)
    find_package(PythonLibs 2 REQUIRED)

	set(HELICS_PYTHON_INCLUDES ${PYTHON_INCLUDE_DIRS})
	set(HELICS_PYTHON_LIBS ${PYTHON_LIBRARIES})
  endif()
  

include_directories(${HELICS_PYTHON_INCLUDES})

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
  swig_link_libraries(helics ${HELICS_PYTHON_LIBS})
endif()

set_target_properties(_helics PROPERTIES FOLDER interfaces)
if (MSVC)
	target_compile_options(_helics PRIVATE "/wd4100")
	target_compile_options(_helics PRIVATE "/wd4459")
endif()

install(TARGETS _helics DESTINATION python COMPONENT python)

  copy_key_files_to_target_location(_helics)
  copy_shared_target(_helics)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.py DESTINATION python COMPONENT python)
install_key_files_with_comp(python)
