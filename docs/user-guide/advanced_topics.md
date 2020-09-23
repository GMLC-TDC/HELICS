# Advanced Topics


```eval_rst
.. toctree::
    :maxdepth: 1

    multiSourceInputs
    broker_hierarchies
    CoreTypes
    queries
    multibroker
    orchestration
    simultaneous_cosimulations


```
- [**Multi-Source Inputs**](./multiSourceInputs.md) - Using inputs (rather and subscriptions) can accept value signals from multiple sources. This section discusses the various tools HELICS provides for managing how to handle/resolve what can be conflicting or inconsistent signal data.
- [**Queries**](./queries.md) - How queries can be used to get information on HELICS brokers, federates, and cores
- [**Cores**](./CoreTypes.md) - Discussion of the different types of message-passing buses and their implementation as HELICS cores
- **Multiple Brokers**
	- [**Connecting Multiple Core Types**](./multibroker.md) What to do when one type of communication isn't sufficient.
	- [**Broker Hierarchies**](./broker_hierarchies.md) - Purpose of broker hierarchies and how to
- **Reiteration (forthcoming)** - Discussion of why reiteration is used and how to successfully execute it in HELICS
- [**Simultaneous co-simulations**](./simultaneous_cosimulations.md) - Options for running multiple independent co-simulations on a single system
- [**Orchestration Tool (Merlin)**](./orchestration.md) Brief guide on using [Merlin](https://github.com/LLNL/merlin) to handle situations where a HELICS co-simulation is just one step in an automated analysis process (_e.g._ uncertainty quantification) or where assistance is needed deploying a large co-simulation in an HPC environment.
- **Large Co-Simulations in HELICS (forthcoming)** - How to run HELICS co-simulations with a large (100+) number of federates
- **HELICS Timing Algorithm (forthcoming)** - Detailed description of the HELICS timing algorithm and the finer points of manipulating it for maximum co-simulation performance.
- **Value Message Types (forthcoming)** - Detailed description of the four types of value messages in HELICS and how they can be used to minimize federation configuration effort.