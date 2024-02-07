# Co-simulation Stages

HELICS has several stages to the co-simulation. Creation, initialization, execution, and final state. A call to `helicsFederateEnterExecutingMode()` is the transition between initialization and execution.

- [Creation](#creation)
  - [Registration](#registration)
    - [Using a JSON Config File](#using-a-json-config-file)
    - [Using PyHELICS API Calls](#using-pyhelics-api-calls)
  - [Collecting the Interface Objects](#collecting-the-interface-objects)
- [Initialization](#initialization)
- [Execution](#execution)
  - [Get Inputs](#get-inputs)
  - [Internal Updates and Calculations](#internal-updates-and-calculations)
  - [Publish Outputs](#publish-outputs)
- [Final State](#final-state)

<!-- In the init mode values can be exchanged prior to time beginning. Normally values published in init mode are available at time 0, but if the iteration is used they can be available inside the initialization mode. There are 3 available options for the iterate parameter.

`no_iteration` -- don't iterate

`force_iteration` -- guaranteed iteration, stay in iteration mode

`iterate_if_needed` -- If there is data available from other federates Helics will iterate, if no additional data is available it will move to execution mode and have granted time=0.

ou could manage the timing of the federation to use the `initialize` mode in HELICS. This would allow you to communicate initial conditions (`t=0`) information prior to the start of execution mode. It would also allow you to publish out the `t=1` information so that all federates would have access to it once they were granted `t=1`. Look into `helicsFederateEnterInitializingMode()`. -->

## Creation

For the purposes of these examples, we will assume the use of a Python binding. If, as the simulator integrator, you have needs beyond what is discussed here you'll have to dig into the [developer documentation on the APIs](../../references/api-reference/index.md) to get the details you need.

To begin, at the top of your Python module ([after installing the Python HELICS module](../installation/index.md)), you'll have to import the HELICS library, which will look something like this:

```python
import helics as h
```

### Registration

As discussed in the previous section on [Federate Interface Configuration](./interface_configuration.md), configuration of federates can be done with either JSON config files or with the simulator's API.

#### Using a JSON Config File

In HELICS there is a single API call that can be used to read in all of the necessary information for creating a federate from a JSON configuration file. The JSON configuration file, as discussed earlier in this guide, contains both the federate info as well as the metadata required to define the federate's publications, subscriptions and endpoints. The API calls for creating each type of federate are given below.

For a value federate:

```python
fed = h.helicsCreateValueFederateFromConfig("fed_config.json")
```

For a message federate:

```python
fed = h.helicsCreateMessageFederateFromConfig("fed_config.json")
```

For a combination federate:

```python
fed = h.helicsCreateCombinationFederateFromConfig("fed_config.json")
```

In all instances, this function returns the federate object `fed` and requires a path to the JSON configuration file as an input.

#### Using PyHELICS API Calls

Additionally, there are ways to create and configure the federate directly through HELICS API calls, which may be appropriate in some instances. First, you need to create the federate info object, which will later be used to create the federate:

```python
fedinfo = h.helicsCreateFederateInfo()
```

Once the federate info object exists, HELICS API calls can be used to set the [configuration parameters](../../references/configuration_options_reference) as appropriate. For example, to set the `only_transmit_on_change` flag to true, you would use the following API call:

```python
h.helicsFederateInfoSetFlagOption(fed, 6, True)
```

(The "6" there is the integer value for appropriate HELICS enumeration. The definition of the enumerations can be found in the [C++ API reference](https://docs.helics.org/en/latest/doxygen/helics__enums_8h.html) and also cross shown in the [Configurations Options Reference](../../references/configuration_options_reference.md).)

Once the federate info object has been created and the appropriate options have been set, the helics federate can be created by passing in a unique federate name and the federate info object into the appropriate HELICS API call. For creating a value federate, that would look like this:

```python
fed = h.helicsCreateValueFederate(federate_name, fedinfo)
```

Once the federate is created, you now need to define all of its publications, subscriptions and endpoints. The first step is to create them by registering them with the federate with an API call that looks like this:

```python
pub = h.helicsFederateRegisterPublication(fed, key, data_type)
```

This call takes in the federate object, a string containing the publication key (which will be prepended with the federate name), and the data type of the publication. It returns the publication object. Once the publication, subscription and endpoints are registered, additional API calls can be used to set the info field in these objects and to set certain options. For example, to set the only transmit on change option for a specific publication, this API call would be used:

```python
pub = h.helicsPublicationSetOption(pub, 454, True)
```

Once the federate is created, you also have the option to set the federate information at that point, which - while functionally identical to setting the federate info in either the federate config file or in the federate info object - provides integrators with additional flexibility, which can be useful particularly if some settings need to be changed dynamically during the cosimulation. The API calls are syntactically very similar to the API calls for setting up the federate info object, except instead they target the federate itself. For example, to revisit the above example where the `only_transmit_on_change` flag is set to true in the federate info object, if operating on an existing federate, that call would be:

```python
h.helicsFederateSetFlagOption(fed, 6, True)
```

### Collecting the Interface Objects

Having configured the publications, subscriptions and endpoints and registered this information with HELICS, the channels for sending and receiving this information have been created within the cosimulation framework. If you registered the publication, subscriptions and endpoints within your code (i.e., using HELICS API calls), you already have access to each respective object as it was returned when made the registration call. However, if you created your federate using a configuration file which contained all of this information, you now need to retrieve these objects from HELICS so that you can invoke them during the execution of your cosimulation. The following calls will allow you to query HELICS for the metadata associated with each publication. Similar calls can be used to get input (or subscription) and endpoint information.

```python
pub_count = h.helicsFederateGetPublicationCount(fed)
pub = h.helicsFederateGetPublicationByIndex(fed, index)
pub_key = h.helicsPublicationGetKey(pub)
```

The object returned when the helicsFederateGetPublicationByIndex() method is invoked is the interface object used for retrieving other publication metadata (as in the helicsPublicationGetKey() method) and when publishing data to HELICS (as described in the execution section below).

## Initialization

Initialization mode exists to help a federation reach a consistent state or otherwise generally prepare to begin the advancement through time. Each federate can call `helicsFederateEnterInitializingMode()` and perform whatever internal set-up it needs to do as well as publish outputs that will be available to the rest of the federation at simulation time t=0 when entering execution mode (see the next section).

If the federation needs to iterate in initialization mode prior to entering execution mode each federate calls `helicsFederateEnterExecutingModeIterative()`. This API has two special aspects:

1. Calling the API requires that the federate declare its needs for iteration using an enumeration:

   `NO_ITERATION` -- don't iterate

   `FORCE_ITERATION` -- guaranteed iteration, stay in iteration mode

   `ITERATE_IF_NEEDED` -- If there is data available from other federates Helics will iterate, if no additional data is available it will move to execution mode and have granted time=0.

2. The API returns an enumeration indicating the federation's iteration state:

   `NEXT_STEP` - Iteration has completed and the federation should move to the next time step. In the case of exiting initialization, this will be the time between t=0 (which was just completed by the iteration process) and the next time grant.

   `ITERATING` - Federation has not ceased iterating and will iterate once again. During this time the federate will need to check all its inputs and subscriptions, recalculate its model, and produce new outputs for the rest of the federation.

To implement this initialization iteration, all federates need to implement a loop where `helicsFederateEnterExecutingModeIterative()` is repeatedly called and the output of the call is evaluated. The call to the API needs to use the federate's internal evaluation of the stability of the solution to determine if needs to request another iteration. The returned value of the API will determine whether the federate needs to re-solve its model with new inputs from the of the federation or enter normal execution.

## Execution

Once the federate has been created, all subscriptions, publications and endpoints have been registered and the federation initial state has been appropriately set, it is time to enter execution mode. This can be done with the following API call:

```python
h.helicsFederateEnterExecutingMode(fed)
```

This method call is a blocking call; your custom federate will sit there and do nothing until all other federates have also finished any set-up work and have also requested to begin execution of the co-simulation. Once this method returns, the federation is effectively at simulation time of zero.

At this point, each federate will now step through time, exchanging values with other federates in the cosimulation as appropriate. This will be implemented in a loop where each federate will go through a set of prescribed steps at each time step. At the beginning of the cosimulation, time is at the zeroth time step (t = 0). Let's assume that the cosimulation will end at a pre-determined time, t = max_time. The nature of the simulator will dictate how the time loop is handled. However, it is likely that the cosimulation loop will start with something like this:

```python
t = 0
while t < end_time:
    pass  # cosimulation code would go here
```

Now, the federate begins to step through time. For the purposes of this example, we will assume that during every time step, the federate will first take inputs in from the rest of the cosimulation, then make internal updates and calculations and finish the time step by publishing values back to the rest of the cosimulation before requesting the next time step.

### Get Inputs

The federate will first listen on each of its inputs (or subscriptions) and endpoints to see whether new information has been sent from the rest of the federation. The first code sample below shows how information can be retrieved from an input (or subscriptions) through HELICS API calls by passing in the subscription object. As can be seen, HELICS has built in type conversion ([where possible](https://www.youtube.com/watch?v=mZOAn-3aATY)) and regardless of how the sender of the data has formatted it, HELICS can present it as requested by the appropriate method call.

```python
int_value = h.helicsInputGetInteger(sub)
float_value = h.helicsInputGetDouble(sub)
real_value, imag_value = h.helicsInputGetComplex(sub)
string_value = h.helicsInputGetChar(sub)
...
```

It may also be worth noting that it is possible on receipt to check whether an input has been updated before retrieving values. That can be done using the following call:

```python
updated = h.helicsInputIsUpdated(sid)
```

Which returns true if the value has been updated and false if it has not.

Receiving messages at an endpoint works a little bit differently than when receiving values through a subscription. Most fundamentally, there may be multiple messages queued at an endpoint while there will only ever be one value received at a subscription (the last value if more than one is sent prior to being retrieved). To receive all of the messages at an endpoint, they need to be popped off the queue. An example of how this might be done is given below.

```python
while h.helicsEndpointPendingMessages(end) > 0:
    msg_obj = h.helicsEndpointGetMessageObject(end)
```

To get the source of each of the messages received at an endpoint, the following call can be used:

```python
msg_source = h.helicsMessageGetOriginalSource(msg_obj)
```

### Internal Updates and Calculations

At this point, your federate has received all of its input information from the other federates in the co-simulation and is now ready to run whatever updates or calculations it needs to for the current time step.

### Publish Outputs

Once the new inputs have been collected and all necessary calculations made, the federate can publish whatever information it needs to for the rest of the federation to use. The code sample below shows how these output values can be published out to the federation using HELICS API calls. As in when reading in new values, these output values can be published as a variety of data types and HELICS can handle type conversion if one of the receivers of the value asks for it in a type different than published.

```python
h.helicsPublicationPublishInteger(pub, int_value)
h.helicsPublicationPublishDouble(pub, float_value)
h.helicsPublicationPublishComplex(pub, real_value, imag_value)
h.helicsPublicationPublishChar(pub, string_value)
...
```

For sending a message through an endpoint, that once again looks a little bit different, in this case because - unlike with a publication - a message requires a destination. If a default destination was set when the endpoint was registered (either through the config file or through calling `h.helicsEndpointSetDefaultDestination()`), then an empty string can be passed. Otherwise, the destination must be provided as shown in API call below where `dest` is the destination and `msg` is the message to be sent.

```python
h.helicsEndpointSendMessageRaw(end, dest, msg)
```

## Final State

Once the federate has completed its contribution to the co-simulation, it needs to close out its connection to the federation. Typically a federate knows it has reached the end of the co-simulation when it is granted `maxTime`. To leave the federation cleanly (without causing errors for itself or others in the co-simulation) the following process needs to be followed:

```python
h.helicsFederateFinalize(fed)
h.helicsFederateFree(fed)
h.helicsCloseLibrary()
```

`helicsFederateFinalize()` signals to the core and brokers that this federate is leaving the co-simulation. This process will take an indeterminate amount of time and thus it is necessary to poll the connection status to the broker. Once that connection has closed, the memory of the federate (associated with HELICS) is freed up with `helicsFederateFree()` and the processes in the HELICS library are terminated with `helicsCloseLibrary()`. At this point, the federate can safely end execution completely.
