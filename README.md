<p align="center">
<img src="docs/logos/helics-logo-long-primary-black.svg" width="700">
</p>

A multi-language, cross-platform library that enables different simulators to easily exchange data and stay synchronized in time. Scalable from two simulators on a laptop to 100,000+ running on supercomputers, the cloud, or a mix of these platforms.

[![](https://badges.gitter.im/GMLC-TDC/HELICS.png)](https://gitter.im/GMLC-TDC/HELICS)
[![](https://img.shields.io/badge/docs-ready-blue.svg)](https://helics.readthedocs.io/en/latest)
[![](https://img.shields.io/conda/pn/gmlc-tdc/helics.svg)](https://anaconda.org/gmlc-tdc/helics/)
[![](https://ci.appveyor.com/api/projects/status/9rnwrtelsa68k5lt/branch/develop?svg=true)](https://ci.appveyor.com/project/HELICS/helics/history)
[![Cirrus Status](https://api.cirrus-ci.com/github/GMLC-TDC/HELICS.svg)](https://cirrus-ci.com/github/GMLC-TDC/HELICS)
[![](https://codecov.io/gh/GMLC-TDC/HELICS/branch/develop/graph/badge.svg)](https://codecov.io/gh/GMLC-TDC/HELICS/branch/develop)
[![Releases](https://img.shields.io/github/tag-date/GMLC-TDC/HELICS.svg)](https://github.com/GMLC-TDC/HELICS/releases)
[![](https://img.shields.io/badge/License-BSD-blue.svg)](https://github.com/GMLC-TDC/HELICS/blob/main/LICENSE)
[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/GMLC-TDC/HELICS/main.svg)](https://results.pre-commit.ci/latest/github/GMLC-TDC/HELICS/main)

## Table of contents

- [Introduction](#introduction)
  - [Philosophy](#philosophy-of-helics)
- [Getting Started](#getting-started)
  - [Language Bindings](#language-bindings)
- [Documentation](#documentation)
  - [Changelog](#changelog)
  - [RoadMap](#roadmap)
  - [Installation](#installation)
  - [Quick links](#quick-links)
- [Tools with HELICS support](#tools-with-helics-support)
- [Contributing](#contributing)
- [Build Status](#build-status)
- [Publications](#publications)
- [In the News](#in-the-news)
- [History and Motivation](#history-and-motivation)
- [Release](#release)

## Introduction

Welcome to the repository for the Hierarchical Engine for Large-scale Infrastructure Co-Simulation (HELICS). HELICS provides a general-purpose, modular, highly-scalable co-simulation framework that runs cross-platform and has bindings for multiple languages. It is a library that enables multiple existing simulation tools (and/or instances of the same tool), known as "federates", to exchange data during runtime and stay synchronized in time such that together they act as one large simulation, or "federation". This enables bringing together simulation tools from multiple domains to form a complex software simulation without having to change the individual tools.

It is important to note that HELICS cannot in and of itself simulate anything, rather it is a framework to make it easy to bring together other existing (or novel) simulation tools to tackle problems that can't readily be solved by a single tool alone. After all "simulations are better together," and HELICS is designed to help get you there easily and quickly. HELICS has also already worked out many of the more subtle aspects of synchronizing simulations so you don't have to.

Today the core uses of HELICS are in the energy domain, where there is extensive and growing support for a wide-range of electric power system, natural gas, communications and control-schemes, transportation, buildings, and related domain tools ([Supported Tools](docs/references/Tools_using_HELICS.md)). However, it is possible to use HELICS for co-simulation in any domain; the HELICS API and language bindings make it straightforward to connect any simulation tool that provides a scripting interface or access to source code.

Previous and existing use cases have stretched across a wide range of scales in time and spatial area, from transient dynamics to long-term planning studies, and from individual appliance behavior to nation-wide simulations.

### Philosophy of HELICS

The design and development of HELICS is driven by a number of philosophical considerations that have a clear path to design decisions in the code and reflect the needs of the use cases that drive HELICS development.

- Make it as easy as possible for federates of all kinds to work together
- Federates cannot impose restrictions or requirements on other federates
- Federates should maintain control and autonomy
- The design should be layered and modular to be adaptable to a wide variety of circumstances
- Centralized control should be minimized

These design priorities directed much of the design of HELICS and supporting tools, including operation as a library vs a run time interface that requires simulations be loaded as modules into HELICS, the use of distributed timing and control, and giving federates fine grained control over the time management and operations that is independent of operations of other federates. These core philosophies support an underlying belief driving co-simulation that "Simulations are Better Together".

## Getting Started

A [User Guide](https://docs.helics.org/en/latest/) is available with some tutorial examples. We suggest starting here if you are looking for more information on HELICS, whether it is for getting started, or learning about more advanced features, the new documentation should have something for everyone (Please let us know if it doesn't via [![](https://badges.gitter.im/GMLC-TDC/HELICS.png)](https://gitter.im/GMLC-TDC/HELICS) or by [creating an issue on github](https://github.com/GMLC-TDC/HELICS/issues/new/choose)).

The [Orientation](https://docs.helics.org/en/latest/user-guide/orientation.html) goes through a series of examples that step through the basic usage and concepts of HELICS.

You can also [Try HELICS online](https://mybinder.org/v2/gh/kdheepak/openmod-2019-helics-tutorial/master?urlpath=lab/tree/notebooks/cosimulation-introduction.ipynb) without having to install any software.

Earlier we also created a series of roughly 10-minute mini-tutorial videos that discuss various design topics, concepts, and interfaces, including how to use the tool. They can be found on our [YouTube channel](https://www.youtube.com/channel/UCPa81c4BVXEYXt2EShTzbcg). These videos do not reflect recent HELICS advances but do introduce some basic concepts.

Several examples of HELICS federates and projects are located in HELICS-Examples with corresponding documentation in the [User Guide](https://docs.helics.org/en/latest/user-guide/examples/examples_index.html). This repo provides a number of examples using the different libraries and interfaces, including those used in the user guide.

The [HELICS-Tutorial repository](https://github.com/GMLC-TDC/HELICS-Tutorial) provides a series of tutorials using HELICS to build a co-simulation using domain-specific external modeling tools that is built around an electric power system use case with integrated transmission-distribution-market-communication quasi-steady-state-timeseries (QSTS) simulation.

The [HELICS-Use-Cases repository](https://github.com/GMLC-TDC/HELICS-Use-Cases) includes examples for a growing range of research use cases for inspiration.

A [Tutorial](https://github.com/GMLC-TDC/pesgm-2019-helics-tutorial) was prepared for the IEEE PES General meeting in Atlanta. The example materials are available on Binder.

The HELICS team holds office hours [every-other Thursday](https://helics.org/HELICSOfficeHours.ics); bring your questions and get help from the development team.

### Language Bindings

HELICS provides a rich set of APIs for other languages including [Python](https://github.com/GMLC-TDC/pyhelics), C, Java, Octave, [Julia](https://github.com/GMLC-TDC/HELICS.jl), and [Matlab](https://github.com/GMLC-TDC/matHELICS). [nim](https://github.com/GMLC-TDC/helics.nim) and C# APIs are available on an experimental basis, and with an active open-source community, the set of supported languages is growing all the time. See [Language bindings](https://docs.helics.org/en/latest/user-guide/installation/language.html) for additional details.

## Documentation

Our [ReadTheDocs](https://docs.helics.org/en/latest/) site provides a set of documentation including a set of introductory [examples](https://docs.helics.org/en/latest/user-guide/examples/examples_index.html), a [developers guide](https://docs.helics.org/en/latest/developer-guide/index.html), complete Doxygen generated [API documentation](https://helics.readthedocs.io/en/latest/doxygen/annotated.html), [API references for the supported languages](https://docs.helics.org/en/latest/references/api-reference/index.html#c-api-doxygen). A few more questions and answers are available on the [Wiki](https://github.com/GMLC-TDC/HELICS/wiki).

[Installation Guide](https://docs.helics.org/en/latest/user-guide/installation/index.html)

### Documentation downloads

- [PDF](https://docs.helics.org/_/downloads/en/latest/pdf/)
- [HTML Zip file](https://docs.helics.org/_/downloads/en/latest/htmlzip/)
- [EPUB](https://docs.helics.org/_/downloads/en/latest/epub/)

Additionally, our initial design requirements document can be found [here](docs/introduction/original_specification.md), which describes a number of our early design considerations and some directions that might be possible in the future.

### [CHANGELOG](CHANGELOG.md)

A history of changes to HELICS

### [ROADMAP](docs/ROADMAP.md)

A snapshot of some current plans for what is to come.

### [Installation](https://docs.helics.org/en/latest/user-guide/installation/index.html)

A guide to installing HELICS on different platforms

### Quick links

- [configuration option reference](docs/references/configuration_options_reference.md)
- [Queries](docs/user-guide/advanced_topics/queries.md)
- [Environment variables](docs/user-guide/advanced_topics/environment_variables.md)
- [C function reference](https://docs.helics.org/en/latest/doxygen/C_api_index.html)
- [CMake Variables](docs/user-guide/installation/helics_cmake_options.md)
- [HELICS Apps](docs/references/apps/index.md)

### Docker

Some of the HELICS apps are available from [docker](https://cloud.docker.com/u/helics/repository/docker/helics/helics). This image does not include any libraries for linking just the executables. `helics_broker`, `helics_app`, `helics_recorder`, `helics_player`, and `helics_broker_server`. Other images are expected to be available in the future. See [Docker](https://docs.helics.org/en/latest/user-guide/installation/docker.html) for a few more details.

## Tools with HELICS support

As a co-simulation framework, HELICS is designed to bring together domain-specific modeling tools so they interact during run time. It effectively tries to build on the shoulders of giants by not reinventing trusted simulation tools, but instead, merely acting as a mediator to coordinate such interactions. HELICS's full power is only apparent when you use it to combine these domain-specific tools.

Thankfully the HELICS API is designed to be minimally invasive and make it straightforward to connect most any tool that provides either a scripting interface or access to source code. As listed on [Tools using HELICS](docs/references/Tools_using_helics.md), a growing set of energy domain tools have HELICS support either natively or through an external interface. We also provide a set of helper apps for various utility and testing purposes.

We are always looking for help adding support for more tools, so please contact us if you have any additions.

[Supported Tools](docs/references/Tools_using_HELICS.md)

### HELICS helper Apps

- [HELICS CLI](https://github.com/GMLC-TDC/helics-cli) provides a simple way to automate configuring, starting, and stopping HELICS co-simulations. This helps in overcoming the challenges associated with successfully sequencing and starting simulations of all sizes and is particularly helpful for larger simulations.
- [Broker](./docs/references/apps/Broker.md), which is a command line tool for running a Broker, the core hub in HELICS for data exchange. One or more brokers are what tie the simulation tools together in a HELICS federation. There is also a [Broker Server](https://helics.readthedocs.io/en/latest/user-guide/simultaneous_cosimulation) which can generate brokers as needed, and can include a REST API.
- [Player](./docs/references/apps/Player.md), which acts as a simple send-only federate that simply publishes a stream of timed HELICS messages from a user-defined file. This can be very useful when testing a federate in isolation by mimicking the data that will eventually come from other sources, and in assembling or debugging federations to stand in for any federates which might not be quite ready or that take a long time to run. The Player can also readily playback the files created by the HELICS Recorder (see below). HELICS Player is included in the HELICS distribution.
- [Recorder](./docs/references/apps/Recorder.md), which acts as a simple receive-only federate that prints out or saves messages from one or more subscribed streams. This makes it easy to monitor some or all of the data exchanged via HELICS and can also be part of debugging and modular workflows. For example it can record the data exchanged during a (partly?) successful run to play back (see Player above) to other federates without having to launch those parts again or to isolate/test changes to a subset of a federation. HELICS Recorder is included in the HELICS distribution.
- [App](./docs/references/apps/App.md) is a general app executable which can run a number of other apps including Player and Recorder, as well as a [Tracer](./docs/references/apps/Tracer.md), [Echo](./docs/references/apps/Echo.md), [Source](./docs/references/apps/Source.md), and [Clone](./docs/references/apps/Clone.md).

## Contributing

Contributors are welcome, see the [Contributing](CONTRIBUTING.md) guidelines for more details on the process of contributing. See the [Code of Conduct](.github/CODE_OF_CONDUCT.md) for guidelines on the community expectations. All prior contributors can be found [here](CONTRIBUTORS.md) along with a listing of included and optional components to HELICS.

## Build Status

<details>
  <summary>Click to expand!</summary>

<table>
  <tr>
    <td><b>Service</b></td>
    <td><b>Main</b></td>
    <td><b>Develop</b></td>
  </tr>
  <tr>
  <td>Azure</td>
  <td><a href="https://dev.azure.com/HELICS-test/HELICS/_build/latest?definitionId=5&branchName=main"><img src="https://dev.azure.com/HELICS-test/HELICS/_apis/build/status/GMLC-TDC.HELICS?branchName=main" alt="Build Status" /></a></td>
  <td><a href="https://dev.azure.com/HELICS-test/HELICS/_build/latest?definitionId=5&branchName=develop"><img src="https://dev.azure.com/HELICS-test/HELICS/_apis/build/status/GMLC-TDC.HELICS?branchName=develop" alt="Build Status" /></a></td>
  </tr>
  <tr>
  <td>Circle-CI</td>
  <td><a href="https://circleci.com/gh/GMLC-TDC/HELICS/tree/main"><img src="https://circleci.com/gh/GMLC-TDC/HELICS/tree/main.svg?style=svg" alt="Build Status" /></a></td>
  <td><a href="https://circleci.com/gh/GMLC-TDC/HELICS/tree/develop"><img src="https://circleci.com/gh/GMLC-TDC/HELICS/tree/develop.svg?style=svg" alt="Build Status" /></a></td>
  </tr>
  <td>Docs</td>
  <td><a href="https://docs.helics.org/en/latest/"><img src="https://readthedocs.org/projects/helics/badge/?version=latest" alt="docs" /></a></td>
  <td><a href="https://docs.helics.org/en/develop/"><img src="https://readthedocs.org/projects/helics/badge/?version=latest" alt="docs" /></a></td>
  </tr>
</table>
</details>

## Publications

If you use HELICS in your research, please cite:

\[1\] B. Palmintier, D. Krishnamurthy, P. Top, S. Smith, J. Daily, and J. Fuller, “Design of the HELICS High-Performance Transmission-Distribution-Communication-Market Co-Simulation Framework,” in _Proc. of the 2017 Workshop on Modeling and Simulation of Cyber-Physical Energy Systems_, Pittsburgh, PA, 2017. [pre-print](https://www.nrel.gov/docs/fy17osti/67928.pdf) | [published](https://ieeexplore.ieee.org/document/8064542/)

## In the News

HELICS was selected as an [R&D 100 Award Finalist](https://www.rdworldonline.com/finalists-announced-for-2019-rd-100-awards/).

## History and Motivation

**Brief History:** HELICS began as the core software development of the Grid Modernization Laboratory Consortium ([GMLC](https://gridmod.labworks.org/)) project on integrated Transmission-Distribution-Communication simulation (TDC, GMLC project 1.4.15) supported by the U.S. Department of Energy's Offices of Electricity ([OE](https://www.energy.gov/oe/office-electricity-delivery-and-energy-reliability)) and Energy Efficiency and Renewable Energy ([EERE](https://www.energy.gov/eere/office-energy-efficiency-renewable-energy)). As such, its first use cases were around modern electric power systems, though today it is used for a much larger range of applications. HELICS's layered, high-performance, co-simulation framework builds on the collective experience of multiple national labs.

**Motivation:** Energy systems and their associated information and communication technology systems are becoming increasingly intertwined. As a result, effectively designing, analyzing, and implementing modern energy systems increasingly relies on advanced modeling that simultaneously captures both the cyber and physical domains in combined simulations.

## Source Repo

The HELICS source code is hosted on GitHub: [https://github.com/GMLC-TDC/HELICS](https://github.com/GMLC-TDC/HELICS)

## Release

HELICS is distributed under the terms of the BSD-3 clause license. All new
contributions must be made under this license. [LICENSE](LICENSE)

SPDX-License-Identifier: BSD-3-Clause

portions of the code written by LLNL with release number
LLNL-CODE-739319
