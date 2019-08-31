cmake_minimum_required(VERSION 3.4.0)

project(HELICS-find-package-test)

# Add HELICS as a subproject
add_subdirectory(../../.. helics)

# Test the CXX static library target
add_executable(static-library-test main.cpp)
target_link_libraries(static-library-test HELICS::helics-static)

# Test the CXX shared library target
if(BUILD_SHARED_LIBS)
    add_executable(shared-library-test main.cpp)
    target_link_libraries(shared-library-test HELICS::helics-shared)
endif()

# Test the C and C++98 targets
if(BUILD_C_SHARED_LIB)
    add_executable(c-shared-library-test main.c)
    target_link_libraries(c-shared-library-test HELICS::helicsSharedLib)

    add_executable(cpp98-shared-library-test main98.cpp)
    target_link_libraries(cpp98-shared-library-test HELICS::helicsCpp98)
    set_property(TARGET cpp98-shared-library-test PROPERTY CXX_STANDARD 98)
endif()

# Test the helics_app static library target
if(BUILD_APPS)
    add_executable(apps-library-test main-apps.cpp)
    target_link_libraries(apps-library-test HELICS::helics_apps)
endif()
