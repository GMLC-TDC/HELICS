# Endpoint Federates

The Federate Message + Communication Configuration Example extends the Base Example to demonstrate how to register federates which send messages from/to endpoints instead of values from/to pub/subs.

This tutorial is organized as follows:

- [Endpoint Federates](#endpoint-federates)
  - [Example files](#example-files)
  - [Federate Communication with Endpoints](#federate-communication-with-endpoints)
    - [When to use pub/subs vs endpoints](#when-to-use-pubsubs-vs-endpoints)
    - [Translation from pub/sub to endpoints](#translation-from-pubsub-to-endpoints)
      - [Config Files](#config-files)
      - [Simulators](#simulators)
        - [Battery](#battery)
        - [Charger](#charger)
    - [Co-simulation execution](#co-simulation-execution)
  - [Questions and Help](#questions-and-help)

## Example files

All files necessary to run the Federate Integration Example can be found in the [Fundamental examples repository:](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_message_comm/endpoints)

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_endpoints_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_message_comm/endpoints)

The files include:

- Python program and configuration JSON for Battery federate
- Python program and configuration JSON for Charger federate
- HELICS runner JSON to enable execution of the co-simulation

## Federate Communication with Endpoints

There are two fundamental cases where you may find yourself using endpoints to send messages.

1. The federate is modeling communication of information (typically as a string)
2. The federate is incorrectly modeling communication of physical dynamics

What's the difference? In the [Base Example](./fundamental_default.md), the federation consists of two "value" federates -- one passes its current, the other passes a voltage. The two depend on this information. The Battery says to the Charging port, "I have a starting current!", to which the Charger responds, "Great, here's your voltage!" These two **value** federates must be coupled with pubs/subs, because they are linked by a physical system.

However, as the author of a HELICS co-simulation, there is nothing preventing you from connecting these two federates with endpoints in place of pub/subs. The co-simulation will give the same results in simple cases, but be wary of taking this type of short cut -- resurrecting code which passes information in the incorrect way may introduce nefarious results. The recommended approach is to register federates modeling **physics** as **value federates**, and those modeling _information_ as _message federates_.

Casting this guidance to the wind, this example walks through how to set up the Base Example (which passes current and voltage) as **message federates** -- landing this example squarely in situation #2 above. This is just for demonstration purposes, and this is the only example in the documentation which violates best practices.

### When to use pub/subs vs endpoints

The easiest way to determine whether you should use pub/subs vs endpoints to connect your federates is to ask yourself: "Does this message have a physical unit of measurement?" As noted above, the Battery-Charger federation **does** model physical components, and the config files make this clear with the inclusion of units:

`BatteryConfig.json`:

```json
"publications":[
  {
    "key":"Battery/EV1_current",
    "type":"double",
    "unit":"A",
    "global": true
  },
]
```

```json
"subscriptions":[
  {
    "key":"Charger/EV1_voltage",
    "type":"double",
    "unit":"V",
    "global": true
  },
]
```

`ChargerConfig.json`:

```json
"publications":[
  {
    "key":"Charger/EV1_voltage",
    "type":"double",
    "unit":"V",
    "global": true
  },
]
```

```json
"subscriptions":[
  {
    "key":"Battery/EV1_current",
    "type":"double",
    "unit":"A",
    "global": true
  },
]
```

With this pub/sub configuration, we have established a **direct** communication link between the Battery and Charger:

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/battery_sub.png)

---

**_If we accept that the information being passed between the two is not physics-based,_** then the communication link only depends on each federate having an endpoint:

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/ep_connection.png)

In departure from the directly-coupled communication links of pub/subs, messages sent from **endpoints** can be intercepted, delayed, or picked up by any federate. In that sense, communication via pub/subs can be thought of as sealed letters sent via pneumatic tubes, and messages sent via endpoints as a return-address labeled letter sent into the postal service system.

You might ask yourself: "How does HELICS know where to send the message?" There are ways to set this up as default. Before we dive into the code, it's important to understand the following about messages and endpoints:

1. Endpoints send messages as encoded strings
2. Endpoints can have default destinations, but this is not required
3. Endpoints should not be used to model physics
4. Messages sent from endpoints are allowed to be corrupted (see [Filters](fundamental_native_filter.md)!)
5. Messages sent from endpoints do not show up on a HELICS `dependency_graph`
   (A `dependency_graph` is a graph of dependencies in a federation. Because pub/subs have explicit connections, HELICS can establish when the information arrives through a `dependency_graph`. See [Queries](../../advanced_topics/queries.md) for more information.)

### Translation from pub/sub to endpoints

We are throwing caution to the wind using endpoints to model the physics in the [Base Example](./fundamental_default.md). Changes need to be made in the config files and the simulator code.

#### Config Files

As with the Base Example, configuration can be done with JSON files. The first change we need to make is to replace the `publications` and `subscriptions` with `endpoints`:

`BatteryConfig.json`:

```json
"endpoints":[
  {
    "name":"Battery/EV1_current",
    "destination":"Charger/EV1_voltage",
    "global": true
  },
]
```

`ChargerConfig.json`:

```json
"endpoints":[
  {
    "name":"Charger/EV1_voltage",
    "destination":"Battery/EV1_current",
    "global": true
  },
]
```

If you have run the Base Example, you will have seen that the information passed between the federates occurs at the same HELICS time; both federates have `"period": 60` in their config files. Recall from the [Configuration Options Reference](../../../references/configuration_options_reference.md#period-1ns) that the `period` controls the minimum time resolution for a federate.

We have a federation sending messages at the same time (`"period": 60`), and HELICS doesn't know which message arrives first. We need to introduce an `offset` to the config file of one of the federates to force it to wait until the message has been received. We also need to keep `"uninterruptible": false`, so that the federate will be granted the time at which it has received a message (which will be `"period": 60`).

The order of operation is:

| Step | HELICS Time | Charger                                                      | Battery                                                      |
| ---- | :---------- | :----------------------------------------------------------- | :----------------------------------------------------------- |
| 1    | 0           | requests time = 60, granted time = 60                        | requests time = 70                                           |
| 2    | 60          | sends message to default destination `"Battery/EV1_current"` | granted time = 60 because message has arrived                |
| 3    | 60          |                                                              | sends message to default destination `"Charger/EV1_voltage"` |

Introducing the `offset` into the config file (along with `"uninterruptible": false`) instructs HELICS about the dependencies. These adjustments are:

`BatteryConfig.json`:

```json
{
  "name": "Battery",
  "loglevel": 7,
  "coreType": "zmq",
  "period": 60.0,
  "offset": 10.0,
  "uninterruptible": false,
  "terminate_on_error": true,
  "endpoints": []
}
```

`ChargerConfig.json`:

```json
{
  "name": "Charger",
  "loglevel": 7,
  "coreType": "zmq",
  "period": 60,
  "uninterruptible": false,
  "terminate_on_error": true,
  "endpoints": []
}
```

Notice that we have only introduced an `offset` into the Battery config file, as we have set up the federates such that the Battery is waiting for information from the Charger.

#### Simulators

The simulators in this co-simulation (`*.py`) must be edited from the Base Example to register endpoints (instead of pub/subs) and ensure that messages are sent and received.

##### Battery

First let's make changes to `Battery.py`. In the Registration Step, we need to register the endpoints and get the endpoint count:

```python
fed = h.helicsCreateMessageFederateFromConfig("BatteryConfig.json")
federate_name = h.helicsFederateGetName(fed)
logger.info(f"Created federate {federate_name}")
print(f"Created federate {federate_name}")

end_count = h.helicsFederateGetEndpointCount(fed)
logging.debug(f"\tNumber of endpoints: {end_count}")

# Diagnostics to confirm JSON config correctly added the required
#   endpoints
endid = {}
for i in range(0, end_count):
    endid[i] = h.helicsFederateGetEndpointByIndex(fed, i)
    end_name = h.helicsEndpointGetName(endid[i])
    logger.debug(f"\tRegistered Endpoint ---> {end_name}")
```

After entering Execution Mode but before the Time Loop begins, we need to extract the offset value:

```python
update_offset = int(h.helicsFederateGetTimeProperty(fed, h.helics_property_time_offset))
```

And add that offset to `requested_time`:

```python
requested_time = grantedtime + update_interval + update_offset
```

The next largest difference with implementing communication between simulators with endpoints vs pub/subs comes from the lack of innate message dependency, as described above with the `dependency_graph`. (Which can be accessed for pub/subs with a [query](../../advanced_topics/queries.md).) Best practice for handling message receipt is to check if a message is waiting to be retrieved for an endpoint. The following code replaces `charging_voltage = h.helicsInputGetDouble((subid[j]))` from the Base Example (we are looping over `end_count`, the number of endpoints for this federate):

```python
endpoint_name = h.helicsEndpointGetName(endid[j])
if h.helicsEndpointHasMessage(endid[j]):
    msg = h.helicsEndpointGetMessage(endid[j])
    charging_voltage = float(h.helicsMessageGetString(msg))
```

If we want to know who sent the message (which can be helpful for both debugging and simplifying code), we invoke:

```python
source = h.helicsMessageGetOriginalSource(msg)
```

An alternative to using `h.helicsMessageGetOriginalSource(msg)` is to set a default `destination` in the JSON config file. Use of both can help with debugging.

The `Battery.py` simulator takes the `charging_voltage` from the `Charger.py` simulator and calculates the `charging_current` to send back. Sending messages to a default `destination` is then done with:

```python
h.helicsEndpointSendBytesTo(endid[j], "", str(charging_current))
```

Where the `""` can also be replaced with a string for the desired destination -- we can check `""` against `source` to confirm the messages are going to their intended destinations.

##### Charger

As with the `Battery.py` simulator, we need to Register the Charger federate as a Message Federate and get the endpoint ids:

```python
fed = h.helicsCreateMessageFederateFromConfig("ChargerConfig.json")
federate_name = h.helicsFederateGetName(fed)
logger.info(f"Created federate {federate_name}")
print(f"Created federate {federate_name}")
end_count = h.helicsFederateGetEndpointCount(fed)
logging.debug(f"\tNumber of endpoints: {end_count}")

# Diagnostics to confirm JSON config correctly added the required
#   endpoints
endid = {}
for i in range(0, end_count):
    endid[i] = h.helicsFederateGetEndpointByIndex(fed, i)
    end_name = h.helicsEndpointGetName(endid[i])
    logger.debug(f"\tRegistered Endpoint ---> {end_name}")
```

The next difference with the Base Example `Charger.py` simulator is in sending the initial voltage to Battery Federate:

```python
for j in range(0, end_count):
    message = str(charging_voltage[j])
    h.helicsEndpointSendBytesTo(endid[j], "", message.encode())  #
```

Notice that we are sending the message to the default `destination` with `""`. We cannot use `h.helicsMessageGetOriginalSource(msg)`, as no messages have been received by the Charger Federate yet.

Within the Time Loop, we change the message receipt component in the same way as the `Battery.py` simulator, where `h.helicsInputGetDouble((subid[j]))` is replaced with:

```python
endpoint_name = h.helicsEndpointGetName(endid[j])
if h.helicsEndpointHasMessage(endid[j]):
    msg = h.helicsEndpointGetMessage(endid[j])
    charging_current[j] = float(h.helicsMessageGetString(msg))
```

There's one final difference. Which API do we call to send the message to the Battery Federate?

<details><summary>ANSWER</summary>
<p>

```
# Send message of voltage to Battery federate
h.helicsEndpointSendBytesTo(endid[j], "",f'{charging_voltage[j]:4f}'.encode())  #
```

</p>
</details>

### Co-simulation execution

We run the co-simulation just as before in the Base Example -- the `runner.json` is exactly the same:

```shell
helics run --path=fundamental_endpoints_runner.json
```

And we get these figures:

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_endpoints_battery_SOCs.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_endpoints_charger_power.png)

Armed now with the knowledge of endpoints and messages, how could you change the research question?

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
