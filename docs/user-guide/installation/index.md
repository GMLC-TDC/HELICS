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

```{mermaid}
%%{
    init: {
        'theme': 'base',
        'themeVariables': {
            'primaryColor': '#0C96D0',
            'edgeLabelBackground':'#646464',
            'tertiaryColor': '#646464',
            'primaryTextColor': '#ffffff'
        },
        'flowchart': {
            'useMaxWidth': true
        }

    }
}%%
graph LR
    Start(Do you want to install a <br/> standard HELICS version?) -->|Yes| languageChoice
    Start -->|No| build
    subgraph Source Build
        build[Build from source for the <br> particular branch or <br/> configuration that you need.]
    end
    subgraph Package Manager/Precompiled
        languageChoice(What language are <br/> you using?)
        languageChoice -->|Python| python["Use pip: <br><br> pip install 'helics[cli]'"]
        languageChoice -->|MATLAB| matlab[Follow instructions in <br> the matHELICS repository]
        languageChoice -->|julia| julia[Use pkg: <br><br> pck> add helics]
        languageChoice -->|Java| java[Build from source with the <br> Java CMAKE option set <br><br>HELICS_BUILD_JAVA_INTERFACE=ON]
        languageChoice -->|nim| nim[Use nimble: <br><br> nimble install <br> https://github.com/GMLC-TDC/helics.nim#head]
        languageChoice -->|C#| csharp[Build from source with the <br> C# CMAKE option set <br><br>HELICS_BUILD_CSHARP_INTERFACE=ON]
        languageChoice -->|C/C++| hpc
        subgraph &nbsp;
            hpc[Will you be using HELICS on an HPC?]
            hpc --> |Yes| spack[use spack]
            hpc --> |No| precompiled
        end
    end
    click build "https://docs.helics.org/en/latest/user-guide/installation/build_from_source.html" "build from source"
    click python "https://python.helics.org/installation/" "pyhelics installation guide"
    click matlab "https://github.com/GMLC-TDC/matHELICS/blob/main/README.md" "matHELICS installation guide"
    click julia "https://github.com/GMLC-TDC/HELICS.jl/blob/master/README.md" "HELICS.jl installation guide"
    click java "https://docs.helics.org/en/latest/user-guide/installation/build_from_source.html" "jHELICS installation guide"
    click nim "https://github.com/GMLC-TDC/helics.nim/blob/master/README.md" "nim installation guide"
    click csharp "https://docs.helics.org/en/latest/user-guide/installation/build_from_source.html" "C# installation guide"
    click precompiled "https://docs.helics.org/en/latest/user-guide/installation/installing_the_pre_compiled_libraries.html" "precompiled libraries installation guide"
    click spack "https://docs.helics.org/en/latest/user-guide/installation/spack.html" "spack installation guide"
```

### Build from source

[Build from source](./build_from_source.md)

### Python via pip install

[`pip install helics`](https://python.helics.org/)

### matHELICS install

Installation instructions are available in the [matHELICS repository README](https://github.com/GMLC-TDC/matHELICS/blob/main/README.md)

### Julia install

[`pkg> add HELICS`](https://github.com/GMLC-TDC/HELICS.jl/blob/master/README.md)

### pip install

[`pip install 'helics[cli]'`](https://python.helics.org/) (Includes the optional but recommended helics_cli tool.)

### jHELICS

[Build from source](./build_from_source.md) with the [CMAKE option](./helics_cmake_options.md) `HELICS_BUILD_JAVA_INTERFACE=ON`

### nimble install

[`nimble install https://github.com/GMLC-TDC/helics.nim#head`](https://github.com/GMLC-TDC/helics.nim)

### C# install

[Build from source](./build_from_source.md) with the [CMAKE option](./helics_cmake_options.md) `HELICS_BUILD_CSHARP_INTERFACE=ON`

### Download pre-compiled

[Download the pre-compiled libraries and add them to your system path](./installing_the_pre_compiled_libraries.md)

### spack install

[`spack install helics`](./spack.md)

## Running an Example

The [Quick Start guide](../../quick_start/quick_start_index.md) walks through the steps of running the first Python-based User Guide example and serves as a good way to test your (Python) installation.

## HELICS runner

Previously a separate executable, `helics_cli` was used to provide functionality to launch a HELICS-based co-simulation by calling a JSON configuration such as

```shell
helics run --path=<path to HELICS runner JSON>
```

This functionality still exists but has been moved to the [PyHELICS code base](https://python.helics.org/) and the `helics_cli` repository has been deprecated. Thus, it is recommended that all users install PyHELICS (via `pip install 'helics[cli]'` as described above) to gain the runner and web interface functionality.
