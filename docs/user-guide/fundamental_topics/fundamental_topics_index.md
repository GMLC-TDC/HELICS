# Fundamental Topics

```eval_rst
.. toctree::

    :maxdepth: 1

    helics_terminology
    federates
    simulator_integration
    timing
    helics_cli
    web_interface
    logging
	

```

```
what do we mean by 'fundamental topics' ?
what are the learning objectives?
by the end of this section, you should be able to...
provide context with examples

give narrative intro to topics introduced in this section
```

- [**HELICS Terminology**](./helics_terminology) - Key terms and concepts to understand before running co-simulations with HELICS
- [**Federate Configuration**](./federates.md) - Discussion of the different types of federates in HELICS and how to configure them.
	- [**Value Federates**](./value_federates.md)
	- [**Message Federates**](./message_federates.md)
	- [**Message Filters**](./filters) - How HELICS message filters can be implemented natively in HELICS or as stand-alone federates
- [**(Design) Integrating a Simulator with HELICS**](./simulator_integration.md) - How to connect an existing simulator with HELICS
	- **With JSON config file**
	- **With HELICS APIs**
- [**Timing**](./timing.md) - How HELICS coordinates the simulation time of all the federates in the federation
- [**(Launch) Execution with `helics_cli`**](./helics_cli.md) - The HELICS team has developed `helics_cli` as a standardized means of running HELICS co-simulations.
- [**Simulation Management**](./web_interface.md) - Using the webserver, HELICS also has a built-in web interface for running, monitoring, and diagnosing co-simulations.
- [**(Termination) Logging**](./logging.md) - Discussion of logging within HELICS and how to control it.

