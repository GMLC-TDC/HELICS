#!/bin/bash
if [[ "$SET_MSYS_PATH" == "true" ]]; then
    export PATH="/c/tools/msys64/mingw64/bin:$PATH"
    echo "$PATH"
fi

# Flag variables are left unquoted for globbing and word splitting by bash (enable them to contain multiple arguments)
echo "cmake -G \"${CMAKE_GENERATOR}\" ${JOB_OPTION_FLAGS} ${HELICS_DEPENDENCY_FLAGS} ${HELICS_OPTION_FLAGS} ${CMAKE_COMPILER_LAUNCHER} .."
# shellcheck disable=SC2086
# some options rely on word splitting to get interpreted by cmake correctly
cmake -G "${CMAKE_GENERATOR}" ${JOB_OPTION_FLAGS} ${HELICS_DEPENDENCY_FLAGS} ${HELICS_OPTION_FLAGS} ${CMAKE_COMPILER_LAUNCHER} ..
echo "cmake --build . -- ${MAKEFLAGS}"
# shellcheck disable=SC2086
# some options rely on word splitting to get interpreted by cmake correctly
cmake --build . -- ${MAKEFLAGS}
