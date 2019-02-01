<p align="center">
<img src="docs/img/HELICS_Logo.png" width="400">
</p>

# Build Status
<table>
  <tr>
    <td><b>Service</b></td>
    <td><b>Master</b></td>
    <td><b>Develop</b></td>
  </tr>
  <tr>
  <td>Travis CI</td>
  <td><a href="https://travis-ci.org/GMLC-TDC/HELICS-src"><img src="https://travis-ci.org/GMLC-TDC/HELICS-src.svg?branch=master" alt="Build Status" /></a></td>
  <td><a href="https://travis-ci.org/GMLC-TDC/HELICS-src"><img src="https://travis-ci.org/GMLC-TDC/HELICS-src.svg?branch=develop" alt="Build Status" /></a></td>
  </tr>
  <tr>
  <td>Appveyor</td>
  <td><a href="https://ci.appveyor.com/project/nightlark/helics-src/branch/master"><img src="https://ci.appveyor.com/api/projects/status/afpa4mv0kgsjwvtn/branch/master?svg=true" alt="Build Status" /></a></td>
  <td><a href="https://ci.appveyor.com/project/nightlark/helics-src/branch/develop"><img src="https://ci.appveyor.com/api/projects/status/afpa4mv0kgsjwvtn/branch/develop?svg=true" alt="Build Status" /></a></td>
  </tr>
  <tr>
  <td>Codacy</td>
  <td><a href="https://www.codacy.com/app/phlptp/HELICS-src?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=GMLC-TDC/HELICS-src&amp;utm_campaign=Badge_Grade;branch=master"><img src="https://api.codacy.com/project/badge/Grade/83ba19b36b714c729ec3a3d18504505e?branch=master" alt="codacy" /></a></td>
  <td><a href="https://www.codacy.com/app/phlptp/HELICS-src?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=GMLC-TDC/HELICS-src&amp;utm_campaign=Badge_Grade;branch=develop"><img src="https://api.codacy.com/project/badge/Grade/83ba19b36b714c729ec3a3d18504505e?branch=develop" alt="codacy" /></a></td>
  </tr>
  <tr>
  <td>Coverage</td>
  <td><a href="https://codecov.io/gh/GMLC-TDC/HELICS-src"><img src="https://codecov.io/gh/GMLC-TDC/HELICS-src/branch/master/graph/badge.svg" alt="codecov" /></a></td>
  <td><a href="https://codecov.io/gh/GMLC-TDC/HELICS-src/branch/develop"><img src="https://codecov.io/gh/GMLC-TDC/HELICS-src/branch/develop/graph/badge.svg" alt="codecov" /></a></td>
  </tr>
  <tr>
  <td>Gitter</td>
  <td colspan="2" align="center">
  <a href="https://gitter.im/GMLC-TDC/HELICS-src"><img src="https://badges.gitter.im/GMLC-TDC/HELICS-src.png" alt="Gitter Chat" /></a></td>
  </tr>
  <tr>
  <td>Documentation</td>
  <td colspan="2" align="center">
  <a href="http://gmlc-tdc.github.io/HELICS-src"><img src="https://img.shields.io/badge/docs-ready-blue.svg" alt="Documentation" /></a></td></tr>
</table>


# Introduction

Welcome to the repository for the Hierarchical Engine for Large-scale Infrastructure Co-Simulation (HELICS).  HELICS provides a general-purpose, modular, highly-scalable co-simulation framework that runs cross-platform (Linux, Windows, and Mac OS X). It provides users a high-performance way to have multiple individual simulation model "federates" from various domains interact during execution to create a larger co-simulation "federation" able to capture rich interactions. Written in modern C++ (C++14), HELICS provides a rich set of APIs for other languages including Python, C, Java, and MATLAB, and has native support within a growing number of energy simulation tools.

**Brief History:** HELICS began as the core software development of the Grid Modernization Laboratory Consortium ([GMLC](https://gridmod.labworks.org/)) project on integrated Transmission-Distribution-Communication simulation (TDC, GMLC project 1.4.15) supported by the U.S. Department of Energy's Offices of Electricity Delivery and Energy Reliability ([OE](https://www.energy.gov/oe/office-electricity-delivery-and-energy-reliability)) and Energy Efficiency and Renewable Energy ([EERE](https://www.energy.gov/eere/office-energy-efficiency-renewable-energy)). As such, it's first use cases center around modern electric power systems, though it can be used for co-simulation in other domains. HELICS's layered, high-performance, co-simulation framework builds on the collective experience of multiple national labs.

**Motivation:** Energy systems and their associated information and communication technology systems are becoming increasingly intertwined. As a result, effectively designing, analyzing, and implementing modern energy systems increasingly relies on advanced modeling that simultaneously captures both the cyber and physical domains in combined simulations.  It is designed to increase scalability and portability in modeling advanced features of highly integrated power system and cyber-physical energy systems.

# Installation Instructions

[Windows](https://gmlc-tdc.github.io/HELICS-src/installation/windows.html)

[Mac](https://gmlc-tdc.github.io/HELICS-src/installation/mac.html)

[Linux](https://gmlc-tdc.github.io/HELICS-src/installation/linux.html)

# Getting Started

We've created a series of roughly 10-minute mini-tutorial videos that discuss various design topics, concepts, and interfaces, including how to use the tool. They can be found on our [YouTube channel](https://www.youtube.com/channel/UCPa81c4BVXEYXt2EShTzbcg).   

The [Introduction to the HELICS documentation](https://gmlc-tdc.github.io/HELICS-src/introduction/index.html) goes through a series of examples that step through the basic usage and concepts of HELICS.

Several examples of HELICS federates and projects are located in [HELICS-Examples](https://github.com/GMLC-TDC/HELICS-Examples).  This repo provides a number of examples using the different libraries and interfaces.

The [HELICS-Tutorial repository](https://github.com/GMLC-TDC/HELICS-Tutorial) provides a series of tutorials using HELICS to build a co-simulation using domain-specific external modeling tools that is built around an electric power system use case with integrated transmission-distribution-market-communication quasi-steady-state-timeseries (QSTS) simulation.

The [HELICS-Use-Cases repository](https://github.com/GMLC-TDC/HELICS-Use-Cases) includes examples for a growing range of research use cases for inspiration.

A [Users guide](https://gmlc-tdc.github.io/HELICS-src/users_guide/index.html) for getting started

# Documentation

Our GitHub pages provides a rich set of [documentation](https://gmlc-tdc.github.io/HELICS-src/index.html) including a set of introductory [examples](https://gmlc-tdc.github.io/HELICS-src/introduction/index.html), a [developers guide](https://gmlc-tdc.github.io/HELICS-src/developer-guide/index.html), complete doxygen-auto-produced [API documentation](https://gmlc-tdc.github.io/HELICS-src/doxygen/), and more.  A few more questions and answers are available on the [Wiki](https://github.com/GMLC-TDC/HELICS-src/wiki).

Additionally, our initial requirements document can be found [here](https://github.com/GMLC-TDC/specification-doc/blob/master/src/specification.md), which describes a number of our early design considerations.

#### [CHANGELOG](CHANGELOG.md)
#### [ROADMAP](docs/ROADMAP.md)

# Tools with HELICS support

As a co-simulation framework, HELICS is designed to bring together domain-specific modeling tools so they interact during run time. It effectively tries to build on the shoulders of giants by not reinventing trusted simulation tools, but instead, merely acting as a mediator to coordinate such interactions. HELICS's full power is only apparent when you use it to combine these domain-specific tools.  

Thankfully the HELICS API is designed to be minimally invasive and make it straightforward to connect most any tool that provides either a scripting interface or access to source code. As listed below, a growing set of energy domain tools have HELICS support either natively or through an external interface. We also provide a set of helper apps for various utility and testing purposes.

We are always looking for help adding support for more tools, so please contact us if you have any additions.

#### Power systems Tools

* [GridLAB-D](https://www.gridlabd.org/), an open-source tool for distribution power-flow, DER models, basic house thermal and end-use load models, and more. HELICS support currently (8/15/2018) provided in the [`develop` branch](https://github.com/gridlab-d/gridlab-d/tree/develop) which you have to build yourself as described [here](https://github.com/GMLC-TDC/HELICS-Tutorial/tree/master/setup).
* [Griddyn](https://github.com/LLNL/GridDyn), an open-source transmission power flow and dynamics simulator. HELICS support currently (4/14/2018) provided through the [`cmake_updates` branch](https://github.com/LLNL/GridDyn/tree/cmake_update).
* [PSST](https://github.com/kdheepak/psst), an open-source python-based unit-commitment and dispatch market simulator. HELICS examples are included in the  [HELICS-Tutorial](https://github.com/GMLC-TDC/HELICS-Tutorial)
* [MATPOWER](http://www.pserc.cornell.edu/matpower/), an open-source Matlab based power flow and optimal power flow tool. HELICS support currently (4/14/2018) under development.
* [InterPSS](http://www.interpss.org/), a Java-based power systems simulator. HELICS support currently (4/14/2018) under development.

#### Communication Tools

* [ns-3](https://www.nsnam.org/), a discrete-event communication network simulator. Supported via the [HELICS ns-3 module](https://github.com/GMLC-TDC/helics-ns3)
* HELICS also includes built-in support for simple communications manipulations such as delays, lossy channels, etc. through its built-in filters.

#### HELICS helper Apps

* [HELICS runner](https://github.com/GMLC-TDC/helics-runner) provides a simple way to automate configuring, starting, and stopping HELICS co-simulations.
* [Player](https://gmlc-tdc.github.io/HELICS-src/apps/Player.html), which acts as a simple send-only federate that simply publishes a stream of timed HELICS messages from a user-defined file. HELICS Player is included in the HELICS distribution.
* [Recorder](https://gmlc-tdc.github.io/HELICS-src/apps/Recorder.html), which acts as a simple receive-only federate that prints out or saves messages from one or more subscribed streams. HELICS Recorder is included in the HELICS distribution.

# [Contributors](CONTRIBUTORS.md)

# [Contributing](CONTRIBUTING.md)  [Code of Conduct](.github/CODE_OF_CONDUCT.md)

HELICS is distributed under the terms of the BSD-3 clause license. All new
contributions must be made under this license. [LICENSE](LICENSE)

# Publications

#### General HELICS

[1] B. Palmintier, D. Krishnamurthy, P. Top, S. Smith, J. Daily, and J. Fuller, “Design of the HELICS High-Performance Transmission-Distribution-Communication-Market Co-Simulation Framework,” in *Proc. of the 2017 Workshop on Modeling and Simulation of Cyber-Physical Energy Systems*, Pittsburgh, PA, 2017. [pre-print](https://www.nrel.gov/docs/fy17osti/67928.pdf) [published](https://ieeexplore.ieee.org/document/8064542/)

SPDX-License-Identifier: BSD-3-Clause
