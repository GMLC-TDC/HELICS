# helicsMEX

## Prerequisites

- Install [SWIG with MATLAB](https://github.com/jaeandersson/swig/)
- `./configure --prefix=/Users/$USER/local/swig_install; make; make install;`
- Ensure that SWIG and MATLAB are in the PATH

## Build SWIG MATLAB source

```bash
cd ~/GitRepos/GMLC-TDC/HELICS-src/swig/
mkdir matlab
swig -I../src/helics/shared_api_library -outdir ./matlab -matlab ./helics.i
mv helics_wrap.cxx matlab/helicsMEX.cxx
```

## Compile MATLAB extension

```bash
cd ~/GitRepos/GMLC-TDC/HELICS-src/swig/
mex -I../src/helics/shared_api_library ./matlab/helics_wrap.cxx -lhelicsSharedLib -L/path/to/helics_install/lib/helics/
mv helicsMEX.* matlab/
```

## Test HELICS MATLAB extension

Run the following in two separate windows.

```bash
matlab -nodesktop -nosplash
cd ~/GitRepos/GMLC-TDC/HELICS-src/swig/matlab
pireceiver
```

```bash
matlab -nodesktop -nosplash
cd ~/GitRepos/GMLC-TDC/HELICS-src/swig/matlab
pisender
```

