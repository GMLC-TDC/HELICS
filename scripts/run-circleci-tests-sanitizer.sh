#!/bin/bash
tests=(
    "/root/project/build/bin/core-tests --gtest_filter=-*ci_skip*:*nosan*"
    "/root/project/build/bin/common-tests --gtest_filter=-*ci_skip*:*nosan*"
    "/root/project/build/bin/system-tests --gtest_filter=-*realtime*:*nosan*:*error*:*ci_skip*"
    "/root/project/build/bin/helics_apps-tests --gtest_filter=-*ci_skip*:*nosan*"
    "/root/project/build/bin/shared-library-tests-cpp --gtest_filter=-*ci_skip*:*death*:*nosan*"
    "/root/project/build/bin/shared-library-tests --gtest_filter=-*ci_skip*:*bad_input*:*evil*:*after_close*:*death*:*nosan*"
    "/root/project/build/bin/applicationApiTests --gtest_filter=-*ci_skip*:*nosan*"
    "/root/project/build/bin/filterTranslatorTests --gtest_filter=-*ci_skip*:*nosan*"
    "/root/project/build/bin/messageFederateTests --gtest_filter=-*ci_skip*:*nosan*"
    "/root/project/build/bin/valueFederateTests --gtest_filter=-*ci_skip*:*nosan*"
    "/root/project/build/bin/helics_webserver-tests --gtest_filter=-*ci_skip*:*nosan*"
    "/root/project/build/bin/network-tests --gtest_filter=-*ci_skip*:*nosan*"
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
