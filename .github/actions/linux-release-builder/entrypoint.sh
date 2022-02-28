#!/bin/bash

if [[ -n "${INPUT_CPACK_GEN}" ]]; then
    CPACK_GEN="${INPUT_CPACK_GEN}"
    export CPACK_GEN
fi

/hbb/activate-exec "${INPUT_SCRIPT}"
