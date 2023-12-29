# Core Types

HELICS leverages a number of underlying communication technologies to achieve the coordination between the co-simulation components. Some of these technologies are designed to be general in nature (such as [ZeroMQ](https://zeromq.org)) and others are designed for particular situations (such as [MPI](https://hpc-wiki.info/hpc/MPI) in HPC contexts).

There are several different core/broker types available in HELICS, each providing advantages in particular circumstances depending on the architecture of the federation and the underlying computation and network environment on which it is executing.

Generally speaking, the performance of the various cores is as follows (from best to worst)

1. MPI
2. IPC
3. UDP
4. TCP
5. ZMQ

## Test

The Test core functions in a single process, and works through inter-thread communications. It's primary purpose is to test communication patterns and algorithms. However, in situations where all federates can be run in a single process it is probably the fastest and easiest to setup.

## Interprocess (IPC)

The Interprocess core leverages Boost's interprocess communication (a part of the HELICS library) and uses memory-mapped files to transfer data rather than the network stack; in some circumstances it can be faster than the other cores. It can only be used inside a single, shared-memory compute environment (generally a single compute node). It also has some limitations on message sizes. It does not support multi-tiered brokers.

## ZMQ

The ZMQ is the default core type and provides effective and robust communication for federations spread across multiple compute nodes. It uses the [ZMQ](https://zeromq.org) mechanisms. Internally, it makes use of the REQ/REP mechanics for priority communications (such as [queries](./queries.md)) and PUSH/PULL for non-priority communication messages.

## ZMQ_SS

The ZMQ_SS core also uses ZMQ for the underlying messaging technology but was developed to minimize the number of sockets in use, supporting very high federate counts on a single machine. It uses the DEALER/ROUTER mechanics instead of PUSH/PULL

## UDP

UDP cores sends IP messages and caries with it the traditional limitation of UDP messaging: no guaranteed delivery or order of received messages. It may be faster in cases with highly reliable networking. It's primary use is for performance testing and the UDP core uses [asio](https://think-async.com/Asio/) for networking.

## TCP

TCP communications is an alternative to ZMQ on platforms where ZMQ is not available. Since the ZMQ messaging bus is built on-top of TCP it is expected that TCP provides higher performance than ZMQ. Performance comparisons have not been done, so it is unclear as to the relative performance differences between TCP, UDP, and ZMQ. It uses the [asio](https://think-async.com/Asio/) library for networking

## TCP_SS

The TCP_SS core uses TCP as the underlying messaging technology and is targeted at at networking environments where it is convenient or required that outgoing connections be made from the cores or brokers but have only a single external socket exposed.

## MPI

MPI communications is often used in HPC systems. It uses the message passing interface to communicate between nodes in an HPC system. It is still in testing and over time there is expected to be a few different levels of the MPI core used in different platforms depending on MPI versions available and federation needs.

## Example

Generally, all federates in a federation utilize the same core type. There could be reasons for this not to be the case, though. For example, part of the federation could be running on MPI in an HPC environment while the rest is running on one or more compute nodes outside that environment. To allow the federates in the HPC environment to take advantage of the high-speed MPI bus but to allow the rest of the federation without access to MPI to use ZMQ, a "multi-broker" or "multi-protocol broker" must be set up.

A full co-simulation example showing how to implement a multi-core federation [is written up here](../examples/advanced_examples/advanced_brokers_multibroker.md) (and the source code can be found [HELICS Examples repository](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_brokers/multi_broker)).
