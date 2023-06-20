# Combination Federation

The Federate Message + Communication Configuration Example extends the Base Example to demonstrate how to register federates which can send/receive messages from endpoints and values from pub/subs. This example assumes the user has already worked through the [Endpoints Example](./fundamental_endpoints.md).

This tutorial is organized as follows:

- [Example files](#example-files)
- [Combination Federates](#combination-federates)
  - [Co-simulation Execution](#co-simulation-execution)
- [Questions and Help](#questions-and-help)

## Example files

All files necessary to run the Federate Integration Example can be found in the [Fundamental examples repository:](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_message_comm/combo)

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_combo_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_message_comm/combo)

- Python program and configuration JSON for Battery federate
- Python program and configuration JSON for Charger federate
- Python program and configuration JSON for Controller federate
- HELICS runner JSON to enable execution of the co-simulation

## Combination Federates

A quick glance at the [Fundamental examples repository](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/) on github will show that almost all these introductory examples are mocked up with two federates. These two federates pass information back and forth, and the examples show different ways this can be done.

This is the only example in the Fundamental series which models three federates -- it is also exactly the same model as the [Base Example](../advanced_examples/advanced_default.md) in the Advanced series. Why are we introducing a third federate?

In the [Endpoints Example](./fundamental_endpoints.md), we learned how to pass messages between two federates. The problem with this setup -- which we will resolve in this example -- is that **physical values** should not be modeled with messages/endpoints (see [the example](./fundamental_endpoints.md#federate-communication-with-endpoints) for a reminder). We introduce a third federate -- a **combination federate** -- to preserve the handling of physical values among _value federates_ and allow for nuanced message passing (and interruption) among _message federates_. The key with combo federates is that they are the go-between for these two types. Combination federates can update (send) values _and_ intercept messages. (For a refresher on values and messages, see the section on [Types of Federates](../../fundamental_topics/federates.md). In brief: values have a physics-based unit, and messages are typically strings).

Here is our new federation of three federates:

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_complete.png)

We have:

- Battery (**value federate**: passes values with Charger through pub/subs)
- Charger (**combo federate**: passes values with Battery, passes messages with Controller)
- Controller (**message federate**: passes messages with Charger through endpoints)

### Redistribution of Federate Roles

The full co-simulation is still asking the same question: "What is the expected instantaneous power draw from a dedicated EV charging garage?" With the introduction of a _Controller_ federate, we now have additional flexibility in addressing the nuances to this question. For example, the charging controller does not have direct knowledge of the instantaneous current in the battery -- the onboard charger needs to estimate this in order to calculate the EV's state of charge. Let's walk through the roles of each federate.

#### Battery

The Battery federate operates in the same way as in the Base Example. The only difference is that it is now allowed to request a new battery when an existing one is deemed to have a full SOC. This information is in the `charging_voltage` value from the Battery's subscription to the Charger; if the Charger applies zero voltage, this means the Battery can no longer charge. The Battery federate selects a new battery randomly from three sizes -- small, medium, and large -- and assigns a random SOC between 0% and 80%.

There are no differences in the config file. As in the Base Example, the Battery federate logs and plots the internally calculated SOC over time at each charging port.

#### Charger

The Charger federate is now a combination federate -- it will communicate via pub/subs with the Battery, and via endpoints with the Controller. This difference from the Base Example Charger is seen in the config file; in addition to the pub/subs with the Battery, there are now also endpoints. Notice that the default destination for each of these named endpoints is the same -- there is one controller for all the charging ports.

```json
  "endpoints": [
    {
      "name": "Charger/EV1.soc",
      "destination": "Controller/ep",
      "global": true
    },
```

Since this federate also communicated via endpoints, we need to register them along with the existing pub/subs:

```python
##############  Registering  federate from json  ##########################
fed = h.helicsCreateCombinationFederateFromConfig("ChargerConfig.json")
federate_name = h.helicsFederateGetName(fed)
logger.info(f"Created federate {federate_name}")
end_count = h.helicsFederateGetEndpointCount(fed)
logger.info(f"\tNumber of endpoints: {end_count}")
sub_count = h.helicsFederateGetInputCount(fed)
logger.info(f"\tNumber of subscriptions: {sub_count}")
pub_count = h.helicsFederateGetPublicationCount(fed)
logger.info(f"\tNumber of publications: {pub_count}")
```

The Charger federate is gaining the new role of _estimating the Battery's current_ and shifting the role of _deciding when to stop charging_ to the Controller federate.

The Charger federate estimates the Battery federate's current with a new helper function call `estimate_SOC`. The Charger does not know the exact SOC of the Battery; it must estimate the SOC from the effective resistance, which is a function of applied voltage (from the Charger) and the measured current (from the Battery). This is the same function as used in the Battery federate, but with noise added to the measurement of the current.

```python
def estimate_SOC(charging_V, charging_A):
    socs = np.array([0, 1])
    effective_R = np.array([8, 150])
    mu = 0
    sigma = 0.2
    noise = np.random.normal(mu, sigma)
    measured_A = charging_A + noise
    measured_R = charging_V / measured_A
    SOC_estimate = np.interp(measured_R, effective_R, socs)

    return SOC_estimate
```

This function is called after the Charger has received the charging current from the Battery federate and needs to update the SOC; if the current is not zero, the Charger estimates the SOC with the inclusion of measurement error on the current. This allows the co-simulation to model the separation of knowledge of the physics between the two federates: the Battery knows its internal current, but the on board Charger must estimate it.

If the current received from the Battery federate is zero, this means that we have plugged a new EV into the charging port and we need to determine the voltage to apply with the Charger. This is accomplished by calling `get_new_EV(1)` and `calc_charging_voltage()`. `get_new_EV(1)` is a helper function which selects the charging level (1, 2, or 3) based on a set probability distribution and `calc_charging_voltage()` gives the applied voltage for that level. Once a "new EV" (the charging level) has been retrieved, the federate is assigned a SOC of 0 as an initial estimate prior to measuring the current.

The estimated SOC is sent to the Controller every 15 minutes -- this mimics an on board charging agent regularly pinging the charging port to confirm if it should continue charging:

```python
# Send message to Controller with SOC every 15 minutes
if grantedtime % 900 == 0:
    h.helicsEndpointSendBytesTo(endid[j], "", f"{currentsoc[j]:4f}".encode())
```

The Charger federate is allowed to be interrupted if there is a message from the Controller.

```python
# Check for messages from EV Controller
endpoint_name = h.helicsEndpointGetName(endid[j])
if h.helicsEndpointHasMessage(endid[j]):
    msg = h.helicsEndpointGetMessage(endid[j])
    instructions = h.helicsMessageGetString(msg)
```

The Charger will receive a message every 15 minutes as well, however it will only change actions if it is told to stop charging. When this happens, the Charger "disengages" from the charging port by applying zero voltage to the Battery.

```python
if int(instructions) == 0:
    # Stop charging this EV
    charging_voltage[j] = 0
    logger.info(f"\tEV full; removing charging voltage")
```

#### Controller

The Controller is a new federate whose role is to decide whether to keep charging an EV based. This decision is based entirely on the estimated SOC calculated by the Charger. Since this decision logic is simple and can be applied to all the EVs modeled by the federation, we can set up the config file with one endpoint:

```json
  "endpoints": [
    {
      "name": "Controller/ep",
      "global": true
    }
  ]
```

Note that there is no default destination -- the Controller will _respond_ to a request for instructions from the Charger. This is accomplished by calling the `h.helicsMessageGetOriginalSource()` API:

```python
while h.helicsEndpointHasMessage(endid):

    # Get the SOC from the EV/charging terminal in question
    msg = h.helicsEndpointGetMessage(endid)
    currentsoc = h.helicsMessageGetString(msg)
    source = h.helicsMessageGetOriginalSource(msg)
```

And then sending the message to this source:

```python
message = str(instructions)
h.helicsEndpointSendBytesTo(endid, source, message.encode())
```

The Controller federate only operates when it receives a message -- it is a _passive_ federate. This can be set up by:

1. Initializing the start time of the federate to `h.HELICS_TIME_MAXTIME`:

   ```python
   fake_max_time = int(h.HELICS_TIME_MAXTIME)
   starttime = fake_max_time
   logger.debug(f"Requesting initial time {starttime}")
   grantedtime = h.helicsFederateRequestTime(fed, starttime)
   ```

2. Allow the federate to be interrupted and set a minimum `timedelta` (`ControllerConfig.json`):

   ```json
   {
     "name": "Controller",
     "timedelta": 1,
     "uninterruptible": false
   }
   ```

3. Only execute an action when there is a message:

   ```python
   while h.helicsEndpointHasMessage(endid):
       pass  # placeholder for loop body
   ```

4. Re-request the `h.HELICS_TIME_MAXTIME` after a message has been received:

   ```python
   grantedtime = h.helicsFederateRequestTime(fed, fake_max_time)
   ```

The message the Controller receives is the SOC estimated by the Charger. If the estimated SOC is greater than 95%, the Controller sends the message back to stop charging.

```python
soc_full = 0.95
if float(currentsoc) <= soc_full:
    instructions = 1
else:
    instructions = 0
```

### Co-simulation execution

With these three federates -- Battery, Charger, and Controller -- we have partitioned the roles into the most logical places. Execution of this co-simulation is done as before with the HELICS runner:

```shell
helics run --path=fundamental_combo_runner.json
```

The resulting figures show the actual on board SOC at each EV charging port, the instantaneous power draw, and the SOC estimated by the on board charger.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_combo_battery_SOCs.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_combo_charging_power.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_combo_estimated_SOCs.png)

Note that we have made a number of simplifying assumptions in this analysis:

- There will always be an EV waiting to be charged (the charging ports are never idle).
- There is a constant number of charging ports -- we know what the power draw will look like given a static number of ports, but we do not know the underlying demand for power from EVs.
- The equipment which ferries the messages between the Charger and the Controller never fails -- we haven't incorporated [Filters](./fundamental_native_filter.md).

How would you model an unknown demand for vehicle charging? How would you model idle charging ports? What other simplifications do you see that can be addressed?

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
