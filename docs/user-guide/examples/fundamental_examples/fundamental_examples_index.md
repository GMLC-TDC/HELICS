# Fundamental Examples

The Fundamental Examples teach three concepts to build on a default setup:

```{eval-rst}
.. toctree::
    :maxdepth: 1

    fundamental_default
    fundamental_fedintegration
    fundamental_communication

```

The Fundamental examples are meant to build in complexity -- if you are new to HELICS, we recommend you start with the Base Example, which is also the recommended default setup. The examples in this section start with the simplest configuration method, which makes assumptions about the system which may not be completely valid but are reasonable for learning purposes.

This page describes the model -- what is the research question addressed, and what are the components to a simple HELICS co-simulation:

- <a href="#where-is-the-code">Where is the Code?</a>
- <a href="#what-is-this-co-simulation-doing">What is this Co-simulation Doing?</a>
- <a href="#helics-components">HELICS Components</a>
  - <a href="#register-and-configure-federates">Register and Configure Federates</a>
  - <a href="#enter-execution-mode">Enter Execution Mode</a>
  - <a href="#define-time-variables">Define Time Variables</a>
  - <a href="#initiate-time-steps-for-the-time-loop">Initiate Time Steps for the Time Loop</a>
  - <a href="#send-receive-communication-between-federates">Send/Receive Communication between Federates</a>
  - <a href="#finalize-co-simulation">Finalize Co-simulation</a>

<a name="where-is-the-code">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
Where is the code?
</span>
</strong>
</a>

The code for the [Fundamental examples](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental) can be found in the HELICS-Examples repository on github. If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_examples_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental)

<a name="what-is-this-co-simulation-doing">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
What is this Co-simulation doing?
</span>
</strong>
</a>

The Fundamental Examples model the interaction between five electric vehicles (EVs) each connected to a charging station -- five EVs, five charging stations. You can imagine that five EVs enter a parking garage filled with charging stations (charging garage). We will be modeling the EVs as if they are solely their on-board batteries.

Imagine you are the engineer assigned to assess how much power will be needed to serve EVs in this charging garage. The goal of the co-simulation is to calculate the **instantaneous power draw** from the EVs in the garage.

Some questions you might ask yourself include:

1. How many EVs are likely to be charging at any given time?
2. What is the charge rate (power draw limitation) for these EVs?
3. What is the battery size (total capacity) for these EVs?

For now, we've decided that there will always be five EVs and five charging stations. We can also define a few functions to assign the charge rates and the battery sizes. This requires some thinking about _which_ federate will manage _what_ information.

The co-simulation has two federates: one for the EVs, and one for the Chargers. The batteries on board the EVs will remain the domain of the EVs and inform the EV state of charge (SOC). The Chargers will manage the voltage applied to the EV batteries, and will retain knowledge of the rate of charge.

Within the federate `Battery.py`, EV battery sizes can be generated using the function `get_new_battery`:

```python
def get_new_battery(numBattery):
    # Probabilities of a new EV battery having small capacity (lvl1),
    # medium capacity (lvl2), and large capacity (lvl3).
    lvl1 = 0.2
    lvl2 = 0.2
    lvl3 = 0.6

    # Batteries have different sizes:
    # [25,62,100]
    listOfBatts = np.random.choice(
        [25, 62, 100], numBattery, p=[lvl1, lvl2, lvl3]
    ).tolist()
    return listOfBatts
```

Within the federate `Charger.py`, the charge rate for each EV is generated using the function `get_new_EV`:

```python
def get_new_EV(numEVs):
    # Probabilities of a new EV charging at the specified level.
    lvl1 = 0.05
    lvl2 = 0.6
    lvl3 = 0.35
    listOfEVs = np.random.choice([1, 2, 3], numEVs, p=[lvl1, lvl2, lvl3]).tolist()
    numLvl1 = listOfEVs.count(1)
    numLvl2 = listOfEVs.count(2)
    numLvl3 = listOfEVs.count(3)

    return numLvl1, numLvl2, numLvl3, listOfEVs
```

The probabilities assigned to each of these functions are placeholders -- a more advanced application can be found in the [Orchestration Tutorial](../advanced_examples/advanced_orchestration.md).

Now that we know these three quantities -- the number of EVs, the capacity of their batteries, and their charge rates, we can build a co-simulation from the two federates. The `Battery.py` federate will update the SOC of each EV after it receives the voltage from the `Charger.py` federate. The `Charger.py` federate will send a voltage signal to the EV until it tells the Charger it has reached its full capacity.

The `Battery.py` federate can tell us the SOC of each EV throughout the co-simulation, and the `Charger.py` federate can tell us the aggregate power draw from all the EVs throughout the co-simulation. The co-simulation will be run for one week.

<a name="helics-components">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
HELICS Components
</span>
</strong>
</a>

We know conceptually what we want to (co-)simulate. What are the necessary HELICS components to knit these two federates into one co-simulation?

<a name="register-and-configure-federates">
<strong>
<span style="color:black;text-decoration:underline;">
Register and Configure Federates
</span>
</strong>
</a>

The first task is to register and configure the federates with HELICS within each python program:

```python
##########  Registering  federate and configuring from JSON################
fed = h.helicsCreateValueFederateFromConfig("BatteryConfig.json")
federate_name = h.helicsFederateGetName(fed)
logger.info(f"Created federate {federate_name}")
```

Since we are configuring with external JSON files, this is done in one line!

<a name="enter-execution-mode">
<strong>
<span style="color:black;text-decoration:underline;">
Enter Execution Mode
</span>
</strong>
</a>

The HELICS co-simulation starts by instructing each federate to enter execution mode.

```python
##############  Entering Execution Mode  ##################################
h.helicsFederateEnterExecutingMode(fed)
logger.info("Entered HELICS execution mode")
```

<a name="define-time-variables">
<strong>
<span style="color:black;text-decoration:underline;">
Define Time Variables
</span>
</strong>
</a>

Time management is a vital component to HELICS co-simulations. Every HELICS co-simulation needs to be provided information about the start time (`grantedtime`), the end time (`total_interval`) and the time step (`update_interval`). Federates can [step through time at different rates](../../fundamental_topics/timing_configuration.md), and it is allowable to have federates start and stop at different times, but this must be curated to meet the needs of the research question.

```python
hours = 24 * 7
total_interval = int(60 * 60 * hours)
update_interval = int(
    h.helicsFederateGetTimeProperty(fed, h.helics_property_time_period)
)
grantedtime = 0
```

<a name="initiate-time-steps-for-the-time-loop">
<strong>
<span style="color:black;text-decoration:underline;">
Initiate Time Steps for the Time Loop
</span>
</strong>
</a>

Starting the co-simulation time sequence is also a function of the needs of the research question. In the Base Example, the EVs will already be "connected" to the Chargers and will be waiting for the voltage signal from the Charger. This means we need to set up a signal to send from the Charger to the EV _before_ the EV requests the signal.

In the `Battery.py` federate, Time is initiated by starting a `while` loop and requesting the first time stamp:

```python
while grantedtime < total_interval:

    # Time request for the next physical interval to be simulated
    requested_time = grantedtime + update_interval
    logger.debug(f"Requesting time {requested_time}")
    grantedtime = h.helicsFederateRequestTime(fed, requested_time)
    logger.debug(f"Granted time {grantedtime}")
```

In the `Charger.py` federate, we need to send the first signal **before** entering the time `while` loop. This is accomplished by requesting an initial time (outside the `while` loop), sending the signal, and then starting the time `while` loop:

```python
# Blocking call for a time request at simulation time 0
initial_time = 60
logger.debug(f"Requesting initial time {initial_time}")
grantedtime = h.helicsFederateRequestTime(fed, initial_time)
logger.debug(f"Granted time {grantedtime}")


# Apply initial charging voltage
for j in range(0, pub_count):
    h.helicsPublicationPublishDouble(pubid[j], charging_voltage[j])
    logger.debug(
        f"\tPublishing charging voltage of {charging_voltage[j]} "
        f" at time {grantedtime}"
    )


########## Main co-simulation loop ########################################
# As long as granted time is in the time range to be simulated...
while grantedtime < total_interval:

    # Time request for the next physical interval to be simulated
    requested_time = grantedtime + update_interval
    logger.debug(f"Requesting time {requested_time}")
    grantedtime = h.helicsFederateRequestTime(fed, requested_time)
    logger.debug(f"Granted time {grantedtime}")
```

<a name="send-receive-communication-between-federates">
<strong>
<span style="color:black;text-decoration:underline;">
Send/Receive Communication between Federates
</span>
</strong>
</a>

Once inside the time loop, information is requested and sent between federates at each time step. In the Base Example, the federates first request information from the interfaces to which they have subscribed, and then send information from the interfaces from which they publish.

The `Battery.py` federate first asks for voltage information from the interfaces to which it subscribes:

```python
# Get the applied charging voltage from the EV
charging_voltage = h.helicsInputGetDouble((subid[0]))
logger.debug(
    f"\tReceived voltage {charging_voltage:.2f} from input"
    f" {h.helicsSubscriptionGetKey(subid[0])}"
)
```

And then (after doing some internal calculations) publishes the charging current of the battery at its publication interface:

```python
# Publish out charging current
h.helicsPublicationPublishDouble(pubid[j], charging_current)
logger.debug(f"\tPublished {pub_name[j]} with value " f"{charging_current:.2f}")
```

Meanwhile, the `Charger.py` federate asks for charging current from the interfaces to which it subscribes:

```python
charging_current[j] = h.helicsInputGetDouble((subid[j]))
logger.debug(
    f"\tCharging current: {charging_current[j]:.2f} from "
    f"input {h.helicsSubscriptionGetKey(subid[j])}"
)
```

And publishes the charging voltage at its publication interface:

```python
# Publish updated charging voltage
h.helicsPublicationPublishDouble(pubid[j], charging_voltage[j])
logger.debug(
    f"\tPublishing charging voltage of {charging_voltage[j]} " f" at time {grantedtime}"
)
```

<a name="finalize-co-simulation">
<strong>
<span style="color:black;text-decoration:underline;">
Finalize Co-simulation
</span>
</strong>
</a>

After all the time steps have completed, it's good practice to finalize the co-simulation by freeing the federates and closing the HELICS libraries:

```python
status = h.helicsFederateFinalize(fed)
h.helicsFederateFree(fed)
h.helicsCloseLibrary()
```
