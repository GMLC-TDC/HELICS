# Tools with HELICS Support

The following list of tools is a list of tools that have worked with HELICS at some level either on current projects or in the past, or in some cases funded projects that will be working with certain tools.
These tools are in various levels of development.
Check the corresponding links for more information.

## Power Systems Tools

### Electric Distribution System Simulation

- [GridLAB-D](https://www.gridlabd.org/), an open-source tool for distribution power-flow, DER models, basic house thermal and end-use load models, and more. HELICS support currently (8/15/2018) provided in the [`develop` branch](https://github.com/gridlab-d/gridlab-d/tree/develop) which you have to build yourself as described [here](https://github.com/GMLC-TDC/HELICS-Tutorial/tree/master/setup). Or a CMake based [branch](https://github.com/GMLC-TDC/gridlab-d) maintained as part of the [GMLC-TDC organization](https://github.com/GMLC-TDC).
- [OpenDSS](https://smartgrid.epri.com/SimulationTool.aspx), an open-source tool for distribution powerflow, DER models, harmonics, and other capabilities traditionally found in commercial distribution analysis tools. There are two primary interfaces with HELICS support:
  - [OpenDSSDirect.py](https://github.com/dss-extensions/OpenDSSDirect.py) which provides a "direct" interface to interact with the OpenDSS engine enabling support for non-Windows (Linux, OSX) systems.
  - [PyDSS](https://github.com/NREL/PyDSS) which builds on OpenDSSDirect to provide enhanced advanced inverter models and significantly more robust convergence with high-penetration DER controls along with flexible support for user-defined controls and visualization.
- [CYME](http://www.cyme.com/software/cymdist/) has been used in connection with a python wrapper interface and through FMI wrapper.

### Electric Transmission System Simulation

- [GridDyn](https://github.com/LLNL/GridDyn), an open-source transmission power flow and dynamics simulator. HELICS support provided through the [`cmake_updates` branch](https://github.com/LLNL/GridDyn/tree/cmake_update).
- [PSST](https://github.com/kdheepak/psst), an open-source python-based unit-commitment and dispatch market simulator. HELICS examples are included in the [HELICS-Tutorial](https://github.com/GMLC-TDC/HELICS-Tutorial).
- [MATPOWER](http://www.pserc.cornell.edu/matpower/), an open-source Matlab based power flow and optimal power flow tool. HELICS support under development.
- [InterPSS](http://www.interpss.org/), a Java-based power systems simulator. HELICS support under development. [Use case instructions can be found here](https://gmlc-tdc.github.io/HELICS-Use-Cases/PNNL-TD-Dynamic-Load/index.html).
- [PSLF](https://github.com/GMLC-TDC/PSLF-wrapper) has some level of support using the experimental python interface.
- [PSS/E](https://new.siemens.com/global/en/products/energy/services/transmission-distribution-smart-grid/consulting-and-planning/pss-software/pss-e.html)
- [PowerWorld](https://www.powerworld.com/) Simulator is an interactive power system simulation package designed to simulate high voltage power system operation on a time frame ranging from several minutes to several days.
- [PyPower](https://pypi.org/project/PYPOWER/) does not have a standard HELICS integration but it has been used on various projects. PYPOWER is a power flow and Optimal Power Flow (OPF) solver. It is a port of MATPOWER to the Python programming language. Current features include:
  - DC and AC (Newton’s method & Fast Decoupled) power flow and
  - DC and AC optimal power flow (OPF)

### Real time simulators

- [OpalRT](https://www.opal-rt.com/hardware-in-the-loop/) A few projects are using HELICS to allow connections between Opal RT and other simulations
- [RTDS](https://www.rtds.com/) Some planning or testing for RTDS linkages to HELICS is underway and will be required for some known projects

### Electric Power Market simulation

- [FESTIV](https://github.com/NREL/FESTIV_MODEL), the Flexible Energy Scheduling Tool for Integrating Variable Generation, provides multi-timescale steady-state power system operations simulations that aims to replicate the full time spectrum of scheduling and reserve processes (multi-step commitment and dispatch plus simplified AGC) to meet energy and reliability needs of the bulk power system.
- [PLEXOS](https://energyexemplar.com/solutions/plexos/), a commercial production cost simulator. Support via OpenPLEXOS is under development.
- [MATPOWER](http://www.pserc.cornell.edu/matpower/) (described above) also includes basic optimal powerflow support.
- [PyPower](https://pypi.org/project/PYPOWER/) (described above) also includes basic AC and DC optimal powerflow solvers.

### Contingency Analysis tools

- [CAPE](https://new.siemens.com/global/en/products/energy/services/transmission-distribution-smart-grid/consulting-and-planning/pss-software/psscape.html) protection system modeling.
- [DCAT](https://www.pnnl.gov/main/publications/external/technical_reports/PNNL-26197.pdf) Dynamic contingency analysis tool.

## Communication Tools

- HELICS provides built-in support for simple communications manipulations such as delays, lossy channels, etc. through its built-in filters.
- [ns-3](https://www.nsnam.org/), a discrete-event communication network simulator. Supported via the [HELICS ns-3 module](https://github.com/GMLC-TDC/helics-ns3).
- [OMNet++](https://omnetpp.org/) is a public-source, component-based, modular and open-architecture simulation environment with strong GUI support and an embeddable simulation kernel. Its primary application area is the simulation of communication networks, but it has been successfully used in other areas like the simulation of IT systems, queueing networks, hardware architectures and business processes as well.
  Early stage development with OMNET++ and HELICS is underway and a prototype example is available in [HELICS-omnetpp](https://github.com/GMLC-TDC/helics-omnetpp).

## Gas Pipeline Modeling

- [NGFAST](http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.172.1169).
- [GasModels.jl](https://github.com/lanl-ansi/GasModels.jl).

## Optimization packages

- [GAMS](https://www.gams.com/).
- [JuMP](https://www.juliaopt.org/) support is provided through the HELICS Julia interface.

## Transportation modeling

- [BEAM](http://beam.lbl.gov/).
- [POLARIS](https://www.anl.gov/es/polaris-transportation-system-simulation-tool).

## Buildings

- [Energy Plus](https://energyplus.net/).
