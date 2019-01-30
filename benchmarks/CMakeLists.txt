if(NOT EXISTS "${HELICS_SOURCE_DIR}/extern/benchmark/CMakeLists.txt")
    message(FATAL_ERROR "You have requested tests be built, but googlebenchmark is not downloaded. Please run:
    git submodule update --init")
endif()

include(AddGooglebenchmark)

add_subdirectory(helics)

