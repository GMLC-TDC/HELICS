---
title: Specification document for TDC co-simulation tool

author:
  - name: Phil Top
    membership: Lawrence Livermore National Laboratory
    email:
  - name: Bryan Palmintier
    membership: National Renewable Energy Laboratory
    email: bryan.palmintier@nrel.gov
  - name: Jason Fuller
    membership: Pacific Northwest National Laboratory
    email: jason.fuller@pnnl.gov
  - name: Dheepak Krishnamurthy
    membership: National Renewable Energy Laboratory
    email: dheepak.krishnamurthy@nrel.gov

date: May 12, 2017
abstract: TDC Specification document describing philosophy, layers, APIs, testing requirements, etc.
version: 0.4.0
---

# Introduction

This guiding document describes the development specification for the TDC co-simulation tool.

# Basic Requirements

The following was adapted from TDC Simulation Requirements developed by team November 7, 2016

## Required by Stakeholders

- **High Scalable:** Support co-simulation with 2 to >100,000 federates (WECC capable)
- **Cross-platform:**
  - Windows Laptops and Workstations
  - OSX workstations
  - Linux-based Supercomputers
- **Modular**:
  - Built to support separate software components
  - Components can be used in conjunction with other components of the system
- **support diverse TDC+ tools:**
  - T: GridDyn.
    - Short term integration goals : PFLOW (ANL), MatPower
    - Future integration with PSSE, PSLF
  - D: GridLab-D.
    - Short term integration goals : OpenDSS.
    - Future integration with CyME, Synergi, Power Factor
  - C: ns-3, latency
  - M: FESTIV
  - U: User-provided controller/virtual machine model
  - H: HLA-compliant user models
  - F: FMI-compliant user models
- **Open Source:**
  - under a permissive (BSD-style) licensing
- **Support TDC identified use cases:**
  - "Reiteration" with multiple data transfers during a single time step
- **Support variety of simulation types**
  - Discrete Event
  - Time Series
  - Quasi-Static Time Series
  - Phasor (Dynamics)
- **Usability:**
  - "Research-grade" usability
  - Standard file/folder structures and I/O
  - Execution tools/scripts
- **Documentation and Examples**
  - Templates for common use cases

## Nice to haves

- Check-pointing
- Roll-back
- Load balancing / tracking of location of federates
- Built-in scenario / sensitivity analysis
- Visualization
  - We will need something specific for our use cases, but not a “turn key” solution for all use cases
- Interface / Model Creation / Model Ingestion
  - Keep relatively simple for now, but it can be expanded later
  - Assume the individual tools in standard format for now (no uber or base modeling language)
  - Prototype one example (e.g., CYME->GLD)
- Data Collection and Analysis
  - Standardize / inform on a common data model / API for common classes of analysis

# Overview

## Key questions

### Broker vs. Broker-less vs Hierarchical Broker

- Single broker is useful because it represents single IP address.
  - This mirrors HLA portico approach
- Single broker will not scale
- Hierarchical broker may scale better but may put burden on modelers.

### Location/structure of federate configuration data

- One large definition
- Individual files

### Supported data exchange patterns

#### Time coordination

- Syncing
- Reiteration
- Advancement

### What features should we borrow from

- HLA
- FMI
- FNCS

## Over-arching Design Decisions

### Static Federate Connections

TDC-Tool will assume all federate connections are static, that is known in advance.

_Note: dynamic additions of federates at runtimes can be emulated by predefining connections and only exchanging data at a later time._

# Layers

The following layers will be defined in the TDC co-simulation tool.

![](./img/layers.png)

For each layer, the following are described

- API
- Testing Specification

## User Interface

The complexity of co-simulation drives us to the need for applications that support the user in creating models, configuring the co-simulation engine and simulator connections, automate the deployment of the simulators to the appropriate computers (whether local, hpc, or cloud), and process and visualize the information produced.

This project does not have the resources to tackle all elements of this process, but may address a small subset of them as a function of producing results. This section will lay out an ideal state, while suggesting a path within the resources of the current project.

The following figure provides a block diagram of the elements needed to support the deployment of the TDC tool, and a workflow for how they might all fit together. The following sections will describe each of the purple and gray elements in more detail.

![](./img/workflow.png)

### Scenario Generation

Generating scenarios, and the underlying models to support the scenarios, is fundamental to using the co-simulation environment. However, describing those scenarios can be time consuming and error prone. For example, a GridLAB-D model may contain a million lines of model code, but one errant setting could drastically change the solution. Now, multiply this by 10,000 distribution circuits, add transmission and market simulators, and layer communication networks over the top, and the likelihood of ill-configured simulations grows. At this larger scale, simply managing the correspondingly large set of files and initiating the runs for each simulator requires automation. Automation of scenario generation is necessary to maintain model continuity and ensure proper alignment across domains.

There are range of existing example tool sets that we may be able to draw from:

- The Open Modeling Framework (OMF) by NRECA, uses a Python-based tool to construct a GridLAB-D model from user inputs and a pre-defined power flow model. More information can be found on the GridLAB-D website: [OMF Documentation](http://gridlab-d.sourceforge.net/wiki/index.php/OMF_Scripting_Documentation). However, the OMF is currently limited to distribution system models and GridLAB-D.
- [Arion](https://github.com/pnnl/arion) is an open-source, Java-based tool that allows the user to construct models from a library of objects, providing randomization and other functions for easier construction. It currently supports limited elements of ns-3, GridLAB-D, FNCS, and MATPOWER. It also creates a [Heat Orchestration Map](http://docs.openstack.org/developer/heat/template_guide/hot_spec.html) on an OpenStack environment that configures the necessary simulators, network parameters, and virtual machines to orchestrate an experiment. At this stage, the technology is available on GitHub, but is not mature.
- The IGMS project developed extensive scenario generation tools including an evolved form of the OMF GridLAB-D generator (on github as glmgen), and facilities for setting up directory and file for T+D simulations with GridLAB-D, FESTIV, MATPOWER, and PFLOW, that can be extended for other tools. These tools also support matching distribution load to transmission system buses to create integrated T+D datasets when real data is not available.

The scenario generator shall:

1. ​

Additionally, the first time a scenario or use case is created is the most difficult. To ease user entrance into the tool(s), it is of utmost value to re-use existing model sets and allow users to not only create their own models, but also modify existing, previously used models. This implies a model library or repository that users can draw from and add to.

**_Open question_: Do we use our existing tools to do this? Which ones?**

**_Open question_: How far do we define this (e.g., all the way down to the programming language)? Or do we want to define general parameters of a framework that we can then incorporate functions into later? Do we want to define a philosophy or specific software requirements?**

### Configuration

### Automation

### Visualization and Data Processing

### API

### Testing Specification

## Simulators

While the co-simulation environment will support any simulation element that meets the minimum requirements, it is envisioned many TDC applications will rely heavily on a common style of simulators. As a result we will define two classes of simulator interfaces:

1.  General purpose, including HLA and FMI interface standards.
2.  TDC optimized for common TDC application types:
    - Transmission Simulator
    - Distribution Simulator
    - Communication Simulator
    - Market Simulator

The lower-level details of these interfaces are defined in the "Application" Section below; however, this section provides two key extensions: standardized data exchange patterns (variable naming, types, timing/synchronization, etc.) and a higher-level API for certain common operations.

### Common Configuration

Note that as we are modeling multiple layers of cyber-physical power systems, there is a natural split between simulation coordination (e.g. synchronizing simulation data), and communication-system data. In some cases the same datapoint will be represented in both layers, though with subtle differences. For instance the voltage at a particular node may be exchanged for both simulation coordination and via the communication system. The simulation coordination physical data should be exchanged as accurately and fast as possible, likely with re-iteration since it represents a physical link. In contrast the same data seen in the communication system would be exchanged only once (no reiteration), likely with sensor delay and possibly sensor noise introduced.

As such the configurations here can in some cases be mixed and matched, for instance to combine some type of simulation with various types of communication situations. As such, there are multiple partial configurations described for each group, each with a unique identifier. For a given simulation, the complete configuration could then be described using a combination of idenfiers, such as P1-C1-M2 for an integrated transmission-distribution quasi-steady state simulation with a XXX market, where the communication of the market signals is represented as a delay.

Note: in each group, a \* is used to indicate the configurations (proposed) to be built out first. These are defined in more detail below.

#### Multiple Power-flows

Many configurations: T+D, T+D, D+D, resulting in many combinations First distinguished as **single-pass vs reiterative**, and further distinguished by the type of power flow interface presented:

- 3-ph unbalanced
- positive sequence
- 3-sequence

For each, the raw power signals will typically be exchanged using 3-ph representation, but the higher-level APIs provide mechanisms to manage positive sequence and 3-sequence to 3-ph unbalanced conversion.

**\*Config P0—Single Power flow:** The null power flow case with only a single simulator. This could be for T+C, D+C, or either with building or external controllers.

**\*Config P1—T_ps+multi-D_3ph QSTS:** Perhaps the most common is expected to be steady-state transmission with multiple distribution federates for quasi-steady-state time series (QSTS) analysis. In this case, a single transmission simulator typically provides positive sequence powerflow (e.g. MatPower, PSSE, etc.) that interacts with with multiple 3-ph unbalanced distribution simulation and nominally utilizes reiteration, though single-pass interactions also acceptable. The single pass configuration can be clarified using an "A" suffix (e.g. P1A), while reiteration is fully "P1B".

**\*Config P2—T_ps+multi-D_3ph Dynamic:** For dynamic-scale (ms time step), transient stability simulations, we have a very similar interface to QSTS, but primarily differentiated by the need to provide richer dynamic phasor and frequency data for the interface. Ideally both federates support this data with higher fidelity models, though in some cases only transmission will have full dynamics, while distribution continues to use QSTS. Here reiteration is effectively required, though an "A" suffix can be appended in the case where no-iteration is used.

**Config P3—T_3sq+multi-D_3ph QSTS\[^1\]:** This use case builds on recent work by Q. Huang and Vittal \[^2\] that represents the transmission dynamics using separate simulations for each of the 3-sequences and then converts to 3-phase for the distribution simulators. As above, reiteration is effectively required and an "A" should be appended if no reiteration used.

**Config P4—T_3sq+multi-D_3ph Dynamic\[^3\]:** This use case builds on recent work by Q. Huang and Vittal \[^2\] that represents the transmission dynamics using separate simulations for each of the 3-sequences and then converts to 3-phase for the distribution simulators. As above, reiteration is effectively required and an "A" should be appended if no reiteration used.

**Config P5—T_3ph+multi-D_3ph QSTS:** This configuration extends P1 to use full 3-phase simulation for the transmission system. As above the the suffix "A" indicates single pass and "B" indicates reiteration.

**Config P6—T_3ph+multi-D_3ph Dynamic:** This configuration extends P2 to use full 3-phase simulation for the transmission system. As above, reiteration is effectively required and an "A" should be appended if no reiteration used.

\[^1\]: QSTS version inserted for v0.4.0 May 12, 2017 (Previously P3 referred to the same setup for dynamics. The new ordering allows consistency by alternating QSTS and Dynamics
\[^2\]: Huang, Q. & V. Vittal. "Integrated Transmission and Distribution System Power Flow and Dynamic Simulation Using Mixed Three-Sequence/Three-Phase Modeling" IEEE Transactions in Power Systems, 2016
\[^3\]: This was called "P3" in version 0.3.0 dated 2/11/2017

- #### Power Systems (Physical) Data to/from Communication (Cyber) Exchange

  There are a wide range of communication links relevant for TDC use cases. Including:

  - SCADA Data Exchange
  - Synchrophasors
  - AGC Dispatch
  - DER control/DERMS signaling (e.g. OpenADR, SEP2.0, etc.)
  - Emerging enterprise distributed protocols (e.g. OpenFMB)
  - Research-grade distributed communication and control schemes

  In general all of these use cases share a common style where selected points from the physical, powerflow simulation are presented as raw data and/or control signals to electrical equipment are passed to/from another federate.

  Current thinking is that physical domain simulators would always present only higher level data (e.g. 3ph voltage at a node) to the co-simulator. Then an additional federate would handle protocol-specific packetization when required. This allows the domain simulators to maintain a constant interface independent of the type of communication simulation implemented.

  This opens up three levels of communication simulation that might be used:

  **\*Config C0—Direct Communication/Control Data:** This is effectively the null case with no communication system simulation, but where data availability is captured. IMPORTANT: this only applies to data for the communication system under test. That is data collected from sensors or controlled remotely. It is distinct from the simulation-coordination data exchange (e.g. T-D power flow data exchange) at the physical level.

  **\*Config C1—Delay Only:** In this case, the same direct data is subject to a simulated delay before delivery.

  _Question: Should the co-simulation framework provide this delay service? Or do we build a simple federate that handles the delays, likely translating from one subscribed topic name and publishing on a different one._

  **\*Config C2—Packetized Network Simulation:** This is full network simulation including packetization and detailed multi-OSI-level stack simulation as provided by ns-3 and similar tools. _Note: stared instead of simple delays for political reasons… seems we really need to have ns-3 in our next demo to bolster the "C" in TDC_

- #### Market Simulation

  There are a wide range of market scenarios that might be captured with the TDC tool including:

  **\*Config M1A—Wholesale LMPs**: Multi-period (e.g. day ahead unit commitment, intra-daily unit commitment, real time dispatch) markets with support for price forecasts and price data (e.g. LMPs) passed to distribution

  **Config M1B—Wholesale LMPs with demand bids:** Extends M1 to include demand bidding

  **Config M1C—Wholesale LMP with generator federates:** extends M1 to include separate generator federates who bid through the co-simulator to a wholesale market

  **Config M2x—Distribution Markets:** Rich Distribution markets, typically computing DLMPs (or LMP+D) as separate markets below wholesale markets. Configuration modifier A, B, C same as for M1x

  **Config M3—Transactive Energy:** Full transactive energy, where there could be multiple layers of interactively bidding markets, including aggregators.

- #### External Models

  In addition to the core TDCM federates, and generic HLA and FMI interfaces, it seems there is value in having templates/interfaces for a few common external model types.

  - User provided load model
  - User provided controller
  - Detailed communication server

  _Note: In writing short codes for the configuration, these are appended with the `+` connecting character._

  **Config +UL—User-provided load model**

  **\*Config +UC—User-provided Control System:** In this configuration, the user provides their own control system(s) which could be some form of energy management system (EMS, DMS, BMS, HEMS or the like) or research controllers.

  **Config +US—User provided communication server model:** This configuration targets cybersecurity use cases, where the user may add a detailed communication server representation that includes additional cybersecurity monitoring or other protections.

### Standard Data Schemas

#### Conventions

This section defines a standard naming convention for valueNames, and communication federate source/destinations.

_Open Issue_ : Discuss naming convention of hierarchy. Currently implemented in this document as "/".

**Naming convention:**

Power flow:

\[federate_id/\]transmission_bus0..x\[/feeder0…x\]\[/node0…x\]/value

Here the convention is to match hierarchy of the power systems going as deep as appropriate and ending with the specific value desired. The initial

For consistency, values should be taken from the following list:

- voltage—multiphase array of complex values, default units = V (volts)
- current—multiphase array of complex values, default units = A (amps)
- power—multiphase array of complex values, default units = W (watts)
- lmp—double, default units (\$/MWh)

For each level in the hierarchy, the user can pick names to match the data. If generic names (e.g. "bus01") are used, the numbered suffix should be zero pad the number to match the maximum expected order of magnitude.

**Capitalization:** all lower case with underscores as needed
**Units:** By default, base SI units are assumed, unless otherwise specified, with support for unit conversion for common units abbreviations.

For example, voltage is nominally presented and passed as volts (V), but can be specified in kV or MV.

#### Config P1A-M1A+UC

_Note: this is roughly the main setup for IGMS_

| Publisher    | Topic                     | value                                                                                                     | Comment                                                                                                              |
| ------------ | ------------------------- | --------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| transmission | bus00x/voltage            | positive sequence, complex voltage                                                                        |                                                                                                                      |
| distribution | bus00x/feeder00x/power    | 3phase complex                                                                                            |                                                                                                                      |
| market       | bus00x/da_lmp             | array of hourly Locational Marginal Prices (LMPs) from the day ahead unit commitment                      | time interval may changed per simulation for all market values                                                       |
| market       | bus00x/ha_lmp             | array of 15minute LMPs for the next 4 hours from the hour ahead unit commitment                           |                                                                                                                      |
| market       | bus00x/rt_lmp             | Current, 5minute LMPs from real time economic dispatch                                                    |                                                                                                                      |
| market       | gen00x/da_dispatch        | array of hourly dispatch levels (as complex P) from the day ahead unit commitment                         | Note: pass imaginary dispatch of zero when working with pure real power. complex power included for future ACOPF use |
| market       | gen00x/ha_dispatch        | array of 15minute dispatch levels (as complex P) for the next 4 hours from the hour ahead unit commitment |                                                                                                                      |
| market       | gen00x/rt_dispatch        | Current, 5minute dispatch levels (as complex P) from real time economic dispatch                          |                                                                                                                      |
| transmission | bus00x/gen0x/power_actual | Actual power output from generator. Total, complex                                                        |                                                                                                                      |
| agc          | bus00x/gen0x/agc          | Percent of 5minute dispatch level                                                                         | Based on an off-hand comment, I am under the impression that AGC signals may be sent this way… not confirmed.        |

#### Additional Specified Configurations

_Additional input required: Target of 3-5 total._

### Timing/Synchronization Conventions

_The intent of this section is to capture the order of data I/O during standard configurations. Crux could be agreeing on the order/master for reiteration._

### Higher-level Simulator-Type-Specific API

This Simulator-Type-Specific API builds on top of the raw application API described below to provide standardized convenience functions for common use cases.

**Power Flow Helpers**

- pfPublish3ph(valueName, value_array, input_format='3ph')—Helper function for publishing voltage and current values. The function accepts any input format from {'3ph(ase)', '3seq(uence)', 'pos(itive sequence)', 'sum'} and automatically converts the values to three phase representation. The 'sum' type is particularly useful in the Get3ph function to combine the total (complex) power across all 3 phases.
- pfGet3ph(valueName, output_format='3ph')—Complementary helper function to receive 3-phase values and internally convert to other representations

#### Additional Simulator-Types

### Testing Specification

Dummy simulators for each implemented configuration should be made available as the interface, API, and schema's evolve. These can be tested/verified (ideally with continuous integration checks) using standard configuration combinations. In addition any configurations not included in these combinations can be tested individually with appropriate test player/tracer support.

Initial test configuration combinations:

- P1B: T+D QSTS with reiteration
- P1A-M1A: T+D QSTS, no reiteration, with wholesale LMP passed to D
- P2: T+D dynamics
- P2-C2: T+D dynamics with packetized cyber data exchange. Use case: Wide-Area control with synchrophasor exchange

## Application

The general application API will act as a basic interface for applications to translate between the programs and the Core API. It is anticipated application specific API's will be build on top of these basic API's to provide additional functionality.

- _The API will provide functionality for_
  - Federate or Object Management
  - Event Communication
  - Data Transfer
  - Scheduler or Time Synchronization

This discussion is open for debate. Application federates can be described in a number of ways and I think it makes a difference in how the federate is handled by the broker and API, and so can the data being passed back and forth. I propose 3 distinct interfaces for the different types of data transfer, An individual federate may use one or more of the interfaces. A single federate may create multiple interface objects of the same or different types

all types of interfaces will have some properties

- oberver [yes/no] means the federate does not publish or transmit any data or message (observe only)
- timing -- the allowable times at which the federate interacts
  - periodic -only interact at specific time windows requires specification of the minimum time interval
  - arbitrary - can interact at any time interval up to cosim resolution (ns?)
- rollback [yes/no]

### API

The API code itself should have version in C, C++, python, and matlab at a minimum. The API code will be written in C++, with a shared library layer using only C constructs. An application could use the C shared library, or directly include the C++ code in the application.

#### Value based API

This API is used by federates to pass values directly, it can be iterative, And is meant to emulate a direct physical coupling, though other uses are possible.

##### Federate Management

- register(simConfiguration) --simConfiguration should be structure describing the simulator properties and capabilities, this should be able to be an object or file name containing the description in some format.
  Borrowing ideas from FMI the objects are in a couple different states (startup, initialization, continuous, event, error, terminate). I would propose reducing this to 4 (startup, initialization, operation, finalize), may be want the error state as well. Certain functions then only work in certain modes
- setMode(mode) -- change the mode of operation, probably should be a blocking function that only returns once the mode has been successfully changed.
  In the startup mode a federate should register its publish and subscribe communication values either by functions or by file
- registerPublication(valueName,NAMESPACE, type, defaultValue\*, units\*,) – type is one of (numeric, complex, array, string, raw), units are optional and could be used as part of the type checking if the units are not translatable from one to another.
  NAMESPACE is a flag either GLOBAL or LOCAL publications must be unique in the given namespace, GLOBAL is across the entire cosimulation, LOCAL is within the federate.
- registerSubscription(valueName, type, defaultValue\*,units\*) –type is one of (numeric, complex, array, string, raw)
- registerInterfaces(filename) -- load a file describing the publications and subscriptions This should be done in the startup and should be type and name checked by the core before moving to the initialization mode.
- terminate() -- the result should depend on the type of simulation, an observation only federate should have no impact on the overall simulation.
- error() -- the federate encountered an error and cannot continue

##### Value Exchange Interface

everything should be type checked for matching with the defined values above) These functions should be available in initialization and operation mode, getValue should also be available in the finalize(or error) state as well

- getValue(valueName) -- a set of functions for querying values of the different types
- publish(valueName, value) – a set of functions for publishing values
- queryUpdate(valueName) –query if a value has been updated since the last getValue Call
  What I am a little unclear about is the value of historical data to applications. I would propose any buffering for these types of values be done in an higher level API built on this one.

##### time synchronization

Only available in operation mode (currently based on FNCS model)

- allowedTime=requestTime(nextInternalTimeStep)

a different option might be to have the time advancement be callback based in which case the API would be responsible for calling the federate function call to advance in time when appropriate.

#### Packet Based API

I propose a separate interface for communication messages
Basically data packets. What I would ideally want is a system that routed specific data packets through a comm system model if one were present and just delivered them if one wasn’t. The applications shouldn’t care what if any communication system model was present or not. This means that the power system model should not have to be aware of the communication system model and directly send data to and from it. It should be automatically routed through the appropriate communication system. This implies that the setup for a communication system will have to declare which federates it links with. Some mechanics of cross comm system linking will also need to be worked out but that is at a lower level. These functions should only work in operation mode. These packets will need to be buffered.

##### Federate Management

- register(simConfiguration) --simConfiguration should be structure describing the simulator properties and capabilities, this should be able to be an object or file name containing the description in some format.
  Borrowing ideas from FMI the objects are in a couple different states (startup, initialization, continuous, event, error, terminate). I would propose reducing this to 4 (startup, initialization, operation, finalize), may be want the error state as well. Certain functions then only work in certain modes
- setMode(mode) -- change the mode of operation, probably should be a blocking function that only returns once the mode has been successfully changed.
  In the startup mode a federate should register its publish and subscribe communication values either by functions or by file

- RegisterEndPoint(name, NAMESPACE, userType) --there should be able to be a large number of comm points in a single federate.
  NAMESPACE is a flag either GLOBAL or LOCAL Endpoints must be unique in the given namespace, GLOBAL is across the entire cosimulation, LOCAL is within the federate. userType is a user defined string nameing the type of packets that this endpoint generates or receives it should be optional and meant to aid in type checking, if included additional error checking should be included.
  -RegisterTargetDestination(name, userTYpe) -- function to give guidance to the Core about communication paths and setup, also to aid in validity checking.

- terminate() -- the result should depend on the type of simulation, an observation only federate should have no impact on the overall simulation.
- error() -- the federate encountered an error and cannot continue

##### message exchange interface

- Transmit(source, destination, data, datalength) -destination could be inside the same or different federate.
- packetCount(destination) --return the number of packets available.
- Receive(destination) --get one of the packets available or block until one is available. the source should be part of the return information --destination should be able to be grouped (like a subnet) so a single receive function could be called for many destinations if there is internal routing.

We may want to allow the ability to specify a callback here to be called on packet arrival?

#### Packet Filter API

The means by which a comm simulation interacts with the other types of simulations is not clear cut, there are timing issues and delays and packet translation an other issues which get awkward if they are not designed in. In the cosimulation framework there is a need for simulating direct physical connections with real values passed back and forth, and those same simulations interact with the digital world translating between physical phenomenon and digital communications, and still others types of federates that interact purely in the digital communication world. This separations of purpose lead to the concept of a separation of functionality to better tune the interface to correspond to the physical world. So In addition to the value based interface and a packet based interface we add a third interface intended to operate on packets, for translating, manipulating, or delaying them.

##### Federate Management

- register(simConfiguration) --simConfiguration should be structure describing the simulator properties and capabilities, this should be able to be an object or file name containing the description in some format.
  Borrowing ideas from FMI the objects are in a couple different states (startup, initialization, continuous, event, error, terminate). I would propose reducing this to 4 (startup, initialization, operation, finalize), may be want the error state as well. Certain functions then only work in certain modes
- setMode(mode) -- change the mode of operation, probably should be a blocking function that only returns once the mode has been successfully changed.
  In the startup mode a federate should register its publish and subscribe communication values either by functions or by file

- RegisterSrcFilter(srcEndpoint, srcUserType) SrcEndpoint is a named src endpoint to filter packats from, it can be from a packet generation API or packate pfilter API
  -RegisterFilterOutput(outputName, NAMESPACE, outputType)
  outputName is the endpoint packets reappear from, it should be unique in the federate but multiple srcEndpints registrations can output from the same outputName endpoint. srcUserType must match the definition from the src endpoint, outputUserType defines what the output type is. THis function defines an endpoint which processed packets may exit. The name may be the same as one defined by a message exchange interface

NAMESPACE is a flag either GLOBAL or LOCAL Endpoints must be unique in the given namespace, GLOBAL is across the entire cosimulation, LOCAL is within the federate. userType is a user defined string nameing the type of packets that this endpoint generates or receives it should be optional and meant to aid in type checking, if included additional error checking should be included.

- RegisterDestFilter(destEndpoint, inputType) destEndpoint is a named dest endpoint to filter packats before sending to the destination, it can be from a packet generation API or packate filter API
- terminate() -- the result should depend on the type of simulation, an observation only federate should have no impact on the overall simulation.
- error() -- the federate encountered an error and cannot continue

##### message exchange interface

- Transmit(source, origSource, destination, data, datalength,delay\*) -destination could be inside the same or different federate. optional delay
- packetCount(destination) --return the number of packets available.
- Receive(destination) --get one of the packets available or block until one is available. the source should be part of the return information --destination should be able to be grouped (like a subnet) so a single receive function could be called for many destinations if there is internal routing.

We probably want to allow the ability to specify a callback here to be called on packet arrival? particularly for packat processors that do not depend on time, and are simply functions.

### Testing Specification

We should develop a set of semi-configurable mini-federates that test all the possible combinations of federates with an assortment of communication patterns with dummy data and exercise the API fully.
The same set could be used for testing the broker functionality.

## Core

The core represents a distributed key/value store. The specification does not dictate the on-the-wire protocol for the distribution of values. The specification does not dictate whether zero or more brokers are used.

Initialization occurs by parsing a configuration file or using the step-by-step API. The return from initiatlize() or exitInitializationMode() implies that all initial values have been exchanged. It is valid to then request a time advance of zero or more time steps.

Time advances are allowed to be greater than or equal to the current time. If the same time is requested more than once, an internal counter logs the number of times the same time has been requested. In other words, internally a pair of values is incremented -- (time,counter) -- where the caller only is exposed to the time value. Every instance where time is actually incremented, the counter is reset to zero.

### API

```cpp
#ifndef _GMLCTDC_CORE_
#define _GMLCTDC_CORE_

#include <string>
#include <utility>
#include <vector>

using ::std::pair;
using ::std::string;
using ::std::vector;

namespace gmlctdc {

    typedef unsigned long long time;
    typedef double real;
    typedef long integer;
    typedef int boolean;

    void initialize();
    void initialize(const string &filename);
    void enterInitializationMode();
    void setName(const string &name);
    void setTimeDelta(const string &value_and_unit);
    void setTimeDelta(const time &nanoseconds);
    void setTimeDelta(const time &base, const time &multiplier);
    void registerSubscription(const string &key, const string &ns, const string &type, const string &units);
    void registerPublication(const string &key, const string &ns, const string &type, const string &units);
    void exitInitializationMode();

    bool isInitialized();
    string getName();
    vector< pair<string,string> > getSubscriptions();
    int getID();
    int getFederationSize();

    real getReal(const string &key, const string &ns);
    vector<real> getRealVector(const string &key, const string &ns);
    integer getInteger(const string &key, const string &ns);
    vector<integer> getIntegerVector(const string &key, const string &ns);
    boolean getBoolean(const string &key, const string &ns);
    vector<boolean> getBooleanVector(const string &key, const string &ns);

    void setReal(const string &key, const string &ns, const real &value);
    void setRealVector(const string &key, const string &ns, const vector<real> &value);
    void setInteger(const string &key, const string &ns, const integer &value);
    void setIntegerVector(const string &key, const string &ns, const vector<integer> &value);
    void setBoolean(const string &key, const string &ns, const boolean &value);
    void setBooleanVector(const string &key, const string &ns, const vector<boolean> &value);

    time timeRequest(time next);

    void die();
    void finalize();

    vector< pair<string,string> > getEvents();
}

#endif /* _GMLCTDC_CORE_ */
```

### Testing Specification

## Platform

A goal of the co-simulation environment is to be portable across a
wide range of computing environments ranging from desktop, cloud services,
and supercomputers.

OS specific APIs are being avoid and additional library requirements
are being kept to a minimum to enable easier deployment.

### Operating Systems

Operating systems to be supported are Unix (Linux), Windows, and
MacOS. It is not expected that there will be large numbers of
operating specific requirements.

Operating system specific features that are required will supported by
introducing small wrapper classes and methods. OS specific versions
of these will be supplied for the supported operating systems.

### Transport Layer

The targeted range of platforms have different communication transport
layers that are optimized for the platform. Supercomputers
traditionally have specialized communication fabrics such as Myrinet
while workstations traditional do not. Due to these differences the
TDC tool will support multiple communications layers. The two initial
targets are Message Passing Interface (MPI) and ZeroMQ. MPI is the
most commonly used standard and portable message-passing system
designed for parallel computing architectures. ZeroMQ is a an open
source distributed message engine portable to a wide verity of
systems. ZeroMQ is the target for the non-supercomputer
architectures.

The TD&C tool will be architected with a internal transport layer API
to serve as the interface to the specific communications layer. An
MPI and ZeroMQ implementation will be supplied but other communication
systems could be added if needed.

#### MPI

The MPI-3 specification will be targeted excluding dynamic process
management features. The batch scheduling systems deployed at most
supercomputer centers does not support dynamic process management.
The TD&C tool is initially is targeting static federate
configurations.

#### ZeroMQ

ZeroMQ version 4.2.0 or greater is being targeted.

### Languages and Compilers

#### C++

The TD&C tool will principally be written in C++ for speed and memory efficiency.

The C++14 standard as supported by gcc 4.9.3 is the C++ language
target. Later standards are not available on the large scale HPC
machines being targeted.

Specific compilers that are being targeted are:

- Visual Studio 2015
- GNU GCC 4.9.3
- Clang 3.5
- Intel 16

#### Python

Python bindings can be supported for both Python2 and Python3. The bindings will at a minimum support Python >=2.7.x. Python >=3.5 is expected to be supported. Cython will be used to provide C/CPP to Python bindings, since that will allow better error reporting.

**Examples**

1. Player example
2. Recorder example

#### MATLAB

MATLAB bindings will be provided using the mex functionality.

**Examples**

1. Player example
2. Recorder example

### Custom plugin API

Documentation will list the minimum necessary functions that will have to be implemented to build a custom plugin.

### External Package Dependencies

The Question of using Boost is still up for debate.

_Open Issue_ : Steve's comments on Boost - Steve has used Boost virtually all C++ projects lately. There is a lot provided that is still missing in STL. The team might want to restrict to a specific set of packages in boost.

_Open Issue_ : I/O libraries

### Configuration, Build, and Test

CMake is the targeted build/configuration system. CMake is a cross
platform set of tools that support the targeted operating systems and
compilers. It supports building standard Unix Makefile as well as
common IDE systems such as Visual Studio and XCode. CMake 3.4 or
greater will be required.

In addition to configuration and build support, CMake provides a test
framework which will be utilized to drive an automated set of
regression tests.

_Open Issue_ : Should CTest be used? Since we are using CMake CTest is already there.

## Source Code Control System and Development Environment

Github will be employed as the SCCS system as well as issue tracking.
This will enable easy interacting with the distributed team and
provides a smooth transition path for enabling access to a wider
community. A continuous intergratiion server like TravisCI will be implemented for testing.

### API

### Testing Specification
