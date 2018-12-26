# User Guide Introduction #


Co-simulation is a powerful analysis technique that allows simulators of different domains to interact through the course of the simulation, typically by dynamically exchanging values that define boundary conditions for other simulators. HELICS is a co-simulation platform that has been designed to allow integration of these simulators across a variety of computation platforms and languages. HELICS has been designed to with power system simulation in mind ([GridLAB-D](https://github.com/gridlab-d/gridlab-d), [GridDyn](https://github.com/LLNL/GridDyn), [MATPOWER](https://github.com/GMLC-TDC/MATPOWER-wrapper), [OpenDSS](xxxxxxx), [PSLF](https://github.com/GMLC-TDC/PSLF-wrapper), [InterPSS](https://github.com/InterPSS-Project/ipss-common), [FESTIV](xxxxxxx)) but is general enough to support a wide variety of simulators and co-simulation tasks.

## Who Is This User Guide For? ##
There are a number of classes of HELICS users: 

* new users that have little to no experience with HELICS and co-simulation in general
* intermediate users that have run co-simulations with HELICS using simulators in which somebody else implemented the HELICS support
* experienced users that are incorporating a new simulator and need to know how to use specific features in the HELICS API
* developers of HELICS who are improving HELICS functionality and contributing to the code base

This manual is primarily targeted to those in the first two categories. Other documentation xxxxxxxx exists that can be helpful for the latter two types of users.

## User Guide Overview ##


* [**Co-simuation Overview**](./co-simulation_overview.md) - A more detailed discussion of what co-simulation is and how it is used
* [**HELICS Key Concepts**](./helics_key_concepts) - Key terms and concepts to understand before running co-simulations with HELICS
* [**HELICS Co-Simulation Walk-through**](./helics_co-sim_sequence.md) - A notional walk-through of a simple transmission and distribution HELICS co-simulation to show the basic steps the software runs through
* [**Federates**](./federates.md) - Discussion of the different types of federates in HELICS ([value federates](./value_federates.md) and [message federates](./message_federates.md)) and how configure them
* [**Message Filters**](./filters) - How HELICS message filters can be implemented natively in HELICS or as stand-alone federates
* [**Co-Simulation Timing**](./timing.md) - How HELICS coordinates the simulation time of all the federates in the federation
* **Running HELICS co-simulations via `helics_cli`(future version)** - The HELICS team has developed `helics_cli` as a standardized means of running HELICS co-simulations.
* **Cores (future version)** - Discussion of the different types of message-passing buses and their implementation as HELICS cores
* **Broker Hierarchies (future version)** - Advantages and disadvantages of implementing hierarchies of brokers and how that is accomplished in HELICS
* **Reiteration (future version)** - Discussion of why reiteration is used and how to successfully execute it in HELICS
* [**Integrating a New Simulator**](./simulator_integration.md) - General overview of the process by which a simulator is integrated with HELICS including usage of the common APIs
*  **Trouble-Shooting HELICS Co-Simulations (future version)** - What to do when the co-simulations don't seem to be working correctly.
* **Large Co-Simulations in HELICS (future version)** - How to run HELICS co-simulations with a large (100+) number of federates

## Additional Resources ##
* [HELICS API](https://gmlc-tdc.github.io/HELICS-src/doxygen/) - Doxygen of the current API. If you need to know the details of how the APIs work, this is the place.
* [HELICS federate configuration](https://gmlc-tdc.github.io/HELICS-src/developer-guide/index.html) - Details on how the federates can be configured
* [Installation](https://gmlc-tdc.github.io/HELICS-src/installation/index.html) -  Instructions on how to install HELICS
* [C API](https://gmlc-tdc.github.io/HELICS-src/c-api-reference/index.html)
* [Developer's Guide](https://gmlc-tdc.github.io/HELICS-src/developer-guide/index.html) - Details on how the software is assembled
* [Youtube Channel](https://www.youtube.com/channel/UCPa81c4BVXEYXt2EShTzbcg/featured) - Throughout the development of HELICS, developers and users have given mini-tutorials providing overviews of the work they have been doing. Due to its nature, many of the specifics of the content are out of date but many of the general concepts of HELICS haven't changed. A good, broad overview of the project as a whole.