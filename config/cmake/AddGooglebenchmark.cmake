# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE INTERNAL "")
set(BENCHMARK_ENABLE_TESTING OFF CACHE INTERNAL "Suppressing benchmark's tests")
set(BENCHMARK_ENABLE_INSTALL OFF CACHE INTERNAL "")
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON CACHE INTERNAL "")
set(BENCHMARK_ENABLE_ASSEMBLY_TESTS OFF CACHE INTERNAL "")
set(BENCHMARK_INSTALL_DOCS OFF CACHE INTERNAL "")
# tell google benchmarks to use std regex since we only compile on compilers with std regex
set(HAVE_STD_REGEX ON CACHE INTERNAL "")
set(HAVE_POSIX_REGEX OFF CACHE INTERNAL "")
set(HAVE_GNU_POSIX_REGEX OFF CACHE INTERNAL "")

add_subdirectory(
    ${CMAKE_SOURCE_DIR}/ThirdParty/benchmark ${CMAKE_BINARY_DIR}/ThirdParty/benchmarks
    EXCLUDE_FROM_ALL
)

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
hide_variable(BENCHMARK_ENABLE_DOXYGEN)
hide_variable(BENCHMARK_ENABLE_LIBPFM)
hide_variable(BENCHMARK_ENABLE_WERROR)
hide_variable(BENCHMARK_FORCE_WERROR)
hide_variable(BENCHMARK_USE_BUNDLED_GTEST)
hide_variable(CXXFEATURECHECK_DEBUG)
hide_variable(LIBRT)

set_target_properties(benchmark benchmark_main PROPERTIES FOLDER "Extern")
target_compile_options(benchmark_main PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4244 /wd4800>)
target_compile_options(benchmark PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4244 /wd4800>)

if(MSVC)
    target_compile_definitions(benchmark PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    target_compile_definitions(benchmark_main PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()
