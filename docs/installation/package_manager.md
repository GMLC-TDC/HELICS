# Package Manager

## Install using conda (Windows, MacOS, Linux)

**Recommended**

Install [Anaconda](https://www.anaconda.com/download/) or [Miniconda](https://conda.io/en/latest/miniconda.html). It is a Python distribution but also provides a cross platform package manager called `conda`.

You can then use `conda` to install HELICS.

```bash
conda install -c gmlc-tdc helics
```

## Install using pip (Windows, macOS, Linux, other)

Install Python with pip. Upgrade pip to a recent version using `python -m pip install --upgrade`.

If you're on a supported version of Windows, macOS, or Linux (see the [HELICS PyPI page](https://pypi.org/project/helics/) for details) you can then use `pip` to install the HELICS Python interface and helics-apps.

```bash
pip install helics
pip install helics-apps
```

If you are on an unsupported OS or Python version, you will need to install a copy of HELICS first.
Depending on your OS, there could be a copy in the package manager, or you may need to build HELICS from source.
From there, you can use `pip install helics` as above (NOTE: `pip install helics-apps` _will not work_, your package manager or HELICS build from source should install these).
The [source distributions section of the PyPI page](https://pypi.org/project/helics/) has some additional useful information on this process.

## Install using Spack (macOS, Linux)

Install Spack (a HELICS package is included in the Spack develop branch and Spack releases after v0.14.1).

Run the following command to install HELICS (this may take a while, Spack builds all dependencies from source!):

```bash
spack install helics
```

To get a list of installation options, run:

```bash
spack info helics
```

To enable or disable options, use `+`, `-`, and `~`. For example, to build with MPI support on the command run would be:

```bash
spack install helics +mpi
```

## Troubleshooting shared library errors on Windows

If you encounter an error along the lines of `DLL load failed: The specified module could not be found` when attempting to use the C shared library installed by a package manager, it is likely a required system dependency is missing. You can determine which DLL it is unable to find using a tool like <https://github.com/lucasg/Dependencies> to see what dependency is missing for the helics C shared library DLL. It is fine if it shows it can't find `WS2_32.dll`, but all other DLLs should be found.
The most likely to be missing is `vcruntime140_1.dll`, which can be fixed by downloading the latest Visual C++ Redistributable from <https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads> and installing it.
