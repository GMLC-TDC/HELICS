# Tools with HELICS Support

The following list of tools is a list of tools that have worked with HELICS at some level either on currenct projects or in the past.

## Power systems Tools

### Open Source Distribution system simulation

-   [GridLAB-D](https://www.gridlabd.org/), an open-source tool for distribution power-flow, DER models, basic house thermal and end-use load models, and more. HELICS support currently (8/15/2018) provided in the [`develop` branch](https://github.com/gridlab-d/gridlab-d/tree/develop) which you have to build yourself as described [here](https://github.com/GMLC-TDC/HELICS-Tutorial/tree/master/setup).

### Open Source Transimission system simulation 
-   [GridDyn](https://github.com/LLNL/GridDyn), an open-source transmission power flow and dynamics simulator. HELICS support currently (4/14/2018) provided through the [`cmake_updates` branch](https://github.com/LLNL/GridDyn/tree/cmake_update).
-   [PSST](https://github.com/kdheepak/psst), an open-source python-based unit-commitment and dispatch market simulator. HELICS examples are included in the  [HELICS-Tutorial](https://github.com/GMLC-TDC/HELICS-Tutorial)
-   [MATPOWER](http://www.pserc.cornell.edu/matpower/), an open-source Matlab based power flow and optimal power flow tool. HELICS support currently (4/14/2018) under development.
-   [InterPSS](http://www.interpss.org/), a Java-based power systems simulator. HELICS support currently (4/14/2018) under development.

### Commercial tools 
-   [Cyme](http://www.cyme.com/software/cymdist/) 
-   [PSLF](https://github.com/GMLC-TDC/PSLF-wrapper) Some Level of PSLF support is available using the experimental python interface.
-   [PSS/e](https://new.siemens.com/global/en/products/energy/services/transmission-distribution-smart-grid/consulting-and-planning/pss-software/pss-e.html)
-   [Power World](https://www.powerworld.com/) PowerWorld Simulator is an interactive power system simulation package designed to simulate high voltage power system operation on a time frame ranging from several minutes to several days.


## Communication Tools

-   [ns-3](https://www.nsnam.org/), a discrete-event communication network simulator. Supported via the [HELICS ns-3 module](https://github.com/GMLC-TDC/helics-ns3)
-   HELICS also includes built-in support for simple communications manipulations such as delays, lossy channels, etc. through its built-in filters.
-  [OMNet++](https://omnetpp.org/)  OMNeT++ is a public-source, component-based, modular and open-architecture simulation environment with strong GUI support and an embeddable simulation kernel. Its primary application area is the simulation of communication networks, but it has been successfully used in other areas like the simulation of IT systems, queueing networks, hardware architectures and business processes as well.
  Early stage development with OMNET++ and HELICS is underway

## Gas Pipeline Modeling

- [NGFAST](http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.172.1169)
- [GasModels.jl](https://github.com/lanl-ansi/GasModels.jl)

## Optimization packages

- [GAMS]()
- []

