# HELICS Terminology

Before digging into the specifics of how a HELICS co-simulation runs, there are a number of key terms and concepts that need to be clarified first.

![Relational diagram for key HELICS concepts](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/HELICS_terminology.png)

**Simulator** - A simulator is the executable that is able to perform some analysis function, often but not always, by solving specific problems to generate a time series of values.

- Simulators are abstract in the sense that it largely refers to the software in a non-executing state, outside of the co-simulation. We might say things like, "That simulator doesn't have feature xyz." or "This simulator has been parallelized and runs incredibly quickly on many-core computers."
- Any time we are talking about a specific instance of a simulator running a specific model you are really talking about a federate.
  - Simulators, on their own, do not have the ability to join a HELICS co-simulation but can still have a model associated them.

**Federate** - Federates are the running instances of simulators that have been assigned specific models and/or have specific values they are providing to and receiving from other federates.

- For example, we can have ten distribution circuit models that we are connecting for a co-simulation. Each could be run by the simulator GridLAB-D, and when they are running, they become ten unique federates providing unique values to each other.
- A collection of federates working together in a co-simulation is called a "federation."
  - Additional documentation can be found in [this tutorial's page on federates](./federates.md).

**Model** - A model is the representation of some portion of reality being managed by a federate.

- A simulator contains the calculations for the model.
- Depending on the needs of the co-simulation, a federate can be configured to contain one or many models.
- For example, if we want to create a co-simulation of electric vehicles, we may write a simulator (executable program) to model the physics of the electric vehicle battery. We can then designate any number of agents/models of this battery by configuring the transfer of signals between the "Battery Federate" (which has _N_ batteries modeled) and another federate.

**Signals** - Signals are the the information passed between federates during the execution of the co-simulation. Fundamentally, co-simulation is about message-passing via these signals.

- We can notionally think of federates talking directly to each other by passing signals back and forth. Under the hood, the path the data takes is through the core and then broker.
- HELICS divides these messages into two types: value signals and message signal. The former is used when coupling two federates that share physics (_e.g._ batteries providing power to wheel motors on an electric car) and the later is used to couple two federates with information (_e.g._ a battery charge controller and a charge relay on a battery).
- There are also a variety of mechanisms within a co-simulation to define the nature of the data being exchanged (data type, for example) and how the data is distributed around the federation.

**Interface** - An object by which a federate pass signals to each other.

- Includes Endpoints, Publications, and Inputs.
- Additional documentation on interfaces can be found in [this tutorial's page on federates](./federates.md).

**Core** - The core is the software that has been embedded inside a simulator to allow it to join a HELICS federation. Simulators without a core are not HELICS federates.

- In general, each federate has a single core, making the two synonymous (core <=> federate).
- The two most common configurations are: (1) one core, one federate, one model; (2) one core, one federate, multiple models.
- There are sometimes cases where a single executable is used to represent multiple federates and all of those federates use a single core (one core, multiple federates, multiple models).
- Cores are built around specific messaging protocols with HELICS supporting a number of different protocols (_e.g._ ZMQ, TCP, MPI). Selection of the messaging protocol is part of the configuration process required to form the federation. Additional information about cores can be found in the [Advanced Topics](../advanced_topics/CoreTypes.md).

**Broker** - The broker is a special executable distributed with HELICS; it is responsible for facilitating signal passing between federates.

- Each core (federate) must connect to a broker to be part of the federation.
- Brokers receive and distribute messages from any federates that are connected to it, routing them to the appropriate location.
- HELICS also supports a hierarchy of brokers, allowing brokers to pass messages between each other to connect federates associated with different brokers and thus maintain the integrity of the federation. The broker at the top of the hierarchy is called the "root broker" and it is the message router of last resort. [Additional information about broker hierarchies can be found here](../advanced_topics/broker_hierarchies.md).
