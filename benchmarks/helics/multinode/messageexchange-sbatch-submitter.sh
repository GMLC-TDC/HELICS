#!/bin/bash

# by default this script submits jobs with 1, 2, 4, and 8 nodes
# if arguments are given when running the script, they are used as an array for the number of nodes to use

# mpi is not on this list because starting an mpi federation requires a different setup procedure
coretypes_arr=( "zmq" "zmqss" "tcp" "tcpss" "udp" )
msg_count_arr=( 1 4 16 64 )
# 4096 can cause errors with some core types
msg_size_arr=( 1 16 64 256 1024 2048 )

for ct in "${coretypes_arr[@]}"
do
    for msg_size in "${msg_size_arr[@]}"
    do
        for msg_count in "${msg_count_arr[@]}"
        do
            sbatch --export="CORE_TYPE=${ct},BM_FED=MessageExchangeFederate,MSG_SIZE=${msg_size},MSG_COUNT=${msg_count}" -N "2" index-msg_size-msg_count-bm.sbatch
        done
    done
done
