
set(HELICS_BENCHMARKS
    ActionMessageBenchmarks
    )

# Only affects current directory, so safe
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach(T ${HELICS_BENCHMARKS})

    add_executable(${T} ${T}.cpp )
    target_link_libraries(${T} PUBLIC helics-static)
    add_benchmark(${T})
	set_target_properties(${T} PROPERTIES FOLDER benchmarks)
   
endforeach()
