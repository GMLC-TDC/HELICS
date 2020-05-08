# MATLAB

## Prerequisites

- Install [SWIG with MATLAB](https://github.com/jaeandersson/swig/)
- `./configure --prefix=/Users/$USER/local/swig_install; make; make install;`
- Ensure that SWIG and MATLAB are in the PATH

## Building HELICS with MATLAB extension

HELICS can be built with the MATLAB extension by enabling the `BUILD_MATLAB_INTERFACE` option in cmake
HELICS will also need to know the location of swig with MATLAB that was built.

It can also be built without that version of swig using existing files in the repo, but this will not work if there are any library changes.
After installing the mex file will be placed in the matlab folder of the install directory.

## Build SWIG MATLAB source

```bash
cd ~/GitRepos/GMLC-TDC/HELICS/swig/
mkdir matlab
swig -I../src/helics/shared_api_library -outdir ./matlab -matlab ./helicsMATLAB.i
mv helics_wrap.cxx matlab/helicsMEX.cxx
```

## Compile MATLAB extension

```bash
cd ~/GitRepos/GMLC-TDC/HELICS/swig/
mex -I../src/helics/shared_api_library ./matlab/helics_wrap.cxx -lhelicsSharedLib -L/path/to/helics_install/lib/helics/
mv helicsMEX.* matlab/
```

## Test HELICS MATLAB extension

Run the following in two separate windows.

```bash
matlab -nodesktop -nosplash
cd ~/GitRepos/GMLC-TDC/HELICS-examples/matlab/pi-exchange
pisender
```

The pisender starts a broker so it may work slightly better to start that process first.

```bash
matlab -nodesktop -nosplash
cd ~/GitRepos/GMLC-TDC/HELICS-examples/matlab/pi-exchange
pireceiver
```
