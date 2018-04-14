# HELICS-SRC [![Build Status](https://travis-ci.org/GMLC-TDC/HELICS-src.svg?branch=master)](https://travis-ci.org/GMLC-TDC/HELICS-src)
[![Build status](https://ci.appveyor.com/api/projects/status/afpa4mv0kgsjwvtn/branch/develop?svg=true)](https://ci.appveyor.com/project/nightlark/helics-src/branch/develop)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/83ba19b36b714c729ec3a3d18504505e)](https://www.codacy.com/app/phlptp/HELICS-src?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=GMLC-TDC/HELICS-src&amp;utm_campaign=Badge_Grade)
[![Gitter chat](https://badges.gitter.im/GMLC-TDC/HELICS-src.png)](https://gitter.im/GMLC-TDC/HELICS-src)
[![Documentation](https://img.shields.io/badge/docs-ready-blue.svg)](http://gmlc-tdc.github.io/HELICS-src)

<p align="center">
<img src="docs/img/HELICS_Logo.png" width="400">
</p>

Welcome to the repository for the Hierarchical Engine for Large-scale Infrastructure Co-Simulation (HELICS).  HELICS provides a general-purpose, modular, highly-scalable co-simulation framework that runs cross-platform (Linux, Windows, and Mac OSX). It provides users a high-performance way to have multiple individual simulation model "federates" from various domains interact during execution to create a larger co-simulation "federation" able to capture rich interactions. Written in modern C++ (C++ 2014), HELICS provides includes a rich set of APIs for other languages including Python, C, Java, and MATLAB, and has native support within a growing number of energy simulation tools.

**Brief History:** HELICS began as the core software development of the Grid Modernization Laboratory Consortium ([GMLC](https://gridmod.labworks.org/)) project  on integrated Transmission-Distribution-Communication simulation (TDC, GMLC project 1.4.15) supported by the U.S. Department of Energy's Offices of Electricity Delivery and Energy Reliability ([OE](https://www.energy.gov/oe/office-electricity-delivery-and-energy-reliability)) and Energy Efficiency and Renewable Energy ([EERE](https://www.energy.gov/eere/office-energy-efficiency-renewable-energy)). As such, it's first use cases center around modern electric power system, though it can be used for co-simulation in other domains. HELICS's  layered, high-performance, co-simulation framework builds on the collective experience of multiple national labs.

**Motivation:** Energy systems and their associated information and communication technology systems are becoming increasingly intertwined. As a result, effectively designing, analyzing, and implementing modern energy systems increasingly relies on advanced modeling that simultaneously captures both the cyber and physical domains in combined simulations.  It is designed to increase scalability and portability in modeling advanced features of highly integrated power system and cyber-physical energy systems.

# Install Instructions

[Windows](https://gmlc-tdc.github.io/HELICS-src/installation/windows.html)

[Mac](https://gmlc-tdc.github.io/HELICS-src/installation/mac.html)

[Linux](https://gmlc-tdc.github.io/HELICS-src/installation/linux.html)

# Getting Started
We've created a series of roughly 10-minute mini-tutorial videos that discuss various design topics, concepts, and interfaces, including how to use the tool. They can be found on our [YouTube channel](https://github.com/GMLC-TDC/HELICS-Tutorial).   

The [Introduction to the HELICS documentation](https://gmlc-tdc.github.io/HELICS-src/introduction/index.html) steps through a series of examples that step through the basic usage and concepts of HELICS.

The [HELICS-Tutorial repository](https://github.com/GMLC-TDC/HELICS-Tutorial) provides a series of tutorials using HELICS to build a co-simulation using domain-specific external modeling tools that is built around an electric power system use case with integrated transmission-distribution-market-communication quasi-steady-state-timeseries (QSTS) simulation.

The [HELICS-Use-Cases repository](https://github.com/GMLC-TDC/HELICS-Use-Cases) includes examples for a growing range of research use cases for inspiration.


# Documentation

Our GitHub pages provides a rich set of [documentation](https://gmlc-tdc.github.io/HELICS-src/index.html) including a set of introductory [examples](https://gmlc-tdc.github.io/HELICS-src/introduction/index.html), a [developers guide](https://gmlc-tdc.github.io/HELICS-src/developer-guide/index.html), complete doxygen-auto-produced [API documentation](https://gmlc-tdc.github.io/HELICS-src/doxygen/), and more. 

Additionally, our initial requirements document can be found [here](https://github.com/GMLC-TDC/specification-doc/blob/master/src/specification.md), which describes a number of our early design considerations.

# Models, Applications, and Tools

As a co-simulation framework, HELICS is designed to bring together domain-specific modeling tools so they interact during run time. It effectively tries to build on the sholders of giants by not reinventing trusted simulation tools, but instead, mearly acts as a mediator to coordinate such interactions. HELICS's full power is only apparent when you use it to combine these domain-specific tools. 

# Publications

#### General HELICS

[1] B. Palmintier, D. Krishnamurthy, P. Top, S. Smith, J. Daily, and J. Fuller, “Design of the HELICS High-Performance Transmission-Distribution-Communication-Market Co-Simulation Framework,” in *Proc. of the 2017 Workshop on Modeling and Simulation of Cyber-Physical Energy Systems*, Pittsburgh, PA, 2017. [pre-print](https://www.nrel.gov/docs/fy17osti/67928.pdf) [published](https://ieeexplore.ieee.org/document/8064542/) 

#### Research using HELICS






