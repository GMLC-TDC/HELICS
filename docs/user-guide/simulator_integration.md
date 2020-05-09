# Integrating a Simulator with HELICS

At some point, maybe from the very beginning of your time with HELICS co-simulation, you'll have an interest or need to include a simulator in your co-simulation that HELICS doesn't support. Maybe it's an existing open-source simulator, maybe it's commercial software, maybe it's a small controller simulator you'd like to test in an existing model. HELICS has been designed to make it as easy as possible to integrate a new simulator. Before writing code, though, it is important to more specifically define the task.

## Simulator Integration Clarifying Questions

1. **What is the nature of the code-base being integrated?** Is this open-source code that can be fully modified? Is it a simulator, perhaps commercial, that provides an API that will be used? How much control do you, the integrator, have in modifying the behavior of the simulator?
2. **What programming language will be used?** - HELICS has bindings for a number of languages and the one that is best to use may or may not be obvious. If you're integration of the simulator will be through the API of the existing simulator, then you'll likely be writing a standalone executable that wraps that API. You may be constrained on the choice of languages based on the method of interaction with that API. If the API is accessed through a network socket then you likely have a lot of freedom in language choice. If the API is a library that you call from within wrapper, you will likely be best of using the language of that library.

   If you're writing your own simulator then you have a lot more freedom and the language you use may come down to personal preference and/or performance requirements of the federate.

   The languages currently supported by HELICS are:

   - C++
   - C
   - Python (2 and 3)
   - Java
   - MATLAB
   - Octave
   - C# (somewhat limited as of yet)
   - Julia
   - Nim

3. **What is the simulators concept of time?** - Understanding how the simulator natively moves through time is essential when determining how time requests will need to be made. Does the simulator have a fixed time-step? Is it user-definable? Does the simulator have any concept of time or is it event-based?
4. **What is the nature of the values it will send to and receive from the rest of the federation?** Depending on the nature of the simulator, this may or may not be specifically definable but a general understanding of how this simulator will be used in a co-simulation should be clear. As a stand-alone simulator, what are its inputs and outputs? What are its assumed or provided boundary conditions? What kinds of values will it be providing to the rest of the federation?

## The Essential APIs

With the answers to those clarifying questions in mind, let's look at the normal execution process used by a HELICS federate when co-simulating and the associated APIs for each of the languages. Many of these APIs are wrappers for one or more lower level APIs; additionally, there are many more detailed APIs that won't be discussed at all. If, as the simulator integrator, you have needs beyond what is discussed here you'll have to dig into the [developer documentation on the APIs](../doxygen/index.md) to get the details you need.

For the remainder of this section of the guide, we'll assume the use of a Python binding and thus, at the top of the Python script ([after installing the Python HELICS module](https://helics.readthedocs.io/en/latest/installation/index.html)), you'll have to do something like this:

```python
import helics as h
```

### Broker Creation

Though not technically a pat of integrating a simulator its important to remember that as a part of running a co-simulation, a broker will need to be created. This can be done as part of what an existing federate does, as a part of a stand-alone broker-creation federate, or with helics_cli. Broker creation is done with just a single API call:

```python
broker = h.helicsCreateBroker("zmq", "main_broker", "--federates 2")

```

The [Doxygen on this function](../doxygen/helics_8h.html#aeb64e4cbbfd666b121a2814a0baef4de) shows that the first argument defines the core, the second the name of the broker, and the third is an initialization string which in this case, only specifies the number of federates in the federation.

### HELICS Core Creation

Given one or more existing simulators that need to be integrated, at some point in the code it will be necessary to create a federate instance of that simulator. Doing so established the message-passing and synchronization infrastructure that is required to be part of a HELICS co-simulation.

The easiest way to do this is using a specific API that reads in a user-defined JSON file to create the federate. These are the configuration files that we have been examining in part or whole throughout the this guide and are used in the examples. By placing all the configuration information in the JSON file, it allows maximum modularity and flexibility of the simulator being integrated. Using the JSON file allows all future users of the simulator to modify and customize the connection between that simulator and any given HELICS federation without having to modify the source code.

There are ways to programmatically ("hard-code") the configuration of the federate and for small, one-off simulators (like an EV charge controller, for example), doing so may be the fastest way to get the HELICS co-simulation up and running. Then again, how often does one-off code stay one-off....

The JSON configuration file, as discussed earlier in this guide, contains information both about the federate in general (which core type is being used, what its time-step is) as well as the information it will be providing to the federate and receiving from it. HELICS has a single API command to read in that file and create the federate:

```python
fed = h.helicsCreateValueFederateFromConfig('Control.json')
```

(There are equivalent methods for `helicsCreateMessageFederateFromConfig()` and `h.helicsCreateCombinationFederateFromConfig()`). This function creates the federate object `fed` based on the path to the JSON configuration file.

### Value/Endpoint Configuration

With all the information provided in the configuration JSON, HELICS is fully aware of what your custom federate is going to be sending and receiving to and from the federation but, ironically, your federate probably doesn't. That is, as the creator of this federate you will need to define in your code somewhere what values you are going to be sending out and what to do with the values you will be subscribing.

The HELICS names for all those messages is in the JSON configuration and you could write a parser to read in that file and make the connection to your internal variables. HELICS has already read and parsed the file, though, and to avoid everybody having to reinvent the wheel, it provides methods to access the necessary information:

```python
input_count = h.helicsFederateGetInputCount(fed)
input_ID = h.helicsFederateGetInputByIndex(fed, index)
input_key = h.helicsSubscriptionGetKey(input_ID)
```

```python
pub_count = h.helicsFederateGetPublicationCount(fed)
pub_ID = h.helicsFederateGetPublicationByIndex(fed, index)
pub_key = helicsPublicationGetKey(pub_ID)
```

```python
endpoint_count = h.helicsFederateGetPublicationCount(fed)
endpoint_ID = h.helicsFederateGetEndpointByIndex(fed, index)
endpoint_name = helicsEndpointGetName(endpoint_ID)

```

Getting the number of the inputs/publications/endpoints and then looping over them making a call to get the unique ID of each one allows specific information from the JSON to be accessed by that ID. Most importantly, it allows access to the name (sometimes called the "key") and the "info" field in the JSON. "info" is specifically not used by HELICS and is purely there as a means to allow you, the simulator developer, to do whatever you need to do to complete the federates integration into the co-simulation.

### Federate Execution

Once any linking between wider federation and the custom federate being created is complete, the federate itself indicates it is ready to begin advancing in time:

```python
h.helicsFederateEnterExecutingMode(fed)
```

This method call is a blocking call; your custom federate will sit there and do nothing until all other federates have also finished any set-up work and have also requested to begin execution of the co-simulation. Once this method returns, the federation is effectively at simulation time of zero.

And now begins the core of the co-simulation where the following several steps are looped over for the duration of the simulated time:

- **Request a simulation time**

  ```python
  grantedtime = h.helicsFederateRequestTime (fed, time)
  ```

  Assuming any necessary calculations have been completed, the federate requests a simulated time. This time is determined by the nature of the simulator and generally represents the maximum time over which, in none of the inputs of the simulator change, no new outputs would need to be calculated. For simulators with a fixed time-step, the time requested will be the next time-step. (For these types of simulators, it's a good idea to [set the "uninterruptible" flag](./timing.md) as well, just to keep the simulator on these intervals.)

  For other types of simulators, controller for example, you may want to change an output every time an input changes, but never any other time. In these cases, you can make the time request of `maxTime`; this is the end of the simulation time and thus the federate will do nothing until a new input value changes and the federate is granted that time. (In this case, you would want to make sure the "uninterruptible" flag was NOT set so that the federate is woken up on these input changes.)

  Like `helicsFederateEnterExecutingMode`, this method is a blocking call. Your federate will do nothing until the HELICS core has granted a time to it.

- **Get new input values**

  ```python
  int_value = h.helicsInputGetInteger(sub_ID)
  float_value = h.helicsInputGetDouble(sub_ID)
  real_value, imag_value = h.helicsInputGetComplex(sub_ID)
  string_value = h.helicsInputGetChar(sub_ID)
  ...

  ```

  Once granted a time, the federate is woken up and can begin execution. The granted time may or may not be the requested time as the arrival of new inputs from the federation can cause the federate to be woken up prior to the requested time. More than likely, your federate will want to check what time has been granted and may choose different paths of execution based on whether this was the requested time or not.

  As part of this execution the federate will almost certainly want to update all its inputs from the federation and use these in performing the key operations of the federate. The APIs above show how these call can be made. As can be seen, HELICS has built in type conversion ([where possible](https://www.youtube.com/watch?v=mZOAn-3aATY)) and regardless of how the sender of the data has formatted it, HELICS can present it as requested by the appropriate method call.

- **Output new values**

  ```python
  helicsPublicationPublishInteger(pub_ID, int_value)
  helicsPublicationPublishDouble(pub_ID, float_value)
  helicsPublicationPublishComplex(pub_ID, real_value, imag_value)
  helicsPublicationPublishChar(pub_ID, string_value)
  ...

  ```

  Once the new inputs have been collected and all necessary calculations made, the federate can update it's values for the rest of the federation to use. The API calls above allow these output values to be published out to the federation. As in when reading in new values, these output values can published as a variety of data types and HELICS can handle type conversion if one of the receivers of the value asks for it in a type different than published.

### Federate Finalization

Once the federate has completed its contribution to the it needs to close out its connection to the federation. Typically a federate knows it has reached the end of the co-simulation when it is granted `maxTime`. To leave the federation cleanly (without causing errors for itself or others in the co-simulation) the following process needs to be followed:

```python
h.helicsFederateFinalize(fed)
#wait until the broker is finished (-1 is indefinite timeout otherwise it is the number of ms to wait)
h.helicsBrokerWaitForDisconnect(broker, -1);
h.helicsFederateFree(fed)
h.helicsCloseLibrary()
```

`helicsFederateFinalize()` signals to the core and brokers that this federate is leaving the co-simulation. This process will take an indeterminate amount of time and thus it is necessary to poll the connection status to the broker. Once that connection has closed, the memory of the federate (associated with HELICS) is freed up with `helicsFederateFree()` and the processes in the HELICS library are terminated with `helicsCloseLibrary()`. At this point, the federate can safely end execution completely.
