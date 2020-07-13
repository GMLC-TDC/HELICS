# User Guide

Co-simulation is a powerful analysis technique that allows simulators of different domains to interact through the course of the simulation, typically by dynamically exchanging values that define boundary conditions for other simulators. HELICS is a co-simulation platform that has been designed to allow integration of these simulators across a variety of computation platforms and languages. HELICS has been designed with power system simulation in mind ([GridLAB-D](https://github.com/gridlab-d/gridlab-d), [GridDyn](https://github.com/LLNL/GridDyn), [MATPOWER](https://github.com/GMLC-TDC/MATPOWER-wrapper), [OpenDSS](https://sourceforge.net/projects/electricdss/), [PSLF](https://github.com/GMLC-TDC/PSLF-wrapper), [InterPSS](https://github.com/InterPSS-Project/ipss-common), [FESTIV](https://www.nrel.gov/grid/festiv-model.html)) but is general enough to support a wide variety of simulators and co-simulation tasks. Support for other domains is anticipated to increase over time.

## Who Is This User Guide For?

There are a number of classes of HELICS users:

- New users that have little to no experience with HELICS and co-simulation in general
- Intermediate users that have run co-simulations with HELICS using simulators in which somebody else implemented the HELICS support
- Experienced users that are incorporating a new simulator and need to know how to use specific features in the HELICS API
- Developers of HELICS who are improving HELICS functionality and contributing to the code base

## Fundamental Topics

```eval_rst
.. toctree::
    :hidden:
    :maxdepth: 1

    co-simulation_overview
    helics_key_concepts
    helics_co-sim_sequence
    federates
    value_federates
    message_federates
    filters
    timing
    helics_cli
    logging
    webserver
    simulator_integration
    configuration_options_reference
```

- [**Co-Simulation Overview**](./co-simulation_overview.md) - A more detailed discussion of what co-simulation is and how it is used
- [**HELICS Key Concepts**](./helics_key_concepts) - Key terms and concepts to understand before running co-simulations with HELICS
- [**HELICS Co-Simulation Walk-through**](./helics_co-sim_sequence.md) - A notional walk-through of a simple transmission and distribution HELICS co-simulation to show the basic steps the software runs through
- [**Federates**](./federates.md) - Discussion of the different types of federates in HELICS ([value federates](./value_federates.md) and [message federates](./message_federates.md)) and how configure them
- [**Message Filters**](./filters) - How HELICS message filters can be implemented natively in HELICS or as stand-alone federates
- [**Co-Simulation Timing**](./timing.md) - How HELICS coordinates the simulation time of all the federates in the federation
- [**Running HELICS co-simulations via `helics_cli`**](./helics_cli.md) - The HELICS team has developed `helics_cli` as a standardized means of running HELICS co-simulations.
- [**Logging**](./logging.md) - Discussion of logging within HELICS and how to control it.
- [**Getting Information from a running simulation**](./webserver.md) - Getting live information from a running co-simulation through a webserver.
- [**Using the web interface**](./web_interface.md) - Using the webserver, HELICS also has a built-in web interface for running, monitoring, and diagnosing co-simulations.
- **Trouble-Shooting HELICS Co-Simulations (forthcoming)** - What to do when the co-simulations don't seem to be working correctly.
- [**Configuration Options Reference**](./configuration_options_reference.md) - Comprehensive reference to all the HELICS configuration options.

## Advanced Topics

```eval_rst
.. toctree::
    :hidden:
    :maxdepth: 1

    broker_hierarchies
    CoreTypes
    queries
    multibroker
    simultaneous_cosimulations


```

- [**Broker Hierarchies**](./broker_hierarchies.md) - Purpose of broker hierarchies and how to
- **Reiteration (forthcoming)** - Discussion of why reiteration is used and how to successfully execute it in HELICS
- **[Cores](./CoreTypes.md)** - Discussion of the different types of message-passing buses and their implementation as HELICS cores
- [**Queries**](./queries.md) - How queries can be used to get information on HELICS brokers, federates, and cores
- [**Connecting Multiple Core Types**](./multibroker.md) - What to do when one type of communication isn't sufficient.
- [**Simultaneous co-simulations**](./simultaneous_cosimulations.md) - Options for running multiple independent co-simulations on a single system
- **Large Co-Simulations in HELICS (forthcoming)** - How to run HELICS co-simulations with a large (100+) number of federates
- **HELICS Timing Algorithm (forthcoming)** - Detailed description of the HELICS timing algorithm and the finer points of manipulating it for maximum co-simulation performance.
- **Value Message Types (forthcoming)** - Detailed description of the four types of value messages in HELICS and how they can be used to minimize federation configuration effort.
