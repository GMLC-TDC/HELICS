#!/bin/bash
tests=(
    "$1/bin/core-tests --gtest_filter=-*ci_skip*"
    "$1/bin/common-tests --gtest_filter=-*ci_skip*"
    "$1/bin/system-tests --gtest_filter=-*realtime*"
    "$1/bin/helics_apps-tests --gtest_filter=-*ci_skip*"
    "$1/bin/shared-library-tests-cpp --gtest_filter=-*death*"
    "$1/bin/shared-library-tests --gtest_filter=-*bad_input*:*evil*:*after_close*:*death*"
    "$1/bin/applicationApiTests --gtest_filter=-*ci_skip*"
    "$1/bin/filterTranslatorTests --gtest_filter=-*ci_skip*"
    "$1/bin/messageFederateTests --gtest_filter=-*ci_skip*"
    "$1/bin/valueFederateTests --gtest_filter=-*ci_skip*"
    "$1/bin/helics_webserver-tests --gtest_filter=*"
    "$1/bin/network-tests --gtest_filter=*"
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
