##############################################################################
#Copyright © 2017-2018,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

IF (DISABLE_SWIG OR NOT SWIG_EXECUTABLE)

	MESSAGE(ERROR "Python 2 build requires swig")

ENDIF()

# https://stackoverflow.com/a/3818084/5451769
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long -Wno-overlength-strings -Wno-ignored-attributes")
endif()

find_package(PythonLibs 2 REQUIRED )

INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

SET(CMAKE_SWIG_FLAGS "")

SET_PROPERTY(SOURCE ../helics.i PROPERTY SWIG_MODULE_NAME helics)


IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.7)
  SWIG_ADD_LIBRARY(helics TYPE MODULE LANGUAGE python SOURCES helicsPython2.i)
ELSE ()
  SWIG_ADD_MODULE(helics python helicsPython2.i)
ENDIF ()

SWIG_LINK_LIBRARIES(helics helicsSharedLib)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # https://groups.google.com/a/continuum.io/d/msg/anaconda/057P4uNWyCU/Iem6OtjBCQAJ
  set_target_properties(_helics PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
else ()
  SWIG_LINK_LIBRARIES(helics ${PYTHON_LIBRARIES})
endif()

set_target_properties (_helics PROPERTIES FOLDER interfaces)

INSTALL(TARGETS _helics DESTINATION python COMPONENT python)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET _helics POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:_helics>/")                 # <--this is out-file path
endforeach(keyfile)


INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.py DESTINATION python COMPONENT python)
INSTALL(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION python COMPONENT python)
INSTALL(FILES ${KEY_LIBRARY_FILES} DESTINATION python COMPONENT python)

