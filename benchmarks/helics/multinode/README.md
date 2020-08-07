# Multi-node Benchmarks

This is a set of bash scripts and SLURM sbatch files for running multi-node HELICS benchmarks on clusters that use SLURM for scheduling jobs.

For running on the Quartz cluster at LLNL, and other clusters that use `pbatch` as the partition name that allow scheduling jobs with a time of up to 1 hour and 8 nodes, these scripts should work with the default settings. Otherwise, it _will_ be necessary to go in and edit the default partition/times in the sbatch files, and alter the submitter bash scripts to use a number of nodes that is suitable for your cluster.

Files are output from the directory that the job was submitted in. For the least painful experience with finding executables, it's best to run it from inside this folder of a git checkout, and a build folder in the root of the git repository called `build`.

Some clusters may require a `--exclusive` argument getting added to the srun/sbatch commands in the scripts.

## \*-sbatch-submitter.sh Files

The submitter bash scripts take care of submitting multiple jobs with different parameters (number of nodes, federates per node, etc) to SLURM quickly.

Variables of interest for controlling what jobs get submitted include: (note that the number of jobs submitted will increase very quickly as entries are added to array variables):

- `coretypes_arr` - a list of the core types to submit benchmark jobs for
- `numnodes_arr` - a list of the number of nodes to run submitted benchmark jobs on
- `fedcount_arr` - a list of the number of federates to run per node in submitted benchmark jobs
- `msg_count_arr` - the number of messages to send for the MessageExchange benchmark
- `msg_size_arr` - the size of messages to send for the MessageExchange benchmark

Two things to note: not every benchmark submitter supports all of these variables, and the number of jobs submitted will increase very quickly as entries are added to array variables.

## index\*-bm.sbatch files

The sbatch files are the script that runs when a SLURM allocates resources to a job that's been waiting in the queue. They take care of launching the broker and federates on the nodes given to the job.

The main things of interest in these files to change are the SBATCH directives at the top of the file. They are just SLURM command line arguments, see the SLURM documentation for details.

```shell
#SBATCH -t 1:00:00
#SBATCH -p pbatch
```

`-t` is the estimated time the benchmark needs to run. 1 hour is a big overestimate for most benchmark runs smaller than 100 or so federates, changing this to something smaller may make so that the job gets scheduled and run sooner if SLURM sees an opportunity to schedule a quick job.

`-p` is the partition of nodes to use for running the job. _This is the most likely thing that will need changing to run benchmarks on a cluster other than Quartz or LC._

## launch_node_federates.sh

This is a helper script used by most of the sbatch launching scripts to ensure the right number of federates get started on a single node. It should not require any tweaks to get working on other clusters.
