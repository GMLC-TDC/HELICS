
Generating SWIG extension
=========================

**Python**

The easiest way to generate the latest C files for the Python extension is to use CMake itself.
For example, you can run the following on an OSX machine where you have `swig` installed.

```bash
git clone https://github.com/GMLC-TDC/HELICS-src
cd HELICS-src
mkdir build-osx
cmake -DBUILD_PYTHON=ON -DPYTHON_INCLUDE_DIR=$(python3-config --prefix)/include/python3.6m/ -DCMAKE_INSTALL_PREFIX=/Users/$(whoami)/local/helics-develop/ .. && make -j 8 && make install
cd swig/python
cp helicsPYTHON_wrap.c ../../../swig/python/helics_wrap.c
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

