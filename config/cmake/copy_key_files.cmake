
macro(copy_key_files_to_target_location target)
  if(WIN32)

  foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
    add_custom_command(
      TARGET
      ${target}
      POST_BUILD # Adds a post-build event to api tests
      COMMAND
        ${CMAKE_COMMAND}
        -E
        copy_if_different # which executes "cmake - E copy_if_different..."
        "${keyfile}" # <--this is in-file
        "$<TARGET_FILE_DIR:${target}>/"
    )
    # <--this is out-file path
  endforeach(keyfile)
  
   if (TARGET libzmq)
add_custom_command(
    TARGET
     ${target}
    POST_BUILD # Adds a post-build event to core tests
    COMMAND
      ${CMAKE_COMMAND}
      -E
      copy_if_different # which executes "cmake - E copy_if_different..."
      "$<TARGET_FILE:libzmq>" # <--this is in-file
      "$<TARGET_FILE_DIR:${target}>/"
  ) # <--this is out- file path
endif()
endif(WIN32)
endmacro()

macro(copy_shared_target target)
 add_custom_command(
    TARGET
    ${target}
    POST_BUILD # Adds a post-build event to api tests
    COMMAND
      ${CMAKE_COMMAND}
      -E
      copy_if_different # which executes "cmake - E copy_if_different..."
      "$<TARGET_FILE:helicsSharedLib>" # <--this is in- file
      "$<TARGET_FILE_DIR:${target}>/"
  ) # <--this is out- file path
endmacro()