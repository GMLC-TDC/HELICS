# Generating SWIG extension

**Python**

The easiest way to generate the latest C files for the Python extension is to use CMake itself.
For example, you can run the following in a POSIX/Unix environment where you have `swig` installed with Python 3.6.

```bash
git clone https://github.com/GMLC-TDC/HELICS
cd HELICS
mkdir build
cmake -DBUILD_PYTHON_INTERFACE=ON -DPYTHON_INCLUDE_DIR=$(python3-config --prefix)/include/python3.6m/ -DCMAKE_INSTALL_PREFIX=$HOME/local/helics-develop/ .. && make -j 8 && make install
cd swig/python
cp helicsPYTHON_wrap.c ../../../swig/python/helics_wrap.c
cp helics.py ../../../swig/python/helics.py
```

This method verifies that the C file generated from CMake using SWIG compiles correctly into a Python extension.

For building a Python 2 compatible interface, use BUILD_PYTHON2_INTERFACE instead of BUILD_PYTHON_INTERFACE.

**MATLAB**

For the MATLAB extension, you need a special version of SWIG. Get it [here](https://github.com/jaeandersson/swig).

```bash
git clone https://github.com/jaeandersson/swig
cd swig
./configure --prefix=/Users/$(whoami)/local/swig-matlab/ && make -j8 && make install
```

The matlab interface can be built using BUILD_MATLAB_INTERFACE in the cmake build of HELICS. This will use a MATLAB installation to build the interface. See [installation](../installation/language.html)

**Octave**

[Octave](https://www.gnu.org/software/octave/) is a free program that works similarly to MATLAB
Building the octave interface requires swig, and currently will work with Octave 4.0 through 4.2. 4.4 is not currently supported by SWIG unless you build from the current master of the swig repo and use that version. The next release of swig will likely support it. It does work on windows, though the actual generation is not fully operational for unknown reasons and will be investigated at some point. A mkhelicsOCTFile.m is generated in the build directory this needs to be executed in octave, then a helics.oct file should be generated, the libHelicsShared.dll needs to be copied along with the libzmq.dll files Once this is done the library can be loaded by calling helics. On linux this build step is done for you with BUILD_OCTAVE_INTERFACE.

**C#**
A C# interface can be generated using swig and enabling BUILD_CSHARP_INTERFACE in the CMake. The support is partial; it builds and can be run but not all the functions are completely usable and it hasn't been fully tested.

**Java**
A JAVA interface can be generated using swig and enabling BUILD_JAVA_INTERFACE in the CMake. This interface is tested regularly as part of the CI test system.
