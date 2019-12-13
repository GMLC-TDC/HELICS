#!/bin/bash
tests=(
    /root/project/build/tests/helics/core/core-tests
    /root/project/build/tests/helics/common/common-tests
    "/root/project/build/tests/helics/system_tests/system-tests --gtest_filter=-*realtime*"
    /root/project/build/tests/helics/apps/helics_apps-tests
    /root/project/build/tests/helics/shared_library/shared-library-tests-cpp
    "/root/project/build/tests/helics/shared_library/shared-library-tests --gtest_filter=-*bad_input*:*evil*"
    "/root/project/build/tests/helics/application_api/application-api-tests --gtest_filter=-*ci_skip*"
)

SUMRESULT=0
for test in "${tests[@]}"; do
    echo "${test}"
    eval "${test}"
    RESULT=$?
    echo "***Latest test result: "${RESULT}
    SUMRESULT=$(( SUMRESULT + RESULT ))
done
# Return 0 or a positive integer for failure
exit ${SUMRESULT}

