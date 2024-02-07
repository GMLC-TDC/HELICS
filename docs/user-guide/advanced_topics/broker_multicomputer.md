# Multi-Computer Co-simulations

Though often it may make sense to put a HELICS broker on every compute node used in the co-simulation (as shown in the [Broker Hierarchy example](./broker_hierarchies.md)), particularly for small co-simulations that for various reasons may not fit on a single computer, it may only be necessary to use a single broker on one computer and point the federates on the other computer(s) towards it.

## Broker Configuration

Generally, there are a few changes that will be necessary for running a multi-computer co-simulation:

- Adding the `--ipv4` flag to the broker initialization string. This opens an external port for federates on other computers to use when connecting to the broker; something like `helics_broker -f 3 --loglevel=warning --ipv4` (NOTE: --ipv4 is a shortcut to open up the ports on all external network interfaces with ipv4 addresses, other options can do similar things on specific interfaces or with ipv6).
- Define `broker_address` for each of the federates that are running on another computer. This will look something like `"broker_address": "tcp://10.211.55.23"` if using a JSON configuration file.
- The default port for the HELICS broker is 23405 and if that works in your networking environment then you don't need to do anything. To use another port, each federate must change the value for `broker_port` (_e.g._ `"broker_port": "23500"`)and the broker must set its own `local_port` option to the same value (_e.g._ `helics_broker -f 3 --loglevel=warning --ipv4 --port=23500`).

Which is not to say there can't be other networking complications. Once running on multiple computers the network configuration and configuration can create new challenges. Handling these is beyond the scope of this document but take a look at the some of the other examples to get an idea of how you might be able to handle this. There's also the [Configuration Options Reference](../../references/configuration_options_reference.md) that has a more comprehensive list of the [network configurations available](../../references/configuration_options_reference.md#network).

For those that are doing configuration via APIs, the "broker_address" and "broker_port" options can be included as part of the ["core_init_string"](../../references/configuration_options_reference.md#core_init_string---i-null) for the federates and the ["broker_init_string"](../../references/configuration_options_reference.md#broker_init_string--null) if instantiating the broker via APIs.

## Example

A full co-simulation example showing how to implement a multi-computer co-simulation [has been documented over here](../examples/advanced_examples/advanced_brokers_multicomputer.md) (and the source code is in the [HELICS Examples repository](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_brokers/multi_computer)). Note that this example will require adjusting the `broker_address` option in the Charger and Controller federate and, obviously, will require more than one computer (or virtual machine) to run.
