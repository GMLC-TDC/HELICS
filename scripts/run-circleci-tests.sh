#!/bin/bash
tests=(
    /root/project/build/bin/core-tests
    /root/project/build/bin/common-tests
    "/root/project/build/bin/system-tests --gtest_filter=-*realtime*"
    /root/project/build/bin/helics_apps-tests
    "/root/project/build/bin/shared-library-tests-cpp --gtest_filter=-*death*"
    "/root/project/build/bin/shared-library-tests --gtest_filter=-*bad_input*:*evil*:*after_close*:*death*"
    "/root/project/build/bin/application-api-tests --gtest_filter=-*ci_skip*"
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
