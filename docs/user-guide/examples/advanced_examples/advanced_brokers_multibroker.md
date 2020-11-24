# Brokers - Multi-Protocol Brokers




```eval_rst
.. toctree::
    :maxdepth: 1


```

![placeholder](../../../img/user_guide_combinations_advanced.png)


This example shows how to configure a HELICS co-simulation to implement a broker structure that utilizes multiple core types in a single co-simulation. Typically, all federates in a single federation use the same core type (ZMQ by default) but HELICS can be set up to utilize different core types in the same federation

* [Where is the code?](#where-is-the-code)
* [What is this Co-simulation doing?](#what-is-this-co-simulation-doing)
	* [Differences Compared to the Fundamental Examples](#differences-compared-to-the-advanced-default-example)
		* [HELICS Differences](#helics-differences)
		* [Research Question Complexity Differences](#research-question-complexity-differences)
	* [HELICS Components](#helics-components)
* [Execution and Results](#execution-and-results)


	
	
	

<a name="where-is-the-code">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
Where is the code?
</span>
</strong>
</a>

This example on [multibrokerscan be found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/user_guide_examples/advanced/advanced_brokers/multi_broker). If you have issues navigating to the examples, visit the HELICS Gitter page or the user forum on GitHub.


**TODO: Update screen shot**
[![](../../../img/advanced_examples_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/user_guide_examples/advanced)







<a name="what-is-this-co-simulation-doing">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
What is this Co-simulation doing?
</span>
</strong>
</a>

This example shows you how to configure a co-simulation to use more than one core type in the same federation. The example itself has the same functionality as the Advanced Default example as the only change is a structural to the federation and not the federate code itself.





<a name="differences-compared-to-the-advanced-default-example">
<strong>
<span style="color:black;text-decoration:underline;">
Differences Compared to the Advanced Default example
</span>
</strong>
</a>

For this example, the [Advanced Default example](./advanced_default.md) has been split up so that each federate uses a different core type in a single federation.




<a name="helics-differences">
<strong>
<span style="color:black">
HELICS Differences
</span>
</strong>
</a>

Typically, all federates in a federation use the same core type. There can be cases, though, where a multi-site co-simulation with a more complex networking environment or performance requirements dictate the need for some federates to utilize a difference core type than others. For example, the `IPC` core utilizes a Boost library function to allow two executables both using Boost to communicate between themselves when running on the same compute node; since this is in-memory communication rather than over the network stack, performance is expected to be higher. It could be that a particular federation has been optimized to take advantage of this but must also communicate with federates on a separate compute node via ZMQ. In this case, a so-called "multibroker" can be configured to allow for the federation to run.

In this example, we won't be doing anything like that but, for demonstration purposes, simply using the same federation from the [Advanced Default example.](./advanced_default.md) and configuring it so each federate uses a different core type.
 




<a name="helics-components">
<strong>
<span style="color:black;text-decoration:underline;">
HELICS components
</span>
</strong>
</a>

To configure a multibroker, the broker configuration line is slightly extended from a traditional federation. From the helics_cli runner configuration file `multi_broker_runner.json`

```
...
"exec": "helics_broker -f 3 --coreType=multi --config=multi_broker_config.json --name=root_broker",
...
```

The `coreType` of the broker is set to `multi` and a configuration file is specified. That file looks like this:

```json

{
  "master":{
    "coreType": "test"
  },
  "comms": [
    {
      "coreType": "zmq",
      "port": 23500
    },
    {
      "coreType": "tcp",
      "port": 23700
    },
    {
      "coreType": "udp",
      "port": 23900
    }
  ]
}
```

The first and most important note: `master` and `comms` are reserved words in this context and **MUST** be used. The `master` core type must be `test` but the core types for the federates can be any of the supported cores. Again, as in [other](./advanced_brokers_hierarchies.md) [similar](./advanced_brokers_simultaneous.md) examples, because we are running this on a single compute node, the port for each core type must be specified and the federates using those core types need to have the `brokerPort` property set to the corresponding core's port number. 


 
 
BatteryConfig.json
```json
  ...
  "name": "Battery",
  "loglevel": 1,
  "coreType": "zmq",
  "brokerPort": 23500,
   ...
```

ChargerConfig.json
```json
  ...
  "name": "Charger",
  "loglevel": 1,
  "coreType": "tcp",
  "brokerPort": 23700,
   ...
```

ControllerConfig.json
```json
  ...
  "name": "Controller",
  "loglevel": 1,
  "coreType": "udp",
  "brokerPort": 23900,
   ...
```



<a name="execution-and-results">
<strong>
<span style="color:black;text-decoration:underline;">
Execution and Results
</span>
</strong>
</a>

Unlike the other advanced broker examples, this one can be run with a single helics_cli command:

`$ helics run --path=./multi_broker_runner.json`


As has been mentioned, since this is just a change to the co-simulation architecture, the results are identical to those in the [Advanced Default example.](./advanced_default.md)

**TODO: make these links?**

![placeholder](../../../img/advanced_multibroker_charging_power.png)

![placeholder](../../../img/advanced_multibroker_estimated_SOCs.png)

![placeholder](../../../img/advanced_multibroker_battery_SOCs.png)
