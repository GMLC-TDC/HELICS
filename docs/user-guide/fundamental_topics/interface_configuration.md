# Federate Interface Configuration

As soon as one particular instance of a simulator begins running in a co-simulation it is a federate. Every federate will require configuration of the way it will communicate (send signals) to other federates in the federation. For [simulators that already have HELICS support](../../references/tools_using_helics.md), the configuration takes the form of a JSON (or TOML) file; bespoke simulators can be configured with the HELICS APIs in the code or via a JSON file. The essential information required to configure federate interfaces with HELICS is:

   **Federate name** - The unique name this federate will be known as throughout the federation. It is essential this name is unique so that HELICS messages can route properly.

   **Core type** - The core manages interfaces between the federation and the federate; there are several messaging technologies supported by HELICS. 

   **Publications and Inputs** - Publication configuration contains a listing of source handle, data types, and units being sent by the federate; input configuration does the same for values being received by the federate. If supported by the simulator (e.g., [a Python simulator](../examples/fundamental_examples/fundamental_default.md)), these values can be mapped to internal variables of the simulator from the configuration file. 

   **Endpoints** - Endpoints are sending and receiving points for HELICS messages to and from message federates. They are declared and defined for each federate. 

   **Time step size** - This value defines the resolution of the simulator to prevent HELICS from telling the simulator to step to a time of which it has no concept (e.g. trying to simulate the time of 1.5 seconds when the simulator has a resolution of one second). 
   
This section describes how to configure the federate interfaces using JSON files and API calls.

* [JSON Configuration](#json-configuration)
* [API Configuration](#api-configuration)

## JSON Configuration

Federate interfaces must be configured with JSON files if they are built from non-open-source tools, such as the simulators listed in the reference page on [Tools with HELICS Support](../../references/tools_using_helics.md). Federates built from simulators written by the user can be configured with JSON files or API calls.

The [Examples](../../examples/examples_index.md) illustrate in detail how to integrate federates built from open source tools, such as Python. The [Fundamental Combination Federation](../examples/fundamental_combo.md) configures the Python federate interfaces with JSON files. In this co-simulation, there are three federates: a value Battery federate, a combination Charger federate, and a message Controller federate. The example JSON shows the interface configuration for the combination federate to illustrate the different types of interfaces.

### Sample JSON configuration file

The most common parameters are set in the file `ChargerConfig.json`. There are many, many more configuration parameters that this file could include; a relatively comprehensive list along with explanations of the functionality they provide can be found in the [Configurations Options Reference](../../references/configuration_options_reference.md).


```
{
  "name": "Charger",
  "loglevel": 7,
  "coreType": "zmq",
  "period": 60,
  "uninterruptible": false,
  "terminate_on_error": true,
  "endpoints": [
    {
      "name": "Charger/EV1.soc",
      "destination": "Controller/ep",
      "global": true
    },
    {
    ...
    }
  ],
  "publications":[
    {
      "key":"Charger/EV1_voltage",
      "type":"double",
      "unit":"V",
      "global": true
    },
    {
	...
    }
  ],
  "subscriptions":[
    {
      "key":"Battery/EV1_current",
      "type":"double",
      "unit":"A",
      "global": true
    },
    {
	...
	}
  ]
}
```


### JSON configuration file explanation

- **`name`** - Every federate must have a unique name across the entire federation; this is functionally the address of the federate and is used to determine where HELICS messages are sent. An error will be generated if the federate name is not unique.
- **`loglevel`** - The level of detail exported to the [log files](./logging.md) during run time ranges from 1 (minimal) to 7 (most).
- **`coreType`** - There are a number of technologies or message buses that can be used to send HELICS messages among federates, detailed in [Core Types](../advanced_topics/CoreTypes.md). Every HELICS enabled simulator has code in it that creates a core which connects to a HELICS broker using one of these messaging technologies. ZeroMQ (zmq) is the default core type and most commonly used, but there are also cores that use TCP and UDP networking protocols directly (forgoing ZMQ's guarantee of delivery and reconnection functions), IPC (uses Boost's interprocess communication for fast in-memory message-passing but only works if all federates are running on the same physical computer), and MPI (for use on HPC clusters where MPI is installed).
- **`period`** - The federate needs instruction for how to step forward in time in order to synchronize calculations. This is the simplest way to synchronize simulators to the same time step; this forces the federate to time step intervals of `n*period`. The default units are in seconds.  Timing configuration is explained in greater detail in the [Timing](./timing.md) page, with additional configuration options in the [Configuration Options Reference](../../references/configuration_options_reference.md#timing-options). 
- **`uninterruptible`** - Setting `uninterruptible` to `false` allows the federate to be interrupted if there is a signal available for it to receive. This is a [timing](./timing.md) configuration option.
- **`terminate_on_error`** - Setting `terminate_on_error` frees the federate from the broker if there is an error in execution, which simplifies debugging.
- **`endpoints`**
	- `name` - The string in this field is the unique identifier/handle for the endpoint interface.
	- `destination` - This option can be used to set a default destination for the messages sent from this endpoint. The default destination is allowed to be rerouted or changed during run time.
	- `global` - Just as in value federates, `global` allows for the identifier of the endpoint to be declared unique for the entire federation.
- **`publications`**
	- `key` - The string in this field is the unique identifier/handle (at the federate level) for the value that will be published to the federation. In the example above, `global` is set to `true`, meaning the `key` must be unique to the entire federation.
	- `global` - Indicates that the value in `key` will be used as a global name when other federates are subscribing to the message. This requires that the user ensure that the name is used only once across all federates. Setting `global` to `true` is handy for federations with a small number of federates and a small number of message exchanges as it allows the `key` string to be short and simple. For larger federations, it is likely to be easier to set the flag to `false`.
	- `required` -  At least one federate must subscribe to the publications.
	- `type` - Data type, such as integer, double, complex.
	- `units` - The units can be any sort of unit string, a wide assortment is supported and can be compound units such as m/s^2 and the conversion will convert as long as things are convertible. The unit match is also checked for other types and an error if mismatching units are detected. A warning is also generated if the units are not understood and not matching. The unit checking and conversion is only active if both the publication and subscription specify units. HELICS is able to do some levels of unit conversion, currently only on double type publications but more may be added in the future.
- **`subscriptions`** - These are lists of the values being sent to and from the given federate.
	- `key` - This string identifies the federation-unique value that this federate wishes to receive. If `global` has been set to `false` in the `publications` JSON configuration file, the name of the value is formatted as `<federate name>/<publication key>`. Both of these strings can be found in the publishing federate's JSON configuration file as the `name` and `key` strings, respectively. If `global` is `true` the string is the publishing federate's `key` value.
	- `required` - The message being subscribed to must be provided by some other publisher in the federation.
	- `type` - Data type, such as integer, double, complex.
	- `units` - Same as with `publications`.

## API Configuration

### Sample pyhelics API configuration 

```
def create_combo_federate(fedinitstring,name,period):
    fedinfo = h.helicsCreateFederateInfo()
    # "coreType": "zmq",
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")
    h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)
    # "loglevel": 1,
    h.helicsFederateInfoSetIntegerProperty(fedinfo, h.helics_property_int_log_level, 11)
    # "period": 60,
    h.helicsFederateInfoSetTimeProperty(fedinfo, h.helics_property_time_period, period)
    # "uninterruptible": false,
    h.helicsFederateInfoSetFlagOption(fedinfo, h.helics_flag_uninterruptible, False)
    # "terminate_on_error": true,
    h.helicsFederateInfoSetFlagOption(fedinfo, h.HELICS_FLAG_TERMINATE_ON_ERROR, True)
    #h.helicsFederateInfoSetFlagOption(fedinfo, 72, True)
    # "name": "Charger",
    fed = h.helicsCreateCombinationFederate(name, fedinfo)
    return fed
```


### pyhelics API configuration explanation


## Typical Federate Execution
For the remainder of this section of the guide, we'll walk through the typical stages of co-simulation, providing examples of how these might be implemented using HELICS API calls. For the purposes of these examples, we will assume the use of a Python binding. If, as the simulator integrator, you have needs beyond what is discussed here you'll have to dig into the [developer documentation on the APIs](../../references/api-reference/index.md) to get the details you need.

To begin, at the top of your Python module ([after installing the Python HELICS module](https://helics.readthedocs.io/en/latest/installation/index.html)), you'll have to import the HELICS library, which will look something like this:

```python
import helics as h
```

### Federate Information

Each federate has a core set of configuration information and metadata associated with it, which will either need to be set within your code or will be set based on defaults. When creating a new federate, only one piece of metadata is actually required, and that is the federate name, which must be unique within the federation. However, there are many other configuration options that can be set for the federate, including whether the federate can be interrupted between its native time steps, a minimum time step for its execution and the level to use when the federate logs information. Information on all of these configuration options, including default settings, can be found [here](./../configuration/FederateFlags.md). 



### Create the HELICS Federate

Now that you've decided what kind of federate you are going to use to instantiate your simulator within the federation, you'll need to actually create that federate in your code. There are two ways to do this: from a configuration file or programmatically, using a sequence of HELICS API calls. In most instances, using a configuration file is probably simpler and more modular. However, we will go through both options below as there may be times when creating the federate in your source code is necessary or more appropriate.

#### Using a Config File

In HELICS there is a single API call that can be used to read in all of the necessary information for creating a federate from a JSON configuration file. The JSON configuration file, as discussed earlier in this guide, contains both the federate info as well as the metadata required to define the federate's publications, subscriptions and endpoints. The API calls for creating each type of federate are given below. 

For a value federate:

```python
fed = h.helicsCreateValueFederateFromConfig('fed_config.json')
```

For a message federate:

```python
fed = h.helicsCreateMessageFederateFromConfig('fed_config.json')
```

For a combination federate:

```python
fed = h.helicsCreateCombinationFederateFromConfig('fed_config.json')
```

In all instances, this function returns the federate object `fed` and requires a path to the JSON configuration file as an input.

#### Using HELICS API Calls

Additionally, there are ways to create and configure the federate directly through HELICS API calls, which may be appropriate in some instances. First, you need to create the federate info object, which will later be used to create the federate:

```python
fi = h.helicsCreateFederateInfo()
```

Once the federate info object exists, HELICS API calls can be used to set the [configuration parameters](./../configuration/FederateFlags.md) as appropriate. For example, to set the the only_transmit_on_change flag to true, you would use the following API call:

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

Once the federate is created, you also have the option to set the federate information at that point, which - while functionally identical to setting the federate info in either the federate config file or in the federate info object - provides integrators with additional flexibility, which can be useful particularly if some settings need to be changed dynamically during the cosimulation. The API calls are syntatically very similar to the API calls for setting up the federate info object, except instead they target the federate itself. For example, to revist the above example where the only_transmit_on_change on change flag is set to true in the federate info object, if operating on an existing federate, that call would be:

```python
h.helicsFederateSetFlagOption(fi, 6, True)
```

### Error Handling

By default, HELICS will not terminate execution of every participating federate if an error occurs in one. However, in most cases, if such an error occurs, the cosimulation is no longer valid. It is therefore generally a good idea to set the following flag in your simulator federate so that its execution will be terminated if an error occurs anywhere in the cosimulation.

```python
h.helicsFederateSetFlagOption(fed, helics_flag_terminate_on_error)
```
This will prevent your federate from hanging in the event that another federate fails.


