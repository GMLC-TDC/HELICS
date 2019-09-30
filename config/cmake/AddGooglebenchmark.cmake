# 
#
# Downloads Google benchmark and provides a helper macro to add benchmarks. 
#
#

set(HELICS_GBENCHMARK_VERSION v1.5.0)

string(TOLOWER "gbenchmark" gbName)

if(NOT CMAKE_VERSION VERSION_LESS 3.11)
include(FetchContent)

FetchContent_Declare(
  gbenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        ${HELICS_GBENCHMARK_VERSION}
)

FetchContent_GetProperties(gbenchmark)

if(NOT ${gbName}_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(gbenchmark)

endif()
else() #cmake <3.11

# create the directory first
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/_deps)

include(GitUtils)
git_clone(
             PROJECT_NAME                    gbenchmark
             GIT_URL                         https://github.com/google/benchmark.git
             GIT_TAG                         ${HELICS_GBENCHMARK_VERSION}
			 DIRECTORY                       ${PROJECT_BINARY_DIR}/_deps
       )
	   
set(${gbName}_BINARY_DIR ${PROJECT_BINARY_DIR}/_deps/${gbName}-build)

endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(${${gbName}_SOURCE_DIR} ${${gbName}_BINARY_DIR} EXCLUDE_FROM_ALL)

# Target must already exist
macro(add_benchmark TESTNAME)
    target_link_libraries(${TESTNAME} PUBLIC benchmark benchmark_main Threads::Threads)
    if (WIN32)
		target_link_libraries(${TESTNAME} PUBLIC shlwapi)
	endif()
	set_target_properties(${TESTNAME} PROPERTIES FOLDER "benchmarks")

endmacro()

 hide_variable(BENCHMARK_BUILD_32_BITS)
  hide_variable(BENCHMARK_DOWNLOAD_DEPENDENCIES)
  hide_variable(BENCHMARK_ENABLE_ASSEMBLY_TESTS)
  hide_variable(BENCHMARK_ENABLE_EXCEPTIONS)
  hide_variable(BENCHMARK_ENABLE_GTEST_TESTS)
  hide_variable(BENCHMARK_ENABLE_INSTALL)
  hide_variable(BENCHMARK_ENABLE_LTO)
  hide_variable(BENCHMARK_ENABLE_TESTING)
  hide_variable(BENCHMARK_USE_LIBCXX)
  hide_variable(LIBRT)

set_target_properties(benchmark benchmark_main
    PROPERTIES FOLDER "Extern")

if(MSVC AND MSVC_VERSION GREATER_EQUAL 1900)
    target_compile_definitions(benchmark PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    target_compile_definitions(benchmark_main PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()
