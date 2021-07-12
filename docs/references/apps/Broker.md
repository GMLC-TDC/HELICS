# Broker

Brokers function as intermediaries or roots in the HELICS hierarchy
The Broker can be run through the helics_broker or via helics-app

## Command line arguments

```text
helics_broker term <broker args...> will start a broker and open a terminal control window for the broker run help in a terminal for more commands
helics_broker --autorestart <broker args ...> will start a continually regenerating broker there is a 3 second countdown on broker completion to halt the program via ctrl-C
helics_broker <broker args ..> just starts a broker with the given args and waits for it to complete
allowed options:

command line only:
  -? [ --help ]          produce help message
  -v [ --version ]       display a version string
  --config-file arg      specify a configuration file to use

configuration:
  -n [ --name ] arg      name of the broker
  -t [ --type ] arg      type of the broker ("(zmq)", "ipc", "test", "mpi","test", "tcp", "udp")

 Help for Zero MQ Broker:
allowed options:

configuration:
  --interface arg        the local interface to use for the receive ports
  -b [ --broker ] arg    identifier for the broker
  --broker_address arg   location of the broker i.e network address
  --brokername arg       the name of the broker
  --local                use local interface(default)
  --ipv4                 use external ipv4 addresses
  --ipv6                 use external ipv6 addresses
  --external             use all external interfaces
  --brokerport arg       port number for the broker priority port
  --localport arg        port number for the local receive port
  --port arg             port number for the broker's port
  --portstart arg        starting port for automatic port definitions

 Help for Interprocess Broker:
allowed options:

configuration:
  --queueloc arg         the named location of the shared queue
  -b [ --broker ] arg    identifier for the broker
  --broker_address arg   location of the broker i.e network address
  --brokerinit arg       the initialization string for the broker

 Help for Test Broker:
allowed options:

configuration:
  --brokername arg       identifier for the broker-same as broker
  -b [ --broker ] arg    identifier for the broker
  --broker_address arg   location of the broker i.e network address
  --brokerinit arg       the initialization string for the broker

 Help for TCP Broker:
allowed options:

configuration:
  --interface arg        the local interface to use for the receive ports
  -b [ --broker ] arg    identifier for the broker
  --broker_address arg   location of the broker i.e network address
  --brokername arg       the name of the broker
  --local                use local interface(default)
  --ipv4                 use external ipv4 addresses
  --ipv6                 use external ipv6 addresses
  --external             use all external interfaces
  --brokerport arg       port number for the broker priority port
  --localport arg        port number for the local receive port
  --port arg             port number for the broker's port
  --portstart arg        starting port for automatic port definitions

 Help for UDP Broker:
allowed options:

configuration:
  --interface arg        the local interface to use for the receive ports
  -b [ --broker ] arg    identifier for the broker
  --broker_address arg   location of the broker i.e network address
  --brokername arg       the name of the broker
  --local                use local interface(default)
  --ipv4                 use external ipv4 addresses
  --ipv6                 use external ipv6 addresses
  --external             use all external interfaces
  --brokerport arg       port number for the broker priority port
  --localport arg        port number for the local receive port
  --port arg             port number for the broker's port
  --portstart arg        starting port for automatic port definitions

Broker Specific options:

configuration:
  --root                 specify whether the broker is a root

configuration:
  -n [ --name ] arg      name of the broker/core
  --federates arg        the minimum number of federates that will be
                         connecting
  --minfed arg           the minimum number of federates that will be
                         connecting
  --maxiter arg          maximum number of iterations
  --logfile arg          the file to log message to
  --loglevel arg         the level which to log the higher this is set to the
                         more gets logs (-1) for no logging
  --fileloglevel arg     the level at which messages get sent to the file
  --consoleloglevel arg  the level at which message get sent to the console
  --minbrokers arg       the minimum number of core/brokers that need to be
                         connected (ignored in cores)
  --identifier arg       name of the core/broker
  --tick arg             number of milliseconds per tick counter if there is no
                         broker communication for 2 ticks then secondary actions
                         are taken (can also be entered as a time like '10s' or '45ms')
  --dumplog              capture a record of all messages and dump a complete log to file or console on termination
  --terminate_on_error   Specify that the co-simulation should terminate if any error occurs
  --timeout arg          milliseconds to wait for a broker connection (can also
                         be entered as a time like '10s' or '45ms')

  --error_timeout arg    milliseconds to wait before disconnecting after an error
                         (can also be entered as a time like '10s' or '45ms')

```

If the Broker is started with `term` as the first option, a terminal is opened for user entry of commands all command line arguments following term are passed to the broker.

```bash
starting broker
helics>>help
`quit` -> close the terminal application and wait for broker to finish
`terminate` -> force the broker to stop
`terminate*` -> force the broker to stop and exit application
`help`,`?` -> this help display
`restart` -> restart a completed broker
`status` -> will display the current status of the broker
`info` -> will display info about the broker
`force restart` -> will force terminate a broker and restart it
`query` <queryString> -> will query a broker for <queryString>
`query` <queryTarget> <queryString> -> will query <queryTarget> for <queryString>
helics>>
```

`status` will print out current status of the brokers including counts of federates, brokers, and handles

```bash
helics>>status
Broker (643204-ibrVd-14EWH-unKfh-hExUP) is connected and is accepting new federates
{"brokers":0,
"federates":0,
"handles":0}
helics>>
```

info prints out name, connection status, and connection information

```bash
helics>>info
Broker (643204-ibrVd-14EWH-unKfh-hExUP) is connected and is accepting new federates
address=tcp://127.0.0.1:23404
```

The `query` command allows any query to be executed from the command line, `query counts` displays the same count numbers as `status`.

Other available queries are described in [Queries](../user-guide/queries.html).

various restart options are also available, `terminate`, `restart`, `force restart`. And finally `quit` will exit the terminal and wait for the broker to complete. enter `terminate` before quit or `terminate*` to terminate and quit.
