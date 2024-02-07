# Simulator Integration

A "simulator" is the executable program. As soon as one particular instance of that simulator begins running in a co-simulation it is considered a "federate". Every federate (instance of a simulator) will require configuration of the way it will communicate (send signals) to other federates in the federation. For simulators that already have HELICS support, the configuration takes the form of a JSON (or TOML) file; bespoke simulators can be configured with the HELICS APIs in the code or via a JSON file. The essential information that HELICS configuration defines is:

**Federate name** - The unique name this federate will be known as throughout the federation. It is essential this name is unique so that HELICS messages can route properly.

**Core type** - The core manages interfaces between the federation and the federate; there are several messaging technologies supported by HELICS.

**Publications and Inputs** - Publication configuration contains a listing of source interface name, data types, and units being sent by the federate; input configuration does the same for values being received by the federate. If supported by the simulator (e.g., [a Python simulator](../examples/fundamental_examples/fundamental_default.md)), these values can be mapped to internal variables of the simulator from the configuration file.

**Endpoints** - Endpoints are sending and receiving points for HELICS messages to and from message federates. They are declared and defined for each federate.

**Time step size** - This value defines the resolution of the simulator to prevent HELICS from telling the simulator to step to a time of which it has no concept (e.g. trying to simulate the time of 1.5 seconds when the simulator has a resolution of one second).

## Integration of Federates

A co-simulation is, in some sense, a simulation of simulations. There will be two types of configuration required:

1. Individual federates (identifying models to be used, defining the start and stop time of the simulation, defining how the results of the simulation should be stored, etc...) and
2. How each federate will connect to and interact with the other federates in the co-simulation.

One of the goals of a co-simulation platform like HELICS is to make the connecting easier and more efficient by providing a standardized method of configuration.

Integration of federates requires definition of the message topology (who is passing what information to whom) and the broker topology (which federates/cores are connected to which brokers). Message topology requires understanding the interactions of the system the simulators are trying to replicate and identifying the boundaries where they could exchange data. Broker topology will be kept simple for the Fundamental Topics and Examples.

This section introduces the simplest broker topology for integrating federates into a federation, and the basics for integrating federates with a JSON and with API calls.

```{eval-rst}
.. toctree::
    :maxdepth: 1

```

## Broker Topology

Broker topology is somewhat optional for simple co-simulations, but offers an increase in performance if it is possible to identify groups of federates that interact often with each other but rarely with the rest of the federation. In such cases, assigning that group of federates their own broker will remove the congestion their messages cause with the federation as a whole. The Fundamental Topics and Examples are built with a single broker.

The figure below shows the most common architecture for HELICS co-simulation. Each core has only one federate as an integrated executable, all executables reside on the same computer and are connected to the same broker. This architecture is particularly common for small federates and/or co-simulations under development. This is also the architecture for the [Fundamental Examples](../examples/fundamental_examples/fundamental_examples_index.md).

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/helics_architecture_1.png)

## Configuring the federate

Let's look at a generic JSON configuration file as an example with the more common parameters shown. As we'll see [later in this section](#using-a-config-file), this file is loaded by the federate using a specific API, allowing the same simulator to be used to create many federates that are all unique without having the modify the source code of the simulator. There are many, many more configuration parameters that this file could include; a relatively comprehensive list along with explanations of the functionality they provide can be found in the [federate configuration](../../references/configuration_options_reference.md) guide.

### Sample federate JSON configuration file

```json
{
    "name":"generic_federate",
    "coreType": "zmq"
    "publications" : [
          {
               "key" : "IEEE_123_feeder_0/totalLoad",
               "global" : true,
               "type" : "complex",
               "unit" : "VA",
          }
     ],
     "subscriptions" : [
          {
               "required": true,
               "key" : "TransmissionSim/transmission_voltage",
               "type" : "complex",
               "unit" : "V",
               "info" : "{
                    \"object\" : \"network_node\",
                    \"property\" : \"positive_sequence_voltage\"
                    }"
          }
     ],
     "endpoints" : [
        {
            "name" : "voltage_sensor",
            "global" : true,
            "destination" : "voltage_controller",
            "info" : ""
        }
     ]
}
```

### JSON configuration file explanation

- **`name`** - Every federate must have a unique name across the entire federation; this is functionally the address of the federate and is used to determine where HELICS messages are sent. An error will be generated if the federate name is not unique.
- **`coreType`** - There are a number of technologies or message buses that can be used to send HELICS messages among federates. Every HELICS enabled simulator has code in it that creates a core which connects to a HELICS broker using one of these messaging technologies. ZeroMQ (zmq) is the default core type and most commonly used but there are also cores that use TCP and UDP networking protocols directly (forgoing ZMQ's guarantee of delivery and reconnection functions), IPC (uses Boost's interprocess communication for fast in-memory message-passing but only works if all federates are running on the same physical computer), and MPI (for use on HPC clusters where MPI is installed).
- **`publications` and/or `subscriptions`** - These are lists of the values being sent to and from the given federate.
- **`key`** -
  - `publications` - The string in this field is the unique identifier (at the federate level) for the value that will be published to the federation. If `global` is set (see below) it must be unique to the entire federation.
  - `subscriptions` - This string identifies the federation-unique value that this federate wishes to receive. Unless `global` has been set to `true` in the publishings JSON configuration file, the name of the value is formatted as `<federate name>/<publication key>`. Both of these strings can be found in the publishing federate's JSON configuration file as the `name` and `key` strings, respectively. If `global` is `true` the string is just the `key` value.
- **`global`** - (publications only) `global` is used to indicate that the value in `key` will be used as a global name when other federates are subscribing to the message. This requires that the user ensure that the name is used only once across all federates. Setting `global` to `true` is handy for federations with a small number of federates and a small number of message exchanges as it allows the `key` string to be short and simple. For larger federations, it is likely to be easier to set the flag to `false` and accept the extra naming
- **`required`** -
  - `publications` - At least one federate must subscribe to the publications.
  - `subscriptions` - The message being subscribed to must be provided by some other publisher in the federation.
- **`type`** - HELICS supports data types and data type conversion ([as best it can](https://www.youtube.com/watch?v=mZOAn-3aATY)).
- **`units`** - HELICS is able to do some levels of unit conversion, currently only on double type publications but more may be added in the future. The units can be any sort of unit string, a wide assortment is supported and can be compound units such as m/s^2 and the conversion will convert as long as things are convertible. The unit match is also checked for other types and an error if mismatching units are detected. A warning is also generated if the units are not understood and not matching. The unit checking and conversion is only active if both the publication and subscription specify units.
- **`info`** - The `info` field is entirely ignored by HELICS and is used as a mechanism to pass configuration information to the federate so that it can properly integrate into the federation. Thus, there is no standard content or format for this field; it is entirely up to the individual simulators to decide how the data in this field (if any) should be used. Often it is used by simulators to map the HELICS names into internal variable names as shown in the above example. In this case, the object `network_node` has a property called `positive_sequence_voltage` that will be updated with the value from the subscription `TransmissionSim/transmission_voltage`.
- **`global`** - Just as in value federates, `global` allows for the identifier of the endpoint to be declared unique for the entire federation.
- **`destination`** - For endpoints that send all outgoing messages to only a single endpoint, `destination` allows the endpoint to be specified in the JSON configuration. This allows for a more modular implementation of the federate since this parameter is externally defined rather than being hardcoded in the federate itself.
- **`info`** - Just as in the value federate, the string in this field is ignored by HELICS and can be used by the federate for internal configuration purposes.

## Typical Federate Execution

For the remainder of this section of the guide, we'll walk through the typical stages of co-simulation, providing examples of how these might be implemented using HELICS API calls. For the purposes of these examples, we will assume the use of a Python binding. If, as the simulator integrator, you have needs beyond what is discussed here you'll have to dig into the [developer documentation on the APIs](../../references/api-reference/index.md) to get the details you need.

To begin, at the top of your Python module ([after installing the Python HELICS module](../installation/index.md)), you'll have to import the HELICS library, which will look something like this:

```python
import helics as h
```

### Federate Information

Each federate has a core set of configuration information and metadata associated with it, which will either need to be set within your code or will be set based on defaults. When creating a new federate, only one piece of metadata is actually required, and that is the federate name, which must be unique within the federation. However, there are many other configuration options that can be set for the federate, including whether the federate can be interrupted between its native time steps, a minimum time step for its execution and the level to use when the federate logs information. Information on all of these configuration options, including default settings, can be found in the [Configurations Options Reference](../../references/configuration_options_reference.md).

### Publications, Subscriptions and Endpoints

One of the first design choices you have to make is the type of federate that you will create to instantiate your simulator within the co-simulation. At this point, we will revisit the question on what kind of data you expect your simulator to exchange with the rest of the federation. There are three kinds of federates within HELICS: [value federates](./value_federates.md), [message federates](./message_federates.md), and combination federates.

Value federates are used to exchange physical values through HELICS using a publication/subscription architecture, where only a single value can be received at a given subscription at each time step. Value federates are used to represent physics-based interdependencies. An example of where the exchange of values is probably most appropriate is where the same data point is represented in two different simulators, such as the voltage at a transmission bus that corresponds to the voltage at a distribution feeder head.

By contrast, message federates are used to exchange messages through HELICS that look and behave more like communications-based data. Examples of this might include control signals or measurement data. This is done using endpoints, rather than publications and subscriptions, and unlike in the value case, more than one message can be received at an endpoint at any given time step.

A combination federate is one that handles both values and messages. More details on the differences between these federate types are provided elsewhere in this guide.

### Create the HELICS Federate

Now that you've decided what kind of federate you are going to use to instantiate your simulator within the federation, you'll need to actually create that federate in your code. There are two ways to do this: from a configuration file or programmatically, using a sequence of HELICS API calls. In most instances, using a configuration file is probably simpler and more modular. However, we will go through both options below as there may be times when creating the federate in your source code is necessary or more appropriate.

#### Using a Config File

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

#### Using HELICS API Calls

Additionally, there are ways to create and configure the federate directly through HELICS API calls, which may be appropriate in some instances. First, you need to create the federate info object, which will later be used to create the federate:

```python
fi = h.helicsCreateFederateInfo()
```

Once the federate info object exists, HELICS API calls can be used to set the [configuration parameters](../../references/configuration_options_reference.md) as appropriate. For example, to set the the only_transmit_on_change flag to true, you would use the following API call:

```python
h.helicsFederateInfoSetFlagOption(fi, 6, True)
```

Once the federate info object has been created and the appropriate options have been set, the helics federate can be created by passing in a unique federate name and the federate info object into the appropriate HELICS API call. For creating a value federate, that would look like this:

```python
fed = h.helicsCreateValueFederate(federate_name, fi)
```

Once the federate is created, you now need to define all of its publications, subscriptions and endpoints. The first step is to create them by registering them with the federate with an API call that looks like this:

```python
pub = h.helicsFederateRegisterPublication(fed, key, data_type)
```

This call takes in the federate object, a string containing the publication key (which will be prepended with the federate name), and the data type of the publication. It returns the publication object. Once the publication, subscription and endpoints are registered, additional API calls can be used to set the info field in these objects and to set certain options. For example, to set the only transmit on change option for a specific publication, this API call would be used:

```python
pub = h.helicsPublicationSetOption(pub, 454, True)
```

Once the federate is created, you also have the option to set the federate information at that point, which - while functionally identical to setting the federate info in either the federate config file or in the federate info object - provides integrators with additional flexibility, which can be useful particularly if some settings need to be changed dynamically during the cosimulation. The API calls are syntactically very similar to the API calls for setting up the federate info object, except instead they target the federate itself. For example, to revisit the above example where the only_transmit_on_change on change flag is set to true in the federate info object, if operating on an existing federate, that call would be:

```python
h.helicsFederateSetFlagOption(fi, 6, True)
```

### Error Handling

By default, HELICS will not terminate execution of every participating federate if an error occurs in one. However, in most cases, if such an error occurs, the cosimulation is no longer valid. It is therefore generally a good idea to set the following flag in your simulator federate so that its execution will be terminated if an error occurs anywhere in the cosimulation.

```python
h.helicsFederateSetFlagOption(fed, helics_flag_terminate_on_error)
```

This will prevent your federate from hanging in the event that another federate fails.

### Collecting the Publication, Subscription and Endpoint Objects

Having configured the publications, subscriptions and endpoints and registered this information with HELICS, the channels for sending and receiving this information have been created within the cosimulation framework. If you registered the publication, subscriptions and endpoints within your code (i.e., using HELICS API calls), you already have access to each respective object as it was returned when made the registration call. However, if you created your federate using a configuration file which contained all of this information, you now need to retrieve these objects from HELICS so that you can invoke them during the execution of your cosimulation. The following calls will allow you to query HELICS for the metadata associated with each publication. Similar calls can be used to get input (or subscription) and endpoint information.

```python
pub_count = h.helicsFederateGetPublicationCount(fed)
pub = h.helicsFederateGetPublicationByIndex(fed, index)
pub_key = h.helicsPublicationGetKey(pub)
```

The object returned when the helicsFederateGetPublicationByIndex() method is invoked is the interface object used for retrieving other publication metadata (as in the helicsPublicationGetKey() method) and when publishing data to HELICS (as described in the execution section below).

### Federate Execution

Once the federate has been created, all subscriptions, publications and endpoints have been registered and, all the federate information has been appropriately set, it is time to enter executing mode. This can be done with the following API call:

```python
h.helicsFederateEnterExecutingMode(fed)
```

This method call is a blocking call; your custom federate will sit there and do nothing until all other federates have also finished any set-up work and have also requested to begin execution of the co-simulation. Once this method returns, the federation is effectively at simulation time of zero.

At this point, each federate will now set through time, exchanging values with other federates in the cosimulation as appropriate. This will be implemented in a loop where each federate will go through a set of prescribed steps at each time step. At the beginning of the cosimulation, time is at the zeroth time step (t = 0). Let's assume that the cosimulation will end at a pre-determined time, t = max_time. The nature of the simulator will dictate how the time loop is handled. However, it is likely that the cosimulation loop will start with something like this:

```python
t = 0
while t < end_time:
    pass  # cosimulation code would go here
```

Now, the federate begins to step through time. For the purposes of this example, we will assume that during every time step, the federate will first take inputs in from the rest of the cosimulation, then make internal updates and calculations and finish the time step by publishing values back to the rest of the cosimulation before requesting the next time step.

#### Get Inputs

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

Receiving messages at an endpoint works a little bit differently than when receiving values through a subscription. Most fundamentally, there may be multiple messages queued at an endpoint while there will only ever be one value received at a subscription (the last value if more than one is sent prior to being retrieved). To receive all of the messages at an endpoint, they needed to be popped off the queue. An example of how this might be done is given below.

```python
while h.helicsEndpointPendingMessages(end) > 0:
    msg_obj = h.helicsEndpointGetMessageObject(end)
```

To get the source of each of the messages received at an endpoint, the following call can be used:

```python
msg_source = h.helicsMessageGetOriginalSource(msg_obj)
```

#### Internal Updates and Calculations

At this point, your federate has received all of its input information from the other federates in the co-simulation and is now ready to run whatever updates or calculations it needs to for the current time step.

#### Publish Outputs

Once the new inputs have been collected and all necessary calculations made, the federate can publish whatever information it needs to for the rest of the federation to use. The code sample below shows how these output values can be published out to the federation using HELICS API calls. As in when reading in new values, these output values can published as a variety of data types and HELICS can handle type conversion if one of the receivers of the value asks for it in a type different than published.

```python
h.helicsPublicationPublishInteger(pub, int_value)
h.helicsPublicationPublishDouble(pub, float_value)
h.helicsPublicationPublishComplex(pub, real_value, imag_value)
h.helicsPublicationPublishChar(pub, string_value)
...
```

For sending a message through an endpoint, that once again looks a little bit different, in this case because - unlike with a publication - a message requires a destination. If a default destination was set when the endpoint was registered (either through the config file or through calling `h.helicsEndpointSetDefaultDestination()`), then an empty string can be passed. Otherwise, the destination must be provided as shown in API call below where dest is the destination and msg is the message to be sent.

```python
h.helicsEndpointSendMessageRaw(end, dest, msg)
```
