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
From there, you can use `pip install helics` as above (NOTE: `pip install helics-apps` *will not work*, your package manager or HELICS build from source should install these).
The [source distributions section of the PyPI page](https://pypi.org/project/helics/) has some additional useful information on this process.
