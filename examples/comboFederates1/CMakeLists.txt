

add_executable(comboFed comboFed.cpp)

target_link_libraries(comboFed helics_apps-static)


set_target_properties (comboFed PROPERTIES
    FOLDER examples
)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh
#               ${CMAKE_CURRENT_BINARY_DIR}/run.sh
#               COPYONLY)

add_custom_command(TARGET comboFed POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${CMAKE_CURRENT_SOURCE_DIR}/run.sh"      # <--this is in-file
	"$<TARGET_FILE_DIR:comboFed>/")                 # <--this is out-file path

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET comboFed POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:comboFed>/")                 # <--this is out-file path
endforeach(keyfile)

