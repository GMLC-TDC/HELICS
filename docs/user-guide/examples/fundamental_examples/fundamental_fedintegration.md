# Federate Integration with PyHELICS API

The Federate Integration Example extends the Base Example to demonstrate how to integrate federates using the HELICS API instead of JSON config files. This tutorial is organized as follows:

- [Computing Environment](#computing-environment)
- [Example files](#example-files)
- [Federate Integration using the PyHELICS API](#federate-integration-using-the-pyhelics-api)
  - [Translation from JSON to PyHELICS API methods](#translation-from-json-to-pyhelics-api-methods)
  - [Federate Integration with API calls](#federate-integration-with-api-calls)
  - [Dynamic Pub/Subs with API calls](#dynamic-pubsubs-with-api-calls)
  - [Co-simulation Execution](#co-simulation-execution)
- [Questions and Help](#questions-and-help)

## Computing Environment

This example was successfully run on `Tue Nov 10 11:16:44 PST 2020` with the following computing environment.

- Operating System

  ```shell
  $ sw_vers
  ProductName:    Mac OS X
  ProductVersion:    10.14.6
  BuildVersion:    18G6032
  ```

- python version

  ```shell
  $ python
  Python 3.7.6 (default, Jan  8 2020, 13:42:34)
  [Clang 4.0.1 (tags/RELEASE_401/final)] :: Anaconda, Inc. on darwin
  Type "help", "copyright", "credits" or "license" for more information.
  ```

- python modules for this example

  ```shell
  $ pip list | grep matplotlib
  matplotlib                    3.1.3
  $ pip list | grep numpy
  numpy                         1.18.5
  ```

If these modules are not installed, you can install them with

```shell
$ pip install matplotlib
$ pip install numpy
```

- helics_broker version

  ```shell
  $ helics_broker --version
  2.4.0 (2020-02-04)
  ```

- pyhelics version

  ```shell
  $ helics --version
  0.4.1-HEAD-ef36755
  ```

- pyhelics init file

  ```shell
  $ python

  >>> import helics as h
  >>> h.__file__
  '/Users/[username]/Software/pyhelics/helics/__init__.py'
  ```

## Example files

All files necessary to run the Federate Integration Example can be found in the [Fundamental examples repository:](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_integration)

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fund_fedint_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/fundamental/fundamental_integration)

The files include:

- Python program for Battery federate
- Python program for Charger federate
- HELICS runner JSON to enable execution of the co-simulation

## Federate Integration using the PyHELICS API

This example differs from the Base Example in that we integrate the federates (simulators) into the co-simulation using the API instead of an external JSON config file. Integration and configuration of federates can be done either way -- the biggest hurdle for most beginning users of HELICS is learning how to look for the appropriate API key to mirror the JSON config style.

For example, let's look at our JSON config file of the Battery federate from the Base Example:

```json
{
  "name": "Battery",
  "loglevel": 1,
  "coreType": "zmq",
  "period": 60,
  "uninterruptible": false,
  "terminate_on_error": true,
  "wait_for_current_time_update": true,
  "publications": [],
  "subscriptions": []
}
```

We can see from this config file that we need to find API method to assign the `name`, `loglevel`, `coreType`, `period`, `uninterruptible`, `terminate_on_error`, `wait_for_current_time_update`, and `pub`/`subs`. In this example, we will be using the [PyHELICS API methods](https://python.helics.org/api/capi-py). This section will discuss how to translate JSON config files to API methods, how to configure the federate with these API calls in the co-simulation, and how to dynamically register publications and subscriptions with other federates.

### Translation from JSON to PyHELICS API methods

Configuration with the API is done within the federate, where an API call sets the properties of the federate. With our Battery federate, the following API calls will set all the properties from our JSON file (except pub/sub, which we'll cover in a moment). These calls set:

1. `name`
2. `loglevel`
3. `coreType`
4. Additional core configurations
5. `period`
6. `uninterruptible`
7. `terminate_on_error`
8. `wait_for_current_time_update`

```python
h.helicsCreateValueFederate("Battery", fedinfo)
h.helicsFederateInfoSetIntegerProperty(fedinfo, h.HELICS_PROPERTY_INT_LOG_LEVEL, 1)
h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")
h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)
h.helicsFederateInfoSetTimeProperty(fedinfo, h.HELICS_PROPERTY_TIME_PERIOD, 60)
h.helicsFederateInfoSetFlagOption(fedinfo, h.HELICS_FLAG_UNINTERRUPTIBLE, False)
h.helicsFederateInfoSetFlagOption(fedinfo, h.HELICS_FLAG_TERMINATE_ON_ERROR, True)
h.helicsFederateInfoSetFlagOption(
    fedinfo, h.HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE, True
)
```

If you find yourself wanting to set additional properties, there are a handful of places you can look:

- [C++ source code](https://docs.helics.org/en/latest/doxygen/helics__enums_8h_source.html): Do a string search for the JSON property. This can provide clarity into which `enum` to use from the API.
- [PyHELICS API methods](https://python.helics.org/api/capi-py): API methods specific to PyHELICS, with suggestions for making the calls pythonic.
- [Configuration Options Reference](../../../references/configuration_options_reference.md): API calls for C++, C, Python, and Julia

### Federate Integration with API calls

We now know which API calls are analogous to the JSON configurations -- how should these methods be called in the co-simulation to properly integrate the federate?

It's common practice to rely on a helper function to integrate the federate using API calls. With our Battery/Controller co-simulation, this is done by defining a `create_value_federate` function (named for the fact that the messages passed between the two federates are physical values). In `Battery.py` this function is:

```python
def create_value_federate(fedinitstring, name, period):
    fedinfo = h.helicsCreateFederateInfo()
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")
    h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)
    h.helicsFederateInfoSetIntegerProperty(fedinfo, h.HELICS_PROPERTY_INT_LOG_LEVEL, 1)
    h.helicsFederateInfoSetTimeProperty(fedinfo, h.HELICS_PROPERTY_TIME_PERIOD, period)
    h.helicsFederateInfoSetFlagOption(fedinfo, h.HELICS_FLAG_UNINTERRUPTIBLE, False)
    h.helicsFederateInfoSetFlagOption(fedinfo, h.HELICS_FLAG_TERMINATE_ON_ERROR, True)
    h.helicsFederateInfoSetFlagOption(
        fedinfo, h.HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE, True
    )
    fed = h.helicsCreateValueFederate(name, fedinfo)
    return fed
```

Notice that we have passed three items to this function: `fedinitstring`, `name`, and `period`. This allows us to flexibly reuse this function if we decide later to change the name or the period (the most common values to change).

We create the federate and integrate it into the co-simulation by calling this function at the beginning of the program main loop:

```python
fedinitstring = " --federates=1"
name = "Battery"
period = 60
fed = create_value_federate(fedinitstring, name, period)
```

What step created the value federate?

<details><summary>Click for answer</summary>
<p>
This line from the `create_value_federate` function:

`fed = h.helicsCreateValueFederate(name, fedinfo)`

Notice that we pass to this API the `fedinfo` set by all preceding API calls.

</p>
</details>

### Dynamic Pub/Subs with API calls

In the Base Example, we configured the pubs and subs with an external JSON file, where _each_ publication and subscription between federate interfaces needed to be explicitly defined for a predetermined number of connections:

```json
"publications":[
  {
    "key":"Battery/EV1_current",
    "type":"double",
    "unit":"A",
    "global": true
  },
  {}
],
"subscriptions":[
  {
    "key":"Charger/EV1_voltage",
    "type":"double",
    "unit":"V",
    "global": true
  },
  {}
]
```

With the PyHELICS API methods, you have the flexibility to define the connection configurations _dynamically_ within execution of the main program loop. For example, in the Base Example we defined **five** communication connections between the Battery and the Charger, meant to model the interactions of five EVs each with their own charging port. If we want to increase or decrease that number using JSON configuration, we need to update the JSON file (either manually or with a script).

Using the PyHELICS API methods, we can register any number of publications and subscriptions. This example sets up pub/sub registration using for loops:

```python
num_EVs = 5
pub_count = num_EVs
pubid = {}
for i in range(0, pub_count):
    pub_name = f"Battery/EV{i+1}_current"
    pubid[i] = h.helicsFederateRegisterGlobalTypePublication(
        fed, pub_name, "double", "A"
    )

sub_count = num_EVs
subid = {}
for i in range(0, sub_count):
    sub_name = f"Charger/EV{i+1}_voltage"
    subid[i] = h.helicsFederateRegisterSubscription(fed, sub_name, "V")
```

Here we only need to designate the number of connections to register in one place: `num_EVs = 5`. Then we register the publications using the `h.helicsFederateRegisterGlobalTypePublication()` method, and the subscriptions with the `h.helicsFederateRegisterSubscription()` method. Note that subscriptions are analogous to [_inputs_](../../fundamental_topics/value_federates.md), and as such retain similar properties.

### Co-simulation Execution

In this tutorial, we have covered how to integrate federates into a co-simulation using the PyHELICS API. Integration covers configuration of federates and registration of communication connections. Execution of the co-simulation is done the same as with the Base Example, using a runner JSON. The runner JSON has not changed from the Base Example:

```json
{
  "name": "fundamental_integration",
  "federates": [
    {
      "directory": ".",
      "exec": "helics_broker -f 3 --loglevel=warning",
      "host": "localhost",
      "name": "broker"
    },
    {
      "directory": ".",
      "exec": "python -u Charger.py",
      "host": "localhost",
      "name": "Charger"
    },
    {
      "directory": ".",
      "exec": "python -u Controller.py",
      "host": "localhost",
      "name": "Controller"
    },
    {
      "directory": ".",
      "exec": "python -u Battery.py",
      "host": "localhost",
      "name": "Battery"
    }
  ]
}
```

Execute the co-simulation with the same command as the Base Example

```shell
helics run --path=fundamental_integration_runner.json
```

This results in the same output; the only thing we have changed is the method of configuring the federates and integrating them.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_default_resultbattery.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_default_resultcharger.png)

If your output is not the same as with the Base Example, it can be helpful to pinpoint source of the difference -- have you used the correct API method?

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
