#!/bin/bash
if [[ "$SET_MSYS_PATH" == "true" ]]; then
    export PATH="/c/tools/msys64/mingw64/bin:$PATH"
    echo "$PATH"
fi

# Flag variables are left unquoted for globbing and word splitting by bash (enable them to contain multiple arguments)
echo "cmake -G \"${CMAKE_GENERATOR}\" ${JOB_OPTION_FLAGS} ${HELICS_DEPENDENCY_FLAGS} ${HELICS_OPTION_FLAGS} ${CMAKE_COMPILER_LAUNCHER} .."
cmake -G "${CMAKE_GENERATOR}" ${JOB_OPTION_FLAGS} ${HELICS_DEPENDENCY_FLAGS} ${HELICS_OPTION_FLAGS} ${CMAKE_COMPILER_LAUNCHER} ..
echo "cmake --build . -- ${MAKEFLAGS}"
cmake --build . -- ${MAKEFLAGS}
