
Generating SWIG extension
=========================

**Python**

The easiest way to generate the latest C files for the Python extension is to use CMake itself.
For example, you can run the following on an OSX machine where you have `swig` installed.

```bash
git clone https://github.com/GMLC-TDC/HELICS-src
cd HELICS-src
mkdir build-osx
cmake -DBUILD_PYTHON_INTERFACE=ON -DPYTHON_INCLUDE_DIR=$(python3-config --prefix)/include/python3.6m/ -DCMAKE_INSTALL_PREFIX=/Users/$(whoami)/local/helics-develop/ .. && make -j 8 && make install
cd swig/python
cp helicsPYTHON_wrap.c ../../../swig/python/helics_wrap.c
cp helics.py ../../../swig/python/helics.py
```

This method verifies that the C file generated from CMake using SWIG compiles correctly into a Python extension.

**MATLAB**

For the MATLAB extension, you need a special version of SWIG. Get it [here](https://github.com/jaeandersson/swig).

```bash
git clone https://github.com/jaeandersson/swig
cd swig
./configure --prefix=/Users/$(whoami)/local/swig-matlab/ && make -j8 && make install
```

Next, run the following

```bash
git clone https://github.com/GMLC-TDC/HELICS-src
cd HELICS-src
export PATH=/Users/$(whoami)/local/swig-matlab/bin:$PATH
swig -matlab -I../src/helics/shared_api_library -outdir matlab helics.i
mv helics_wrap.cxx ./matlab/helicsMEX.cxx
```

**Octave**

[Octave](https://www.gnu.org/software/octave/) is a free program that works similarly to MATLAB
Building the octave interface requires swig, and currently will work with Octave 4.0 through 4.2.  4.4 is not currently supported by SWIG unless you build from the current master of the swig repo and use that version.  The next release will likely support it.  It does work on windows, though the actual generation is not fully operational for unknown reasons and will be investigated at some point.  A mkhelicsOCTFile.m is generated in the build directory this needs to be executed in octave, then a helics.oct file should be generated, the libHelicsShared.dll needs to be copied along with the libzmq.dll files Once this is done the library can be loaded by calling helics.   On linux this build step is done for you. with the BUILD_OCTAVE_INTERFACE.
