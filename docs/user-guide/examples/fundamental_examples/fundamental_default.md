# Base Example Co-Simulation




The Base Example walks through a simple HELICS co-simulation between two python federates. This example also serves as the recommended defaults for setting up a co-simulation.

<p align="center">
<img src = "../../../img/default_setup.png" width="300">
</p>

The base example described here will go into detail about the necessary components of a HELICS program. Subsequent examples in the Fundamental Examples section will change small components of the system.

The Base Example tutorial is organized as follows:

```eval_rst
.. toctree::
    :maxdepth: 1
    

    
```


## Example files

All files necessary to run the Base Example can be found in the [Fundamental examples repository:](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/user_guide_examples/fundamental/fundamental_default)

[![](../../../img/fund_default_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/user_guide_examples/fundamental/fundamental_default)

The files include:

* Python program and configuration json for Battery federate
* Python program and configuration json for Charger federate
* "runner" json to enable `helics_cli` execution

## What is this Co-simulation doing?

This Base Example is modeling the interaction between five electric vehicles (EVs) each connected to one charging station (Charger). You can imagine this is like five EVs enter an exclusive parking garage filled with charging stations (charging garage). We will be modeling the EVs as if they are solely their on-board batteries.

Imagine you are the engineer assigned to assess how much power will be needed to serve EVs in this charging garage. The goal of the co-simulation is to calculate the **instantaneous power draw** from the EVs in the garage.

Some questions you might ask yourself include:

1. How many EVs are likely to be charging at any given time?
2. What is the charge rate (power draw limitation) for these EVs?
3. What is the battery size (total capacity) for these EVs?

For now, we've decided that there will always be five EVs. We can also define a few functions to assign the charge rates and the battery sizes. This requires some thinking about *which* federate will manage *what* information. 

The co-simulation has two federates: one for the EVs, and one for the Chargers. The batteries on board the EVs will remain the domain of the EVs and inform the EV state of charge (SOC). The Chargers will manage the injection of power into the EVs, and will retain knowledge of the rate of charge.

Within the federate `Battery.py`, EV battery sizes can be generated using the function `get_new_battery`:

```
def get_new_battery(numBattery):
    # Probabilities of a new EV battery having small capacity (lvl1),
    # medium capacity (lvl2), and large capacity (lvl3).
    lvl1 = 0.2
    lvl2 = 0.2
    lvl3 = 0.6

    # Batteries have different sizes:
    # [25,62,100]
    listOfBatts = np.random.choice([25,62,100],numBattery,p=[lvl1,lvl2,
                                                       lvl3]).tolist()
    return listOfBatts
```

Within the federate `Charger.py`, the charge rate for each EV is generated using the function `get_new_EV`:

```
def get_new_EV(numEVs):
    # Probabilities of a new EV charging at the specified level.
    lvl1 = 0.05
    lvl2 = 0.6
    lvl3 = 0.35
    listOfEVs = np.random.choice([1,2,3],numEVs,p=[lvl1,lvl2,lvl3]).tolist()
    numLvl1 = listOfEVs.count(1)
    numLvl2 = listOfEVs.count(2)
    numLvl3 = listOfEVs.count(3)

    return numLvl1,numLvl2,numLvl3,listOfEVs
```

The probabilities assigned to each of these functions are placeholders -- a more advanced application can be found in the [Orchestration Tutorial](../advanced/orchestration_monte_carlo.md).

Now that we know these three quantities -- the number of EVs, the capacity of their batteries, and their charge rates, we can build a co-simulation built from these two actors. They `Battery.py` federate will update the SOC of each EV it is modeling and the `Charger.py` federate will deliver power to the EV until it tells the Charger it has reached its full capacity.

The `Battery.py` federate can tell us the SOC of each EV throughout the co-simulation, and the `Charger.py` federate can tell us the aggregate power draw from all the EVs throughout the co-simulation. The co-simulation will be run for one week.

## HELICS Components

Now that we know conceptually what we want to (co-)simulate, what are the necessary HELICS components to knit these two federates into one co-simulation?

### Register and Configure Federates

The first task is to register and configure the federates with HELICS within each python program:

```
    ##########  Registering  federate and configuring from JSON################
    fed = h.helicsCreateValueFederateFromConfig("BatteryConfig.json")
    federate_name = h.helicsFederateGetName(fed)
    logger.info(f'Created federate {federate_name}')
    print(f'Created federate {federate_name}')

```

Since we are configuring with external JSON files, this is done in one line!


### Enter Execution Mode

The HELICS co-simulation starts by instructing each federate to enter execution mode.

```
    ##############  Entering Execution Mode  ##################################
    h.helicsFederateEnterExecutingMode(fed)
    logger.info('Entered HELICS execution mode')

```


### Define Time Variables

Time management is a vital component to HELICS co-simulations. Every HELICS co-simulation needs to be provided information about the start time (`grantedtime`), the end time (`total_interval`) and the time step (`update_interval`). Federates can step through time at different rates [LINK TO TIME MD], and it is allowable to have federates start and stop at different times, but this must be curated to meet the needs of the research question.


```
    hours = 24 * 7
    total_interval = int(60 * 60 * hours)
    update_interval = int(h.helicsFederateGetTimeProperty(
                                fed,
                                h.helics_property_time_period))
    grantedtime = 0

```


### Initiate Time Steps for the Time Loop

Starting the co-simulation time sequence is also a function of the needs of the research question. In the Base Example, the EVs will already be connected to the Chargers, and will be waiting for the voltage signal from the Charger. This means we need to set up a signal to send from the Charger to the EV *before* the EV requests the signal.

In the `Battery.py` federate, Time is initiated by starting a `while` loop and requesting the first time stamp:

```
    while grantedtime < total_interval:
    
        # Time request for the next physical interval to be simulated
        requested_time = (grantedtime+update_interval)
        logger.debug(f'Requesting time {requested_time}')
        grantedtime = h.helicsFederateRequestTime (fed, requested_time)
        logger.debug(f'Granted time {grantedtime}')
```

In the `Charger.py` federate, we need send the first signal **before** entering the time `while` loop. This is accomplished by requesting an initial time (outside the `while` loop), sending the signal, and then starting the time `while` loop:

```
    # Blocking call for a time request at simulation time 0
    initial_time = 60
    logger.debug(f'Requesting initial time {initial_time}')
    grantedtime = h.helicsFederateRequestTime(fed, initial_time )
    logger.debug(f'Granted time {grantedtime}')


    # Apply initial charging voltage
    for j in range(0, pub_count):
        h.helicsPublicationPublishDouble(pubid[j], charging_voltage[j])
        logger.debug(f'\tPublishing charging voltage of {charging_voltage[j]} '
                     f' at time {grantedtime}')


    ########## Main co-simulation loop ########################################
    # As long as granted time is in the time range to be simulated...
    while grantedtime < total_interval:

        # Time request for the next physical interval to be simulated
        requested_time = (grantedtime + update_interval)
        logger.debug(f'Requesting time {requested_time}')
        grantedtime = h.helicsFederateRequestTime (fed, requested_time)
        logger.debug(f'Granted time {grantedtime}')

```



### Send/Receive Communication between Federates

Once inside the time loop, information is requested and sent between federates at each time step. In the Base Example, the federates first request information from the handles to which they have subscribed, and then send information from the handles from which they publish.

The `Battery.py` federate first asks for voltage information at the handles to which it subscribes:

```
            # Get the applied charging voltage from the EV
            charging_voltage = h.helicsInputGetDouble((subid[0]))
            logger.debug(f'\tReceived voltage {charging_voltage:.2f} from input'
                         f' {h.helicsSubscriptionGetKey(subid[0])}')
```

And then (after doing some internal calculations) publishes the charging current of the battery at its publication handle:

```
            # Publish out charging current
            h.helicsPublicationPublishDouble(pubid[j], charging_current)
            logger.debug(f'\tPublished {pub_name[j]} with value '
                         f'{charging_current:.2f}')
```

Meanwhile, the `Charger.py` federate asks for charging current at the handles to which it subscribes:

```
            charging_current[j] = h.helicsInputGetDouble((subid[j]))
            logger.debug(f'\tCharging current: {charging_current[j]:.2f} from '
                         f'input {h.helicsSubscriptionGetKey(subid[j])}')

```

And publishes the charging voltage from its publication handle:

```
            # Publish updated charging voltage
            h.helicsPublicationPublishDouble(pubid[j], charging_voltage[j])
            logger.debug(
                f'\tPublishing charging voltage of {charging_voltage[j]} '
                f' at time {grantedtime}')
```






## Default Setup

The default setup integrates the federates configurations with external JSON files. The message and communication configurations are publications and subscriptions.

### Simulator Integration: External JSON

![](../../../img/externaljson_default.png)

- picture of jsons
- where they integrate in the cosim

### Messages + Communication: pub/sub


![](../../../img/default1.png)

![alt text](../../../img/pubs.gif)

![alt text](../../../img/subs.gif)

![alt text](../../../img/pubs2.gif)

![alt text](../../../img/subs2.gif)

![alt text](../../../img/pubsubs.gif)

### Co-simulation Execution: `helics_cli`

- picture of runner json
- link to helics_cli page
- picture of running on the command line
- picture of output files




