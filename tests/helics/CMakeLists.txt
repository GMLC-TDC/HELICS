
OPTION(QUICK_TESTS_ONLY "set to build only the quick tests and turn off many of the longer tests" OFF)

if (QUICK_TESTS_ONLY)
add_definitions(-DQUICK_TESTS_ONLY)
endif()
add_subdirectory (core)
add_subdirectory(application_api)


if (BUILD_SHARED_LIBS)
    add_subdirectory(c_interface)
endif()

if(BUILD_PLAYER OR BUILD_RECORDER)
    add_subdirectory(player_recorder)
endif()
