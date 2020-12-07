# Simultaneous Co-simulations

Sometimes it is necessary or desirable to be able to execute multiple simultaneous simulations on a single computer system. For example, there may be a need to run a large number of co-simulations as part of a sensitivity study. HELICS includes a number of different options for managing this and making it easier.

The key limitation is the network port number used to form the HELICS communication bus. Unless otherwise specified, HELICS uses a default port numbers for this bus, so only a single broker (per core type) with default options is able to run on a single compute node at a given time. There are a number of ways around this and some tools to assist in checking and coordinating.

## Specify port numbers

The most straight-forward way of working around this port number limitation is by manually specifying a given port for each co-simulation to be run. For this to to work, the port number must be defined for all co-simulation participants, including the HELICS broker. 


Broker:
```
helics_broker --type=zmq --port=20200 &
```

Federate (in JSON config file):
```json
{
  "name": "federate_name",
  "loglevel": 1,
  "coreType": "zmq",
  "brokerPort": "20100",
  ...
```

### Example

A [full co-simulation example](./examples/advanced_examples/advanced_brokers_simultaneous.md) demonstrating simultaneous co-simulations in this manner is provided in the [HELICS Examples repository](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/user_guide_examples/advanced/advanced_brokers/simultaneous).


## Use Broker server

For the zmq, zmqss, tcp, and udp core types it is possible to create HELICS co-simulations using a broker server. A broker server is a built-in HELICS application that generates a broker or use by the co-simulation on-demand. The user launches the broker server instead, specifying which core type to use, and when the co-simulation is launched, the broker_server will generate a broker on a unique port number for the federation to use and direct all the federates in the federation to use that port.

broker_server command-line launch:
```sh
helics_broker_server --zmq
```

Multiple broker servers can be run simultaneously:

broker_server command-line launch:
```sh
helics_broker_server --zmq --tcp --udp
```

The broker server has a default timeout of 30 minutes on the default port  The duration of the server can be controlled via a command-line option on launch:

```sh
helics_broker_server --zmq --duration=24hours
```


By default the servers will use the default ports and all interfaces. This can be set through a configuration file

broker_server command-line launch:
```sh
helics_broker_server --zmq --duration=24hours --config=broker_config.json
```


borker_config.json
```json
{
  "zmq": {
    "interface": "tcp://127.0.0.1"
  },
  "tcp": {
    "interface": "tcp://127.0.0.1",
    "port": 9568
  }
}
```


## Use of keys

The previous techniques have used methods to create separate brokers and network connections to segregate the traffic for each simultaneously running co-simulation. Alternatively, the use of broker keys allows traffic from multiple co-simulations to flow through the same broker yet not interfere with each other. **NOTE:** _This key is not a cryptographic key, it is just a string to mark co-simulation traffic as belonging to a specific federation._

broker_server command-line launch:
```sh
helics_broker --type=zmq --key=my_broker_key
```

Federate configuration:
```json
{
  "name": "federate_name",
  "loglevel": 1,
  "coreType": "zmq",
  "coreInitString": "--key=my_broker_key"
  ...
```


## Orchestration Tool
We would be remise to not mention that the HELICS team has been using a specific orchestration tool, [Merlin](https://github.com/LLNL/merlin), to manage the work around running multiple co-simulations. This tool allows for higher-level management of co-simulations when a particular analysis requires multiple co-simulation runs to get the final results. See the [User Guide section](./orchestration.md) on the tool to learn more.