# Additional targets to perform clang-format/clang-tidy
# derived from http://www.labri.fr/perso/fleury/posts/programming/using-clang-tidy-and-clang-format.html
# Get all project files
#src/*.[ch]pp src/*.[ch]xx src/*.cc src/*.hh  src/*.[CHI] src/*.[ch]
file(GLOB_RECURSE ALL_TEST_FILES tests/helics/*.[ch]pp tests/helics/*.[ch])

file(GLOB_RECURSE ALL_EXAMPLE_FILES examples/*.[ch]pp examples/*.[ch])

file(GLOB ALL_APPLICATION_API_FILES src/helics/application_api/*.[ch]pp)

file(GLOB ALL_APP_FILES src/helics/apps/*.[ch]pp)

file(GLOB ALL_SHARED_API_FILES src/helics/shared_api_library/*.[ch]pp
     src/helics/shared_api_library/*.[ch] src/helics/shared_api_library/internal/*.h
)

file(
    GLOB
    ALL_CORE_FILES
    src/helics/core/*.[ch]pp
    src/helics/core/zmq/*.[ch]pp
    src/helics/core/ipc/*.[ch]pp
    src/helics/core/mpi/*.[ch]pp
    src/helics/core/udp/*.[ch]pp
    src/helics/core/tcp/*.[ch]pp
)

file(GLOB ALL_COMMON_FILES src/helics/common/*.[ch]pp)

set(ALL_APPLICATION_FILES ${ALL_APPLICATION_API_FILES} ${ALL_APP_FILES} ${ALL_SHARED_API_FILES})

set(ALL_CORE_FILES ${ALL_CORE_FILES} ${ALL_COMMON_FILES})

set(INCLUDE_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/src/helics/core
    ${PROJECT_SOURCE_DIR}/src/helics/application_api
    ${PROJECT_SOURCE_DIR}/src/helics/apps
    ${PROJECT_SOURCE_DIR}/src/helics/common
    ${PROJECT_SOURCE_DIR}/src/helics/shared_api_library
    ${PROJECT_SOURCE_DIR}/test
    ${ZMQ_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/helics
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/libs/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/src/helics/shared_api_library
    ${PROJECT_SOURCE_DIR}/ThirdParty
    ${Boost_INCLUDE_DIR}
)

set(INCLUDES "")
foreach(f ${INCLUDE_DIRECTORIES})
    list(APPEND INCLUDES "-I${f}")
endforeach(f)

# Adding clang-format target if executable is found
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    add_custom_target(clang-format-test COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_TEST_FILES})

    add_custom_target(
        clang-format-application COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_APPLICATION_FILES}
    )

    add_custom_target(clang-format-core COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_CORE_FILES})

    add_custom_target(
        clang-format-examples COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_EXAMPLE_FILES}
    )

    add_custom_target(
        clang-format-all DEPENDS clang-format-test clang-format-application clang-format-core
                                 clang-format-examples
    )
endif()

# Adding clang-tidy target if executable is found
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
    add_custom_target(
        clang-tidy-test COMMAND ${CLANG_TIDY} ${ALL_TEST_FILES} -config='' -- -std=c++14
                                ${INCLUDES}
    )

    add_custom_target(
        clang-tidy-application COMMAND ${CLANG_TIDY} ${ALL_APPLICATION_FILES} -config='' --
                                       -std=c++14 ${INCLUDES}
    )

    add_custom_target(
        clang-tidy-core COMMAND ${CLANG_TIDY} ${ALL_CORE_FILES} -config='' -- -std=c++14
                                ${INCLUDES}
    )
    add_custom_target(
        clang-tidy-ccore
        COMMAND
            ${CLANG_TIDY} ${PROJECT_SOURCE_DIR}/src/helics/core/CommonCore.cpp
            ${PROJECT_SOURCE_DIR}/src/helics/core/CoreBroker.cpp -config='' -- -std=c++14
            ${INCLUDES}
    )

    add_custom_target(clang-tidy-all DEPENDS clang-tidy-test clang-tidy-application clang-tidy-core)
endif()
