# Multi-Input

This demonstrates the use of federation queries and performs dynamic configuration by using the information from the query to configure the Battery federate.

- [Where is the code?](#where-is-the-code)
- [What is this Co-simulation doing?](#what-is-this-co-simulation-doing)
  - [Differences Compared to the Fundamental Examples](#differences-compared-to-the-advanced-default-example)
    - [HELICS Differences](#helics-differences)
  - [HELICS Components](#helics-components)
- [Execution and Results](#execution-and-results)

## Where is the code?

This example on [multi-inputs can be found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_message_comm/multi_input). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_multi_input_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced)

## What is this co-simulation doing?

This example shows how to use inputs, allowing multiple publications to arrive at the same input interface (similar to a subscription, as you'll see) and a demonstration on one method of managing data conflicts that can arise.

### Differences compared to the Advanced Default example

This example deviates fairly significantly from the [Advanced Default example](./advanced_default.md) in that it only has a Battery and Charger federate. The Charger federate was modeled with one charging terminal that branches out to the five Battery terminals. That is, from the Charger federates perspective, there is only one charging voltage and one charging current even though the federation is still constructed to charge five batteries.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_multi_input_differences.png)

The difference in the model is entirely implied by the HELICS configuration; the physics of the system is modeled through the configuration and this is one valid interpretation.

Additionally, since the protocol (to use the term loosely) in the Advanced Default example for a Battery indicating it was fully charged and the Charger confirming was the removal of the charging voltage, this simple protocol won't work as written when using multi-inputs. Rather than implementing a more sophisticated protocol and to keep the code simple, we decided to just charge one battery at each terminal over the duration of the simulation. Also, due to the removal of the protocol, there is no need of the Controller federate to determine when to stop charging the battery as the batteries self-terminate their charging.

#### HELICS differences

With a single charger being used to charge five batteries, each battery still publishes it's charging current as before but only has one subscription, the single charging voltage. This shows up on the Battery federate configuration:

[BatteryConfig.json](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/advanced/advanced_message_comm/multi_input/BatteryConfig.json)

```json
{
  "name": "Battery",
  "loglevel": 1,
  "coreType": "zmq",
  "period": 60,
  "uninterruptible": false,
  "terminate_on_error": true,
  "wait_for_current_time_update": true,
  "publications": [
    {
      "key": "Battery/EV1_current",
      "type": "double",
      "unit": "A",
      "global": true
    },
    {
      "key": "Battery/EV2_current",
      "type": "double",
      "unit": "A",
      "global": true
    }
  ],
  "subscriptions": [
    {
      "key": "Charger/EV_voltage",
      "type": "double",
      "unit": "V",
      "global": true
    }
  ]
}
```

The Charger federate configuration is also altered, using an `input` rather than a `subscription` interface to allow all publications from the Battery federates to be received on one interface. The input has been configured to allow multiple inputs and lists the publications that should be targeted toward it and to handle these multiple inputs by summing them.

```json
{
  "name": "Charger",
  "loglevel": 1,
  "coreType": "zmq",
  "period": 60,
  "uninterruptible": false,
  "terminate_on_error": true,
  "publications": [
    {
      "key": "Charger/EV_voltage",
      "type": "double",
      "unit": "V",
      "global": true
    }
  ],
  "inputs": [
    {
      "key": "Battery/charging_current",
      "type": "double",
      "global": true,
      "multi_input_handling_method": "sum",
      "targets": [
        "Battery/EV1_current",
        "Battery/EV2_current",
        "Battery/EV3_current",
        "Battery/EV4_current",
        "Battery/EV5_current"
      ]
    }
  ]
}
```

### HELICS Components

`Battery.py` and `Charger.py` have both been simplified such that only battery per charging terminal is charged over the duration of the simulation. When the battery reaches full SOC, it self-terminates charging.

## Execution and Results

Run the co-simulation:

```shell-session
$ helics run --path=./multi_input_runner.json
```

The primary result of interest is still the cumulative charging power.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_multi_input_power.png)

As the batteries are not replaced during charging, the initial charging power will be the peak power. The points in time when a battery reaches full charge, though, can be clearly seen as the discrete changes in cumulative charging power.

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
