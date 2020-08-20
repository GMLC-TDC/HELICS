#!/bin/bash

# Args: benchmark exe, number of federates per node, starting index, output folder
# Passthrough at end of arg handling: broker, coretype, msg_size, msg_count, max_index (update max_index script to multiply SLURM_JOB_NUM_NODES by feds per node)

helics_bm_fed_exe=$1
shift
# next argument is positional; output folder prefix, minus -${idx}-out.txt
output_prefix=$1
shift
# last positional argument is starting index
start_index=$1
shift

echo "FEDS_PER_NODE=${FEDS_PER_NODE}"
echo "helics_bm_fed_exe"
echo "output_prefix"
echo "start_index"

for ((i = start_index; i < start_index + FEDS_PER_NODE; i++)); do
    echo "Running: \"${helics_bm_fed_exe}\" \"${BM_FED}\" --index=${i} \"$*\" > \"${output_prefix}-${i}-out.txt\" 2>&1 &"
    "${helics_bm_fed_exe}" "${BM_FED}" --index="${i}" "$@" >"${output_prefix}-${i}-out.txt" 2>&1 &
done

#grep -inr "$@"

# wait until all background processes are done running
wait

# the different commands srun currently launches
# --broker="${HOSTNAME}" --coretype "${CORE_TYPE}"
# --broker="${HOSTNAME}" --coretype "${CORE_TYPE}"
# --max_index="${SLURM_JOB_NUM_NODES}" --broker="${HOSTNAME}" --coretype "${CORE_TYPE}"
# --broker="${HOSTNAME}" --coretype "${CORE_TYPE}" --msg_size="${MSG_SIZE}" --msg_count="${MSG_COUNT}"
