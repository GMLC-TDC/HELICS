#!/bin/bash

# by default this script submits jobs with 1, 2, 4, and 8 nodes
# if arguments are given when running the script, they are used as an array for the number of nodes to use

msg_size_arr=(1 64 256 2048)
msg_count_arr=(1 16 64)
for msg_sz in "${msg_size_arr[@]}"; do
    for msg_cnt in "${msg_count_arr[@]}"; do
        sbatch --export="BM_FED=MessageExchange,MSG_SIZE=$msg_sz,MSG_COUNT=$msg_cnt" -N3 submit-mpi.sbatch
    done
done
