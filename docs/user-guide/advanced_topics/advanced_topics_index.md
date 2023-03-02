# Advanced Topics

```{eval-rst}
.. toctree::
    :maxdepth: 1
    :hidden:

    multiSourceInputs
    architectures
    queries
    webserver
    CoreTypes
    multibroker
    broker_hierarchies
    environment_variables
    simultaneous_cosimulations
    broker_multicomputer
    orchestration
    program_termination
    profiling
    targeted_endpoints
    dynamic_federations
    timeouts
    commandInterface
    iteration
    aliases
    callbacks
    CallbackFederate
    networking
    encrypted_communication
```

Whereas the [Fundamental Topics](../fundamental_topics/fundamental_topics_index.md) provided a broad overview of co-simulation and a good step-by-step introduction to setting up a HELICS co-simulation, the
Advanced Topics section assumes you, the reader, have a familiarity and experience with running HELICS co-simulations. If that's not the case, it's well worth your while to go review the [Fundamental Topics](../fundamental_topics/fundamental_topics_index.md) and [corresponding examples](../examples/fundamental_examples/fundamental_examples_index.md). In this section it will be assumed you know things like:

- The difference between value and message passing in HELICS
- How to configure HELICS federate appropriately
- Familiarity with the common HELICS APIs (_e.g._ requesting time, getting subscribed values, publishing values)
- Experience running HELICS co-simulations

The Advanced Topics section will dig into specific features of HELICS that are less commonly used but can be very useful in particular situations. Each section below provides a description of the feature, what it does, the kind of use case that might utilize it, and then links to examples that demonstrate an implementation. It's important to note that there are many other HELICS features and APIs not demonstrated here that can also be useful. As they say in academia, we'll leave it as an exercise to the reader to discover these. (Hint: The [API references](../../references/api-reference/index.md) and the [Configuration Options Reference](../../references/configuration_options_reference.md) are good starting points to see what's out there in the broader HELICS world.)

The Advanced Topics will cover:

- [**Multi-Source Inputs**](./multiSourceInputs.md) - Using inputs (rather than subscriptions), it is possible to accept value signals from multiple sources. This section discusses the various tools HELICS provides for managing how to handle/resolve what can be conflicting or inconsistent signal data.
- [**Architectures**](./architectures.md) - Introduction to different ways to connect federates, cores, and brokers to manage efficient passing of signals in a co-simulation.
- [**Queries**](./queries.md) - How queries can be used to get information on HELICS brokers, federates, and cores.
- [**API Webserver**](./webserver.md) - How to interact with a running co-simulation using queries.
- [**Cores**](./CoreTypes.md) - Discussion of the different types of message-passing buses and their implementation as HELICS cores.
- **Multiple Brokers**
  - [**Connecting Multiple Core Types (Multi-Protocol Broker)**](./multibroker.md) What to do when one type of communication isn't sufficient.
  - [**Broker Hierarchies**](./broker_hierarchies.md) - Purpose of broker hierarchies and how to configure a HELICS co-simulation to implement one.
- [**Environment Variables**](./environment_variables.md) - HELICS supports some environment variables for configuration of a federate or broker.
- [**Simultaneous co-simulations**](./simultaneous_cosimulations.md) - Options for running multiple independent co-simulations on a single system.
- [**Orchestration Tool (Merlin)**](./orchestration.md) - Brief guide on using [Merlin](https://github.com/LLNL/merlin) to handle situations where a HELICS co-simulation is just one step in an automated analysis process (_e.g._ uncertainty quantification) or where assistance is needed deploying a large co-simulation in an HPC environment.
- [**Program termination**](./program_termination.md) - Some additional features in HELICS related to program shutdown and co-simulation termination.
- [**Profiling**](./profiling.md) - Some profiling capability for co-simulations.
- [**Targeted Endpoints**](./targeted_endpoints.md) - details on the new targeted endpoints in HELICS 3.
- [**Dynamic Federations**](./dynamic_federations.md) - Sometimes it is useful to have a federate that is not ready at the beginning of co-simulation. This is a dynamic federation. There are various levels of this (not all are available yet) and this document discusses some aspects of dynamic co-simulation.
- [**Timeouts**](./timeouts.md) - HELICS includes a number of timeouts to prevent failed operations from continuing indefinitely, the various timeout options are discussed in this document.
- [**Command Interface**](./commandInterface.md) - HELICS v3 introduced the command interface as a method of asynchronously communicating between federates.
- [**Aliases**](./aliases.md) - HELICS 3.3 introduced the notion of aliases. Aliases allow a mapping of an interface key to a different string.
- [**Callbacks**](./callbacks.md) - Over time a number of callbacks have been added for various operations and stages of the life cycle of a federate. This document describes the different callbacks available.
- [**Callback Federates**](./CallbackFederate.md) - HELICS 3.3 introduced a beta test for callback federates which allow a federate to operate purely inline with a core based solely on callbacks. This can allow a much higher number of federates on a given system than was previously possible.
- [**Networking**](./CallbackFederate.md) - HELICS provides several ways of working in more restrictive networking environments.
- [**Encrypted Communication**](./encrypted_communication.md) - How to encrypt communication between HELICS brokers/federates.
