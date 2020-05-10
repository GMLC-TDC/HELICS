# Multi Broker

Starting in HELICS 2.5 there is a Multi Broker type that allows connection with multiple communication types simultaneously. The multibroker allows an unlimited number of communication operations to interact.

## Starting a multiBroker

A multibroker can be started as BrokerApp or a helics_broker. For the HELICS Broker the configuration must given as a file since each of the core types linked must be configured independently. Using the helics_broker the startup commands would look something like

```sh
helics_broker --type multi --config=helics_mb_config.json --name=broker1
```

A couple example configurations follow.

```json
{
  "master": {
    "type": "test"
  },
  "comms": [
    {
      "type": "zmq",
      "interfaceport": 23410
    },
    {
      "type": "zmq",
      "interfaceport": 23700
    }
  ]
}
```

The primary communication pathway can be specified in a master object or on the root of the configuration file.

```json
{
  "type": "test",
  "comms": [
    {
      "type": "zmq"
    },
    {
      "type": "tcp"
    }
  ]
}
```

Master comm information can also be given through the command line. The master comm is the only on in which higher level broker information may be specified. Any broker related specification in the comms sections will result in an error. If the MultiBroker is intended to be a root broker then no master section is required. Multiple network communication pathways of the same type are allowed assuming they use different ports.

Programmatically multibrokers can also be started using the BrokerApp and giving it the type `helics::core_type::MULTI` for arguments to the multibroker the type of the master comm can be specified on the command line arguments as well.

## Limitations

- Using TCPSS comms in the multibroker does not currently support outgoing connections like a full TCPSS broker would. This will likely be fixed in upcoming releases.
- Configuration files must currently be in JSON, in a few limited cases TOML files may work, but configuration of multiple comms in a toml file will not work. This will also likely be fixed in upcoming releases.
- General support for multibrokers is not provided in the webServer due to limitations on the configuration files. Some mechanism for this will be allowed in a future release.
