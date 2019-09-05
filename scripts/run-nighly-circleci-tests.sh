#!/bin/bash
tests=(
    /root/project/build/tests/helics/core/core-tests
    /root/project/build/tests/helics/common/common-tests
    "/root/project/build/tests/helics/system_tests/system-tests --gtest_filter=-*realtime*"
	/root/project/build/tests/helics/helics_apps-tests/helics_apps-tests
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

