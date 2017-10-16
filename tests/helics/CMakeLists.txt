
add_subdirectory (core)
add_subdirectory(application_api)

if (BUILD_SHARED_LIBS)
    add_subdirectory(c_interface)
endif()
