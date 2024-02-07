# Brokers - Multi-computer Co-simulation

```{eval-rst}
.. toctree::
    :maxdepth: 1


```

This example shows how to configure a HELICS co-simulation to run across multiple computers.

- [Where is the code?](#where-is-the-code)
- [What is this Co-simulation doing?](#what-is-this-co-simulation-doing)
  - [Differences Compared to the Advanced Default Example](#differences-compared-to-advanced-default-example)
    - [HELICS Differences](#helics-differences)
  - [HELICS Components](#helics-components)
- [Execution and Results](#execution-and-results)

## Where is the code?

The code for the [Advanced examples](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced) can be found in the HELICS-Examples repository on GitHub. This example on [multi-computer co-simulation can be found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_brokers/multi_computer). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

[![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/advanced_broker_multicomputer_github.png?raw=true)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced)

## What is this co-simulation doing?

This example shows you how to configure a co-simulation to take advantage of multiple brokers. Though we'll be running this example on a single computer, the application of broker hierarchies is more common when running a co-simulation across multiple computers.

### Differences compared to Advanced Default example

As will be shown, the use of multiple computers to run the co-simulation will not affect the results of the co-simulation.

#### HELICS differences

Running a HELICS co-simulation across multiple computers is useful in situations when a federate is too large to fit on the same computer node as the rest of the federates, uses proprietary software that can't be installed on the same computer as the rest of the federation, or there are other limitations that require the federation to be split across multiple computers. (See the [User Guide section on broker hierarchies](../../advanced_topics/broker_multicomputer.md) for further details.)

### HELICS components

When running across multiple compute nodes, the relevant portion of the broker instantiation looks like this on the computer where broker is running:

```shell-session
"$ helics_broker --loglevel=debug --timeout='10s' --ipv4",
```

The "ipv4" flag opens up an externally accessible ports (by default, 23405) on externally facing network interfaces with an ipv4 address.

On the computer(s) where the broker is not running, each of the federates has to define the "broker_address" as part of the configuration. In this case, that's happening in a JSON configuration file and as such we just need to add a single line to the file:

```json
"broker_address": "tcp://<IP address of broker>",
```

If for whatever reason the federation need to run on a different port, this can easily be done with minor alterations. The broker instantiation would be:

```shell-session
$ helics_broker --loglevel=debug --timeout='10s' --ipv4 --port=<port number>",
```

All federates would also need to know to use the new port number. For those that have already specified the IP address of the broker, appending the port in standard networking syntax works:

```json
"brokerAddress": "tcp://<IP address>:<port number>",
```

Alternatively, the "broker_port" option can be set:

```json
"broker_port": "<port number>",
```

Note that any federates running on the same machine as the broker would also need to set this port option.

## Execution and Results

To run this example you'll need to use two computers, running one part of the federation on each. Each computer can launch its part of the federation with the following HELICS runner commands

```shell-session
$ helics run --path=./multi_computer_1_runner.json
```

```shell-session
$ helics run --path=./multi_computer_2_runner.json
```

The peak charging results are shown below and are identical to the results from the similarly configured [broker hierarchy example](advanced_brokers_hierarchies.md).

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_charging_power.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_estimated_SOCs.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_battery_SOCs.png)

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
