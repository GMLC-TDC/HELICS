# Additional targets to perform clang-format/clang-tidy
# derived from http://www.labri.fr/perso/fleury/posts/programming/using-clang-tidy-and-clang-format.html
# Get all project files
#src/*.[ch]pp src/*.[ch]xx src/*.cc src/*.hh  src/*.[CHI] src/*.[ch]
file(GLOB_RECURSE
     ALL_TEST_FILES
      tests/helics/*.[ch]pp
     )
	 
file(GLOB
     ALL_APPLICATION_API_FILES
      src/helics/application_api/*.[ch]pp
     )
	 
file(GLOB
     ALL_PLAYER_FILES
      src/helics/player/*.[ch]pp
     )
	 
file(GLOB
     ALL_SHARED_LIB_FILES
      src/helics/shared_api_library/*.[ch]pp src/helics/shared_api_library/*.[ch] src/helics/shared_api_library/*.[CH]
     )
	 
file(GLOB
     ALL_CORE_LIB_FILES
      src/helics/core/*.[ch]pp src/helics/core/zmq/*.[ch]pp src/helics/core/ipc/*.[ch]pp src/helics/core/mpi/*.[ch]pp
     )
	 
file(GLOB
     ALL_COMMON_FILES
      src/helics/common/*.[ch]pp
     )

set(ALL_APPLICATION_FILES
		${ALL_APPLICATION_API_FILES}
		${ALL_PLAYER_FILES}
		${ALL_SHARED_LIB_FILES}
		)
		
set(ALL_CORE_FILES
		${ALL_CORE_LIB_FILES}
		${ALL_COMMON_FILES}
		)
		
set(INCLUDE_DIRECTORIES
${PROJECT_SOURCE_DIR}/src/core
${ROJECT_SOURCE_DIR}/src/application_api
${PROJECT_SOURCE_DIR}/src/player
${PROJECT_SOURCE_DIR}/src/common
${PROJECT_SOURCE_DIR}/src/shared_api_library
${PROJECT_SOURCE_DIR}/test
${ZMQ_INCLUDE_DIR}
${PROJECT_SOURCE_DIR}/src/helics
${PROJECT_SOURCE_DIR}/src
${PROJECT_BINARY_DIR}/libs/include
${PROJECT_BINARY_DIR}/include
${PROJECT_SOURCE_DIR}/ThirdParty
${Boost_INCLUDE_DIR}
)
 

SET(INCLUDES "")
   FOREACH(f ${INCLUDE_DIRECTORIES})
      LIST(APPEND INCLUDES "-I${f}")
   ENDFOREACH(f)
   
   
# Adding clang-format target if executable is found
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
  add_custom_target(
    clang-format-test
    COMMAND ${CLANG_FORMAT}
    -i
    -style=file
    ${ALL_TEST_FILES}
    )

  add_custom_target(
    clang-format-application
    COMMAND ${CLANG_FORMAT}
    -i
    -style=file
    ${ALL_APPLICATION_FILES}
    )
	
	add_custom_target(
    clang-format-core
    COMMAND ${CLANG_FORMAT}
    -i
    -style=file
    ${ALL_CORE_FILES}
    )
	
	
endif()

# Adding clang-tidy target if executable is found
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  add_custom_target(
    clang-tidy-test
    COMMAND ${CLANG_TIDY}
    ${ALL_TEST_FILES}
    -config=''
	--
    -std=c++14
    ${INCLUDES}
    )

  add_custom_target(
    clang-tidy-application
    COMMAND ${CLANG_TIDY}
    ${ALL_APPLICATION_FILES} 
   -config='' 
   --
    -std=c++14
    ${INCLUDES}
    )
	
	add_custom_target(
    clang-tidy-core
    COMMAND ${CLANG_TIDY}
    ${ALL_CORE_FILES}
   -config='' 
   --
    -std=c++14
    ${INCLUDES}
    )
	
endif()