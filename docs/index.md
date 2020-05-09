# HELICS documentation

[![](https://badges.gitter.im/GMLC-TDC/HELICS.png)](https://gitter.im/GMLC-TDC/HELICS)
[![](https://img.shields.io/badge/docs-ready-blue.svg)](https://helics.readthedocs.io/en/latest)
[![](https://img.shields.io/conda/pn/gmlc-tdc/helics.svg)](https://anaconda.org/gmlc-tdc/helics/)
[![](https://img.shields.io/github/tag-date/GMLC-TDC/HELICS.svg)](https://github.com/GMLC-TDC/HELICS/releases)
[![](https://img.shields.io/badge/License-BSD-blue.svg)](https://github.com/GMLC-TDC/HELICS/blob/master/LICENSE)

This is the documentation for the Hierarchical Engine for Large-scale Infrastructure Co-Simulation (HELICS). HELICS is an
open-source cyber-physical-energy co-simulation framework for energy systems, with a strong tie to the electric
power system. Although, HELICS was designed to support very-large-scale (100,000+
federates) co-simulations with off-the-shelf power-system,
communication, market, and end-use tools; it has been built to provide a general-purpose, modular, highly-scalable co-simulation framework that runs cross-platform (Linux, Windows, and Mac OS X) and supports both event driven and time
series simulation. It provides users a high-performance way for multiple individual simulation model "federates" from various domains to interact during execution--exchanging data as time advances--and create a larger co-simulation "federation" able to capture rich interactions. Written in modern C++ (C++14), HELICS provides a rich set of APIs for other languages including Python, C, Java, and MATLAB, and has native support within a growing number of energy simulation tools.

Brief History: HELICS began as the core software development of the Grid Modernization Laboratory Consortium (GMLC) project on integrated Transmission-Distribution-Communication simulation (TDC, GMLC project 1.4.15) supported by the U.S. Department of Energy's Offices of Electricity Delivery and Energy Reliability (OE) and Energy Efficiency and Renewable Energy (EERE). As such, its first use cases center around modern electric power systems, though it can be used for co-simulation in other domains. HELICS's layered, high-performance, co-simulation framework builds on the collective experience of multiple national labs.

Motivation: Energy systems and their associated information and communication technology systems are becoming increasingly intertwined. As a result, effectively designing, analyzing, and implementing modern energy systems increasingly relies on advanced modeling that simultaneously captures both the cyber and physical domains in combined simulations. It is designed to increase scalability and portability in modeling advanced features of highly integrated power system and cyber-physical energy systems.

- [Gitter](https://gitter.im/GMLC-TDC/HELICS)

```eval_rst
.. toctree::
   :maxdepth: 1
   :caption: Basics

   installation/index
   introduction/index
   user-guide/index
   Tools_using_HELICS
```

```eval_rst
.. toctree::
   :maxdepth: 1
   :caption: Reference

   configuration/index
   apps/index
```

```eval_rst
.. toctree::
   :maxdepth: 1
   :caption: API Docs

   c-api-reference/index
   doxygen/index
```

```eval_rst
.. toctree::
   :maxdepth: 1
   :caption: Contributing

   developer-guide/index
   ROADMAP
```

You can find [Doxygen documentation here](doxygen/index.html).
