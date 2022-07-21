# HELICS Installation

```{eval-rst}
.. toctree::
    :maxdepth: 2
    :hidden:

    installing_the_pre_compiled_libraries
    spack
    build_from_source
    package_manager
```

## HELICS Installation Methods

The first step to using HELICS is to install it. Since there are several ways to do this, the flow chart below provides some insight into what approach is likely to be the easiest depending upon a number of factors, most predominantly the programming language bindings that you intend to use. Below the flow chart are links to more complete instructions for each method. Note that you’ll need an internet connection for this process, as we’ll be downloading HELICS from the internet.

As of HELICS v3, the only supported language bindings that are included with the core HELICS library downloads are C and C++98, in addition to C++17 when building from source. If you end up needing to build from source AND use one of the supported language bindings you'll need to follow the instructions for installing HELICS for said language. This would also be the case if you were needing to run a co-simulation that used tools that provided their HELICS implementation in a variety of languages. Generally speaking, as long as all supported languages are on similar versions, each one can use it own installed version of HELICS without any trouble. The supported languages also have ways of being pointed towards a specific HELICS installation (rather than the one they install for themselves) if that is preferred or necessary for a particular use case.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/install-decision-tree.png)

### pip install

[pip install helics](https://python.helics.org/)

### Download pre-compiled

[Download the pre-compiled libraries and add them to your system path](./installing_the_pre_compiled_libraries.md)

### spack install

[spack install helics](./spack.md)

### nimble install

[nimble install https://github.com/GMLC-TDC/helics.nim#head](https://github.com/GMLC-TDC/helics.nim)

### Build from source

[Build from source](./build_from_source.md)

## HELICS runner

Previously a separate executable, `helics_cli` was used to provide functionality to launch a HELICS-based co-simulation by calling a JSON configuration such as

```shell
helics run --path=<path to HELICS runner JSON>
```

This functionality still exists but has been moved to the [PyHELICS code base](https://python.helics.org/) and the `helics_cli` repository has been deprecated. Thus, it is recommended that all users install PyHELICS (via `pip install helics` as described above) to gain the runner and web interface functionality.
