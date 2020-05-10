# Installation

The first step to using HELICS is to install it.
You’ll need an internet connection to run the commands in this chapter, as we’ll be downloading HELICS from the internet.

We’ll be showing a number of commands as code snippets in the following presentation using a terminal, and those lines may start with `$`.
You don’t need to type in the `$` character; they are there to indicate the start of each command.
Lines that don’t start with `$` are typically showing the output of the previous command.

## Quick start

Get the latest installers from [GitHub Releases](https://github.com/GMLC-TDC/HELICS/releases/latest).

OR

Use conda to install the HELICS Python interface and apps:

```bash
conda install -c gmlc-tdc helics
```

OR

Use pip to install the HELICS Python interface and apps:

```bash
pip install helics
pip install helics-apps
```

OR

Use Spack (requires Spack develop branch or versions released after v0.14.1) on HPC systems (Linux/macOS) to install C/C++ HELICS components and apps:

```bash
spack install helics
```

For more information on supported options (e.g. using a custom HELICS build with MPI support) and troubleshooting tips, see the [package managers](https://helics.readthedocs.io/en/latest/installation/package_manager.html) page for more details, or the documentation for your package manager.

## Using an installer for your operating system

Download pre-compiled libraries from the [releases page](https://github.com/GMLC-TDC/HELICS/releases/latest) and add them to your path.
Windows users should install the latest version of the [Visual C++ Redistributable](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads).
The installers come with bindings for Python (3.6), MATLAB, and Java extensions precompiled as part of the installation.
All you need to do is add the relevant folders to your User's PATH variables.

On Windows, you can visit `Control Panel -> System -> Advanced System Settings -> Environment Variables` and edit your user environment variables to add the necessary Path, PYTHONPATH, JAVAPATH, MATLABPATH environment variables to the corresponding HELICS installed locations.

On MacOS or Linux, you can edit your `~/.bashrc` to add the necessary PATH, PYTHONPATH, JAVAPATH, MATLABPATH environment variables to the corresponding HELICS installed locations.

Be sure to restart your CMD prompt on Windows or Terminal on your MacOS/Linux to ensure the new environment variables are in effect.

## Using a package manager for your operating system

You can install it using one of the supported package managers.

```eval_rst
.. toctree::
    :maxdepth: 1

    package_manager
```

Alternatively, you can install from source. See the next section for more information.

## OS Specific installation from source

```eval_rst
.. toctree::
    :maxdepth: 1

    windows
    mac
    linux
    docker
    language
    linking
    helics_cmake_options
```

The following are a few things that could be useful to know before starting out.

Firstly, you can follow HELICS development on our [GitHub](https://github.com/GMLC-TDC/HELICS) page.
HELICS is open-source. The development team uses `git` for version control, and GitHub to host the code publicly.
The latest HELICS will be on the `develop` branch.
Tagged releases occur on the `master` branch.
If you clone the HELICS repository, you will be placed in the `master` branch by default.
To switch to the `develop` branch, you can type the following:

```bash
git checkout develop
```

To switch to a tagged release, you can type the following:

```bash
git checkout v2.3.0
```

You will not need a full understanding of how `git` works for installing HELICS, but if you are interested you can find a good `git` resource in [this page](https://git-scm.com/book/en/v2).

Secondly, HELICS is a modern C++ cross-platform software application.
One challenge while maintaining the same codebase across multiple operating systems is that we have to ensure that everything installs correctly everywhere.
The development team uses `CMake` to build HELICS.
`CMake` is a cross-platform tool designed to build, test and package software.
Having the latest version of `CMake` can make the build process much smoother.
`CMake` reads certain files (CMakeLists.txt) from the HELICS repository, and creates a bunch of build files.
These build files specify how different files depend on each other and when these build files are run, HELICS is built.
The exact instructions to run on each operating system are given in the individual installation instructions, but one important thing to remember is that these build files are essentially temporary files.
If you have an issue building HELICS, once you make a change (installing/removing/adding anything), you probably need to delete these temporary files and re-generate them.
We've found in practice that you don't have to do this too often, but it can save hours of frustration if you are already aware that this needs to be done.

Another valuable piece of information about `CMake` is that almost every "OPTION" is configurable while you generate the build files.
This means you can pass it configurations settings as a key value pair by adding `-D{NAME_OF_OPTION}={VALUE_OF_OPTION}` to the `cmake` command line interface.
For example, to build the Python extension all you need to do is pass in `-DBUILD_PYTHON_INTERFACE=ON`.
You can also run `ccmake .` in the build folder, to get a command line interactive prompt to change configuration settings.
On Windows, you can use the cmake GUI to do the same.
Again, there are more instructions in the individual installation pages but a useful trick to know if something isn't documented or a slightly more advanced feature is required. Available CMake options for HELICS are documented [here](https://helics.readthedocs.io/en/latest/installation/helics_cmake_options.html).
