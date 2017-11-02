##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
set(PLAYER_SRC_FILES
    player.cpp
	PrecHelper.cpp
	PrecHelper.h
)
set(PLAYER_INCLUDE_FILES

)


set(RECORDER_SRC_FILES
    recorder.cpp
	PrecHelper.cpp
	PrecHelper.h
)
set(RECORDER_INCLUDE_FILES

)

if (BUILD_PLAYER)
add_executable(helics_player ${PLAYER_SRC_FILES} ${PLAYER_INCLUDE_FILES})
target_link_libraries(helics_player helics)
target_link_libraries(helics_player ${external_library_list})
IF(MINGW)
target_link_libraries(helics_player ws2_32)
ENDIF(MINGW)
endif()


if (BUILD_RECORDER)
add_executable(helics_recorder ${RECORDER_SRC_FILES} ${RECORDER_INCLUDE_FILES})
target_link_libraries(helics_recorder helics)
target_link_libraries(helics_recorder ${external_library_list})
IF(MINGW)
target_link_libraries(helics_recorder ws2_32)
ENDIF(MINGW)
endif()

include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)

if (BUILD_PLAYER)
INSTALL(TARGETS helics_player DESTINATION bin)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics_player POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:helics_player>/")                 # <--this is out-file path
endforeach(keyfile)

endif()

if (BUILD_RECORDER)
INSTALL(TARGETS helics_recorder DESTINATION bin)
if (NOT BUILD_PLAYER)
foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics_recorder POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:helics_recorder>/")                 # <--this is out-file path
endforeach(keyfile)
endif()
endif()


