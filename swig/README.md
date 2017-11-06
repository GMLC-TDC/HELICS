# HELICS SWIG extension


### Python

```bash
export HELICS_INSTALL="../path/to/helics_install"
export PYHELICS_PACKAGE_VERSION="0.1.0" # number required for building a package. Ideally, this should match that of HELICS

cmake -DCMAKE_INSTALL_PREFIX=/absolute/path/to/helics_install -DBUILD_PYTHON=ON -DBUILD_SHARED_LIBS=ON -DPYTHON_LIBRARY=$(python-config --prefix)/lib/libpython3.6m.dylib -DPYTHON_INCLUDE_DIR=$(python-config --prefix)/include/python3.6m ../
```

**Test python extension**

```bash
export DYLD_FALLBACK_LIBRARY_PATH=/absolute/path/to/helics_install/lib  # Mac
# OR
export LD_LIBRARY_PATH=/absolute/path/to/helics_install/lib  # Linux
# OR
export PYTHON_PATH=/absolute/path/to/helics_install/lib

python pisender.py
```

```bash
export DYLD_FALLBACK_LIBRARY_PATH=/absolute/path/to/helics_install/lib  # Mac
# OR
export LD_LIBRARY_PATH=/absolute/path/to/helics_install/lib  # Linux
# OR
export PYTHON_PATH=/absolute/path/to/helics_install/lib

python pireceiver.py
```
