#!/bin/bash

if [[ "$1" ]]; then
    subcmd=$(tr '[:upper:]' '[:lower:]' <<<"$1")
    case "${subcmd}" in
    setup-counters)
        lcov --directory . --zerocounters >/dev/null
        lcov \
            --rc geninfo_unexecuted_blocks=1 \
            --ignore-errors gcov,mismatch,inconsistent \
            --capture \
            --initial \
            --directory . \
            --output-file coverage.base >/dev/null
        ;;
    gather-coverage-info)
        shift
        GCOV_TOOL=gcov
        while [[ $# -gt 0 ]]; do
            key="$1"
            case $key in
            -g | --gcov)
                GCOV_TOOL="$2"
                shift
                shift
                ;;
            --coveralls)
                SUBMIT_COVERALLS=true
                shift
                ;;
            *)
                shift
                ;;
            esac
        done

        # Get the coverage info from the test runs into a file
        lcov \
            --gcov-tool "$GCOV_TOOL" \
            --rc geninfo_unexecuted_blocks=1 \
            --ignore-errors gcov,mismatch,inconsistent \
            --directory . \
            --capture \
            --output-file coverage.info &>/dev/null
        # Combine the base coverage info with info from running programs
        lcov -a coverage.base -a coverage.info --output-file coverage.total >/dev/null
        # Clean-up the coverage info
        lcov \
            --remove coverage.total \
            --ignore-errors inconsistent,unused \
            'test/*' 'tests/*' 'ThirdParty/*' 'dependencies/*' '/usr/*' \
            --output-file coverage.info.cleaned >/dev/null

        # Submit coverage info to dashboard
        if [[ "$SUBMIT_COVERALLS" == "true" ]]; then
            coveralls --gcov "${GCOV_TOOL}" --lcov-file coverage.info.cleaned >/dev/null
        fi
        ;;
    *)
        echo "Usage: $0 setup-counters|gather-coverage-info"
        ;;
    esac
else
    echo "Usage: $0 setup-counters|gather-coverage-info"
fi
