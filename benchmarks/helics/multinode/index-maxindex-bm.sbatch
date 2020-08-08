#!/bin/bash
#SBATCH -t 1:00:00
#SBATCH -p pbatch
#SBATCH -N 1

# the sbatch arguments set above can be changed by passing the same argument to the sbatch command (e.g. -N100 to use 100 nodes instead of 1)
# see the https://slurm.schedmd.com/ website for docs

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
if [ -n "$SLURM_JOB_ID" ]; then
    sbatch_orig_file="$(scontrol show job "$SLURM_JOBID" | awk -F= '/Command=/{print $2}')"
    SCRIPT_DIR="$(cd "$(dirname "${sbatch_orig_file}")" >/dev/null 2>&1 && pwd)"
fi
echo "sbatch script location: $SCRIPT_DIR"

# Give a way to override the name of the federate used... though things will likely break due to different benchmark federates using different arguments
if [[ "${BM_FED}" == "" ]]; then
    BM_FED="Phold"
fi

# set coretype to other cores using --export=CORE_TYPE='<type>' argument to sbatch
if [[ "${CORE_TYPE}" == "" ]]; then
    CORE_TYPE="zmq"
fi
echo "Running ${BM_FED} benchmark on ${SLURM_JOB_NUM_NODES} nodes with core type ${CORE_TYPE}"

# set BUILD_PATH using sbatch argument --export=BUILD_PATH='<path>'
# default path is to a folder named build in the root of the git repository
if [[ "$BUILD_PATH" == "" ]]; then
    # if the build was in a git repository, use it as a hint for where the build folder could be located
    git_toplevel="$(git rev-parse --show-toplevel)"
    rv=$?
    if [[ "$rv" == "0" ]]; then
        BUILD_PATH="${git_toplevel}/build"
    else
        # guess at where the build path is
        BUILD_PATH="$PWD/../../../build"
    fi
fi

if [[ "${helics_broker_exe}" == "" ]]; then
    helics_broker_exe="${BUILD_PATH}/bin/helics_broker"
fi
command -v "${helics_broker_exe}" || {
    echo >&2 "unable to find helics broker at ${helics_broker_exe}; check for working build and set BUILD_PATH if needed"
    exit 1
}

if [[ "${helics_bm_fed_exe}" == "" ]]; then
    helics_bm_fed_exe="${BUILD_PATH}/bin/helics_benchmarks"
fi
command -v "${helics_bm_fed_exe}" || {
    echo >&2 "unable to find benchmark federate at ${helics_bm_fed_exe}; check for working build and set BUILD_PATH if needed"
    exit 1
}

# create an output folder for results
output_folder="${BM_FED}Federate-${CORE_TYPE}-N${SLURM_JOB_NUM_NODES}-job-${SLURM_JOB_ID}"
mkdir -p "${output_folder}"

# output some basic info for this benchmark run
sys_info=$("${helics_bm_fed_exe}" --print_systeminfo)
echo "${sys_info}
DATE: $(date --universal +"%Y-%m-%d %T")
CLUSTER: ${SLURM_CLUSTER_NAME}
NUM NODES: ${SLURM_JOB_NUM_NODES}
FEDS PER NODE: ${FEDS_PER_NODE}
TOPOLOGY:  ${TOPOLOGY}" >"${output_folder}/summary.txt"

# Run info and srun job launch
echo "Running broker on $HOSTNAME"
num_feds=$((SLURM_JOB_NUM_NODES * FEDS_PER_NODE))
"${BUILD_PATH}/bin/helics_broker" --all -f "${num_feds}" --coretype "${CORE_TYPE}" --loglevel=5 >"${output_folder}/helics-broker-out.txt" 2>&1 &

# give some time for the broker to start up
sleep 5

for i in $(seq 1 "${SLURM_JOB_NUM_NODES}"); do
    ((idx = i - 1))
    srun -N1 "$SCRIPT_DIR/launch_node_federates.sh" "${helics_bm_fed_exe}" "${output_folder}/${BM_FED}Federate" $((idx * FEDS_PER_NODE)) --max_index="${num_feds}" --broker="${HOSTNAME}" --coretype "${CORE_TYPE}" &
done

wait
echo 'Done'
