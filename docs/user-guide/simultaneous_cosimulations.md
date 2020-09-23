# Simultaneous Co-simulations

Sometimes it is necessary or desirable to be able to execute multiple simultaneous simulations on a single computer system. Either for increased parallelism or from multiple users or as part of a larger coordinated execution for sensitivity analysis or uncertainty quantification. HELICS includes a number of different options for managing this and making it easier.

## General Notes

HELICS starts with some default port numbers for network communication, so only a single broker (per core type) with default options is allowed to be running on a single computer at a given time. This is the general restriction on running multiple simultaneous co-simulations. It is not allowed to have multiple default brokers running at the same time, the network ports will interfere and the co-simulation will fail.

There are a number of ways around this and some tools to assist in checking and coordinating.

## Specify port numbers

The manual approach works fine. All the network core types accept user specified port numbers. The following script will start up two brokers on separate port numbers:

```sh
helics_broker --type=zmq --port=20200 &
helics_broker --type=zmq --port=20400 &
```

Federates connecting to the broker would need to specify the `--brokerport=X` to connect with the appropriate broker. These brokers operate independently of each other. The port numbers assigned to the cores and federates can also be user assigned but if left to default will be automatically assigned by the broker and should not interfere with each other.

## Use Broker server

For the zmq, zmqss, tcp, and udp core types it is possible to use the broker server.

```sh
helics_broker_server --zmq
helics_broker_server --zmqss
helics_broker_server --tcp
helics_broker_server --udp
```

multiple broker servers can be run simultaneously

```sh
helics_broker_server --zmq --tcp --udp
```

The broker server currently has a default timeout of 30 minutes on the default port and will automatically generate brokers on separate ports and direct federates which broker to use. The duration of the server can be controlled via

```sh
helics_broker_server --zmq --duration=24hours
```

It will also generate brokers as needed so the `helics_broker` does not need to be restarted for every run.

By default the servers will use the default ports and all interfaces. This can be configured through a configuration file

```sh
helics_broker_server --zmq --duration=24hours --config=broker_config.json
```

this is a json file. The sections in the json file include the server type For example

```json
{
  "zmq": {
    "interface": "tcp://127.0.0.1"
  },
  "tcp": {
    "interface": "127.0.0.1",
    "port": 9568
  }
}
```

There is also a [webserver](./webserver.md) that can be run with the other broker servers.

## Use of keys

If there are multiple users and you want to verify that a specific broker can only be used with federates you control. It is possible to add a key to the broker that is required to be supplied with the federates to connect to the broker. **NOTE:** _this is not a cryptographic key, it is just a string that is not programmatically accessible to others._

```sh
helics_broker --type=zmq --key=my_broker_key
```

Federates then need to supply the key as part of the configuration string otherwise the broker will return an error on connection. This is like a fence that prevents some accidental interactions. The rule is that both the federate and broker must provide no key or the same key.
