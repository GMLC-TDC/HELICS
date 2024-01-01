# OS Specific installation from source

```{eval-rst}
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
Tagged releases occur on the `main` branch.
If you clone the HELICS repository, you will be placed in the `main` branch by default.
To switch to the `develop` branch, you can type the following:

```bash
git checkout develop
```

To switch to a tagged release, you can type the following:

```bash
git checkout v3.3.1
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
For example, to build the Java extension all you need to do is pass in `-DHELICS_BUILD_JAVA_INTERFACE=ON`.
You can also run `ccmake .` in the build folder, to get a command line interactive prompt to change configuration settings.
On Windows, you can use the cmake GUI to do the same.
Again, there are more instructions in the individual installation pages but a useful trick to know if something isn't documented or a slightly more advanced feature is required. Available CMake options for HELICS are documented [here](helics_cmake_options.md).
