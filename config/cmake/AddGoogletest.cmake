# 
#
# Downloads GTest and provides a helper macro to add tests. Add make check, as well, which
# gives output on failed tests without having to set an environment variable.
#
#

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/googletest/CMakeLists.txt")
	submod_update(${HELICS_SOURCE_DIR}/ThirdParty/googletest)
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE BOOL "")
add_subdirectory("${HELICS_SOURCE_DIR}/ThirdParty/googletest" "${HELICS_BINARY_DIR}/ThirdParty/googletest" EXCLUDE_FROM_ALL)

if (NOT MSVC)
#target_Compile_options(gtest PRIVATE "-Wno-undef")
#target_Compile_options(gmock PRIVATE "-Wno-undef")
#target_Compile_options(gtest_main PRIVATE "-Wno-undef")
#target_Compile_options(gmock_main PRIVATE "-Wno-undef")
endif()

if(GOOGLE_TEST_INDIVIDUAL)
    if(NOT CMAKE_VERSION VERSION_LESS 3.9)
        include(GoogleTest)
    else()
        set(GOOGLE_TEST_INDIVIDUAL OFF)
    endif()
endif()

# Target must already exist
macro(add_gtest TESTNAME)
    target_link_libraries(${TESTNAME} PUBLIC gtest gmock gtest_main)
    
    if(GOOGLE_TEST_INDIVIDUAL)
        if(CMAKE_VERSION VERSION_LESS 3.10)
            gtest_add_tests(TARGET ${TESTNAME}
                            TEST_PREFIX "${TESTNAME}."
                            TEST_LIST TmpTestList)
            set_tests_properties(${TmpTestList} PROPERTIES FOLDER "Tests")
        else()
            gtest_discover_tests(${TESTNAME}
                TEST_PREFIX "${TESTNAME}."
                PROPERTIES FOLDER "Tests")
            
        endif()
    else()
        add_test(${TESTNAME} ${TESTNAME})
        set_target_properties(${TESTNAME} PROPERTIES FOLDER "Tests")
    endif()

endmacro()

mark_as_advanced(
gmock_build_tests
gtest_build_samples
gtest_build_tests
gtest_disable_pthreads
gtest_force_shared_crt
gtest_hide_internal_symbols
BUILD_GMOCK
BUILD_GTEST
INSTALL_GTEST
)

set_target_properties(gtest gtest_main gmock gmock_main
    PROPERTIES FOLDER "Extern")

if(MSVC)
     #  add_compile_options( /wd4459)
    if (MSVC_VERSION GREATER_EQUAL 1900)
        target_compile_definitions(gtest PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
        target_compile_definitions(gtest_main PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
        target_compile_definitions(gmock PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
        target_compile_definitions(gmock_main PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    endif()
endif()
