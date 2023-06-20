# Combination Federation with Native HELICS Filters

This custom filter federate example expands the [combination federation example](./fundamental_combo.md) to demonstrate how HELICS filters can be added to endpoints.

This tutorial is organized as follows:

- [Example files](#example-files)
- [HELICS Filters](#helics-filters)
- [Co-simulation Execution](#co-simulation-execution)
- [Questions and Help](#questions-and-help)

## Example files

All files necessary to run the Native Filter Example can be found in the [Fundamental examples repository:](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_message_comm/filter_native)

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_filter_native_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_message_comm/filter_native)

- Python program and configuration JSON for Battery federate
- Python program and configuration JSON for Charger federate
- Python program and configuration JSON for Controller federate
- HELICS runner JSON to enable execution of the co-simulation

## HELICS filters

As discussed in the [User Guide](../../fundamental_topics/federates.md#native-helics-filters), filters have the ability to act on messages being sent between endpoints and perform various functions on them (delay, drop, reroute, etc). In this example, we're going to take the [Combination Federate Example](./fundamental_combo) and add a filter between the Charger and the Controller (since this is where the messages are flowing). Specifically, we're adding a destination filter to the controller endpoint such that all messages received by that endpoint will be delayed. Note that this filter will only act on the messages received at this endpoint and not those sent out from it. Here's the entire Controller JSON config:

```json
{
  "name": "Controller",
  "log_level": "warning",
  "core_type": "zmq",
  "time_delta": 1,
  "uninterruptible": false,
  "terminate_on_error": true,
  "endpoints": [
    {
      "name": "Controller/ep",
      "global": true
    }
  ],
  "filters": [
    {
      "name": "ep_filter",
      "destination_target": "Controller/ep",
      "operation": "delay",
      "properties": {
        "name": "delay",
        "value": "900s"
      }
    }
  ]
}
```

The delay value is set to 900 seconds so that the impact of the fictitious communication system delay is obvious in the results of this example and the delay can be set to much lower or higher values.

## Co-simulation execution

Execution of this co-simulation is done as before with the `helics run` command:

```shell
helics run --path=fundamental_combo_runner.json
```

Below are pairs of output graphs with the first from each pair being from the original Combination Federate Example and the second from this Native Filter Example with the communication delay.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_combo_battery_SOCs.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_filter_native_battery_SOCs.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_combo_charging_power.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_filter_native_charging_power.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_combo_estimated_SOCs.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_filter_native_estimated_SOCs.png)

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
