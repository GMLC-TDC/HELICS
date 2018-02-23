

OPTION(QUICK_TESTS_ONLY "set to build only the quick tests and turn off many of the longer tests" OFF)

if (QUICK_TESTS_ONLY)
add_definitions(-DQUICK_TESTS_ONLY)
endif()
add_subdirectory(common)

add_subdirectory (core)
add_subdirectory(application_api)

if (BUILD_C_SHARED_LIB)
    add_subdirectory(shared_library)
endif()

if(BUILD_PLAYER OR BUILD_RECORDER OR BUILD_APPS)
    add_subdirectory(apps)
endif()

if (TRAVIS_TESTS_ENABLE)
	add_subdirectory(travis_tests)
endif()
