#!/bin/bash

if [[ -n "${INPUT_CPACK_GEN}" ]]; then
    CPACK_GEN="${INPUT_CPACK_GEN}"
    export CPACK_GEN
fi

# Enable GCC 14 if available (commonly found in manylinux images)
if [ -f /opt/rh/gcc-toolset-14/enable ]; then
    source /opt/rh/gcc-toolset-14/enable
fi

# Mark the workspace as safe for git
git config --global --add safe.directory /github/workspace

# Execute the script
exec "${INPUT_SCRIPT}"
