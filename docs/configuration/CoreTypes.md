# Core Types

There are several different core/broker types available in HELICS
These can be used in different circumstances depending on the platform and system desires

## Test

The Test core functions in a single process, and works through inter-thread communications.
Its primary purpose is to test communication patterns and algorithms. However, in situations
where all federates can be run in a single process it is probably the fastest and easiest to setup, and it is fully operational.

## Interprocess

The Interprocess core uses memory mapped files to transfer data, In some circumstances it can be faster than the other cores
It can only be used inside a single shared memory platform. It also has some limitations on Message sizes. It does not support
multi tiered brokers.

## ZMQ

The ZMQ is the primary core to use for multi-machine systems. It uses the
[ZMQ](https://zeromq.org) mechanisms. Internally it makes use of the REQ/REP mechanics for priority
communications and PUSH/PULL for non-priority communication messages.

## ZMQ_SS

The ZMQ_SS core was developed to minimize the number of sockets in use to support very high federate counts on a single machine. It uses the DEALER/ROUTER mechanics instead of PUSH/PULL

## UDP

UDP communications sends IP messages. UDP communication is not guaranteed or ordered, but may be faster in cases with highly reliable networking
Its primary use is for performance testing. The UDP core uses asio for networking

## TCP

TCP communications is an alternative to ZMQ on platforms where ZMQ is not available, performance comparisons have not been done, so it is unclear as to the relative performance differences
between TCP, UDP, and ZMQ. It uses the asio library for networking

## TCP_SS

The TCP_SS core is targeted at firewall applications to allow the outgoing connections to be made from the cores or brokers and have only a single external socket exposed

## MPI

MPI communications is often used in HPC systems. It uses the message passing interface to communicate between nodes in an
HPC system. It is still in testing and over time there is expected to be a few
different levels of the MPI core used in different platforms depending on MPI versions available and federation needs.
