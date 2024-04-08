#!/bin/bash
tests=(
    "$1/bin/core-tests --gtest_filter=-*ci_skip*"
    "$1/bin/common-tests --gtest_filter=-*ci_skip*"
    "$1/bin/system-tests --gtest_filter=-*ci_skip*:*realtime*"
    "$1/bin/helics_apps-tests --gtest_filter=-*ci_skip*"
    "$1/bin/shared-library-tests-cpp --gtest_filter=-*ci_skip*:*death*"
    "$1/bin/shared-library-tests --gtest_filter=-*ci_skip*:*bad_input*:*evil*:*after_close*:*death*"
    "$1/bin/applicationApiTests --gtest_filter=-*ci_skip*"
    "$1/bin/filterTranslatorTests --gtest_filter=-*ci_skip*"
    "$1/bin/messageFederateTests --gtest_filter=-*ci_skip*"
    "$1/bin/valueFederateTests --gtest_filter=-*ci_skip*"
    "$1/bin/helics_webserver-tests --gtest_filter=-*ci_skip*"
    "$1/bin/network-tests --gtest_filter=-*ci_skip*"
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
