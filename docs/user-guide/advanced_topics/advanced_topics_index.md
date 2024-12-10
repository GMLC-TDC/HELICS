# Advanced Topics

```{eval-rst}
.. toctree::
    :maxdepth: 1
    :hidden:

    aliases
    architectures
    broker_hierarchies
    callbacks
    CallbackFederate
    commandInterface
    connector_fed
    CoreTypes
    dynamic_federations
    encrypted_communication
    environment_variables
    iteration
    multibroker
    broker_multicomputer
    multiSourceInputs
    networking
    orchestration
    program_termination
    profiling
    queries
    simultaneous_cosimulations
    targeted_endpoints
    timeouts
    timingOptimization
    translators
    webserver
```

Whereas the [Fundamental Topics](../fundamental_topics/fundamental_topics_index.md) provided a broad overview of co-simulation and a good step-by-step introduction to setting up a HELICS co-simulation, the
Advanced Topics section assumes you, the reader, have a familiarity and experience with running HELICS co-simulations. If that's not the case, it's well worth your while to go review the [Fundamental Topics](../fundamental_topics/fundamental_topics_index.md) and [corresponding examples](../examples/fundamental_examples/fundamental_examples_index.md). In this section it will be assumed you know things like:

- The difference between value and message passing in HELICS
- How to configure HELICS federate appropriately
- Familiarity with the common HELICS APIs (_e.g._ requesting time, getting subscribed values, publishing values)
- Experience running HELICS co-simulations

The Advanced Topics section will dig into specific features of HELICS that are less commonly used but can be very useful in particular situations. Each section below provides a description of the feature, what it does, the kind of use case that might utilize it, and then links to examples that demonstrate an implementation. It's important to note that there are many other HELICS features and APIs not demonstrated here that can also be useful. As they say in academia, we'll leave it as an exercise to the reader to discover these. (Hint: The [API references](../../references/api-reference/index.md) and the [Configuration Options Reference](../../references/configuration_options_reference.md) are good starting points to see what's out there in the broader HELICS world.)

The Advanced Topics will cover:

- [**Aliases**](./aliases.md) - HELICS 3.3 introduced the notion of aliases. Aliases allow a mapping of an interface key to a different string.
- [**Architectures**](./architectures.md) - Introduction to different ways to connect federates, cores, and brokers to manage efficient passing of signals in a co-simulation.
- [**Broker Hierarchies**](./broker_hierarchies.md) - Purpose of broker hierarchies and how to configure a HELICS co-simulation to implement one.
- [**Callbacks**](./callbacks.md) - Over time a number of callbacks have been added for various operations and stages of the life cycle of a federate. This document describes the different callbacks available.
- [**Callback Federates**](./CallbackFederate.md) - HELICS 3.3 introduced a beta test for callback federates which allow a federate to operate purely inline with a core based solely on callbacks. This can allow a much higher number of federates on a given system than was previously possible.
- [**Command Interface**](./commandInterface.md) - HELICS v3 introduced the command interface as a method of asynchronously communicating between federates.
- [**helics_connector**](./commandInterface.md) - App that creates connections between federates as a separate process from the declaration of said interfaces.
- [**Cores**](./CoreTypes.md) - Discussion of the different types of message-passing buses and their implementation as HELICS cores.
- [**Dynamic Federations**](./dynamic_federations.md) - Sometimes it is useful to have a federate that is not ready at the beginning of co-simulation. This is a dynamic federation. There are various levels of this (not all are available yet) and this document discusses some aspects of dynamic co-simulation.
- [**Encrypted Communication**](./encrypted_communication.md) - How to encrypt communication between HELICS brokers/federates.
- [**Environment Variables**](./environment_variables.md) - HELICS supports some environment variables for configuration of a federate or broker.
- [**Iteration**](./iteration.md) - Setting up federates so that they can iterate without advancing simulation time to achieve a more consistent state.
- [**Multi-compute-node Co-simulation**](./broker_multicomputer.md) - Executing a co-simulation across multiple compute nodes.
- [**Multi-Protocol Brokers (Multi-broker) for Multiple Core Types)**](./multibroker.md) What to do when one type of communication isn't sufficient.
- [**Multi-Source Inputs**](./multiSourceInputs.md) - Using inputs (rather than subscriptions), it is possible to accept value signals from multiple sources. This section discusses the various tools HELICS provides for managing how to handle/resolve what can be conflicting or inconsistent signal data.
- [**Networking**](networking.md) - How to configure HELICS to run in more challenging networking environments.
- [**Orchestration Tool (Merlin)**](./orchestration.md) - Brief guide on using [Merlin](https://github.com/LLNL/merlin) to handle situations where a HELICS co-simulation is just one step in an automated analysis process (_e.g._ uncertainty quantification) or where assistance is needed deploying a large co-simulation in an HPC environment.
- [**Program termination**](./program_termination.md) - Some additional features in HELICS related to program shutdown and co-simulation termination.
- [**Profiling**](./profiling.md) - Some profiling capability for co-simulations.
- [**Queries**](./queries.md) - How queries can be used to get information on HELICS brokers, federates, and cores.
- [**Simultaneous co-simulations**](./simultaneous_cosimulations.md) - Options for running multiple independent co-simulations on a single system.
- [**Targeted Endpoints**](./targeted_endpoints.md) - details on the new targeted endpoints in HELICS 3.
- [**Timeouts**](./timeouts.md) - HELICS includes a number of timeouts to prevent failed operations from continuing indefinitely, the various timeout options are discussed in this document.
- [**Timing Optimization**](./timingOptimization.md) - Guidance and recommendation on how to configure and set-up timing to optimize federation performance.
- [**Translators**](./translators.md) - Translators provide a means of HELICS message interfaces to communicate with HELICS value interfaces and vice versa.
- [**Webserver API**](./webserver.md) - How to interact with a running co-simulation using a REST-based web API.
