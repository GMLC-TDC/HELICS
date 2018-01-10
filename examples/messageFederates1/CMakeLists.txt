add_executable(messageFed messageFed.cpp)

include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/src)

target_link_libraries(messageFed helics-static)

set_target_properties (messageFed PROPERTIES
    FOLDER examples
)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh
#               ${CMAKE_CURRENT_BINARY_DIR}/run.sh
#               COPYONLY)
			   
add_custom_command(TARGET messageFed POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${CMAKE_CURRENT_SOURCE_DIR}/run.sh"      # <--this is in-file
	"$<TARGET_FILE_DIR:messageFed>/")                 # <--this is out-file path
