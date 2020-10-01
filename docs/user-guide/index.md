# User Guide



Co-simulation is a powerful analysis technique that allows simulators of different domains to interact through the course of the simulation, typically by dynamically exchanging values that define boundary conditions for other simulators. HELICS is a co-simulation platform that has been designed to allow integration of these simulators across a variety of computation platforms and languages. HELICS has been designed with power system simulation in mind ([GridLAB-D](https://github.com/gridlab-d/gridlab-d), [GridDyn](https://github.com/LLNL/GridDyn), [MATPOWER](https://github.com/GMLC-TDC/MATPOWER-wrapper), [OpenDSS](https://sourceforge.net/projects/electricdss/), [PSLF](https://github.com/GMLC-TDC/PSLF-wrapper), [InterPSS](https://github.com/InterPSS-Project/ipss-common), [FESTIV](https://www.nrel.gov/grid/festiv-model.html)) but is general enough to support a wide variety of simulators and co-simulation tasks. Support for other domains is anticipated to increase over time.

```eval_rst
.. toctree::
    :maxdepth: 1
    
    orientation
    installation/index
    fundamental_topics/fundamental_topics_index
    advanced_topics/advanced_topics_index
    configuration_options_reference
    examples/examples_index
    support

```
