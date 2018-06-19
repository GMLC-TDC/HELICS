##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

if(BUILD_APPS OR BUILD_BROKER)

set(helics_apps_public_headers
	Player.hpp
	Recorder.hpp
	Echo.hpp
	Source.hpp
	Tracer.hpp
	helicsApp.hpp
	BrokerApp.hpp
	)

set(helics_apps_private_headers
	PrecHelper.hpp
	SignalGenerators.hpp
	)

set(helics_apps_library_files
	Player.cpp
	Recorder.cpp
	PrecHelper.cpp
	SignalGenerators.cpp
	Echo.cpp
	Source.cpp
	Tracer.cpp
	helicsApp.cpp
	BrokerApp.cpp
)

add_library(helics_apps-static STATIC ${helics_apps_library_files} ${helics_apps_public_headers} ${helics_apps_private_headers})

target_link_libraries(helics_apps-static PUBLIC helics-static)
target_include_directories(helics_apps-static SYSTEM PRIVATE $<TARGET_PROPERTY:fmt-header-only,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_definitions(helics_apps-static PRIVATE $<TARGET_PROPERTY:fmt-header-only,INTERFACE_COMPILE_DEFINITIONS>)
#add and alias library to match the find_package
add_library(HELICS::helics_apps-static ALIAS helics_apps-static)


	add_executable(helics_player playerMain.cpp)
	target_link_libraries(helics_player helics_apps-static )
	set_target_properties (helics_player PROPERTIES FOLDER apps)
	install(TARGETS helics_player DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)

	add_executable(helics_recorder recorderMain.cpp)
	target_link_libraries(helics_recorder helics_apps-static)
	set_target_properties (helics_recorder PROPERTIES FOLDER apps)
	install(TARGETS helics_recorder DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)


if (BUILD_BROKER)
	add_executable(helics_broker helics-broker.cpp)
   target_link_libraries(helics_broker helics_apps-static)
   set_target_properties (helics_broker PROPERTIES FOLDER apps)
   set(HELICS_BROKER_LOC ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "build folder location of the broker")
   install(TARGETS helics_broker DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)

endif()

	add_executable(helics_app appMain.cpp)
	target_link_libraries(helics_app helics_apps-static)
	set_target_properties (helics_app PROPERTIES FOLDER apps)
	install(TARGETS helics_app DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)
	if (WIN32)
		foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
			add_custom_command(TARGET helics_app POST_BUILD        # Adds a post-build event to core tests
				COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
				"${keyfile}"      # <--this is in-file
				"$<TARGET_FILE_DIR:helics_app>/")                 # <--this is out-file path
		endforeach(keyfile)

	endif()

	install(TARGETS helics_apps-static EXPORT helics-targets
		DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT libs)

	install(FILES ${helics_apps_public_headers} 
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/apps COMPONENT headers)

endif()  #if build any app

#configure executable app

configure_file(helicsConfigMain.h.in ${CMAKE_CURRENT_BINARY_DIR}/helicsConfigMain.h)
add_executable(helics-config helicsConfigMain.cpp ${CMAKE_CURRENT_BINARY_DIR}/helicsConfigMain.h)

target_include_directories(helics-config PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties (helics-config PROPERTIES FOLDER apps)

target_link_libraries(helics-config PUBLIC Boostlibs::core)
target_include_directories(helics-config SYSTEM PRIVATE ${Boost_INCLUDE_DIR})

install(TARGETS helics-config
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libs
)

