# HELICS SWIG extension


### Python

```bash
export HELICS_INSTALL="../path/to/helics_install"
export PYHELICS_PACKAGE_VERSION="0.1.0" # number required for building a package. Ideally, this should match that of HELICS

pip install -e .
```

**Test python extension**

```bash
export DYLD_FALLBACK_LIBRARY_PATH=/absolute/path/to/helics_install/lib  # Mac
# OR
export LD_LIBRARY_PATH=/absolute/path/to/helics_install/lib  # Linux
python pisender.py
```

```bash
export DYLD_FALLBACK_LIBRARY_PATH=/absolute/path/to/helics_install/lib  # Mac
# OR
export LD_LIBRARY_PATH=/absolute/path/to/helics_install/lib  # Linux
python pireceiver.py
```
