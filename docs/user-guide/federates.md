# Federates #

(xxxxxxx TDH: Due to length, I'm likely going to be splitting this page into "Value Federates" and "Message Federates" pages. Maybe even have the introductory "Types of Federates" as its own page as well.)

## Types of Federates ##
A "federate", as previously introduced in the [HELICS Key Concepts section](./helics_key_concepts.md), is a specific instance of simulation executable. For example, a federation may contain a bunch of electric vehicles (EVs), each with their own charge controller implemented as a stand-alone federate (maybe the co-simulation designer is trying out a fancy new coordination algorithm); we'll call this code `EV_coordinator_v12.exe`. The code that does this charge coordination is generic in the sense that it can be used to charge any of these EVs; it works with all makes and models. But to run the co-simulation, each EV will have to have its own running instance of this code that controls the charging of a particular vehicle. If the co-simulation is testing this algorithm with five EVs, then there would be five federates each running their own version of `EV_coordinator_v12.exe` which presumably have unique information particular to each individual EV (battery size, maximum charging rate, etc).

(And, to be clear, a "simulator" is the executable itself. A simulator can never be a part of a co-simulation; as soon as one particular instance of that simulator begins running in a co-simulation it is considered a "federate".)

HELICS defines three specific types of federates defined by the nature of the messages they are passing to and from the federation:

1. **Value federates** - Value federates are used when the federate is simulating the physics of a system. The data in the messages they send and receive are indicating new values at the boundary of the federates system under simulation. Taking the example from the our [high-level overview of the process of HELICS co-simulation execution](./helics_co-sim_sequence.md), the transmission and distribution system were value federates; they each supplied messages that updates the other's boundary condition. Value federates typically will update at very regular intervals based on the fidelity of their models and/or the resolution of any supporting data they are using; this likely is related to the timestep of the federate. 
  
  Value federates interact with the federation through what are called "interfaces". There are a handful of different interface types which will be discussed shortly but all are intended to provide the same essential functionality: allowing values in HELICS messages to be sent from and received by a given federate. For value federates, these messages will often have one-to-one correspondence to internal variables within the federate. Using the [previous simple transmission and distribution example ](./helics_co-sim_sequence.md), the distribution system may send out a HELICS message that contains the value of its internal positive sequence load variable and receive HELICS messages that it uses for its internal positive sequence substation voltage variable.


2. **Message federates** - Message federates are used when the HELICS messages being passed to and from the simulation are generic information, often for control purposes. Though these values may be physical measurements, they are treated as data to be used by an algorithm, processor, or controller. If the inputs to the federate can be thought of traveling over a communication network of some kind, it is probably coming from and/or going to a message federate. For example, (xxxxxxx)
  Message federates interact with the federation through a conceptually different mechanism than a value federate. Message federates use the concept of an "endpoint" to enable communication between each other. An endpoint can be thought of the sending or receiving address associated with a specific message federate. (xxxxxxx - Federate globally? Is it up to the federate to determine how to handle the message?) All federates with endpoints can thus send these messages to each other by specifying the endpoint for which the message is intended. Endpoints can also be told to subscribe to other federates publications.
  
  
3. **Combination federates** - As you might guess, this type of federate makes use of both the publication/subscription method and the endpoint method for transferring data between federates. An example of a federate like this could be a transmission system simulator that is acting both as a physical model of a system as well as a collection of phasor measurement units that are sending data to a centralized generator dispatcher. The solution to the powerflow could be used to define substation voltages to some attached distribution circuits (physical values sent via publication) and the generator output powers could be sent to the centralized controller (control/measurement values being sent over a communication network via endpoint messages).

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

 

## Federate Configuration Options via JSON ##
For any simulator that you didn't write for yourself, the most common way of configuring that simulator for use in a HELICS co-simulation will be through the use of an external JSON configuration file. This file will be read when a federate is being created and initialized and it will provide all the necessary information to incorporate that federate into the co-simulation.

As the fundamental role of the co-simulation platform is to manage the synchronization and data exchange between the federates, you may or may not be surprised to learn that there are generic configuration options available to all HELICS federates that deal precisely with these. In this section, we'll focus on the options related to data exchange and in [Timing section](./timing.md) we'll look at the timing parameters. Let's look at a generic JSON configuration file as an example with the more common parameters shown; the default values are shown in "[ ]". (Further parameters and explanations can be found in the [developer documentation](../configuration/Timing.md)


### Data Exchange Options ###
```
{
"name":"generic_federate",
...
"only_update_on_change":false, //indicator that the federate should only indicate updated values on change
"only_transmit_on_change":false,  //indicator that the federate should only publish if the value changed
"source_only":false,
"observer":false,
```
* **`only_update_on_change` [false]** - In some cases a federate may have subscribed to a value that changes infrequently. If the publisher of that makes new publications regularly but the value itself has not changed, setting this flag on the receiving federate will prevent that federate from being sent the new, but unchanged value and having to reprocess it's received data when nothing has changed. Note that this flag will only prevent the old value from getting through if it is bit-for-bit identical to the old one. 

* **`only_transmit_on_change` [false]** - Complementarily to `only_update_on_change`, this flag can be set to prevent identical values from being published to the federation if they have not changed.

* **`source_only` [false]** - Some federates may exist only to provide data for the federation to use in their calculations. If using such a federate, set the `source_only` flag to `true` because xxxxxxx

* **`observer` [false]** - Conversely, some federates may only participate in the federation by recording values (perhaps for diagnostic purposes or for logging results). If using such a federate, set the `observer` flag to `true` because xxxxxxx

(xxxxxxx - How do we configure a subscription so that the subscribing federate is only woken up if the value changes by, say, 10%?)


### Value Federate Interface Configuration ###

(xxxxxxx - examples of all four interface types as well as an example of the broker JSON for publications and named inputs)

(xxxxxxx - warnings of shooting yourself in the foot by sending overwriting values to the same interface)

(xxxxxxx - discussion of the use of the `info` item in JSON configs)

(xxxxxxxx - data type conversion discussion)


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

## Message Federate Endpoints ##
As previously discussed, message federates interact with the federation by defining an "endpoint" that acts as their address to receive messages. Message federates are typically sending and receiving measurements, control signals, commands, and other discrete data with HELICS acting as a perfect communication system (infinite bandwidth, virtually no latency, guaranteed delivery). 

In fact, as you'll see in [a later section](./filters.md), it is possible to create more realistic communication-system effects natively in HELICS (as well as use a full-blown communication simulator like [ns-3](https://github.com/GMLC-TDC/ns-3-dev-git) to do the same). This is relevant now, though, because it influences how the endpoints are created and, as a consequence, how the simulator handles messages. You could, for example, have a system with three federates that are in communication: a remote voltage sensor, a voltage controller, and a voltage regulation actuator (we'll pretend for the case of this example that the last two are physically separated though they often aren't). In this case, you could imagine that the voltage sensor only sends messages to the voltage controller and the voltage controller only sends messages to the voltage regulation actuator. That is, those two paths between the three entities are distinct, have no interaction, and have unique properties (though they may not be modeled). Given this, referring to the figure below, the voltage sensor could have one endpoint ("Endpoint 1") to send the voltage signal, the voltage regulator could receive the measurement at one endpoint ("Endpoint 2") and send the control signal on another ("Endpoint 3"), and the voltage regulation actuator would receive the control signal on its own endpoint ("Endpoint 4").

![voltage regulation message federates](../img/voltage_reg_message_federate.pdf)

The federate code handling these messages can be relatively simple because the data coming in or going out of each endpoint is unique. The voltage controller always receives (and only receives) the voltage measurement at one endpoint and similarly only sends the control signal on another.

Consider a counter-example: automated meter-reading (AMI) using a wireless network that connects all meters in a distribution system to a data-aggregator in the substation (where, presumably, the data travels over a dedicated wired connection to a control room). All meters will have a single endpoint over which they will send their data but what about the receiver? The co-simulation could be designed with the data-aggregator having a unique endpoint for each meter but this implies come kind of dedicated communication channel for each meter; this is not the case with wireless communication. Instead, it is probably better to create a single endpoint representing the wireless connection the data aggregator has with the AMI network. In this case, messages from any of the meters in the system will be flowing through the same endpoint and to differentiate the messages from each other, the federate will have to be written to examine the metadata with the message to determine its original source.

![ami message federates](../img/ami_message_federate.pdf)


## Message Federate Configuration in JSON ##
Once the message topology considering endpoints has been determined, the definitions of these endpoints in the JSON file is straight-forward. Here's what it could look like for the voltage regulator example from above.

(xxxxxxx - JSON config for voltage regulator example)



## Example 1b - Distribution system EV charge controller ##
To demonstrate how a message federate interacts with the federation, let's take the previous example and add two things to it: electric vehicle (EV) loads in the distribution system, and a centralized EV charge control manager.

Keeping in mind that this a model for demonstration purposes (which is to say, don't take this too seriously), let's make the following assumptions to simplify the behavior of the EV charge controller:

  * All EVs have infinite battery capacity
  * All EVs will be at home all day, desiring to charge all day if they can. 
  * All EVs charge at the same power level.
  * The EV charge controller has direct control over the charging of all EVs in the distribution system. It can tell them when to turn off and on at will.
  * The EV charge controller has the responsibility to limit the total load of the distribution system to a specified level to prevent overloading on the substation transformer.
  * The EV will turn off some EV charging when the total distribution load exceeds the transformer limit by a certain percentage and will turn some EVs back on when below the limit by a certain percentage.
  * Nothing is fair about how the charge controller chooses which EVs to charge and which to disconnect.  

The message topology (including the endpoints) and the not very interesting broker topology are shown below.

![Ex. 1b message topology](../img/ex1b_message_topology.pdf)

![Ex. 1b message topology](../img/ex1b_broker_topology.pdf)


Taking these assumptions and specifications, it is not too difficult to write a simple charge controller as a Python script. And just by opening the JSON configuration file we can learn important details about how the controller works.

(xxxxxxx - insert EV charge controller JSON config)

We can see that the charge controller subscribes to the total substation load and sends out power charge commands to each of the individual EVs.

Running the example (located at xxxxxxxx) and looking at the output file (xxxxxxx) which the charge controller federate wrote out, we can see the total number of EVs charging at any point in time as well as the load on the substation throughout the day.

(xxxxxxx - insert graph of number of EVs and substation load vs time)

As you can see in the data, every time the load on the system exceeded the transformer rating by xxxxxxxx% (xxxxxxxx - specific value), the EV charge controller  disconnected the appropriate number of EVs to drop the load below the limit. Conversely, as the load dropped below the rated limit, the EV charge controller was able to connect more EVs for charging. 

(xxxxxxxx - Look at impacts of EV charge controller on transmission system; this is why we do co-simulation )

