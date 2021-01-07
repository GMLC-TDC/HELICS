# Federates

## Types of Federates

A "federate", as previously introduced in the [HELICS Key Concepts section](./helics_key_concepts.md), is a specific instance of simulation executable. For example, a federation may contain a bunch of electric vehicles (EVs), each with their own charge controller implemented as a stand-alone federate (maybe the co-simulation designer is trying out a fancy new coordination algorithm); we'll call this code `EV_coordinator_v12.exe`. The code that does this charge coordination is generic in the sense that it can be used to charge any of these EVs; it works with all makes and models. But to run the co-simulation, each EV will have to have its own running instance of this code that controls the charging of a particular vehicle. If the co-simulation is testing this algorithm with five EVs, then there would be five federates each running their own version of `EV_coordinator_v12.exe` which presumably have unique information particular to each individual EV (battery size, maximum charging rate, etc).

(And, to be clear, a "simulator" is the executable itself. A simulator can never be a part of a co-simulation; as soon as one particular instance of that simulator begins running in a co-simulation it is considered a "federate".)

HELICS defines three specific types of federates defined by the nature of the messages they are passing to and from the federation:


1. **Value federates** - Value federates are used when the federate is simulating the physics of a system. The data in the messages they send and receive are indicating new values at the boundary of the federates system under simulation. Taking the example from the our [high-level overview of the process of HELICS co-simulation execution](./helics_co-sim_sequence.md), the transmission and distribution system were value federates; they each supplied messages that updates the other's boundary condition. Value federates typically will update at very regular intervals based on the fidelity of their models and/or the resolution of any supporting data they are using; this likely is related to the time-step of the federate.

   Value federates interact with the federation through what are called "interfaces". There are a handful of different interface types which will be discussed shortly but all are intended to provide the same essential functionality: allowing values in HELICS messages to be sent from and received by a given federate. For value federates, these messages will often have one-to-one correspondence to internal variables within the federate. Using the [previous simple transmission and distribution example ](./helics_co-sim_sequence.md), the distribution system may send out a HELICS message that contains the value of its internal positive sequence load variable and receive HELICS messages that it uses for its internal positive sequence substation voltage variable.
   
   There are four interface types for value federates that allow the interactions between the federates (a large part of co-simulation/federation configuration) to be flexibly defined. The difference between the four types revolve around whether the interface is for sending or receiving HELICS messages and whether the sender/receiver is defined by the federate (technically, the core associated with the federate):

- **Publications** - Sending interface where the federate core does not specify the intended recipient of the HELICS message
- **Named Inputs** - Receiving interface where the federate core does not specify the source federate of the HELICS message
- **Directed Outputs** - Sending interface where the federate core specifies the recipient of HELICS message
- **Subscriptions** - Receiving interface where the federate core specifies the sender of HELICS message

In all cases the configuration of the federate core declares the existence of the interface to use for communicating with other federates. The difference between "publication"/"named inputs" and "directed outputs"/"subscriptions" is where that federate core itself knows the specific names of the interfaces on the receiving/sending federate core.

The message type used for a given federation configuration is often an expression of the preference of the user setting up the federation. There are a few important differences that may guide which interfaces to use:

- **Which interfaces does the simulator support?** - Though it is the preference of the HELICS development team that all integrated simulators support all four types, that may not be the case. Limitations of the simulator may limit your options as a user of that simulator.
- **Is portability of the federate and its configuration important?** - Because "publications" and "named inputs" don't require the federate to know who it is sending HELICS messages to and receiving HELICS messages from as part of the federate configuration, it affords a slightly higher degree of portability between different federations. The mapping of HELICS messages still needs to be done to configure a federation, its just done separately from the federate configuration file via a broker or core configuration file. The difference in location of this mapping may offer some configuration efficiencies in some circumstances.

Though all four message types are supported, the remainder of this guide will focus on publications and subscriptions as they are conceptually easily understood and can be comprehensively configured through the individual federate configuration files.


2. **Message federates** - Message federates are used when the HELICS messages being passed to and from the simulation are generic packets of information, often for control purposes. Though these values may be physical measurements, they are treated as data to be used by an algorithm, processor, or controller. If the inputs to the federate can be thought of traveling over a communication network of some kind, it is probably coming from and/or going to a message federate. For example, in the power system world phasor measurement units ([PMUs](https://en.wikipedia.org/wiki/Phasor_measurement_unit)) have been installed throughout the power system and the measurements they make are collected through a communication system.

   Message federates interact with the federation through a conceptually different mechanism than a value federate. If message federates can be thought of as attaching to communication networks, the federate's "endpoints" are the specific interfaces to that communication network. By default, HELICS acts as the communication network, transferring signals between message federates from the endpoint on one to the endpoint on another.
   
   In fact, as you'll see in [a later section](./filters.md), it is possible to create more realistic communication-system effects natively in HELICS (as well as use a full-blown communication simulator like [ns-3](https://www.nsnam.org) to do the same). This is relevant now, though, because it influences how the endpoints are created and, as a consequence, how the simulator handles messages. You could, for example, have a system with three federates communicating with each other: a remote voltage sensor, a voltage controller, and a voltage regulation actuator (we'll pretend for the case of this example that the last two are physically separated though they often aren't). In this case, you could imagine that the voltage sensor only sends messages to the voltage controller and the voltage controller only sends messages to the voltage regulation actuator. That is, those two paths between the three entities are distinct, have no interaction, and have unique properties (though they may not be modeled). Given this, referring to the figure below, the voltage sensor could have one endpoint ("Endpoint 1") to send the voltage signal, the voltage regulator could receive the measurement at one endpoint ("Endpoint 2") and send the control signal on another ("Endpoint 3"), and the voltage regulation actuator would receive the control signal on its own endpoint ("Endpoint 4").

![Voltage regulation message federates](../../img/voltage_reg_message_federate.png)

The federate code handling these messages can be relatively simple because the data coming in or going out of each endpoint is unique. The voltage controller always receives (and only receives) the voltage measurement at one endpoint and similarly only sends the control signal on another.

Consider a counter-example: automated meter-reading (AMI) using a wireless network that connects all meters in a distribution system to a data-aggregator in the substation (where, presumably, the data travels over a dedicated wired connection to a control room). All meters will have a single endpoint over which they will send their data but what about the receiver? The co-simulation could be designed with the data-aggregator having a unique endpoint for each meter but this implies come kind of dedicated communication channel for each meter; this is not the case with wireless communication. Instead, it is probably better to create a single endpoint representing the wireless connection the data aggregator has with the AMI network. In this case, messages from any of the meters in the system will be flowing through the same endpoint and to differentiate the messages from each other, the federate will have to be written to examine the metadata with the message to determine its original source.

![Signal topology for AMI message federates](../../img/ami_message_federate.png)

The following table may be useful in understanding the differences between the two methods by which federates can communicate:

```eval_rst
+--------------------------------------------------------------------+--------------------------------------------------------+
|`Publication <value_federate>`__/`Input` Values  | `Endpoint <message_federate.html>`__                          |
+====================================================================+========================================================+
|Fixed routes defined when federation is initialized                 |Routes determined at time of transmission               |
+--------------------------------------------------------------------+--------------------------------------------------------+
|Publications: 1 to n (broadcast)                                    |1 to 1 - defined sender and receiver                    |
|Inputs: n to 1 (promiscuous)                                        |                                                        |
+--------------------------------------------------------------------+--------------------------------------------------------+
|Current value always available on message bus                       |Message removed from message bus when received          |
+--------------------------------------------------------------------+--------------------------------------------------------+
|Message can be configured with a default value                      |Message can be rerouted and modified through filters    |
+--------------------------------------------------------------------+--------------------------------------------------------+
|Messages can have units associated with them                        |Message contents are generic binary blobs to HELICS     |
+--------------------------------------------------------------------+--------------------------------------------------------+
```

### Interactions Between Messages and Values

Though it is not possible to have a HELICS message show up at a value interface, the converse is possible; message_federates can subscribe to HELICS values. Every time a value federate publishes a new value to the federation, if a message federate has subscribed to that message HELICS will generate a new HELICS message and send it directly to the destination endpoint. These messages are queued and not overwritten (unlike in HELICS values) which means when a message federate is synchronized it may have multiple messages from the same source to manage.

This feature offers the convenience of allowing a message federate to receive messages from pure value federates that have no endpoints defined. This is particularly useful for simulators that do not support endpoints but are required to provide measurement signals controllers. Implemented in this way, though, it is not possible to later implement a full-blown communication simulator that these values-turned-messages can traverse. Such co-simulation architectures in HELICS require the existence of both a sending and receiving endpoint; this feature very explicitly by-passes the need for a sending endpoint.



3. **Combination federates** - As you might guess, this type of federate makes use of both the value method and the endpoint method for transferring data between federates. An example of a federate like this could be a transmission system simulator that is acting both as a physical model of a system as well as a collection of PMUs that are sending data to a centralized generator dispatcher. The solution to the powerflow could be used to define substation voltages to some attached distribution circuits (physical values sent via publication) and the generator output powers could be sent to the centralized controller (control/measurement values being sent over a communication network via endpoints in a message federate).


## Example from Base Model 