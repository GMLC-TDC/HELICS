##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
set(helics_apps_library_headers
	player.h
	recorder.h
	PrecHelper.h
	echo.h
	source.h
	)

set(helics_apps_library_files
	player.cpp
	recorder.cpp
	PrecHelper.cpp
	echo.cpp
	source.cpp
)

add_library(helics_apps-static STATIC ${helics_apps_library_files} ${helics_apps_library_headers})

if (BUILD_PLAYER)
add_executable(helics_player playerMain.cpp)
target_link_libraries(helics_player helics_apps-static helics-static)

endif()


if (BUILD_RECORDER)
add_executable(helics_recorder recorderMain.cpp)
target_link_libraries(helics_recorder helics_apps-static helics-static)

endif()

include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)


INSTALL(TARGETS helics_apps-static EXPORT helics-targets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
	
if (BUILD_PLAYER)
INSTALL(TARGETS helics_player DESTINATION ${CMAKE_INSTALL_BINDIR})

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics_player POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:helics_player>/")                 # <--this is out-file path
endforeach(keyfile)

endif()

if (BUILD_RECORDER)
INSTALL(TARGETS helics_recorder DESTINATION ${CMAKE_INSTALL_BINDIR})
if (NOT BUILD_PLAYER)
foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics_recorder POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:helics_recorder>/")                 # <--this is out-file path
endforeach(keyfile)
endif()
endif()


