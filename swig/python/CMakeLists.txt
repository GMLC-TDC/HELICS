
IF (GENERATE_PYTHON)

# https://stackoverflow.com/a/3818084/5451769
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long -Wno-overlength-strings")
endif()

FIND_PACKAGE(PythonLibs REQUIRED)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

SET(CMAKE_SWIG_FLAGS "")

set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

SET_PROPERTY(SOURCE ../helics.i PROPERTY SWIG_MODULE_NAME helics)


IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.7)
  SWIG_ADD_LIBRARY(helics TYPE MODULE LANGUAGE python SOURCES ../helics.i)
ELSE (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.7)
  SWIG_ADD_MODULE(helics python ../helics.i)
ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.7)

SWIG_LINK_LIBRARIES(helics helicsSharedLib)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # https://groups.google.com/a/continuum.io/d/msg/anaconda/057P4uNWyCU/Iem6OtjBCQAJ
  set_target_properties(_helics PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
else ()
  SWIG_LINK_LIBRARIES(helics ${PYTHON_LIBRARIES})
endif()

if (WIN32)
  foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
    add_custom_command(TARGET _helics POST_BUILD        # Adds a post-build event to _helics
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
      "${keyfile}"      # <--this is in-file
      "${CMAKE_INSTALL_PREFIX}/python/")                 # <--this is out-file path
  endforeach(keyfile)
endif(WIN32)

# execute_process(COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

INSTALL(TARGETS _helics DESTINATION ${CMAKE_INSTALL_PREFIX}/python/)

ELSE (GENERATE_PYTHON)

MESSAGE("Building without swig")

add_custom_target("helicsPYTHON" ALL)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/helics_wrap.c ${CMAKE_CURRENT_BINARY_DIR}/helics_wrap.c)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/helics.py ${CMAKE_CURRENT_BINARY_DIR}/helics.py)

ADD_CUSTOM_TARGET("helicsPYTHONEXT" ALL DEPENDS helicsSharedLib helicsPYTHON)

ADD_CUSTOM_COMMAND(
    TARGET "helicsPYTHONEXT" POST_BUILD
    COMMAND "pip" #
    ARGS "install" "-e" "."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Building Python extension"
)

ENDIF (GENERATE_PYTHON)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.py DESTINATION ${CMAKE_INSTALL_PREFIX}/python/)

