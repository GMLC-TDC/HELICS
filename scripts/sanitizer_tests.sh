#!/bin/bash
tests=(
    tests/helics/core/core-tests
    tests/helics/common/common-tests
    "tests/helics/system_tests/system-tests --gtest_filter=-*realtime*"
    build/tests/helics/apps/helics_apps-tests
)

SUMRESULT=0
for test in "${tests[@]}"; do
    echo "${test}"
    eval "${test}"
    RESULT=$?
    echo "***Latest test result: "${RESULT}
    SUMRESULT=$((SUMRESULT + RESULT))
done
# Return 0 or a positive integer for failure
exit ${SUMRESULT}
