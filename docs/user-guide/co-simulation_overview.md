# Co-Simulation Overview

Co-simulation is an analysis technique that allows simulators from different domains to interact with each other, typically by exchanging values through the course of the simulation that define other simulators' boundary conditions. For example, you may have a model and simulator that is able to describe atmospheric conditions and weather as they progress through time. You may have just found out about another well-established model and simulator that describes the growth of vegetation over large geographic areas. In reality, these two systems clearly interact with the precipitation and temperatures impacting the growth of vegetation and the amount of vegetation impacting air temperature and moisture content. These models, though, may treat this interaction as a boundary condition such that the atmospheric simulator assumes a fixed rate of sunlight reflection from the ground and the vegetation simulator uses a time-series of historical precipitation values.

Co-simulation allows the coupling of existing models and simulators by breaking down these boundary condition barriers and calculating results that are more realistic and dynamic. In this case, while the atmospheric simulator is able to calculate the current weather using the latest values of surface reflectivity and humidity contributions from the vegetation, the vegetation simulator is able to use the latest values from the atmospheric simulator to determine the moisture content of the soil and the ambient temperature.

![Signals sent between domains in a hypothetical co-simulation](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/atmospheric.png)

It's easy to imagine this hypothetical model being extended with other simulators throughout the ecosphere, incorporating models describing the behavior of the ocean, wildlife, even human agriculture. Rather than trying to build a single simulator that models all of this functionality, co-simulation allows us to use existing simulators that have longer-term investment and validation behind them; that experience is leveraged into building simulations of complex systems of systems.

Co-simulation also has a built-in parallelism that allows it to be used to scale up to very large models. Particularly for systems with limited interactions between portions of the models, the value exchanges that take place during co-simulation are limited, allowing the individual instances of the simulators to run independently. Using our previous example, it is easy to imagine that there is limited to no interaction between vegetation in Oregon and vegetation in Texas (except through the atmosphere). Simulations of the vegetations in each of these states can run independently due to this lack of interaction, each providing values to the atmospheric model that indirectly couples them together. When able to be constructed in such a manner, the limitations on the size and complexity of the co-simulation become limited more by computing resources than by the capabilities of the individual simulators.

This being said, coupling these simulators together effectively can be summarized in the execution of two very related functions:

1. Maintaining synchronization of all the simulator instances running
2. Facilitating the data transfer between them

Maintaining synchronization is required when working in a heterogeneous simulation environment where each simulator's concept of time is different and the computation time required by each simulator instance can vary widely. Without the regulation of the universal co-simulation time, individual simulator instances could easily run ahead of the rest, simulating days and weeks ahead of the others.

And when one simulator instance is simulating one week ahead of the others, it becomes impossible for the results from that simulation to affect the rest of the simulator instances; the values it passes and receive are literally from a different point in time then the rest and are effectively meaningless. Without the synchronization of time, there is no way for the simulator instances to affect each other and without this interaction, the results of the co-simulation are meaningless.

HELICS, at its core, has been designed to do these two functions as efficiently, quickly, and comprehensively as possible to support a wide variety of simulators and models. Simulators with HELICS support have been modified in such a way so as to allow HELICS (really, a HELICS core object but we'll get to that in the [section on timing](./fundamental_topics/timing_configuration.md)) to advance the time of the simulator and provide it new values for specific variables that it is interested in. Making this modification to the simulator is not necessarily difficult but it must be done in such a way as to allow for these two fundamental functions to be executed.

Even after a simulator has been modified to support HELICS, users of that simulator who are building co-simulations have the task of constructing or designing a co-simulation to perform the particular analysis they desire. Many simulators have thousands of potential values they can provide to other simulators and generally, most of these values will not be needed by other simulators. It is the role of those designing the co-simulation to correctly configure each of the simulator instances such that the appropriate values are sent and received by the simulator instances and are mapped to the appropriate parameters inside each simulator. For a small number of simulators with a small number of values, this is not necessarily difficult but as the size of the co-simulation grows, this task becomes more challenging.

Thinking about HELICS co-simulation in general and about the nature of the simulators that have been or need to be integrated with HELICS, there are a few questions to consider. Though these questions have most immediate impact as you being planning on how to integrate a particular simulator they are also useful when thinking about how simulators in a federation need to be made to interact properly.

1. **What is the nature of the code-base being integrated?** Is this open-source code that can be fully modified? Is it a simulator, perhaps commercial, that provides an API that will be used? How much control do you, the integrator, have in modifying the behavior of the simulator? A number of existing simulation tools have [already been integrated into HELICS](../references/Tools_using_HELICS.md) and can serve as good references when considering how an integration might be accomplished.

2. **What programming language(s) will be used?** - HELICS has bindings for a number of languages and the one that is best to use may or may not be obvious. If your integration of the simulator will be through the API of the existing simulator, then you'll likely be writing a standalone executable that wraps that API. You may be constrained on the choice of languages based on the method of interaction with that API. If the API is accessed through a network socket then you likely have a lot of freedom in language choice. If the API is a library that you call from within wrapper, you will likely be best served using the language of that library.

   If you're writing your own simulator then you have a lot more freedom and the language you use may come down to personal preference and/or performance requirements of the federate.

   The languages currently supported by HELICS are:

   - C++
   - C
   - Python (2 and 3)
   - Java
   - MATLAB
   - Octave
   - C# (somewhat limited)
   - Julia
   - Nim

3. **What is the simulator's concept of time?** - Understanding how the simulator natively moves through time is essential when determining how time requests will need to be made. Does the simulator have a fixed time-step? Is it user-definable? Does the simulator have any concept of time or is it event-based?

4. **What is the nature of the data the simulator will send to and receive from the rest of the federation?** Often, this answer is in large part provided by the analysis need that is motivating the integration. However, there may be other angles to consider beyond what's immediately apparent. As a stand-alone simulator, what are its inputs and outputs? What are its assumed or provided boundary conditions? Where do interdependencies exist between the simulator and other simulators within the federation? What kinds of data will it be providing to the rest of the federation?
