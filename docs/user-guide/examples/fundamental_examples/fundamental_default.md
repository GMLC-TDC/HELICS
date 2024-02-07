# Base Example Co-Simulation

The Base Example walks through a simple HELICS co-simulation between two python federates. This example also serves as the recommended defaults for setting up a co-simulation. The base example described here will go into detail about the necessary components of a HELICS program. Subsequent examples in the Fundamental Examples section will change small components of the system.

The Base Example tutorial is organized as follows:

- [Example files](#example-files)
- [Default Setup](#default-setup)
  - [Messages + Communication: pub sub](#messages-and-communication-pubsub)
  - [Simulator Integration: External JSON](#simulator-integration-external-json)
  - [Co-simulation Execution:](#co-simulation-execution)
- [Questions and Help](#questions-and-help)

## Example files

All files necessary to run the Base Example can be found in the [Fundamental examples repository:](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_default)

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fund_default_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_default)

The files include:

- Python program and configuration JSON for Battery federate
- Python program and configuration JSON for Charger federate
- HELICS runner JSON to enable execution of the co-simulation

## Default Setup

The default setup, used in the Base Example, integrates the federate configurations with external JSON files. The message and communication configurations are publications and subscriptions. This section introduces federate configuration of publications (pubs) and subscriptions (subs) with JSON files and how to launch the co-simulation with the HELICS runner.

### Messages and Communication: pub/sub

In the Base Example, the information being passed between the `Battery.py` federate and the `Charger.py` federate is the **voltage** applied to the battery, and the **current** measured across the battery and fed back to the charger. Voltage and current are both physical quantities, meaning that unless we act on these quantities to change them, they will retain their values. For this reason, in HELICS, physical quantities are called **values**. Values are sent via publication and subscription -- a federate can publish its value(s), and another federate can subscribe this value(s).

When configuring the communication passage between federates, it is important to connect the federate to the correct **interface**. In the image below, we have a Battery federate and a Charger federate. Each federate has a **publication** interface (red) and a **subscription** interface (yellow). The publication interface is also called the **output**, and the subscription interface the **input**. How are values passed between federates with pubs and subs?

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/handles.png)

We have **named** the publication interface for the `Battery` federate `EV_current` to indicate the information being broadcast -- we can also call the publication interface the **named output**. This is what the publication is doing -- we are telling the Battery federate that we want to publish the EV_current. The full interface designation for the current is `Battery/EV_current` (within the JSON, this is also called the `key`).

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/battery_pub.png)

How does the current value get from the Battery federate's publication to the Charger federate? The Charger must subscribe to this publication interface -- the Charger will subscribe to `Battery/EV_current`. The Charger subscription interface has not been given a name (e.g., `Charger/EV_current`), but it will receive **input** -- the Charger subscription is a defined unnamed input with a targeted publication. In this example, we configure the target of the Charger subscription in the JSON to the publication interface name `Battery/EV_current`.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/charger_sub.png)

Thus far we have established that the Battery is publishing its current from the named interface `Battery/EV_current` and the Charger is subscribing to this named interface. The Charger is also sending information about values. The Charger federate will be publishing the voltage value from the `Charger/EV_voltage` interface (a named output).

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/charger_pub.png)

In order to guarantee that the Battery federate receives the voltage value from the Charger federate, the Battery will have an unnamed input subscription which targets the `Charger/EV_voltage` interface.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/battery_sub.png)

With a better understanding of how we want to configure the pubs and subs, we can now move on to the mechanics of integrating the two simulators.

### Simulator Integration: External JSON

Configuration of federates may be done with JSON files. Each federate will have its own configuration ("config") file. It's good practice to mirror the name of the federate with the config file. For example, the `Battery.py` federate will have a config file named `BatteryConfig.json`.

There are [extensive ways](../../../references/configuration_options_reference.md) to configure federates in HELICS. The `BatteryConfig.json` file contains the most common as defaults:

```json
{
  "name": "Battery",
  "log_level": 1,
  "core_type": "zmq",
  "period": 60,
  "uninterruptible": false,
  "terminate_on_error": true,
  "wait_for_current_time_update": true,
  "publications": [],
  "subscriptions": []
}
```

In this configuration, we have named the federate `Battery`, set the `log_level` to 1 ([what do loglevels mean and which one do I want?](../../../references/configuration_options_reference.md#logging-options)), and set the `core_type` to `zmq` ([the most common](../../advanced_topics/CoreTypes.md)). The next four options control timing for this federate. The final options are for message passing.

This federate is configured with pubs and subs, so it will need an option to indicate the publication and the subscription configurations (for brevity, only the first pub and sub are printed below):

```json
"publications":[
  {
    "key":"Battery/EV1_current",
    "type":"double",
    "unit":"A",
    "global": true
  },
],
"subscriptions":[
  {
    "key":"Charger/EV1_voltage",
    "type":"double",
    "unit":"V",
    "global": true
  },
]
```

This pub and sub configuration is telling us that the `Battery.py` federate is publishing in units of amps (`A`) for current from the named interface (`key`) `Battery/EV1_current`. This federate is also subscribing to information from the `Charger.py` federate. It has subscribed to a value in units of volts (`V`) at the named interface (`key`) `Charger/EV1_voltage`.

As discussed in ["Simulator Integration: External JSON""](#simulator-integration-external-json), the federate registration and configuration with JSON files in the python federate is done with one line of code:

```python
fed = h.helicsCreateValueFederateFromConfig("BatteryConfig.json")
```

Recall that federate registration and configuration is typically done **before** entering execution mode.

### Co-simulation Execution

At this point in setting up the Base Example co-simulation, we have:

1. Placed the necessary HELICS components in each federate program
2. Written the configuration JSON files for each federate

It's now time to launch the co-simulation with the HELICS runner. This is accomplished by creating a **runner** JSON file. The HELICS runner allows the user to launch multiple simulations in one command line, which otherwise would have required multiple terminals.

The runner JSON for the Base Example is called `fundamental_default_runner.json`:

```json
{
  "name": "fundamental_default",
  "broker": true,
  "federates": [
    {
      "directory": ".",
      "exec": "python -u Charger.py 1",
      "host": "localhost",
      "name": "Charger"
    },
    {
      "directory": ".",
      "exec": "python -u Battery.py 1",
      "host": "localhost",
      "name": "Battery"
    }
  ]
}
```

This runner tells `helics_broker` that there are three federates and to take a specific action for each federate:

1. Launch `helics_broker` in the current directory: `helics_broker -f 2 --loglevel=7`
2. Launch the `Charger.py` federate in the current directory: `python -u Charger.py 1`
3. Launch the `Battery.py` federate in the current directory: `python -u Battery.py 1`

The final step is to launch our Base Example with the HELICS runner from the command line (making sure you've [installed the HELICS cli extension](../../installation/index.md)):

```shell
helics run --path=fundamental_default_runner.json
```

If all goes well, this will reward us with two figures:

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_default_resultbattery.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_default_resultcharger.png)

We can see the state of charge of each battery over the duration of the co-simulation in the first figure, and the aggregated instantaneous power draw in the second. As the engineer tasked with assessing the power needs for this charging garage, do you think you have enough information at this stage? If not, how would you change the co-simulation to better model the research needs?

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
