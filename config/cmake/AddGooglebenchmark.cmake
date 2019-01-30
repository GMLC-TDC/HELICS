# 
#
# Downloads Google benchmark and provides a helper macro to add benchmarks. 
#
#

set(BUILD_SHARED_LIBS OFF)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/extern/benchmark" "${CMAKE_BINARY_DIR}/extern/benchmark" EXCLUDE_FROM_ALL)


# Target must already exist
macro(add_benchmark TESTNAME)
    target_link_libraries(${TESTNAME} PUBLIC benchmark benchmark_main Threads::Threads)
    if (WIN32)
		target_link_libraries(${TESTNAME} PUBLIC shlwapi)
	endif()
	set_target_properties(${TESTNAME} PROPERTIES FOLDER "benchmarks")

endmacro()

mark_as_advanced(
 BENCHMARK_BUILD_32_BITS
 BENCHMARK_DOWNLOAD_DEPENDENCIES
 BENCHMARK_ENABLE_ASSEMBLY_TESTS
 BENCHMARK_ENABLE_EXCEPTIONS
 BENCHMARK_ENABLE_GTEST_TESTS
 BENCHMARK_ENABLE_INSTALL
 BENCHMARK_ENABLE_LTO
 BENCHMARK_ENABLE_TESTING
 BENCHMARK_USE_LIBCXX
 LIBRT
)

set_target_properties(benchmark benchmark_main
    PROPERTIES FOLDER "Extern")

if(MSVC AND MSVC_VERSION GREATER_EQUAL 1900)
    target_compile_definitions(benchmark PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    target_compile_definitions(benchmark_main PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()
