# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(gbenchmark_version v1.5.0)

string(TOLOWER "gbenchmark" gbName)

if(NOT CMAKE_VERSION VERSION_LESS 3.11)
    include(FetchContent)

    fetchcontent_declare(
        gbenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG ${gbenchmark_version}
    )

    fetchcontent_getproperties(gbenchmark)

    if(NOT ${gbName}_POPULATED)
        # Fetch the content using previously declared details
        fetchcontent_populate(gbenchmark)

    endif()

    hide_variable(FETCHCONTENT_SOURCE_DIR_GBENCHMARK)
    hide_variable(FETCHCONTENT_UPDATES_DISCONNECTED_GBENCHMARK)

else() # cmake <3.11

    # create the directory first
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/_deps)

    include(GitUtils)
    git_clone(
        PROJECT_NAME
        gbenchmark
        GIT_URL
        https://github.com/google/benchmark.git
        GIT_TAG
        ${gbenchmark_version}
        DIRECTORY
        ${PROJECT_BINARY_DIR}/_deps
    )

    set(${gbName}_BINARY_DIR ${PROJECT_BINARY_DIR}/_deps/${gbName}-build)

endif()

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE INTERNAL "")
set(BENCHMARK_ENABLE_TESTING OFF CACHE INTERNAL "Suppressing benchmark's tests")
set(BENCHMARK_ENABLE_INSTALL OFF CACHE INTERNAL "" )
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON CACHE INTERNAL "")
set(BENCHMARK_ENABLE_ASSEMBLY_TESTS OFF CACHE INTERNAL "")
# tell google benchmarks to use std regex since we only compile on compilers with std regex
set(HAVE_STD_REGEX ON CACHE INTERNAL "" )
set(HAVE_POSIX_REGEX OFF CACHE INTERNAL "" )
set(HAVE_GNU_POSIX_REGEX OFF CACHE INTERNAL "" )
add_subdirectory(${${gbName}_SOURCE_DIR} ${${gbName}_BINARY_DIR} EXCLUDE_FROM_ALL)

# Target must already exist
macro(add_benchmark_with_main TESTNAME)
    target_link_libraries(${TESTNAME} PUBLIC benchmark benchmark_main Threads::Threads)
    if(WIN32)
        target_link_libraries(${TESTNAME} PUBLIC shlwapi)
    endif()
    set_target_properties(${TESTNAME} PROPERTIES FOLDER "benchmarks")

endmacro()

macro(add_benchmark TESTNAME)
    target_link_libraries(${TESTNAME} PUBLIC benchmark Threads::Threads)
    if(WIN32)
        target_link_libraries(${TESTNAME} PUBLIC shlwapi)
    endif()
    set_target_properties(${TESTNAME} PROPERTIES FOLDER "benchmarks")

endmacro()

hide_variable(BENCHMARK_BUILD_32_BITS)
hide_variable(BENCHMARK_DOWNLOAD_DEPENDENCIES)
hide_variable(BENCHMARK_ENABLE_ASSEMBLY_TESTS)
hide_variable(BENCHMARK_ENABLE_EXCEPTIONS)
hide_variable(BENCHMARK_ENABLE_LTO)
hide_variable(BENCHMARK_USE_LIBCXX)
hide_variable(LIBRT)

set_target_properties(benchmark benchmark_main PROPERTIES FOLDER "Extern")
target_compile_options(benchmark_main PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4244 /wd4800>)
target_compile_options(benchmark PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4244 /wd4800>)

if(MSVC AND MSVC_VERSION GREATER_EQUAL 1900)
    target_compile_definitions(benchmark PUBLIC
                               _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    target_compile_definitions(benchmark_main PUBLIC
                               _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()
