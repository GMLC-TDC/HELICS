# Federates

```{eval-rst}
.. toctree::
    :hidden:
    :maxdepth: 1

    value_federates
    message_federates
    filters

```

This section on Federates covers:

- [What is a Federate?](#what-is-a-federate)
- [Types of Federates](#types-of-federates)
  - [Value Federates](#value-federates)
    - [Value Information](#value-information)
    - [Value Federate Interfaces](#value-federate-interfaces)
  - [Message Federates](#message-federates)
    - [Message Information](#message-information)
    - [Message Federate Interfaces](#message-federate-interfaces)
      - [Endpoints](#endpoints)
      - [Native HELICS Filters](#native-helics-filters)
    - [Interactions Between Messages and Values](#interactions-between-messages-and-values)
  - [Combination Federates](#combination-federates)

## What is a Federate?

A "federate" is an instance of a simulation executable that models a group of objects or an individual objects. For example, one can write a simulator to model the battery of an electric vehicle (EV). If we want to model multiple EVs connected to charging ports in a dedicated EV charging garage, we can use the EV simulator to model a group of EVs. We will need a second simulator to model a group of charging ports. Once we launch the simulators, each is called a "federate". Together, they are called a "federation" -- multiple federates running simultaneously. This federation performs a co-simulation to achieve a particular analysis objective (_e.g._ replicate the behavior of a fleet of EVs charging to understand the charging power requirements).

This co-simulation is described in more detail in the [Fundamental Examples](../examples/fundamental_examples/fundamental_examples_index.md). There are two federates in this co-simulation; one modeling the batteries on board the EVs, and one modeling the chargers of the batteries. Each federate in this example has multiple objects it is modeling; five batteries for the battery federate, and five chargers for the charger federate. Because the objects being modeled with the federates share most of the same properties -- _e.g._ battery size, charge rate -- a single federate can be used to model the five batteries. As the complexity of the co-simulation increases, it becomes increasingly difficult to group objects into one federate. In these situations, we could also design the co-simulation with one federate for each EV.

Co-simulations are designed to answer a research question. The question addressed by the [Fundamental Examples](../examples/fundamental_examples/fundamental_examples_index.md) is: How much power is needed to serve five EVs in a dedicated charging garage?

With this research question, we have identified that we want to model **batteries** and **chargers** and we want to monitor the power draw in **kW** over time. It's important to first identify the types of objects you want to model, as co-simulation in HELICS requires constructing federates based on the type of information they pass to other federates.

## Types of Federates

Federates are distinguished by the types of information they model and the interfaces they use to pass the information. Interfaces define how signals are connected between federates in a federation. HELICS has three types of federates: **Value Federates, Message Federates, and Combination Federates**. Value federates model physics in a system, message federates model logic (i.e., controls), and combination federates model both.

### Value Federates

Value federates are used when the federate is simulating the physics of a system. The data in the messages they send and receive indicate new values at the boundary of the federate system under simulation. Value federates interface with the federation using one-to-one correspondence to internal variables within the federate. These interfaces are commonly called publications and subscriptions (pubs/subs).

#### Value Information

The information modeled by value federates is physical values in a system with associated units. In the [Fundamental Example](../examples/fundamental_examples/fundamental_examples_index.md), the batteries and chargers are both value federates; charger applies a voltage to the battery (Charger federate sending the Battery federate that value) and the Battery federate responds with the charging current which it sends back to the Charger. These two federates each update the other's boundary condition's state: the voltage and current. Federates that model physics must be configured as **value federates**. Value federates typically will update at very regular intervals based on the fidelity of their models and/or the resolution of any supporting data they are using.

#### Value Federate Interfaces

Value federates have direct fixed connections through interfaces to other federates. There are three interface types for value federates that allow the interactions between the federates to be flexibly defined. The difference between the three types revolves around whether the interface is for sending or receiving values and whether the sender/receiver is defined by the federate:

- Publications
  - Sending interface
  - Interface named with `"key"` in configuration
  - Recipient interface value is not necessary, however it can be specified with `"targets"` in configuration
- Subscriptions (Unnamed Inputs)
  - Receiving interface
  - Not "named" (no identifier to the rest of the federation)
  - Source of value interface is specified with `"key"` in configuration
- Named Inputs
  - Receiving interface
  - Interface named with `"key"` in configuration
  - Source interface of value is not necessary, however it can be specified with `"targets"` -- Named Inputs can receive values from multiple `"targets"`

The most commonly used of these fixed interfaces are publications and subscriptions. In the [Fundamental Example](../examples/fundamental_examples/fundamental_examples_index.md), the Battery federate and the Charger federate have fixed pub/sub connections. In the figure below, publishing interfaces are in <span style="color:red;">**red**</span> and the subscription interfaces are in <span style="color:orange;">**yellow**</span>. The Battery federate **publishes** the current flowing into the battery from the publication interface named `EV_Battery/EV_current` and does not specify the intended recipient. The Charger federate **subscribes** to the amps from the Battery with the subscription interface named `EV_Battery/EV_current` -- the receiving interface only specifies the sender.

![Fundamental Example Configuration](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/battery_sub.png)

In all cases the configuration of the federate core declares the existence of the interface to use for communicating with other federates. The difference between publication/subscription and directed outputs/unnamed inputs is where the federate core knows the specific names of the interfaces on the receiving/sending federate core.

The interface type used for a federation configuration is a preference of the user setting up the federation. There are a few important differences that may guide which interfaces to use:

- Which interfaces does the simulator support?
  - Though it is the preference of the HELICS development team that all integrated simulators support all types, that may not be the case or even possible. Limitations of the simulator may limit your options as a user of that simulator.
- Is portability of the federate and its configuration important?
  - Because publications and subscriptions (unnamed inputs) don’t require the federate to know who it is sending HELICS messages to and receiving HELICS messages from, it affords a slightly higher degree of portability between different federations. The mapping of HELICS messages still needs to be done to configure a federation, it's just done separately from the federate configuration file via a broker or core configuration file. The difference in location of this mapping may offer some configuration efficiencies in some circumstances.

### Message Federates

Message federates send packets of data with unfixed connections for things such as events, communication packets, or triggers. Message federates are used to model information transfers (versus physical values) moving between federates. Measurement and control signals are typical applications for these types of federates.

#### Message Information

Message federates are used when the HELICS signals being passed to and from the simulation are generic packets of information, often for control purposes. They are treated as data to be used by an algorithm, processor, or controller. If the inputs to the federate can be thought of as traveling over a communication network of some kind, it should be modeled as coming from/going to a message federate. For example, in the power system world, phasor measurement units ([PMUs](https://en.wikipedia.org/wiki/Phasor_measurement_unit)) have been installed throughout the power system and the measurements they make are collected through a communication system and would be best modeled through the use of HELICS messages.

#### Message Federate Interfaces

Message federates interact with the federation through endpoints interfaces. Message federates can be thought of as attaching to communication networks, where the federate's **endpoints** are the specific interfaces to that communication network. By default, HELICS acts as the communication network, transferring messages between endpoint interfaces configured for message federates. Just as communications networks can be susceptible to failure, messages can be altered or delayed with **filter** which can be associated with specific endpoints. Filters can only act on messages and thus can only be associated with endpoints. Value signals are meant to replicate physical connections between models and thus are not susceptible to the frailty of communication systems.

##### Endpoints

Endpoints are interfaces used to pass packetized data blocks (messages) to another federate. Message federates interact with the federation by defining an endpoint that acts as their address to send and receive messages. Message federates are typically sending and receiving measurements, control signals, commands, and other signal data with HELICS acting as a perfect communication system (infinite bandwidth, no latency, guaranteed delivery).

In the figure below, Federate A and B are message federates with endpoints epA and epB. They do not have a fixed communication pathway; they have unique addresses (endpoints) to which messages can be sent. An endpoint can send data to any other endpoint in the system -- it just needs the "address" (endpoint interface).

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/ep_connection.png)

Endpoints can have a `"type"` which is a user defined string. HELICS currently does not recognize any predefined types. The data consists of raw binary data and optionally a send time. Messages are delivered first by time order, then federate ID number, then interface ID, then by order of arrival.

Unlike HELICS values which are persistent (meaning they are continuously available throughout the co-simulation), HELICS messages are only readable once when collected from an endpoint. Once that collection is made, the message only exists within the memory of the collecting message federate. If another message federate needs the information, a new message must be created and sent to the appropriate endpoint.

##### Native HELICS Filters

Filters are objects that can be used to disrupt message packets in a manner similar to communications networks. Filters are associated with the HELICS core, which in turn manages a federate's endpoints. Typical filtering actions might be delaying the transmission of a message or randomly dropping a certain percentage of the received messages. Filters can also be defined to operate on messages being sent ("source filters") and/or messages being received ("destination filters").

Messages can be filtered, values cannot. Messages are directed and unique, values are persistent. Internal to HELICS, each message has a unique identifier and can be thought to travel through a generic communication system between the source and destination endpoints. Since HELICS values model direct physical connections, they do not pass through this generic network and they cannot be operated on by filters. It is possible to create a federation where HELICS value interfaces are used to send control signals but this removes the possibility of using filters and the easy integration of communication system simulators.

Filters have the following properties:

1. Inline operations that can alter a message or events
   - Time Delay (Random or Fixed)
   - Packet Translation
   - Random Dropping
   - Message Cloning / Replication
   - Rerouting
   - Firewall
   - Custom
2. Filters are part of the HELICS core and the effect of a filter is not limited to the endpoints of local objects
3. A single Filter can be configured once and then applied to multiple endpoints as unique instances of that configuration. Filters can be triggered by either messages sent from an endpoint (source target) or messages received by an endpoint (destination targets)
4. Filters can be cloning or non-cloning filters. Cloning filters will operate on a copy of the message and in the simple form just deliver a copy to the specified destination locations. The original message gets delivered as it would have without the filter.

The figure below is an example of a representation of the message topology of a generic co-simulation federation composed entirely of message federates. Source and destination filters have been implemented (indicated by the blue endpoints -- gray endpoints do not have filters), each showing a different built-in HELICS filter function.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/messages_and_filters_example.png)

- In this figure, Federate 4 has a single endpoint for sending and receiving messages. Both a source filter and a destination filter can be set up on a single endpoint, or multiple source filters can be used on the same endpoint.
- The source filter on Federate 3 delays the messages to both Federate 2 and Federate 4 by 0.5 seconds. Without establishing a separate source endpoint devoted to each federate, there is no way to produce different delays in the messages sent along these two paths.
- Because the filter on Federate 4 is a destination filter, the message it receives from Federate 3 is affected by the filter but the message it sends to Federate 2 is not affected.
- The source filter on Federate 2 has no impact on this co-simulation as there are no messages sent from that endpoint.

#### Interactions Between Messages and Values

Because HELICS values are used to represent physical reality, they are available to any subscribing federate at any time. If the publishing federate only updates the value, say, once every minute, any subscribing federates that are granted a time during that minute window will all receive the same value.

Though it is not possible to have a HELICS message show up at a value interface, the converse is possible; message federates can subscribe to HELICS values. Every time a value federate publishes a new value to the federation, if a message federate has configured an endpoint with a `"subscription"` parameter defined, HELICS will generate a new HELICS message and send it directly to the subscribing endpoint every time a new value is published. These messages are queued and not overwritten (unlike in HELICS values) which means when a message federate is granted a time, it may have multiple messages from the same source to process.

This feature offers the convenience of allowing a message federate to receive messages from pure value federates that have no endpoints defined. This is particularly useful for simulators that do not support endpoints but are required to provide measurement signals for controllers. Implemented in this way, though, it is not possible to later implement a full-blown communication simulator that these values-turned-messages can traverse. Such co-simulation architectures in HELICS require the existence of both a sending and receiving endpoint; this feature very explicitly by-passes the need for a sending endpoint.

### Combination Federates

Combination federates make use of both value signals and message signals for transferring data between federates. The [Combination Federation](../examples/fundamental_examples/fundamental_combo.md) in the Fundamental Examples learning track introduces a third federate to the [Base Example](../examples/fundamental_examples/fundamental_default.md) -- the combination federate passes values with the battery federate to monitor the physics of the battery charging, and it also passes messages with a controller federate to decide when to stop charging.

![Combination Federate](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_complete.png)

The following table may be useful in understanding the differences between the two methods by which federates can communicate:

|                                                | Values                           | Messages                               |
| :--------------------------------------------- | :------------------------------- | :------------------------------------- |
| Interface Type:                                | Publication/Subscription/Input   | Endpoint/Filter                        |
| [Signal Route](../advanced_topics/queries.md): | Fixed, defined at initialization | Determined at time of transmission     |
| Outgoing signal:                               | 1 to n (broadcast)               | 1 to 1 - defined sender and receiver   |
| Incoming signal:                               | n to 1 (promiscuous)             | 1 to 1 - defined sender and receiver   |
| Status on Message Bus:                         | Current value always available   | Removed when received                  |
| Fidelity:                                      | Default value                    | Rerouting/modification through filters |
| Signal Contents:                               | Physical units                   | Generic binary blobs                   |
