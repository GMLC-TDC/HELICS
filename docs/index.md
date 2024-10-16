# HELICS documentation

[![](https://badges.gitter.im/GMLC-TDC/HELICS.png)](https://gitter.im/GMLC-TDC/HELICS)
[![](https://img.shields.io/badge/docs-ready-blue.svg)](https://docs.helics.org/en/latest)
[![](https://img.shields.io/conda/pn/gmlc-tdc/helics.svg)](https://anaconda.org/gmlc-tdc/helics/)
[![](https://img.shields.io/github/tag-date/GMLC-TDC/HELICS.svg)](https://github.com/GMLC-TDC/HELICS/releases)
[![](https://img.shields.io/badge/License-BSD-blue.svg)](https://github.com/GMLC-TDC/HELICS/blob/main/LICENSE)

This is the documentation for the Hierarchical Engine for Large-scale Infrastructure Co-Simulation (HELICS). HELICS is an
open-source cyber-physical-energy co-simulation framework for energy systems, with a strong tie to the electric
power system. Although HELICS was designed to support very-large-scale (10,000,000+
federates) co-simulations with off-the-shelf power-system,
communication, market, and end-use tools; it has been built to provide a general-purpose, modular, highly-scalable co-simulation framework that runs cross-platform (Linux, Windows, and Mac OS X) and supports both event driven and time
series simulation. It provides users a high-performance way for multiple individual simulation model "federates" from various domains to interact during execution--exchanging data as time advances--and create a larger co-simulation "federation" able to capture rich interactions. Written in modern C++ (C++17), HELICS provides a rich set of APIs for other languages including Python, C, Java, and MATLAB, and has native support within a growing number of energy simulation tools.

Brief History: HELICS began as the core software development of the Grid Modernization Laboratory Consortium (GMLC) project on integrated Transmission-Distribution-Communication simulation (TDC, GMLC project 1.4.15) supported by the U.S. Department of Energy's Offices of Electricity Delivery and Energy Reliability (OE) and Energy Efficiency and Renewable Energy (EERE). As such, its first use cases centered around modern electric power systems and that domain continues to be one of the core use cases. However, HELICS has since expanded into use for many other domains such as Natural Gas, Water, Weather, and Transportation and new use cases appear frequently. HELICS's layered, high-performance, co-simulation framework builds on the collective experience of multiple national labs.

Motivation: Energy systems and their associated information and communication technology systems are becoming increasingly intertwined. As a result, effectively designing, analyzing, and implementing modern energy systems increasingly relies on advanced modeling that simultaneously captures both the cyber and physical domains in combined simulations. It is designed to increase scalability and portability in modeling advanced features of highly integrated power system and cyber-physical energy systems.

General citation for HELICS:

T. Hardy, B. Palmintier, P. Top, D. Krishnamurthy and J. Fuller, "HELICS: A Co-Simulation Framework for Scalable Multi-Domain Modeling and Analysis," in IEEE Access, doi: 10.1109/ACCESS.2024.3363615, available at [https://ieeexplore.ieee.org/document/10424422](https://ieeexplore.ieee.org/document/10424422)

```{eval-rst}
.. toctree::
   :maxdepth: 2

   quick_start/quick_start_index
   user-guide/index
   developer-guide/index
   references/index
   quick-links

```
