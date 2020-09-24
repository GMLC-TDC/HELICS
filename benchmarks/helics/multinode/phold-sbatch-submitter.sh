#!/bin/bash

# by default this script submits jobs on 1, 2, 4, and 8 nodes
# if arguments are given when running the script, they are used as an array for the number of nodes to use

# mpi is not on this list because starting an mpi federation requires a different setup procedure
numnodes_arr=(1 2 4 8)
coretypes_arr=("zmq" "zmqss" "tcp" "tcpss" "udp")
fedcount_arr=(1 2 4 16)
topology="single_broker"

if [ "$#" -ne 0 ]; then
    numnodes_arr=("$@")
fi

for ct in "${coretypes_arr[@]}"; do
    for numnodes in "${numnodes_arr[@]}"; do
        for fedcount in "${fedcount_arr[@]}"; do
            sbatch --export="CORE_TYPE=${ct},BM_FED=Phold,FEDS_PER_NODE=${fedcount},TOPOLOGY=${topology}" -N "${numnodes}" index-maxindex-bm.sbatch
        done
    done
done
