# Package Manager

## Install using brew (MacOS)

Install [brew](https://brew.sh/). It is a package manager for MacOS.

Once you install brew, you can open a terminal and type the following.

```bash
brew tap GMLC-TDC/helics
brew install helics
```

OR

```bash
brew install GMLC-TDC/helics/helics
```

If you want to install it with the Python extension, you can use the
following.

```bash
brew reinstall helics --with-python --with-python-include-dir=$(python3-config --prefix)/include/python3.6m/
```

You must pass `--with-python-include-dir` with a value. The easiest way
to find out what the `--with-python-include-dir` argument should be is
by using `python3-config` as shown above.

If you want to install using Python2 instead, you can use
`--with-python-include-dir=$(python-config --prefix)/include/python2.7/`.
It is important that the Python interpreter used to run `import helics`
was built using the header files included in `python-config --prefix`.
That is to say, you cannot build using Python3 and run using Python2.

Additionally, if required, you can add `--HEAD` to install from the
latest `develop` branch.

## Install using conda (Windows, MacOS, Linux)

Install [Anaconda](https://www.anaconda.com/download/) or [Miniconda](https://conda.io/en/latest/miniconda.html). It is a Python distribution but also provides a cross platform package manager called `conda`.

```bash
conda install -c conda-forge helics
```

OR

```bash
conda install -c kdheepak helics
```
