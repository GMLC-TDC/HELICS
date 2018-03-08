# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
UseSWIG
-------

Defines the following macros for use with SWIG:

.. command:: swig_gen_sources

  Define swig module with given name and specified language but only generate don't build::

    SWIG_GEN_SOURCES(<name>
                     [TYPE <SHARED|MODULE|STATIC|USE_BUILD_SHARED_LIBS>]
                     LANGUAGE <language>
                     SOURCES <file>...
                     )

  The variable ``SWIG_MODULE_<name>_REAL_NAME`` will be set to the name
  of the swig module target library.

``CPLUSPLUS``
  Call SWIG in c++ mode.  For example:

  .. code-block:: cmake

    set_property(SOURCE mymod.i PROPERTY CPLUSPLUS ON)
    swig_add_library(mymod LANGUAGE python SOURCES mymod.i)

``SWIG_FLAGS``
  Add custom flags to the SWIG executable.


``SWIG_MODULE_NAME``
  Specify the actual import name of the module in the target language.
  This is required if it cannot be scanned automatically from source
  or different from the module file basename.  For example:

  .. code-block:: cmake

    set_property(SOURCE mymod.i PROPERTY SWIG_MODULE_NAME mymod_realname)

Some variables can be set to specify special behavior of SWIG:

``CMAKE_SWIG_FLAGS``
  Add flags to all swig calls.

``CMAKE_SWIG_OUTDIR``
  Specify where to write the language specific files (swig ``-outdir`` option).

``SWIG_OUTFILE_DIR``
  Specify an output directory name where the generated source file will be
  placed.  If not specified, ``CMAKE_SWIG_OUTDIR`` is used.

``SWIG_MODULE_<name>_EXTRA_DEPS``
  Specify extra dependencies for the generated module for ``<name>``.
#]=======================================================================]

include(useSWIG)

macro(SWIG_GEN_SOURCES name)
  set(options "")
  set(oneValueArgs LANGUAGE
                   TYPE)
  set(multiValueArgs SOURCES)
  cmake_parse_arguments(_SAM "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT DEFINED _SAM_LANGUAGE)
    message(FATAL_ERROR "SWIG_GEN_SOURCES: Missing LANGUAGE argument")
  endif()

  if(NOT DEFINED _SAM_SOURCES)
    message(FATAL_ERROR "SWIG_GEN_SOURCES: Missing SOURCES argument")
  endif()

  if(NOT DEFINED _SAM_TYPE)
    set(_SAM_TYPE MODULE)
  elseif("${_SAM_TYPE}" STREQUAL "USE_BUILD_SHARED_LIBS")
    unset(_SAM_TYPE)
  endif()

  swig_module_initialize(${name} ${_SAM_LANGUAGE})

  set(swig_dot_i_sources)
  set(swig_other_sources)
  foreach(it ${_SAM_SOURCES})
    if(${it} MATCHES "\\.i$")
      set(swig_dot_i_sources ${swig_dot_i_sources} "${it}")
    else()
      set(swig_other_sources ${swig_other_sources} "${it}")
    endif()
  endforeach()

  set(swig_generated_sources)
  set(swig_generated_targets)
  foreach(it ${swig_dot_i_sources})
    SWIG_ADD_SOURCE_TO_MODULE(${name} swig_generated_source ${it})
    set(swig_generated_sources ${swig_generated_sources} "${swig_generated_source}")
    list(APPEND swig_generated_targets "${swig_gen_target}")
  endforeach()
  get_directory_property(swig_extra_clean_files ADDITIONAL_MAKE_CLEAN_FILES)
  set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${swig_extra_clean_files};${swig_generated_sources}")
  add_library(${SWIG_MODULE_${name}_REAL_NAME} INTERFACE)
    target_sources(${SWIG_MODULE_${name}_REAL_NAME} INTERFACE
    ${swig_generated_sources}
    ${swig_other_sources})
  if(CMAKE_GENERATOR MATCHES "Make")
    # see IMPLICIT_DEPENDS above
	if (swig_generated_targets)
      add_dependencies(${SWIG_MODULE_${name}_REAL_NAME} ${swig_generated_targets})
	endif()
  endif()
endmacro()
