#!/bin/bash
tests=(
    "./bin/core-tests --gtest_filter=-*nocov*"
    "./bin/common-tests --gtest_filter=-*nocov*"
    "./bin/system-tests --gtest_filter=-*nocov*"
    "./bin/helics_apps-tests --gtest_filter=-*nocov*"
    "./bin/shared-library-tests-cpp --gtest_filter=-*death*:*nocov*"
    "./bin/shared-library-tests --gtest_filter=-*death*:*nocov*"
    "./bin/application-api-tests --gtest_filter=-*nocov*"
    "./bin/helics_webserver-tests --gtest_filter=-*nocov*"
    "./bin/network-tests --gtest_filter=-*nocov*"
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
