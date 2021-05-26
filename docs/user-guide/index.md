# User Guide

Co-simulation is a powerful analysis technique that allows simulators of different domains to interact through the course of the simulation, typically by dynamically exchanging values that define boundary conditions for other simulators. HELICS is a co-simulation platform that has been designed to allow integration of these simulators across a variety of computation platforms and languages. HELICS has been designed with power system simulation in mind ([GridLAB-D](https://github.com/gridlab-d/gridlab-d), [GridDyn](https://github.com/LLNL/GridDyn), [MATPOWER](https://github.com/GMLC-TDC/MATPOWER-wrapper), [OpenDSS](https://sourceforge.net/projects/electricdss/), [PSLF](https://github.com/GMLC-TDC/PSLF-wrapper), [InterPSS](https://github.com/InterPSS-Project/ipss-common), [FESTIV](https://www.nrel.gov/grid/festiv-model.html)) but is general enough to support a wide variety of simulators and co-simulation tasks. Support for other domains is anticipated to increase over time.

## Who Is This User Guide For?

There are a number of classes of HELICS users:

- New users that have little to no experience with HELICS and co-simulation in general
- Intermediate users that have run co-simulations with HELICS using simulators in which somebody else implemented the HELICS support
- Experienced users that are incorporating a new simulator and need to know how to use specific features in the HELICS API
- Developers of HELICS who are improving HELICS functionality and contributing to the code base

## User Guide Overview

- [**Co-Simulation Overview**](./co-simulation_overview.md) - A more detailed discussion of what co-simulation is and how it is used
- [**HELICS Key Concepts**](./helics_key_concepts) - Key terms and concepts to understand before running co-simulations with HELICS
- [**HELICS Co-Simulation Walk-through**](./helics_co-sim_sequence.md) - A notional walk-through of a simple transmission and distribution HELICS co-simulation to show the basic steps the software runs through
- [**environment variables**](./environment_variables.md) - A discussion of HELICS supported environment variables for use in setting up a co-simulation
- [**Federates**](./federates.md) - Discussion of the different types of federates in HELICS ([value federates](./value_federates.md) and [message federates](./message_federates.md)) and how configure them
- [**Message Filters**](./filters) - How HELICS message filters can be implemented natively in HELICS or as stand-alone federates
- [**Co-Simulation Timing**](./timing.md) - How HELICS coordinates the simulation time of all the federates in the federation
- **Running HELICS co-simulations via `helics_cli`(forthcoming)** - The HELICS team has developed `helics_cli` as a standardized means of running HELICS co-simulations.
- **Cores (forthcoming)** - Discussion of the different types of message-passing buses and their implementation as HELICS cores
- **Broker Hierarchies (forthcoming)** - Advantages and disadvantages of implementing hierarchies of brokers and how that is accomplished in HELICS
- **Reiteration (forthcoming)** - Discussion of why reiteration is used and how to successfully execute it in HELICS
- [**Queries**](./queries.md) - How queries can be used to get information on HELICS brokers, federates, and cores
- [**Logging**](./logging.md) - Discussion of logging within HELICS and how to control it.
- [**Getting Information from a running simulation**](./webserver.md) - Getting live information from a running co-simulation through a webserver.
- [**Integrating a New Simulator**](./simulator_integration.md) - General overview of the process by which a simulator is integrated with HELICS including usage of the common APIs
- **Trouble-Shooting HELICS Co-Simulations (forthcoming)** - What to do when the co-simulations don't seem to be working correctly.
- [**Simultaneous co-simulations**](./simultaneous_cosimulations.md) - Options for running multiple independent co-simulations on a single system
- [**Connecting Multiple Core Types**](./multibroker.md) - What to do when one type of communication isn't sufficient.
- [**N to 1 input connections**](./multiSourceInputs.md) - Handling multiple publications to a single input
- **Large Co-Simulations in HELICS (forthcoming)** - How to run HELICS co-simulations with a large (100+) number of federates
- [**Debugging**](./debugging.md) - Capabilities to help with debugging
- [**Terminating a co-simulation**](./program_termination.md) - Some helpful tools for terminating a co-simulation

## Additional Resources

- [HELICS API](../doxygen/index.html) - Doxygen of the current API. If you need to know the details of the APIs and function calls, this is the place.
- [HELICS federate configuration](../configuration/index.html) - Details on how the federates can be configured
- [Installation](../installation/index.html) - Instructions on how to install HELICS
- [C API](../c-api-reference/index.html)
- [Developer's Guide](../developer-guide/index.html) - Details on how the software is assembled and some of the underlying components.
- [Existing Tools](../Tools_using_HELICS.md) - List of the existing tools using HELICS and some under development.
- [Youtube Channel](https://www.youtube.com/channel/UCPa81c4BVXEYXt2EShTzbcg/featured) - Throughout the development of HELICS, developers and users have given mini-tutorials providing overviews of the work they have been doing. Due to its nature, many of the specifics of the content are out of date but many of the general concepts of HELICS haven't changed. A good, broad overview of the project as a whole.
