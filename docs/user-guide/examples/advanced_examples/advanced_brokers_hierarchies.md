# Brokers - Hierarchies

```{eval-rst}
.. toctree::
    :maxdepth: 1


```

This example shows how to configure a HELICS co-simulation to allow the use of multiple brokers in a single co-simulation.

- [Where is the code?](#where-is-the-code)
- [What is this Co-simulation doing?](#what-is-this-co-simulation-doing)
  - [Differences Compared to the Advanced Default Example](#differences-compared-to-advanced-default-example)
    - [HELICS Differences](#helics-differences)
  - [HELICS Components](#helics-components)
- [Execution and Results](#execution-and-results)

## Where is the code?

The code for the [Advanced examples](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced) can be found in the HELICS-Examples repository on GitHub. This example on [broker hierarchies can be found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_brokers/hierarchies). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_broker_hierarchies_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced)

## What is this co-simulation doing?

This example shows you how to configure a co-simulation to take advantage of multiple brokers. Though we'll be running this example on a single computer, the application of broker hierarchies is more common when running a co-simulation across multiple computers.

### Differences compared to Advanced Default example

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/broker_hierarchy_example.png)

As will be shown, the use of multiple brokers will not affect the results of the co-simulation.

#### HELICS differences

Broker hierarchies are primarily used to help improve the performance of the co-simulation by allowing federates that interact strongly with each other to run on a single compute node, thereby allowing them to exchange information with each other quickly rather than over a relatively slow network connection to the rest of the federates on other compute nodes. This can be particularly helpful when the other compute nodes reside at off-site locations and the co-simulation communication is taking place between them over the public internet. (See the [User Guide section on broker hierarchies](../../advanced_topics/broker_hierarchies.md) for further details.)

Not all federations lend themselves to segregation like this; the example here doesn't really support such segregation as both the Battery and the Controller talk frequently with the Charger.

### HELICS components

When implementing across compute nodes, the configuration is simpler will be simpler than in this example because the need to segregate the federates and brokers is only a function of IP address where HELICS can use the default port number on each compute node. To get this example to run on a single computer, the hierarchy must be implemented through the use of specific port numbers for specific brokers.

When running across multiple compute nodes, the relevant portion of the HELICS runner files would look like this:

`broker_hierarchy_runner_A.json`

```json
"exec": "helics_broker --loglevel=7 --timeout='10s' ",
```

`broker_hierarchy_runner_B.json` and `broker_hierarchy_runner_C.json`

```json
"exec": "helics_broker -f <number of federates> --loglevel=7 --timeout='10s' --broker_address=tcp://<IP address of broker A>",
```

Additionally, the federates would not need `brokerAddress` in their configuration since they would look on their local node for a broker by default.

Since this example _was_ made to run on a single computer, we use the port number to segregate the federates and broker. `--port=` is use to define the port number on which the broker looks for connections to federates and `--broker_address=` is used to define the IP address with port of the parent broker. (The loopback address of 127.0.0.1 is used to look for the broker on the same node.)

Additionally, each federate has to define the broker to which it is attempting to connect by including the `brokerAddress` parameter in its own configuration; this allows for the definition of the port number the federate should use to connect to it's broker:

```json
"brokerAddress": "tcp://127.0.0.1:25300",
```

## Execution and Results

Since this example requires three brokers and their respective federates to run simultaneously, `helic_cli` will be used to launch the three sets of brokers and federates, just like the in [simultaneous co-simulation example](./advanced_brokers_simultaneous.md)

- `$ helics run --path=./broker_hierarchy_runner_A.json &`
- `$ helics run --path=./broker_hierarchy_runner_B.json &`
- `$ helics run --path=./broker_hierarchy_runner_C.json &`

The peak charging results are shown below. As can be seen, the peak power amplitude and the total time at peak power are impacted by the random number generator seed.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_charging_power.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_estimated_SOCs.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_battery_SOCs.png)

These results are identical to those in the base [Advanced Default example](./advanced_default.md); this is expected as only the structure of the co-simulation has been changed and not any of the federate code itself.

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
