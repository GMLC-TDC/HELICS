##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(application_api)

# -------------------------------------------------------------
# Add the main HELICS library
# -------------------------------------------------------------

add_library(helics-static STATIC
    $<TARGET_OBJECTS:application_api>
    $<TARGET_OBJECTS:helics_core>
    $<TARGET_OBJECTS:helics_common>
)

target_link_libraries(helics-static PUBLIC helics_base)
#add and alias library to match the find_package
add_library(HELICS::helics-static ALIAS helics-static)
IF(BUILD_CXX_SHARED_LIB)
	#if (BUILD_C_SHARED_LIB OR INTERFACE_BUILD)
	#	message(WARNING "Building the CXX shared library and C shared library in the same build is not advisable due to conflicting symbol visibility guidelines and is unlikely to work very well")
	#endif()

	add_library(helics-shared SHARED
	../empty.cpp
    $<TARGET_OBJECTS:application_api>
    $<TARGET_OBJECTS:helics_core>
    $<TARGET_OBJECTS:helics_common>
)

	add_library(HELICS::helics-shared ALIAS helics-shared)
	target_link_libraries(helics-shared PRIVATE helics_base)

	if (WIN32)
			set_target_properties(helics-shared PROPERTIES
		WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

	#	IF(MINGW)
	#	set_target_properties(helics-shared PROPERTIES
  #      LINK_FLAGS "-Wl,--whole-archive")
	#	ELSE(MINGW)
	#		set_target_properties(helics-shared PROPERTIES
	#		LINK_FLAGS "/WHOLEARCHIVE")
	#	endif()
	#
	#elseif (APPLE)
	#	set_target_properties(helics-shared PROPERTIES
    #    LINK_FLAGS "-Wl,-all_load")
	#else ()
	#	set_target_properties(helics-shared PROPERTIES
  #      LINK_FLAGS "-Wl,--whole-archive")
	endif ()

endif()

install(FILES flag-definitions.h helics.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics COMPONENT headers)

install (TARGETS helics-static EXPORT helics-targets
	DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs)

IF(BUILD_CXX_SHARED_LIB)
	install (TARGETS helics-shared EXPORT helics-targets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

	INSTALL(FILES $<TARGET_LINKER_FILE:helics-shared> DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs)
	INSTALL(FILES $<TARGET_FILE:helics-shared> DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libs)

ENDIF(BUILD_CXX_SHARED_LIB)

add_subdirectory(apps)

if (BUILD_C_SHARED_LIB OR INTERFACE_BUILD)
	add_subdirectory(shared_api_library)
	add_subdirectory(cpp98)
	install(FILES chelics.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics COMPONENT headers)
endif()

