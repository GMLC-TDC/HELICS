
add_subdirectory(common)

add_subdirectory (core)
add_subdirectory(application_api)

if (BUILD_C_SHARED_LIB)
    add_subdirectory(shared_library)
endif()

if(BUILD_PLAYER OR BUILD_RECORDER OR BUILD_APPS)
    add_subdirectory(apps)
endif()
