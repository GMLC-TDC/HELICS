add_executable(messageFed messageFed.cpp)
add_executable(messageFedObj messageFedObj.cpp)

target_link_libraries(messageFed helics_apps-static)
target_link_libraries(messageFedObj helics_apps-static)

set_target_properties (messageFed messageFedObj PROPERTIES
    FOLDER examples
)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh
#               ${CMAKE_CURRENT_BINARY_DIR}/run.sh
#               COPYONLY)

add_custom_command(TARGET messageFed POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${CMAKE_CURRENT_SOURCE_DIR}/run.sh"      # <--this is in-file
	"$<TARGET_FILE_DIR:messageFed>/")                 # <--this is out-file path

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET messageFed POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:messageFed>/")                 # <--this is out-file path
endforeach(keyfile)

