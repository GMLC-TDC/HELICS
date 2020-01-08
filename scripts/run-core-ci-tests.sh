#!/bin/bash
if [[ "$SET_MSYS_PATH" == "true" ]]; then
    export PATH="/c/tools/msys64/mingw64/bin:$PATH"
    echo "$PATH"
fi

ctest -R core-ci-tests --timeout 120 --output-on-failure
