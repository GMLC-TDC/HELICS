# Orientation

There are a number of classes of HELICS users:

- **New users** that have little to no experience with HELICS and co-simulation in general
	- Start with [**Installation**](../installation/index.md)
	- Read the [**Fundamental Topics**](./index.html#fundamental-topics)
	- Try the [**Examples**](./index.html#examples)	
- **Intermediate users (Modelers)** that have run co-simulations with HELICS using simulators in which somebody else has done the simulator integration with HELICS.
	- Review [**Fundamental Topics**](./index.html#fundamental-topics) as needed 
	- Look over the [**Advanced Topics**](./index.html#advanced-topics) to see which features of HELICS may be most useful for your analysis.
		- [Multi-Source Inputs](./advanced_topics/multiSourceInputs.md) ([example](./examples/advanced_examples/advanced_multi_input.md))
		- [Broker Hierarchies](./advanced_topics/broker_hierarchies.md) ([example](./examples/advanced_examples/advanced_brokers_hierarchies.md))
		- [HELICS Core Types](./advanced_topics/CoreTypes.md) ([example](./examples/advanced_examples/advanced_brokers_multibroker.md))
		- [Queries](./advanced_topics/queries.md) ([example](./examples/advanced_examples/advanced_query.md))
		- [Simultaneous Co-simulation](./advanced_topics/simultaneous_cosimulations.md) ([example](./examples/advanced_examples/advanced_brokers_simultaneous.md))
		- [Multiple Co-simulation Orchestration](./advanced_topics/orchestration.md) ([example](./examples/advanced_examples/advanced_orchestration.md))
- **Experienced users (Integrators)** that are incorporating a new simulator and need to know how to use specific features in the HELICS API
	- Look in the [**Configurations Options Reference**](./index.html#configurations-options-reference) or jump straight to the API references
		- [C++](https://docs.helics.org/en/latest/doxygen/index.html)
		- C++98
		- [C](https://docs.helics.org/en/helics3userguide/api-reference/C_API.html)
		- [Python](https://python.helics.org/api/) 
		- [Julia](https://julia.helics.org/latest/api/)
		- [nim](https://github.com/GMLC-TDC/helics.nim)
- **Developers** of HELICS who are improving HELICS functionality and contributing to the code base
	- See the [**Developer Guide**](../developer-guide.md)