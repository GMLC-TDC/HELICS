#!/bin/bash

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
build_bin="${repo_root}/build/bin"

tests=(
    "${build_bin}/core-tests --gtest_filter=-*ci_skip*:*nosan*"
    "${build_bin}/common-tests --gtest_filter=-*ci_skip*:*nosan*"
    "${build_bin}/system-tests --gtest_filter=-*realtime*:*nosan*:*error*:*ci_skip*"
    "${build_bin}/helics_apps-tests --gtest_filter=-*ci_skip*:*nosan*"
    "${build_bin}/shared-library-tests-cpp --gtest_filter=-*ci_skip*:*death*:*nosan*"
    "${build_bin}/shared-library-tests --gtest_filter=-*ci_skip*:*bad_input*:*evil*:*after_close*:*death*:*nosan*"
    "${build_bin}/applicationApiTests --gtest_filter=-*ci_skip*:*nosan*"
    "${build_bin}/filterTranslatorTests --gtest_filter=-*ci_skip*:*nosan*"
    "${build_bin}/messageFederateTests --gtest_filter=-*ci_skip*:*nosan*"
    "${build_bin}/valueFederateTests --gtest_filter=-*ci_skip*:*nosan*"
    "${build_bin}/helics_webserver-tests --gtest_filter=-*ci_skip*:*nosan*"
    "${build_bin}/network-tests --gtest_filter=-*ci_skip*:*nosan*"
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
