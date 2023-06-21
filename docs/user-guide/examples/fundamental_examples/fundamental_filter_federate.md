# Combination Federation with Custom Filter Federates

This custom filter federate example expands the Native Filters example to demonstrate . This example assumes the user has already worked through the [Native Filter Example](./fundamental_native_filter.md) and understands the role of filters in a HELICS-based co-simulation.

This tutorial is organized as follows:

- [Example files](#example-files)
- [Filter Federates](#filter-federates)

  - [Co-simulation Execution](#co-simulation-execution)

- [Questions and Help](#questions-and-help) talk

## Example files

All files necessary to run the Federate Integration Example can be found in the [Fundamental examples repository:](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_message_comm/filter_federate)

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_combo_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_message_comm/combo)

- Python program and configuration JSON for Battery federate
- Python program and configuration JSON for Charger federate
- Python program and configuration JSON for Controller federate
- Python program and configuration JSON for Filter federate
- (Bonus Python program where the Filter Federate does no filtering and forwards on all received messages)
- HELICS runner JSON to enable execution of the co-simulation

## Filter Federates

For situations that require filtering beyond what native HELICS filters can provide, it is possible to create a custom filter federate that acts in specific, user-define ways. Not only is it possible to re-create the native HELICS filter functions (_e.g._ delaying or randomly dropping messages) but it is also possible act on the payloads of the messages themselves. Some of these functionalities (such as dropped or delayed messages) can be achieved through existing simulation tools such as an [ns-3](https://www.nsnam.org). Even in an ns-3, though custom functionality that would modify message payloads would require writing custom applications and compiling them into ns-3. Though this example custom filter federate doesn't implement many of the detailed networking models in an ns-3 such as TCP and IP addressing, it's simplicity is a virtue in that it demonstrates how to implement a custom filter federate to act on HELICS messages an arbitrary manner.

Here is our new federation using a custom filter federate:

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_filter_federate_federation.png)

We have:

- Battery (**value federate**): passes values with Charger through pub/subs)
- Charger (**combo federate**): passes values with Battery, passes messages with Controller)
- Controller (**message federate**): passes messages with Charger through endpoints)
- Filter (**message federate**): acts on all messages sent between the Charger and the Controller)

### Filter Federate Configuration

Though not shown in the Federation diagram, the use of native HELICS filters is an essential part of using a custom filter federate. A "reroute" native helix filter is installed on all the federeation endpoints as a part of the configuration of the filter federate. This reroute filter sends all messages from the M points to the filter federate for processing.

```json
{
  "name": "Filter",
  "event_triggered": true,
  "endpoints": [
    {
      "name": "filter/main",
      "global": true
    }
  ],
  "filters": [
    {
      "name": "filterFed",
      "sourcetargets": [
        "Charger/EV1.soc",
        "Charger/EV2.soc",
        "Charger/EV3.soc",
        "Charger/EV4.soc",
        "Charger/EV5.soc",
        "Controller/ep"
      ],
      "operation": "reroute",
      "properties": {
        "name": "newdestination",
        "value": "filter/main"
      }
    }
  ]
}
```

The filter federate will now be granted time whenever a message is sent from any of the existing federation endpoints shown in the `sourcetargets` list. The filter federate has only a single endpoint which it uses to receive the rerouted messages and send on any modified messages.

Additionally, all filter federates should set the `event_triggered` flag as shown above. This increases the timing efficiency managed by HELICS and avoids potential timing lock-ups.

### Filter Federate Operations

The filter federate included in this example implements for functions:

- **Random message drops** - A random number generator and a user-adjustable parameter defines what portion of messages traveling through the filter are randomly dropped/deleted.
- **Message delay** - A random number generator and a user-adjustable parameter defines a random delay time for all messages traveling through the filter.
- **Message hacking** - for all messages originating from the controller, a random number generator and a user defined parameter define whether the contents of that message are adjusted. In this case, the contents of the payload ate either a one or zero used to indicate where the EV should continue charging; the filter federate simply inverts the message value if it is selected to be hacked.
- **Interference** - Messages that are destined to reach the Controller at the same time can interfere with each other. User-defined parameter defines how closely in time two messages must be arriving to interfere with each other.

Though these operations are just a sample of what any filter federate could do, they are representative of the types of communication system effects that are often sought to be represented through more complex simulators such as ns-3. This simulator contains no network topology; all messages are processed as if traveling through a single communication node. The source code shows the implementation of these functions, and the log files generated by the Filter federate are comprehensive.

### Co-simulation execution

Execution of this co-simulation is done as before with the `helics_run` command:

```shell
helics run --path=./fundamental_filter_runner.json
```

The resulting figures show the actual on board SOC at each EV charging port, the instantaneous power draw, and the SOC estimated by the on board charger.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_filter_federate_battery_SOCs.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_filter_federate_charging_power.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_filter_federate_estimated_SOCs.png)

When comparing to the results from the [previous example without any filters](./fundamental_combo.md), the effects of the filter federate are clear. By modifying the control signals between the controller and charger it is relatively easy to cause significantly different behavior in the system.

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
