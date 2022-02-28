#!/bin/bash

if [[ ! -z "${INPUT_CMAKE_GEN}" ]]; then
    CMAKE_GEN="${INPUT_CMAKE_GEN}"
    export CMAKE_GEN
fi

/hbb/activate-exec "${INPUT_SCRIPT}"
