#!/bin/bash
#SBATCH -t 0:20:00
#SBATCH -p pdebug
#SBATCH -N 3

# sbatch --export="CORE_TYPE=${ct},FEDS_PER_NODE=2,BM_FED=Phold" -N1 test.sbatch

if [[ "$1" != "" ]]; then
    BM_FED=$1
fi

numnodes_arr=("${SLURM_JOB_NUM_NODES}")
bm_arr=("${BM_FED}")

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
if [ -n "$SLURM_JOB_ID" ]; then
    sbatch_orig_file="$(scontrol show job "$SLURM_JOBID" | awk -F= '/Command=/{print $2}')"
    SCRIPT_DIR="$(cd "$(dirname "${sbatch_orig_file}")" >/dev/null 2>&1 && pwd)"
fi
export SCRIPT_DIR

# Give a way to override the name of the federate used... though things will likely break due to different benchmark federates using different arguments
if [[ "${BM_FED}" == "" ]]; then
    echo "Using default bmfeds: \"Phold\", \"Echo\", \"EchoMessage\", \"RingTransmit\", \"RingTransmitMessage\", \"Timing\""
    bm_arr=("Phold" "Echo" "EchoMessage" "RingTransmit" "RingTransmitMessage" "Timing")
fi

if [[ "${NUM_NODES}" == "" ]]; then
    echo "Using default numnodes: 2, 3, 5, 9"
    numnodes_arr=(2 3 5 9)
fi

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

srun_conf_dir="$PWD/srun-multi-prog"

output_dir="$PWD/${BM_FED}Federate-mpi-N${SLURM_JOB_NUM_NODES}-job-${SLURM_JOBID}"
mkdir -p "$output_dir"
output_pattern="$output_dir/%t.out"

# output some basic info for this benchmark run
sys_info=$("${BUILD_PATH}/bin/helics_benchmarks" --print_systeminfo)
echo "${sys_info}
DATE: $(date --universal +"%Y-%m-%d %T")
CLUSTER: ${SLURM_CLUSTER_NAME}
NUM NODES: ${SLURM_JOB_NUM_NODES}
FEDS PER NODE: 1
TOPOLOGY: ${TOPOLOGY}
MESSAGE SIZE: ${MSG_SIZE}
MESSAGE COUNT: ${MSG_COUNT}" >"${output_dir}/summary.txt"

pushd "$BUILD_PATH/bin" || exit 1
for bm in "${bm_arr[@]}"; do
    # MessageExchange is special, only 3 nodes (2 federates+broker) but then msg size + count change
    if [[ "$bm" != "MessageExchange" ]]; then
        for node_ct in "${numnodes_arr[@]}"; do
            echo "Running $bm on $node_ct nodes"
            #srun -N "${node_ct}" -ppdebug --output="$output_pattern" --multi-prog srun-mpi.conf
        done
    else
        echo "Running MessageExchange"
        srun -N3 --output="$output_pattern" --multi-prog "${srun_conf_dir}/MessageExchange-sz${MSG_SIZE}-cnt${MSG_COUNT}.conf"
    fi
done
