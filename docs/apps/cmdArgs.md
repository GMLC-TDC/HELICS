# Command Line Arguments

```text
allowed options:

command line only:
  -? [ --help ]          produce help message
  -v [ --version ]       display a version string
  --config-file arg      specify a configuration file to use

configuration:
  -n [ --name ] arg      name of the broker
  -t [ --type ] arg      type of the broker ("(zmq)", "ipc", "test", "mpi",
                         "test", "tcp", "udp")

 Help for Zero MQ Broker:

configuration:
  --interface arg        the local interface to use for the receive ports
  -b [ --broker ] arg    identifier for the broker
  --broker_address arg   location of the broker i.e network address
  --brokerport arg       port number for the broker priority port
  --localport arg        port number for the local receive port
  --port arg             port number for the broker's port
  --portstart arg        starting port for automatic port definitions

 Help for Interprocess Broker:

configuration:
  --queueloc arg         the named location of the shared queue
  -b [ --broker ] arg    identifier for the broker
  --broker_address arg   location of the broker i.e network address
  --brokerinit arg       the initialization string for the broker

 Help for Test Broker:

configuration:
  --brokername arg       identifier for the broker-same as broker
  -b [ --broker ] arg    identifier for the broker
  --broker_address arg   location of the broker i.e network address
  --brokerinit arg       the initialization string for the broker

 Help for UDP Broker:

configuration:
  --interface arg        the local interface to use for the receive ports
  -b [ --broker ] arg    identifier for the broker
  --broker_address arg   location of the broker i.e network address
  --brokerport arg       port number for the broker priority port
  --localport arg        port number for the local receive port
  --port arg             port number for the broker's port
  --portstart arg        starting port for automatic port definitions

```
