#!/bin/bash

if [[ "$SET_MSYS_PATH" == "true" ]]; then
    export PATH="/c/tools/msys64/mingw64/bin:$PATH"
    echo "$PATH"
fi

test_match_regex='.*'
test_exclude_regex=''
while [[ $# -gt 0 ]]; do
    i="$1"
    case $i in
    --valgrind)
        echo "Running Valgrind tests"
        RUN_VALGRIND=true
        ;;
    --cachegrind)
        echo "Running cachegrind tests"
        # shellcheck disable=SC2034
        # this option is currently disabled
        RUN_CACHEGRIND=true
        ;;
    --asan)
        echo "Tests using address sanitizer"
        export ASAN_OPTIONS=detect_leaks=0
        export LSAN_OPTIONS=verbosity=1:log_threads=1
        ;;
    --msan)
        echo "Tests using memory sanitizer"
        ;;
    --tsan)
        echo "Tests using thread sanitizer"
        CTEST_OPTIONS+=" --verbose"
        export TSAN_OPTIONS=second_deadlock_stack=1
        ;;
    --ubsan)
        echo "Tests using undefined behavior sanitizer"
        export UBSAN_OPTIONS=print_stacktrace=1
        CTEST_OPTIONS+=" --verbose"
        ;;
    --no-ctest)
        echo "Disable tests using ctest as a runner"
        NO_CTEST=true
        ;;
    --disable-unit-tests)
        DISABLE_UNIT_TESTS=true
        ;;
    --ctest-xml-output)
        CTEST_OPTIONS+=" -T Test"
        ;;
    --ctest-verbose)
        CTEST_OPTIONS+=" --verbose"
        ;;
    --match-tests)
        test_match_regex="$2"
        shift # get past the value
        ;;
    --exclude-tests)
        test_exclude_regex="$2"
        shift # get past the value
        ;;
    *)
        TEST_CONFIG=$i
        TEST_CONFIG_GIVEN=true
        ;;
    esac
    shift # get past the argument
done

#if [[ "$RUN_CACHEGRIND" == "true" ]]; then
#valgrind --track-origins=yes --tool=cachegrind <cmd>
#fi

if [[ "$NO_CTEST" == "true" ]]; then
    echo "CTest disabled, full set of CI tests may not run"
    if [[ "$RUN_VALGRIND" == "true" ]]; then
        echo "-- Valgrind will not run"
    fi

    # LSan doesn't like being run under CTest; running a single test case instead of hardcoding commands for all unit tests
    # ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=verbosity=1:log_threads=1 <cmd>
else
    # Include releasetest or coveragetest in the branch name to run a longer set of tests
    export CTEST_OUTPUT_ON_FAILURE=true

    if [[ "$TEST_CONFIG_GIVEN" == "true" ]]; then
        test_label=$(tr '[:upper:]' '[:lower:]' <<<"$TEST_CONFIG")
        case "${test_label}" in
        # Recognize aliases/case-insensitive versions of some values for TEST_CONFIG
        *daily*)
            TEST_CONFIG="Daily"
            ;;
        *coverage*)
            TEST_CONFIG="Coverage"
            ;;
        *continuous*)
            TEST_CONFIG="Continuous"
            CTEST_OPTIONS+=" --timeout 600"
            ;;
        *ci*)
            TEST_CONFIG="Continuous"
            CTEST_OPTIONS+=" --timeout 600"
            ;;
        *)
            # Use whatever user gave for TEST_CONFIG
            ;;
        esac

    # If no argument was given, but we are running on GHA that defines the CI environment variable,
    # check the branch ref for tests to run
    elif [[ "$CI" == "true" ]]; then
        case "${GITHUB_REF}" in
        *coveragetest*)
            TEST_CONFIG="Coverage"
            ;;
        *dailytest*)
            TEST_CONFIG="Daily"
            ;;
        *)
            TEST_CONFIG="Continuous"
            CTEST_OPTIONS+=" --timeout 600"
            ;;
        esac
    fi

    if [[ "$RUN_VALGRIND" == "true" ]]; then
        echo "Running Valgrind tests"
        ctest -T memcheck -L Valgrind && cat Testing/Temporary/MemoryChecker*.log
    fi

    # Run the CI tests last so that the execution status is used for the pass/fail status shown
    if [[ "$DISABLE_UNIT_TESTS" != "true" ]]; then
        if [[ -n "${TEST_CONFIG}" ]]; then
            echo "Running ${TEST_CONFIG} tests with filters exclude ${test_exclude_regex} and ${test_match_regex}"
            # shellcheck disable=SC2086
            # some options depend on word splitting to get passed to ctest correctly
            ctest -E "${test_exclude_regex}" -R "${test_match_regex}" -L ${TEST_CONFIG} ${CTEST_OPTIONS}
        else
            echo "Running tests matching ${test_match_regex} but not matching ${test_exclude_regex}"
            # shellcheck disable=SC2086
            # some options depend on word splitting to get passed to ctest correctly
            ctest ${CTEST_OPTIONS} -E "${test_exclude_regex}" -R "${test_match_regex}"
        fi
    fi
fi
