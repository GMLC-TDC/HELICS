# Brokers - Federation Queries




```eval_rst
.. toctree::
    :maxdepth: 1


```

![placeholder](../../../img/user_guide_combinations_advanced.png)


This demonstrates the use of federation queries and performs dynamic configuration by using the information from the query to configure the Battery federate.

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

This example on [queries can be found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/user_guide_examples/advanced/advanced_message_comm/query). If you have issues navigating to the examples, visit the HELICS Gitter page or the user forum on GitHub.


**TODO: Update screen shot**
[![](../../../img/advanced_examples_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/user_guide_examples/advanced)







<a name="what-is-this-co-simulation-doing">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
What is this Co-simulation doing?
</span>
</strong>
</a>

This example shows how to run queries on a federation and to use the output of the queries to configure a federate. Rather than a static configuration that is defined prior to runtime, this dynamic configuration can be useful for federations that change composition frequently.





<a name="differences-compared-to-the-advanced-default-example">
<strong>
<span style="color:black;text-decoration:underline;">
Differences Compared to the Advanced Default example
</span>
</strong>
</a>

This example has the same federates interacting in the same ways as in the [Advanced Default example](./advanced_default.md). The only difference is the use of queries for dynamic configuration rather than static configuration.




<a name="helics-differences">
<strong>
<span style="color:black">
HELICS Differences
</span>
</strong>
</a>

In most of the examples presented here, the configuration of the federation is defined prior to executing the co-simulation via the configuration JSONs. It is possible, though, with extra effort and careful design, to write the federate code such that they self-configure based on the other participants in the co-simulation. This example provides a simple demonstration of this by having the Battery federate query the federation and look for the values that the Charger federate is publishing and subscribing to them.
 




<a name="helics-components">
<strong>
<span style="color:black;text-decoration:underline;">
HELICS components
</span>
</strong>
</a>

`Battery.py` contains all the changes from the [Advanced Default example](./advanced_default.md) that allow it to perform dynamic configuration. Specifically:

* Sleeping a few seconds to ensure all other federates in the federation have configured so that the Battery federate can query the federation and know it is seeing the comprehensive configuration.

	```
	sleep_time = 5
	logger.debug(f'Sleeping for {sleep_time} seconds')
	time.sleep(sleep_time)
	```

* `eval_data_flow_graph` is a new function that performs a data graph query on the federation. This query evaluates the connections between federates, showing who is publishing and subscribing to what. The function also takes the output from the query and parses it into a form that can be easily used for the dynamic configuration.

	```
	def eval_data_flow_graph(fed):
	    query = h.helicsCreateQuery("broker", "data_flow_graph")
	    graph = h.helicsQueryExecute(query, fed)
	...
	```



* The Battery federate subscribes to all the publications from the Charger federate based on the results of the data flow graph.

	```
	for core in graph['cores']:
	    if core['federates'][0]['name'] == 'Charger':
	        for pub in core['federates'][0]['publications']:
	            key = pub['key']
	            sub = h.helicsFederateRegisterSubscription(fed, key)
	            logger.debug(f'Added subscription {key}')
	```


* After making the subscription, the Battery federate re-evaulates the data flow graph and updates its own internal record of the configuration

	```
	# The data flow graph can be a time-intensive query for large
	#   federations
	# Verifying dynamic configuration worked.
	graph, federates_lut, handle_lut = eval_data_flow_graph(fed)
	# logger.debug(pp.pformat(graph))
	sub_count = h.helicsFederateGetInputCount(fed)
	logger.debug(f'Number of subscriptions: {sub_count}')
	subid = {}
	sub_name = {}
	for i in range(0, sub_count):
	    subid[i] = h.helicsFederateGetInputByIndex(fed, i)
	    sub_name[i] = h.helicsSubscriptionGetKey(subid[i])
	    logger.debug(f'\tRegistered subscription---> {sub_name[i]}')
	```







<a name="execution-and-results">
<strong>
<span style="color:black;text-decoration:underline;">
Execution and Results
</span>
</strong>
</a>

Run the co-simulation:

`$ helics run --path=./multi_broker_runner.json`


Since this is only a change to the configuration method of the federation, the results are identical to those in the [Advanced Default example.](./advanced_default.md)

**TODO: make these links?**

![placeholder](../../../img/advanced_query_charging_power.png)

![placeholder](../../../img/advanced_query_estimated_SOCs.png)

![placeholder](../../../img/advanced_query_battery_SOCs.png)


The dynamic configuration can also been seen by looking at the log file for the Battery federate (`Battery.log`). The pre-configure data flow graph only showed five subscriptions, all made by the Charger federate of the Battery federates current

```
Pre-configure data-flow graph query.
Federate Charger (with id 131072) has the following subscriptions:
	Battery/EV1_current from federate Battery
	Battery/EV2_current from federate Battery
	Battery/EV3_current from federate Battery
	Battery/EV4_current from federate Battery
	Battery/EV5_current from federate Battery
Added subscription Charger/EV1_voltage
```

After analyzing the results of the data-flow graph, the Battery federate subscribing to the appropriate publications from the Charger federate, and re-running the query, the subscription list looks like this:

```
Post-configure data-flow graph query.
Federate Battery (with id 131073) has the following subscriptions:
	Charger/EV1_voltage from federate Charger
	Charger/EV2_voltage from federate Charger
	Charger/EV3_voltage from federate Charger
	Charger/EV4_voltage from federate Charger
	Charger/EV5_voltage from federate Charger
Federate Charger (with id 131072) has the following subscriptions:
	Battery/EV1_current from federate Battery
	Battery/EV2_current from federate Battery
	Battery/EV3_current from federate Battery
	Battery/EV4_current from federate Battery
	Battery/EV5_current from federate Battery
```

