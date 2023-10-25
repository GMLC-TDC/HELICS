# Fundamental Topics

```{eval-rst}
.. toctree::
    :hidden:
    :maxdepth: 1

    helics_terminology
    federates
    interface_configuration
    timing_configuration
    stages
    logging
    helics_run
    simulator_integration


```

The fundamental topics listed below cover the material necessary to build a fully functional co-simulation with HELICS, written for users who have little to no experience with co-simulation. Each section makes reference to the [Fundamental Examples](../examples/fundamental_examples/fundamental_examples_index.md) to allow the user to scaffold their learning with concrete and detailed examples. After working through the topics below, the user should be able to write their own simple co-simulation in Python with PyHELICS and understand how access resources improve the development of their co-simulations.

The topics considered "fundamental" to building a co-simualtion with HELICS are:

- [**HELICS Terminology**](./helics_terminology) - Key terms and concepts to understand before running co-simulations with HELICS
- [**Federates**](./federates.md) - Discussion of the different types of federates in HELICS and how to configure them.
  - [Value Federates](./value_federates.md)
  - [Message Federates](./message_federates.md)
  - [Filters](./filters.md)
- [**Federate Interface Configuration**](./interface_configuration.md) - How to connect an existing simulator with HELICS
  - [**With JSON config file**](./interface_configuration.md#json-configuration)
  - [**With HELICS APIs**](./interface_configuration.md#api-configuration)
- [**Timing Configuration**](./timing_configuration.md) - How HELICS coordinates the simulation time of all the federates in the federation
  - [Timing Exercise](./timing_exercise.md)
  - [Timing Exercise answers](./timing_exercise_answers.md)
- [**Stages of the Co-simulation**](./stages.md)
- [**Logging**](./logging.md) - Discussion of logging within HELICS and how to control it.
- [**Execution with `helics run ...`**](./helics_run.md) - The HELICS team has developed a standardized means of running HELICS co-simulations.
- [**Simulator Integration**](./simulator_integration.md) - A guide for integrating HELICS into simulators.
