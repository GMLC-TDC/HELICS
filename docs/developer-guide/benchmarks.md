# HELICS Benchmarks

The HELICS repository has a few benchmarks that are intended to test various aspects of the code and record performance over time

## Baseline benchmarks

These benchmarks run on a single machine using Google Benchmarks and are intended to test various aspects of HELICS over a range of spaces applicable to a single machine.

### ActionMessage

Micro-benchmarks to test some operations concerning the serialization of the underlying message structure in HELICS

### Conversion

Micro-benchmarks to test the serialization and deserialization of common data types in HELICS

## Simulation Benchmarks

### Echo

A set of federates representing a hub and spoke model of communication for value based interfaces

### Echo_c

A set of federates representing a hub and spoke model of communication for value based interfaces using the C shared library.

### Echo Message

A set of federates representing a hub and spoke model of communication for message based interfaces

### Filter

A variant of the Echo message test that add filters to the messages

### Ring Benchmark

A ring like structure that passes a value token around a bunch of times

### Ring Message Benchmark

A ring like structure that passes a message token around a bunch of times.

### Timing Benchmark

Similar to echo but doesn't actually send any data just pure test of the timing messages

## Message Benchmarks

Benchmarks testing various aspects of the messaging structure in HELICS

### MessageLookup

Benchmarks sends messages to random federates, varying the total number of interfaces and federates.

### MessageSend

Sending messages between 2 federates varying the message size and count per timing loop.

## Standardized Tests

### PHold

A standard PHOLD benchmark varying the number of federates.

## Multinode Benchmarks

Some of the benchmarks above have multinode variants. These benchmarks will have a standalone binary for the federate used in the benchmark that can be run on each node. Any multinode benchmark run will require some setup to make it launch in your particular environment and knowing the basics for the job scheduler on your cluster will be very helpful.

Any sbatch files for multinode benchmark runs in the repository are setup for running in the pdebug queue on LC's Quartz cluster. They are unlikely to work as is on other clusters, however they should work as a starting point for other clusters using slurm. The minimum changes required are likely to involve setting the queue/partition correctly and ensuring the right bank/account for charging CPU time is used.
