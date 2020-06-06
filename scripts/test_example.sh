#!/bin/bash

# Take the path to the helics broker as a script argument
# Set VERBOSE env var to output stdout from the federates
if [[ -z $1 || -z $2 ]]; then
    echo "Usage: $0 timeout_length [--broker helics_broker] helics_fed1 [--args ARGS] helics_fed2 ..."
    exit 1
fi
timeout_len=$1
shift

# Function to run example and a broker
function launch_federation_with_broker() {
    # Create a temporary file for broker output
    local broker_output
    broker_output=$(mktemp)
    local broker_returncode
    broker_returncode=$(mktemp)

    local helics_broker=$1
    shift
    local num_federates=${#@}

    # To support arguments for federates, decrement num_federates by 2x for each time '--args' appears in "$@"
    for arg in "$@"; do
        if [[ "${arg}" == "--args" ]]; then
            ((num_federates = num_federates - 2))
        fi
    done

    # Launch broker in the background with the number of federates
    if [[ ${VERBOSE+x} ]]; then
        echo "Launching ${helics_broker} with ${num_federates} federate(s)"
    fi
    {
        echo "${helics_broker}" >"${broker_output}"
        timeout "${timeout_len}" "${helics_broker}" "-f${num_federates}" >>"${broker_output}"
        echo "${helics_broker} exitcode:$?" >"${broker_returncode}"
    } &

    # Store the pid of the broker
    broker_pid=$!
    launch_federation "$@"

    # Wait for the broker to finish running (until timeout is reached)
    wait ${broker_pid}
    if grep -q 124 "${broker_returncode}"; then
        if [[ ${VERBOSE+x} ]]; then
            echo "ERROR Broker exceeded timeout ($(cat "$broker_returncode"))"
        fi
        test_exit_status=1
    fi

    if [[ "$VERBOSE" == "all" ]]; then
        echo "-----Broker Output-----"
        cat "${broker_output}"
    fi
    rm "${broker_output}"
    rm "${broker_returncode}"
}

# Function to launch the example federation
function launch_federation() {
    local outputfiles=()
    local returncodes=()

    # Parse the function arguments for federate command and arguments
    local federates=()
    local fed_cmd=""
    local isarg=false
    for arg in "$@"; do

        if [[ "${fed_cmd}" != "" ]]; then
            if [[ "${isarg}" == "true" ]]; then
                # Current argument is an argument that goes with a command
                federates+=("${fed_cmd} ${arg}")
                # Reset processing to next argument being a command
                isarg=false
                fed_cmd=""
            elif [[ "${arg}" != "--args" ]]; then
                # Current argument is another command, previous command had no arguments
                federates+=("${fed_cmd}")
                fed_cmd="${arg}"
            else
                # Next argument is the argument for the previous command
                isarg=true
            fi
        else
            # Current argument is a command
            fed_cmd="${arg}"
        fi
    done
    # Handle the case of the last argument being a command without arguments
    if [[ "${fed_cmd}" != "" ]]; then
        federates+=("${fed_cmd}")
    fi

    # Launch federates
    for fed in "${federates[@]}"; do
        # Create temp file for federate output
        output_file=$(mktemp)
        returncode_file=$(mktemp)
        outputfiles+=("${output_file}")
        returncodes+=("${returncode_file}")
        # Launch federate
        if [[ ${VERBOSE+x} ]]; then
            echo "Launching ${fed}"
        fi
        {
            echo "${fed}" >"${output_file}"
            timeout "${timeout_len}" "${fed}" >>"${output_file}"
            echo "${fed} exitcode:$?" >"${returncode_file}"
        } &
        pids+=($!)
    done

    # Wait for federates to end -- up until max timeout
    wait "${pids[@]}"

    # Cleanup return code files, check for non-zero return codes
    for returncode_file in "${returncodes[@]}"; do
        if grep -q 124 "${returncode_file}"; then
            if [[ ${VERBOSE+x} ]]; then
                echo "ERROR Federate exceeded timeout ($(cat "$returncode_file)")"
            fi
            test_exit_status=1
        fi
        rm "${returncode_file}"
    done

    # Cleanup output files
    for output_file in "${outputfiles[@]}"; do
        if [[ "$VERBOSE" == "all" ]]; then
            echo "-----Federate Output-----"
            cat "$output_file"
        fi

        # Check for "key output" indicating failure
        if grep -inr error "${output_file}"; then
            if [[ ${VERBOSE+x} ]]; then
                echo "ERROR Problem occurred during federate execution"
            fi
            test_exit_status=1
        fi
        rm "${output_file}"
    done
}

test_exit_status=0

# Shift earlier removed timeout argument, so 1 is now --broker or the first federate executable
if [[ "$1" == "--broker" ]]; then
    shift
    launch_federation_with_broker "$@"
else
    launch_federation "$@"
fi

# Output ok/not ok followed by a string provided by a test runner for TAP format output
if [[ ${TAP_OUTPUT+x} ]]; then
    if [[ "${test_exit_status}" == "0" ]]; then
        echo "ok ${TAP_OUTPUT}"
    else
        echo "not ok ${TAP_OUTPUT}"
    fi
fi

exit ${test_exit_status}
