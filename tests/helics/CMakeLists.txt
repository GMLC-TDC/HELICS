
add_subdirectory(common)

add_subdirectory (core)
add_subdirectory(application_api)

if (BUILD_C_SHARED_LIB OR BUILD_PYTHON_INTERFACE OR BUILD_MATLAB_INTERFACE OR BUILD_OCTAVE_INTERFACE OR BUILD_JAVA_INTERFACE)
    add_subdirectory(shared_library)
endif()

if(BUILD_PLAYER OR BUILD_RECORDER OR BUILD_APPS)
    add_subdirectory(apps)
endif()

