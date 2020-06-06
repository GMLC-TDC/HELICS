# Broker Server

[Brokers](Broker.md) function as intermediaries or roots in the HELICS hierarchy
The broker server is an executable that can be used to automatically generate brokers on an as needed basis
and coordinate their control and management. It is considered experimental as version 2.2 only works with the ZMQ core type. Future versions will expand this significantly.

Future plans include expanding to all networking core types (ZMQ, ZMQSS, TCP, TCPSS, UDP, and MPI), expanding the abilities of a terminal program and making a Restful interface to the server and underlying brokers.

## Command line arguments

```text
The Broker server is a helics broker coordinator that can generate brokers on request
Usage:helics_broker_server [OPTIONS] [config]

Positionals:
  config TEXT                 load a config file for the broker server

Options:
  -h,-?,--help                Print this help message and exit
  -v,--version
  -z,--zmq                    start a broker-server for the zmq comms in helics
  --zmqss                     start a broker-server for the zmq single socket comms in helics
  -t,--tcp                    start a broker-server for the tcp comms in helics
  -u,--udp                    start a broker-server for the udp comms in helics
  --mpi                       start a broker-server for the mpi comms in helics
[Option Group: quiet]
  Options:
    --quiet                     silence most print output

helics broker server command line
helics_broker_server [OPTIONS] [SUBCOMMAND]

Options:
  -h,-?,--help                Print this help message and exit
  -v,--version
  --duration TIME=30 minutes  specify the length of time the server should run
[Option Group: quiet]
  Options:
    --quiet                     silence most print output

Subcommands:
  term                        helics_broker_server term will start a broker server and open a terminal control window for the broker server, run help in a terminal for more commands


helics_broker_server server types starts a broker with the given args and waits for it to complete


```

If the Broker_server is started with `term` as the first option, a terminal is opened for user entry of commands all command line arguments following term are passed to the broker.

```text
starting broker Server
servers started
helics-broker-server>>help
`quit` -> close the terminal application and wait for broker to finish
`terminate` -> force the broker server to stop
`terminate*` -> force the broker server to stop and all existing brokers to terminate
`help`,`?` -> this help display

helics-broker-server>>
```

more commands will be added in future releases
