##############################################################################
#Copyright © 2017-2018,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

if (BUILD_PYTHON_INTERFACE)
	SET(Python_ADDITIONAL_VERSIONS 3.4 3.5 3.6 3.7 3.8)
	find_package(PythonInterp 3)
	find_package(PythonLibs 3 REQUIRED )
endif(BUILD_PYTHON_INTERFACE)

IF (DISABLE_SWIG OR NOT SWIG_EXECUTABLE)

MESSAGE("Building without swig")

add_custom_target("helicsPYTHON" ALL)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/helics_wrap.c ${CMAKE_CURRENT_BINARY_DIR}/helics_wrap.c COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/helics.py ${CMAKE_CURRENT_BINARY_DIR}/helics.py COPYONLY)

ADD_CUSTOM_TARGET("helicsPYTHONEXT" ALL DEPENDS helicsSharedLib helicsPYTHON)

ADD_CUSTOM_COMMAND(
    TARGET "helicsPYTHONEXT" POST_BUILD
    COMMAND "pip" #
    ARGS "install" "-e" "."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Building Python extension"
)
ELSE()
# https://stackoverflow.com/a/3818084/5451769
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long -Wno-overlength-strings -Wno-ignored-attributes")
endif()


INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

SET(CMAKE_SWIG_FLAGS "-py3")

set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")

SET_PROPERTY(SOURCE ../helics.i PROPERTY SWIG_MODULE_NAME helics)


IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.7)
  SWIG_ADD_LIBRARY(helics TYPE MODULE LANGUAGE python SOURCES helicsPython.i)
ELSE ()
  SWIG_ADD_MODULE(helics python helicsPython.i)
ENDIF ()

SWIG_LINK_LIBRARIES(helics helicsSharedLib)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # https://groups.google.com/a/continuum.io/d/msg/anaconda/057P4uNWyCU/Iem6OtjBCQAJ
  set_target_properties(_helics PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
else ()
  SWIG_LINK_LIBRARIES(helics ${PYTHON_LIBRARIES})
endif()

set_target_properties (_helics PROPERTIES FOLDER interfaces)



# execute_process(COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

INSTALL(TARGETS _helics DESTINATION python COMPONENT python)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET _helics POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:_helics>/")                 # <--this is out-file path
endforeach(keyfile)

add_custom_command(TARGET _helics POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helicsSharedLib>"      # <--this is in-file
        "$<TARGET_FILE_DIR:_helics>/")                 # <--this is out-file path
		
ENDIF ()



INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.py DESTINATION python COMPONENT python)
INSTALL(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION python COMPONENT python)
INSTALL(FILES ${KEY_LIBRARY_FILES} DESTINATION python COMPONENT python)

