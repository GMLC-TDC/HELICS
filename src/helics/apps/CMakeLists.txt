##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################
set(helics_apps_public_headers
	Player.hpp
	Recorder.hpp
	Echo.hpp
	Source.hpp
	Tracer.hpp
	helicsApp.hpp
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
)

add_library(helics_apps-static STATIC ${helics_apps_library_files} ${helics_apps_public_headers} ${helics_apps_private_headers})

target_link_libraries(helics_apps-static PUBLIC helics-static)

if (BUILD_PLAYER)
	add_executable(helics_player playerMain.cpp)
	target_link_libraries(helics_player helics_apps-static )
	set_target_properties (helics_player PROPERTIES FOLDER apps)
	INSTALL(TARGETS helics_player DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)
endif()


if (BUILD_RECORDER)
	add_executable(helics_recorder recorderMain.cpp)
	target_link_libraries(helics_recorder helics_apps-static)
	set_target_properties (helics_recorder PROPERTIES FOLDER apps)
	INSTALL(TARGETS helics_recorder DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)
endif()

if (BUILD_BROKER)
add_executable(helics_broker helics-broker.cpp)
   target_link_libraries(helics_broker helics-static)
   set_target_properties (helics_broker PROPERTIES FOLDER apps)
   set(HELICS_BROKER_LOC ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "build folder location of the broker")
   INSTALL(TARGETS helics_broker DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)

endif()

if(BUILD_PLAYER OR BUILD_RECORDER OR BUILD_APPS OR BUILD_BROKER)
	add_executable(helics_app appMain.cpp)
	target_link_libraries(helics_app helics_apps-static)
	set_target_properties (helics_app PROPERTIES FOLDER apps)
	INSTALL(TARGETS helics_app DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)
	if (WIN32)
	foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics_app POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:helics_app>/")                 # <--this is out-file path
endforeach(keyfile)
endif()
endif()

INSTALL(TARGETS helics_apps-static EXPORT helics-targets
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	COMPONENT libs)

INSTALL(FILES ${helics_apps_public_headers} 
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/apps COMPONENT headers)

