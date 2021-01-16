# Types of Federates

A "federate", as previously introduced in the [HELICS Terminology section](./helics_terminology.md), is an instance of a simulation executable that can model a group of objects, or individual objects. For example, a federation may contain five electric vehicles (EVs) connected to charging ports in a dedicated EV charging garage. This co-simulation is the described in more detail in the [Fundamental Examples](../examples/fundamental_examples/fundamental_examples.md). There are two federates in this co-simulation; one modeling the batteries on board the EVs, and one modeling the chargers of the batteries. Each federate in this example has multiple objects it is modeling; in this case, five batteries for the battery federate, and five chargers for the charger federate. Because the entities being modeled with the federates share most of the same properties -- e.g., battery size, charge rate -- a single federate can be used to model the five batteries. As the complexity of the co-simulation increases, it becomes increasingly difficult to group modeled entities into one federate. In these situations, we can design the co-simulation with one federate for each EV. 

The [Fundamental Examples](../examples/fundamental_examples/fundamental_examples.md) are designed to have one battery federate model five batteries and one charger federate model five on board chargers. Co-simulations are designed to answer a research question; the question addressed by the examples is: How much power is needed to serve five EVs in a dedicated charging garage?

With this research question, we have identified that we want to model **batteries** and **chargers** and we want to moniter the power draw in **kW** over time. It's important to first identify the types of objects you want to model, as co-simulation in HELICS requires constructing federates based on the type of information they pass to other federates.

HELICS has three types of federates defined by the nature of the messages they are passing to and from the federation: **Value Federates, Message Federates, and Combination Federates**.



1. **Value federates** - Value federates are used when the federate is simulating the physics of a system. The data in the messages they send and receive indicate new values at the boundary of the federate system under simulation. In the [Fundamental Example](../examples/fundamental_examples/fundamental_examples.md), the batteries and chargers are both value federates; the battery sends the current (amps), and the charger sends the voltage. These two federates each update the other's boundary condition knowledge of the amps/voltage. Federates that model physics must be configured as **value federates**.  Value federates typically will update at very regular intervals based on the fidelity of their models and/or the resolution of any supporting data they are using.

   Value federates interact with the federation through what are called "interfaces". There are a handful of different interface types which will be discussed shortly but all are intended to provide the same essential functionality: allowing values to be sent from and received by a given federate. For value federates, these value signals will often have one-to-one correspondence to internal variables within the federate. 

2. **Message federates** - Message federates are used when the HELICS signals being passed to and from the simulation are generic packets of information, often for control purposes. They are treated as data to be used by an algorithm, processor, or controller. If the inputs to the federate can be thought of traveling over a communication network of some kind, it is probably coming from and/or going to a message federate. For example, in the power system world phasor measurement units ([PMUs](https://en.wikipedia.org/wiki/Phasor_measurement_unit)) have been installed throughout the power system and the measurements they make are collected through a communication system.

   Message federates interact with the federation through a conceptually different mechanism than a value federate. If message federates can be thought of as attaching to communication networks, the federate's "endpoints" are the specific interfaces to that communication network. By default, HELICS acts as the communication network, transferring signals between message federates from the endpoint on one to the endpoint on another.

3. **Combination federates** - Combination federates make use of both value signals and message signals for transferring data between federates. The [Combination Federation](../examples/fundamental_examples/fundamental_combo.md) in the Fundamental Examples learning track introduces a third federate to the [Base Example](../examples/fundamental_examples/fundamental_default.md) -- the combination federate passes values with the battery federate to monitor the physics of the battery charging, and it also passes messages with a controller federate to decide when to stop charging.

The following table may be useful in understanding the differences between the two methods by which federates can communicate:


|  |[Value Federate](./value_federates.md)  | [Message Federate](./message_federates.md)        |
|:---|:-----------------------------------------|:-----------------|
| Handle Type:  | Publication/Subscription |  Endpoint |
| [Signal Route](../advanced_topics/queries.md): |Fixed, defined when federation is initialized    |Determined at time of transmission   |
| Publications: | 1 to n (broadcast)    |1 to 1 - defined sender and receiver        |
| Inputs: | n to 1 (promiscuous)  |       None              |
| Status on Message Bus: |Current value always available|Removed when received |
| Fidelity: |Can be configured with a default value |Can be rerouted and modified through filters |
| Signal Contents: |Physical units  |Generic binary blobs  |

The next three sections present more detail on value federates and message federates, and introduce filters as a method to model communication failure.

```eval_rst
.. toctree::
    :maxdepth: 1
    
    value_federates
    message_federates
    filters

```