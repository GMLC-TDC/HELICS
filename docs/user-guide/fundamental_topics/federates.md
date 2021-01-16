# Federates

A "simulator" is the executable program. As soon as one particular instance of that simulator begins running in a co-simulation it is considered a "federate". Every federate (instance of a simulator) will require configuration of the way it will communicate (send signals) to other federates in the federation. For simulators that already have HELICS support, the configuration takes the form of a JSON (or TOML) file; bespoke simulators can be configured with the HELICS APIs in the code or via a JSON file. The essential information that HELICS configuration defines is:

   **Federate name** - The unique name this federate will be known as throughout the federation. It is essential this name is unique so that HELICS messages can route properly.

   **Core type** - The core manages interfaces between the federation and the federate; there are several messaging technologies supported by HELICS. 

   **Publications and Inputs** - Publication configuration contains a listing of source handle, data types, and units being sent by the federate; input configuration does the same for values being received by the federate. If supported by the simulator (e.g., [a Python simulator](../examples/fundamental_examples/fundamental_default.md)), these values can be mapped to internal variables of the simulator from the configuration file. 

   **Endpoints** - Endpoints are sending and receiving points for HELICS messages to and from message federates. They are declared and defined for each federate. 

   **Time step size** - This value defines the resolution of the simulator to prevent HELICS from telling the simulator to step to a time of which it has no concept (e.g. trying to simulate the time of 1.5 seconds when the simulator has a resolution of one second). 



## Types of Federates

A "federate", as previously introduced in the [HELICS Terminology section](./helics_terminology.md), is a specific instance of a simulation executable. For example, a federation may contain a five electric vehicles (EVs) connected to charging ports in a dedicated EV charging garage. This co-simulation is the described in more detail in the [Fundamental Examples](../../examples/fundamental_examples/fundamental_examples.md). There are two federates in this co-simulation; one modeling the batteries on board the EVs, and one modeling the chargers of the batteries. 

smaller computational executables can be standalone, 
with big models, difficult to lump into one model
if the federates are very similar -- parameter differences are small
less complex vs 

, each with their own charge controller implemented as a stand-alone federate.

```
In the examples, see the default example -- brief description of what it's trying to do.

describe the example here, link to page, and use this to describe the concepts below

ask Trevor to clarify what I write
```

 (maybe the co-simulation designer is trying out a fancy new coordination algorithm); we'll call this code `EV_coordinator_v12.exe`. The code that does this charge coordination is generic in the sense that it can be used to charge any of these EVs; it works with all makes and models. But to run the co-simulation, each EV will have to have its own running instance of this code that controls the charging of a particular vehicle. If the co-simulation is testing this algorithm with five EVs, then there would be five federates each running their own version of `EV_coordinator_v12.exe` which presumably have unique information particular to each individual EV (battery size, maximum charging rate, etc).



HELICS defines three specific types of federates defined by the nature of the messages they are passing to and from the federation:

```eval_rst
.. toctree::
    :maxdepth: 1
    
    value_federates
    message_federates
    filters

```


1. **Value federates** - Value federates are used when the federate is simulating the physics of a system. The data in the messages they send and receive are indicating new values at the boundary of the federates system under simulation. Taking the example from the our [high-level overview of the process of HELICS co-simulation execution](./helics_co-sim_sequence.md), the transmission and distribution system were value federates; they each supplied messages that updates the other's boundary condition. Value federates typically will update at very regular intervals based on the fidelity of their models and/or the resolution of any supporting data they are using; this likely is related to the time-step of the federate.

   Value federates interact with the federation through what are called "interfaces". There are a handful of different interface types which will be discussed shortly but all are intended to provide the same essential functionality: allowing values in HELICS messages to be sent from and received by a given federate. For value federates, these messages will often have one-to-one correspondence to internal variables within the federate. Using the [previous simple transmission and distribution example ](./helics_co-sim_sequence.md), the distribution system may send out a HELICS message that contains the value of its internal positive sequence load variable and receive HELICS messages that it uses for its internal positive sequence substation voltage variable.

2. **Message federates** - Message federates are used when the HELICS messages being passed to and from the simulation are generic packets of information, often for control purposes. Though these values may be physical measurements, they are treated as data to be used by an algorithm, processor, or controller. If the inputs to the federate can be thought of traveling over a communication network of some kind, it is probably coming from and/or going to a message federate. For example, in the power system world phasor measurement units ([PMUs](https://en.wikipedia.org/wiki/Phasor_measurement_unit)) have been installed throughout the power system and the measurements they make are collected through a communication system.

   Message federates interact with the federation through a conceptually different mechanism than a value federate. If message federates can be thought of as attaching to communication networks, the federate's "endpoints" are the specific interfaces to that communication network. By default, HELICS acts as the communication network, transferring signals between message federates from the endpoint on one to the endpoint on another.

The following table may be useful in understanding the differences between the two methods by which federates can communicate:

```
fix the table below
```

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

3. **Combination federates** - As you might guess, this type of federate makes use of both the value method and the endpoint method for transferring data between federates. An example of a federate like this could be a transmission system simulator that is acting both as a physical model of a system as well as a collection of PMUs that are sending data to a centralized generator dispatcher. The solution to the powerflow could be used to define substation voltages to some attached distribution circuits (physical values sent via publication) and the generator output powers could be sent to the centralized controller (control/measurement values being sent over a communication network via endpoints in a message federate).


## Example from Base Model 