# HELICS Installation

```{eval-rst}
.. toctree::
    :maxdepth: 2


```

## HELICS applications

The first step to using HELICS is to install it. Since there are several ways to do this, the flow chart below provides some insight into what approach is likely to be the easiest depending upon a number of factors, most predominantly the programming language bindings that you intend to use. Below the flow chart you'll provide links to more complete instructions for each method. Note that you’ll need an internet connection for this process, as we’ll be downloading HELICS from the internet.

As of HELICS v3, the only supported language bindings that are included with the core library are C++, C98, and C. If you end up needing to build from source AND use one of the supported language bindings you'll need to follow the instructions for installing HELICS for said language. This would also be the case if you were needing to run a co-simulation that used tools that provided their HELICS implementation in a variety of languages. Generally speaking, as long as all supported languages are on similar versions, each one can use it own installed version of HELICS without any trouble. The supported languages also have ways of being pointed towards a specific HELICS installation (rather than the one they install for themselves) if that is preferred or necessary for a particular use case.

![](./images/install-decision-tree.png)

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


## helics-cli Installation

[helics-cli](https://github.com/GMLC-TDC/helics-cli) is a supporting tool that provides a simple and standardized way of launching HELICS-based co-simulations. This tool is used for [all the examples in this User Guide](../examples/examples_index.md) and thus its installation is highly recommended. (helics-cli also provides access to the [web interface for monitoring and debugging co-simulations](../fundamental_topics/web_interface), another good reason to install it.)

Installation of helics-cli is straightforward:

```shell session
pip install git+git://github.com/GMLC-TDC/helics-cli.git@main
```

