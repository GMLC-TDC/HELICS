# 
#
# Downloads GTest and provides a helper macro to add tests. Add make check, as well, which
# gives output on failed tests without having to set an environment variable.
#
#

set(HELICS_GTEST_VERSION dc1ca9ae4c206434e450ed4ff535ca7c20c79e3c)
#depending on what the version is set to the git_clone command may need to change to GIT_TAG||GIT_BRANCH|GIT_COMMIT

string(TOLOWER "googletest" gtName)

if(NOT CMAKE_VERSION VERSION_LESS 3.11)
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        ${HELICS_GTEST_VERSION}
)

FetchContent_GetProperties(googletest)

if(NOT ${gtName}_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(googletest)

endif()
else() #cmake <3.11

# create the directory first
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/_deps)

include(GitUtils)
git_clone(
             PROJECT_NAME                    googletest
             GIT_URL                         https://github.com/google/googletest.git
             GIT_COMMIT                         ${HELICS_GTEST_VERSION}
			 DIRECTORY                       ${PROJECT_BINARY_DIR}/_deps
       )
	   
set(${gtName}_BINARY_DIR ${PROJECT_BINARY_DIR}/_deps/${gtName}-build)

endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE BOOL "")
add_subdirectory(${${gtName}_SOURCE_DIR} ${${gtName}_BINARY_DIR} EXCLUDE_FROM_ALL)

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

HIDE_VARIABLE(gmock_build_tests)
HIDE_VARIABLE(gtest_build_samples)
HIDE_VARIABLE(gtest_build_tests)
HIDE_VARIABLE(gtest_disable_pthreads)
HIDE_VARIABLE(gtest_force_shared_crt)
HIDE_VARIABLE(gtest_hide_internal_symbols)
HIDE_VARIABLE(BUILD_GMOCK)
HIDE_VARIABLE(BUILD_GTEST)
HIDE_VARIABLE(INSTALL_GTEST)


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
