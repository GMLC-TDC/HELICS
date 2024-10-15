# Advanced Examples

```{eval-rst}
.. toctree::
    :maxdepth: 1
    :hidden:

    advanced_default
    advanced_brokers_simultaneous
    advanced_brokers_hierarchies
    advanced_brokers_multibroker
    advanced_brokers_multicomputer
    advanced_query
    advanced_multi_input
    advanced_orchestration
    advanced_iteration
    advanced_FMU
    advanced_async_time_request
    advanced_helics_connector

```

The Advanced Examples are modular, each building upon the [base example](./advanced_default.md). Users are encouraged to dive into a specific concept to build their HELICS knowledge once they have mastered the default setup in the [base example](./advanced_default.md).

This page describes the model for the Advanced Examples. This model is topically the same as the Fundamental Examples, but more complex in its execution. This allows the research question to become more nuanced and for the user to define new components to a HELICS co-simulation.

- <a href="#where-is-the-code">Where is the Code?</a>
- <a href="#what-is-this-co-simulation-doing">What is this Co-simulation Doing?</a>
  - <a href="#differences-compared-to-the-fundamental-examples">Differences Compared to the Fundamental Examples</a>
    - <a href="#helics-differences">HELICS Differences</a>
    - <a href="#research-question-complexity-differences">Research Question Complexity Differences</a>
- <a href="#helics-components">HELICS Components</a>
  - <a href="#federates-with-infinite-time">Federates with infinite time</a>
  - <a href="#initial-time-requests-and-model-initialization">Initial time requests and model initialization</a>

<a name="where-is-the-code">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
Where is the code?
</span>
</strong>
</a>

The code for the [Advanced examples](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced) can be found in the HELICS-Examples repository on github. If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_examples_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced)

<a name="what-is-this-co-simulation-doing">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
What is this co-simulation doing?
</span>
</strong>
</a>

The Advanced Examples are similar in theme to the [Base Example](../fundamental_examples/fundamental_default.md) in that both are looking at power management for an EV charging garage. The implemented federates, however, are slightly more sophisticated and include a new centralized charging controller federate.

- **Battery.py** - Models a set of the EV batteries being charged. The EV is randomly assigned to support a particular charging level and receives an applied charging voltage based on that level. Using the applied voltage and the current SOC (initially randomly assigned), a charging current is calculated returned to the charger.
- **Charger.py** - Models a set of EV charging terminals all capable of supporting three defined charging levels: level 1, 2, and 3. Applies a charging voltage based on the charging terminal power rating and (imperfectly) measures the returned current. Based on this current, it estimates the SOC and sends that information to the controller. When commanded to terminate charging it removes the applied charging voltage.
- **Controller.py** - Receives periodic updates about the SOC of each charging vehicle and when they are considered close enough to full, command the charger to terminate charging.

Every time charging is terminated on an EV, a new EV to take its place is randomly assigned a supported charging level and initial SOC.

<a name="differences-compared-to-the-fundamental-example">
<strong>
<span style="color:black;text-decoration:underline;">
Differences compared to the fundamental examples
</span>
</strong>
</a>

There are a few important distinctions between the Fundamental Examples and the Advanced Examples, which can be grouped into **HELICS** differences and **research question complexity** differences.

<a name="helics-differences">
<strong>
<span style="font-size:small;color:black;text-decoration:underline;">
HELICS differences
</span>
</strong>
</a>

1. **Communication:** Both physical value exchanges and abstract information exchanges are modeled. The exchange of physical values takes place between the Battery and Charger federates (this was also introduced in a slimmed-down fashion in the [Fundamental Communication Example](../fundamental_examples/fundamental_communication.md)). The message exchange (control signals, in this case) takes place between the Charger and Controller federates. For a further look at the difference between these two messaging mechanisms see our User Guide page on [value federates](../../fundamental_topics/value_federates.md) and [message federates.](../../fundamental_topics/message_federates.md)
2. **Timing:** The Controller federate has no regular update interval. The Controller works in pure abstract information and has no regular time steps to simulate. As such, it requests a maximum simulated time supported by HELICS (`HELICS_TIME_MAXTIME`) and makes sure it can be interrupted by setting `uninterruptible` to `false` in its configuration file. Any time a message comes in for the Controller, HELICS grants it a time, the Controller performs the required calculation, sends out a new control signal, and requests `HELICS_TIME_MAXTIME` again.

<a name="reaserach-question-complexity-differences">
<strong>
<span style="font-size:small;color:black;text-decoration:underline;">
Research question complexity differences
</span>
</strong>
</a>

In the [Fundamental Base Example](../fundamental_examples/fundamental_default.md), a similar research question is being addressed by this co-simulation anlaysis: estimate the **instantaneous power draw** from the EVs in the garage. And though you may have similar questions, there are several complicating changes in the new model:

1. A **third federate** (where previously there were only two) models responsibilities of a charger. The charger stops charging the battery by removing the charging voltage rather than the battery stopping the charging process. The Battery federate synthesizes an EV battery when the existing EV is considered fully charged.
2. The measurement of the charging current (used to calculate the actual charging power) has some **noise** built into it. This is modeled as random variation of the charging current in the federate itself and is a percentage of the charging current. The charging current decreases as the SOC of the battery increases leading to a noisier SOC estimate by the Charger federate at higher SOCs. This results in the Controller tending to terminate charging prematurely as a single sample of the noisy charging current can lead to over-estimation of the SOC.
3. We can now model both **physics** and **measurement of physics**. There are two SOC values modeled in this co-simulation: the "actual" SOC of the battery modeled in the Battery federate and the estimate of the SOC as measured by the Charger federate. Both federates calculate the SOC in the same manner: use the effective resistive load of the battery, R, and a pre-defined relationship between R and SOC. You can see that both the Battery and Charger federates use the exact same relationship between SOC and effective R (SOC of zero is equivalent to an effective resistance of 8 ohms; SOC of 1 has an effective resistance of 150 ohms). Due to the noise in the charger current measurement, there is error built into its calculation of the SOC and therefore should be considered an estimate of the SOC.

This existence of two values for one property is not uncommon and is as much a feature as a bug. If this system were to be implemented in actual hardware, the only way that a charger would know the SOC of a battery would be through some kind of external **measurement**. And certainly there would be times where the charger would have even less information (such as the specific relationship between SOC and effective resistance) and would have to use historical data, heuristics, or smarter algorithms to know how to charge the battery effectively. Simulation allows us to use two separate models and thus independently model the actual SOC as known by the battery and the estimated SOC as calculated by the charger.

Since the decision to declare an EV fully charged has been abstracted away from the Charger and Battery (to the Controller), a slightly different procedure is used to disconnect a charged EV from the charger and replace it with a new one to be charged. In this advanced example, a mini-protocol has been designed and implemented:

1. The Charger receives a message from the Controller indicating the EV should be considered fully charged.
2. The Charger reduces the Charging voltage to zero volts and publishes it.
3. The Battery, detecting this change in charging voltage, infers that it is fully charged. The Battery federate instantiates a new EV with a battery at a random initial state of charge. The Battery federate also calculates a charging current of zero amps and publishes it.
4. The Charger federate, seeing a charging current of zero amps, infers a new EV has been set up to charge, randomly assigns one of three charging powers, and publishes this new charging voltage.

At this point the co-simulation proceeds as previously defined. The Battery uses its internal knowledge of the state-of-charge to define the charging current which the Charger uses to estimate the state-of-charge and sends on to the Controller. The Controller sends back a message to the Charger based on this state-of-charge estimate indicating whether the EV should continue to be charged or not.

<a name="helics-components">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
HELICS components
</span>
</strong>
</a>

The HELICS components introduced in the Fundamental Examples are extended in the Advanced Examples with additional discussion of timing and initialization of federates. These new components enter into the sequence as follows:

1. Register and Configure Federates
2. <span style="color:red">Initialization</span>
3. Enter Execution Mode
4. Define Time Variables
5. <span style="color:red">Tell Controller federates to request `h.HELICS_TIME_MAXTIME`</span>
6. Initiate Time Steps for the Time Loop
7. Send and Receive Communication between Federates
8. Finalize Co-simulation

<a name="federates-with-infinite-time">
<strong>
<span style="color:black;text-decoration:underline;">
Federates with infinite time
</span>
</strong>
</a>

Federates which are abstractions of reality (e.g., controllers) do not need regular time interval updates. These types of federates can be set up to request `HELICS_TIME_MAXTIME` (effectively infinite time) and only update when a new message arrives for it to process. This component is placed prior to the main time loop.

```python
hours = 24 * 7  # one week
total_interval = int(60 * 60 * hours)
grantedtime = 0
starttime = int(h.HELICS_TIME_MAXTIME)
logger.debug(f"Requesting initial time {starttime}")
grantedtime = h.helicsFederateRequestTime(fed, starttime)
logger.debug(f"Granted time {grantedtime}")
```

<a name="initial-time-requests-and-model-initialization">
<strong>
<span style="color:black;text-decoration:underline;">
Initial time requests and model initialization
</span>
</strong>
</a>

As in the [Base Example](../fundamental_examples/fundamental_default.md), the EV batteries are assumed connected to the chargers at the beginning of the simulation and information exchange is initiated by the Charger federate sending the charging voltage to the Battery federate. In the Advanced Examples, this is a convenient choice as the charging voltage is constant and thus is never a function of the charging current. In a more realistic model, it's easy to imagine that the charger has an algorithm that adjusts the charging voltage based on the charging current to, say, ensure the battery is charged at a particular power level. In that case, **the dependency of the models is circular**; this is common component that needs to be addressed.

If the early time steps of the simulation are not as important (a model warm up period), then ensuring each federate has a default value it will provide when the input is null (and assuming the controller dynamics are not overly aggressive) will allow the models to bootstrap and through several iterations reach a consistent state. If this is not the case then HELICS does have a provision for getting models into a consistent state prior to the start of execution: initialization mode (see an example in Battery.py of the [query example](./advanced_query.md) to see the use of the initialization mode API). This mode allows for this same iteration between models with no simulated time passing. It is the responsibility of the modeler to make sure there is a method to reach and detect convergence of the models and when such conditions are met, enter execution mode as would normally be done. We've put together an [example on iteration](advanced_iteration.md) to demonstrate one way of managing convergence.

<a name="examples-covered-in-advanced-examples">
<strong>
<span style="color:black;text-decoration:underline;">
Examples Covered in Advanced Examples
</span>
</strong>
</a>

Using the [Advanced Default Example](./advanced_default.md) as the starting point, the following examples have also been constructed:

- [**Iteration**](./advanced_iteration) - Setting up federates so that they can iterate without advancing simulation time to achieve a more consistent state.
- [**Orchestration Tool (Merlin)**](./advanced_orchestration.md) Demonstration of using [Merlin](https://github.com/LLNL/merlin) to handle situations where a HELICS co-simulation is just one step in an automated analysis process (_e.g._ uncertainty quantification) or where assistance is needed deploying a large co-simulation in an HPC environment.
- **Multiple Brokers**
  - [**Connecting Multiple Core Types (Multi-Protocol Broker)**](./advanced_brokers_multibroker.md) - Demonstration of how to configure a multi-protocol broker
  - [**Broker Hierarchies**](./advanced_brokers_hierarchies.md) - Purpose of broker hierarchies and how to configure a HELICS co-simulation to implement one.
  - [**Simultaneous co-simulations**](./advanced_brokers_simultaneous.md) - Demonstration of how to run multiple independent federations simultaneously on a single compute node.
  - [**Multi-Protocol Brokers (Multi-broker) for Multiple Core Types)**](advanced_brokers_multibroker) What to do when one type of communication isn't sufficient.
  - [**Multi-compute-node Co-simulation**](advanced_brokers_multicomputer) - Executing a co-simulation across multiple compute nodes.
- [**Queries**](./advanced_query.md) - Demonstration of the use of queries for dynamic federate configuration.
- [**Multi-Source Inputs**](./advanced_multi_input.md) - Demonstration of use and configuration of a a multi-sourced input value interface.
- [**Orchestration Tool (Merlin)**](./advanced_orchestration.md) Demonstration of using [Merlin](https://github.com/LLNL/merlin) to handle situations where a HELICS co-simulation is just one step in an automated analysis process (_e.g._ uncertainty quantification) or where assistance is needed deploying a large co-simulation in an HPC environment.
