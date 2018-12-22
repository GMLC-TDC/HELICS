#Value Federates #

xxxxxxx - add introduction

## Value Federate Message Types ##
There are four interface types for value federates that allow the interactions between the federates (a large part of co-simulation/federation configuration) to be flexibly defined. The difference between the four types revolve around whether the interface is for sending or receiving HELICS messages and whether the sender/receiver is defined by the federate (technically, the core associated with the federate):

* **Publications** - Sending interface where the federate core does not specify the intended recipient of the HELICS message
* **Named Inputs** - Receiving interface where the federate core does not specify the source federate of the HELICS message
* **Directed Outputs** - Sending interface where the federate core specifies the recipient of HELICS message
* **Subscriptions** - Recieving interface where the federate core specifies the sender of HELICS message

In all cases the configuration of the federate core declares the existance of the interface to use for communicating with other federates. The difference between "publication"/"name inputs" and "directed outputs"/"subscriptions" is where that federate core itself knows the specific names of the interfaces on the receiving/sending federate core. 


The message type used for a given federation configuration is often an expression of the preference of the user setting up the federation. There are a few important differences that may guide which interfaces to use:

* **Which interfaces does the simulator support?** - Though it is the preference of the HELICS development team that all integrated simulators support all four types, that may not be the case. Limitations of the simulator may limit your options as a user of that simulator.
* **Is portability of the federate and its configuration important?** - Because "publications" and "named inputs" don't require the federate to know who it is sending HELICS messages to and receiving HELICS messages from as part of the federate configuration, it affords a slightly higher degree of portability between different federations. The mapping of HELICS messages still needs to be done to configure a federation, its just done separately from the federate configuration file via a broker configuration file. The difference in location of this mapping may offer some configuration efficiencies in some circumstances.

Though all four message types are supported, the remainder of this guide will focus on publications and subscriptions as they are conceptually easily understood and can be configured inside the individual federate JSONs.

(xxxxxxx Add at least one example of how directed outputs and inputs are configured.) 


## Federate Configuration Options via JSON ##
For any simulator that you didn't write for yourself, the most common way of configuring that simulator for use in a HELICS co-simulation will be through the use of an external JSON configuration file. This file will be read when a federate is being created and initialized and it will provide all the necessary information to incorporate that federate into the co-simulation.

As the fundamental role of the co-simulation platform is to manage the synchronization and data exchange between the federates, you may or may not be surprised to learn that there are generic configuration options available to all HELICS federates that deal precisely with these. In this section, we'll focus on the options related to data exchange and in [Timing section](./timing.md) we'll look at the timing parameters. Let's look at a generic JSON configuration file as an example with the more common parameters shown; the default values are shown in "[ ]". (Further parameters and explanations can be found in the [developer documentation](../configuration/Timing.md)

### General Configuration Parameter ###


```
{
...
"name":"generic_federate",
"coreType": "zmq"
...
```
* **`name`** - Every federate must have a unique name across the entire federation; this is functionally the address of the federate and is used to determine where HELICS messages are sent.
* **`coreType` [zmq]** - There are a number of technologies or message buses that can be used to send HELICS messages among federates. Every HELICS enabled simulator has code in it that creates a core which connects to a HELICS broker using one of these messaging technologies. ZeroMQ (zmq) is the default core type and most commonly used but there are also cores that use TCP and UDP networking protocols directly (forgoing ZMQ's guarantee of delivery), IPC (uses Boost's interprocess communication for fast in-memory message-passing but only works if all federates are running on the same physical computer), and MPI (for use on HPC clusters where MPI is installed).


### Data Exchange Options ###

```
{
...
"only_update_on_change":false, //indicator that the federate should only indicate updated values on change
"only_transmit_on_change":false,  //indicator that the federate should only publish if the value changed
"source_only":false,
"observer":false,
...
```
* **`only_update_on_change` [false]** - In some cases a federate may have subscribed to a value that changes infrequently. If the publisher of that makes new publications regularly but the value itself has not changed, setting this flag on the receiving federate will prevent that federate from being sent the new, but unchanged value and having to reprocess it's received data when nothing has changed. Note that this flag will only prevent the old value from getting through if it is bit-for-bit identical to the old one. 

* **`only_transmit_on_change` [false]** - Complementarily to `only_update_on_change`, this flag can be set to prevent identical values from being published to the federation if they have not changed.

* **`source_only` [false]** - Some federates may exist only to provide data for the federation to use in their calculations. If using such a federate, set the `source_only` flag to `true` because xxxxxxx

* **`observer` [false]** - Conversely, some federates may only participate in the federation by recording values (perhaps for diagnostic purposes or for logging results). If using such a federate, set the `observer` flag to `true` because xxxxxxx

(xxxxxxx - NV (currently unsupported) -  How do we configure a subscription so that the subscribing federate is only woken up if the value changes by, say, 10%?)


### Value Federate Interface Configuration ###
```
...
     "publications" : [
          {
               "global" : true,
               "key" : "IEEE_123_feeder_0/totalLoad",
               "type" : "complex",
               "unit" : "VA",
               "info" : "{
                    \"object\" : \"network_node\",
                    \"property\" : \"distribution_load\"
               }"
          },
          {
          ...
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
          },
          {
          ...
          }
     ]
 ...
```
 
* **`publications` and/or `subscriptions`** - These are lists of the values being sent to and from the given federate.
* **`global` [false]** - `global` is used to indicate whether the string defining the `key` has the federate name already included in it. As the default value is `false`, when this parameter is omitted HELICS takes care of prepending the federate name behind the scenes.
* **`required` [false]** - This parameter can be set to `true` in subscriptions to indicate that a message must be published by another federate; if this condition is not met HELICS will error-out the co-simulation. This effectively provides built-in partial error-checking of the intended publication and subscription configuration (xxxxxxx this is Trevor's speculated definition).
* **`key`** - The `key` is used to define a unique name for the HELICS message that is being sent or received.
* **`type`** - HELICS supports data types and data type conversion ([as best it can](https://www.youtube.com/watch?v=mZOAn-3aATY)).
* **`units`** - Eventually, HELICS will be able to do automatic unit conversion between federates. For example, one federate could publish a value in Watts and another federate could subscribe to it in kilo-Watts; rather than throwing an error, HELICS will do the unit conversion itself. As of v2.0 this feature is not implemented and the `units` field is ignored.
* **`info`** - The `info` field is entirely ignored by HELICS and is used as a mechanism to pass configuration information to the federate so that it can properly integrate into the federation. Thus, there is no standard content or format for this field; it is entirely up to the individual simulators to decide how the data in this field (if any) should be used. Often it is used by simulators to map the HELICS names into internal variable names as shown in the above example. In this case, the object `network_node` has a property called `positive_sequence_voltage` that will be updated with the value from the subscription `TransmissionSim/transmission_voltage`.


(xxxxxxx - NV - warnings of shooting yourself in the foot by sending overwriting values to the same interface. Only applicable to named inputs/outputs)


## Example 1a - Basic transmission and distribution powerflow ##

To demonstrate how a to build a co-simulation, an example of a simple integrated transmission system and distribution system powerflow can be built; all the necessary files are found [here](xxxxxxx) but to use them you'll need to get some specific software installed; here are the instructions:

  1. [HELICS](https://gmlc-tdc.github.io/HELICS-src/installation/index.html)
  2. [GridLAB-D](https://github.com/gridlab-d/gridlab-d/tree/develop) - Enable HELICS, see instructions [here](http://gridlab-d.shoutwiki.com/wiki/Connection:helics_msg)
  3. [Python](https://www.anaconda.com/download/) - Anaconda installation, if you don't already have Python installed
  4. [PyPower](https://pypi.org/project/PYPOWER/) - `pip install pypower`


This example has a very simple message topology (with only one message being sent by either federate at each time step) and uses only a single broker. Diagrams of the message and broker topology can be found below:

![Ex. 1a message topology](../img/ex1a_message_topology.pdf)

![Ex. 1a broker topology](../img/ex1a_broker_topology.pdf)

* **Transmission system** - The transmission system model used is the IEEE-118 bus model. To a single bus in this  model the GridLAB-D distribution system is attached. All other load buses in the model use a static load shape scaled proportionately so the peak of the load shape matches meet the model-defined load value. The generators are re-dispatchted every fifteen minutes by running an optimal power flow (the so-called "ACOPF" which places constraints on the voltage at the nodes in the system) and every 1 minute a powerflow is run the update the state of the system.

* **Distribution system** - A GridLAB-D model xxxxxxx 

This simulation is run for 24 hours.

Let's make a comparison between running these two simulators independently and running them as a co-simulation. To run them independently, simply turn off the publications and subscriptions by xxxxxxx and to run as a co-sim just xxxxxxx (helics-cli?).

We can compare the results of the two simulations to see the impact of the interaction between the two. The figure below shows the total load on the transmission node to which the distribution system model is attached over the course of the simulated day. As you can see xxxxxxx

(xxxxxxxx - Insert graph comparing transmission system load with and without co-sim)

Next we can compare the substation voltage at the feeder head of the distribution system. We see xxxxxxx

(xxxxxxx - insert graph comparing substation voltage with and without co-sim.)