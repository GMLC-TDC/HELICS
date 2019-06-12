cmake_minimum_required(VERSION 3.6.0)

project(HELICS-package_config-tests)

find_package(PkgConfig)
pkg_check_modules(HELICS REQUIRED IMPORTED_TARGET helics)

# Test the CXX static library target
add_executable(static-library-test main.cpp)

target_link_libraries(static-library-test PkgConfig::HELICS)

# Test the CXX shared library target
if(BUILD_SHARED_LIBS)
	pkg_check_modules(HELICS-SHARED REQUIRED helics-shared)
    add_executable(shared-library-test main.cpp)
    target_link_libraries(shared-library-test PkgConfig::HELICS-SHARED)
endif()

# Test the C and C++98 targets
if(BUILD_C_SHARED_LIB)

    pkg_check_modules(HELICSSHAREDLIB REQUIRED helicsSharedLib)
    add_executable(c-shared-library-test main.c)
    target_link_libraries(c-shared-library-test PkgConfig::HELICSSHAREDLIB)

    add_executable(cpp98-shared-library-test main98.cpp)
    target_link_libraries(cpp98-shared-library-test PkgConfig::HELICSSHAREDLIB)
    set_property(TARGET cpp98-shared-library-test PROPERTY CXX_STANDARD 98)
endif()

# Test the helics_app static library target
if(BUILD_APPS)
    pkg_check_modules(HELICS_APPS REQUIRED helics_apps)
    add_executable(apps-library-test main-apps.cpp)
    target_link_libraries(apps-library-test PkgConfig::HELICSS_APPS)
endif()
