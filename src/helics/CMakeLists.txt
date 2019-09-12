#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(application_api)

# -------------------------------------------------------------
# Add the main HELICS library
# -------------------------------------------------------------

add_library(
  helics-static
  STATIC
  $<TARGET_OBJECTS:application_api>
  $<TARGET_OBJECTS:helics_core>
  $<TARGET_OBJECTS:helics_common>
  $<$<BOOL:${GMLC_UTILITIES_OBJECT_LIB}>:$<TARGET_OBJECTS:gmlc_utilities_obj>>
  $<$<BOOL:${HELICS_UNITS_OBJLIB}>:$<TARGET_OBJECTS:units-object>>
)

target_link_libraries(helics-static PUBLIC helics_base helics_base_includes)
# add and alias library to match the find_package
add_library(HELICS::helics-static ALIAS helics-static)

if (JSONCPP_OBJLIB)
  target_sources(helics-static PRIVATE $<TARGET_OBJECTS:jsoncppobj_lib>)
else()
  target_link_libraries(helics-static PUBLIC jsoncpp_lib)
endif()

if (NOT GMLC_UTILITIES_OBJECT_LIB)
  target_link_libraries(helics-static PUBLIC HELICS::utilities)
endif()

if (NOT HELICS_UNITS_OBJLIB)
  target_link_libraries(helics-static PUBLIC units::units)
endif()

if(BUILD_SHARED_LIBS)
  add_library(
    helics-shared
    SHARED
    ../empty.cpp
    $<TARGET_OBJECTS:application_api>
    $<TARGET_OBJECTS:helics_core>
    $<TARGET_OBJECTS:helics_common>
	$<$<BOOL:${GMLC_UTILITIES_OBJECT_LIB}>:$<TARGET_OBJECTS:gmlc_utilities_obj>>
	$<$<BOOL:${HELICS_UNITS_OBJLIB}>:$<TARGET_OBJECTS:units-object>>
  )

  add_library(HELICS::helics-shared ALIAS helics-shared)
  target_link_libraries(helics-shared PRIVATE helics_base helics_base_includes)
  target_include_directories(helics-shared
    INTERFACE
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
      $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
  )

  set_target_properties(helics-shared PROPERTIES
    VERSION ${HELICS_VERSION}
    SOVERSION ${HELICS_VERSION_MAJOR}
  )

  if (JSONCPP_OBJLIB)
    target_sources(helics-shared PRIVATE $<TARGET_OBJECTS:jsoncppobj_lib>)
  else()
    target_link_libraries(helics-shared PRIVATE jsoncpp_lib)
  endif()

  if (NOT GMLC_UTILITIES_OBJECT_LIB)
	target_link_libraries(helics-shared PUBLIC HELICS::utilities)
  endif()

if (NOT HELICS_UNITS_OBJLIB)
  target_link_libraries(helics-shared PUBLIC units::units)
endif()
  if(WIN32)
    set_target_properties(helics-shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

  endif()

endif()

if (NOT HELICS_BINARY_ONLY_INSTALL)
install(
  FILES helics.hpp
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
  COMPONENT headers
)

install(
  TARGETS helics-static
  ${HELICS_EXPORT_COMMAND}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  COMPONENT libs
)
endif()

if(BUILD_SHARED_LIBS)
  install(
    TARGETS helics-shared
    ${HELICS_EXPORT_COMMAND}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  if(WIN32)
    install(
      FILES $<TARGET_LINKER_FILE:helics-shared>
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT libs
    )
    install(
      FILES $<TARGET_FILE:helics-shared>
      DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT Runtime
    )
  else()
    install(
      TARGETS helics-shared
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT Runtime
    )
  endif()

endif(BUILD_SHARED_LIBS)

	if (NOT HELICS_ROOT_PROJ)
		set_target_properties(application_api helics_common helics_core helics-static PROPERTIES FOLDER helics)
	endif()


option(
    BUILD_APP_LIBRARY
    "Build helics application library (a static library containing objects for broker, player, recorder, and helics_apps"
    ON
)

if (HELICS_ROOT_PROJ)
	option(
		BUILD_APP_EXECUTABLES
		"Build helics applications to run various apps, including broker, player, recorder, and helics_apps"
		${BUILD_APP_LIBRARY}
	)
else()
	SET(BUILD_APP_EXECUTABLES OFF)
endif()

if (BUILD_APP_LIBRARY)
	add_subdirectory(apps)
	if (NOT HELICS_ROOT_PROJ)
		set_target_properties(helics_apps PROPERTIES FOLDER helics)
	endif()
endif(BUILD_APP_LIBRARY)

if(NOT DISABLE_C_SHARED_LIB)
  add_subdirectory(shared_api_library)
  add_subdirectory(cpp98)
  if (NOT HELICS_BINARY_ONLY_INSTALL)
  install(
    FILES chelics.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
    COMPONENT headers
  )
  endif()
endif()


