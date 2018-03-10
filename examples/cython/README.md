# HELICS python extension

**Note**: As of HELICS v1.0.0, the recommended way to use HELICS in Python is using the SWIG generated extension.
The Cython extension is deprecated and will not be maintained.
The following code is left here as an example of how an integration with Cython may work.

### Install instructions

```bash
pip install cython
# OR
conda install cython

# Optional
pip install click # For better terminal output
```

```bash
export HELICS_INSTALL="/path/to/helics_install/"
make
```

### Run example

```bash
cd /path/to/helics_install/bin
helics_broker 2
```

```bash
python test1.py
```

```bash
python test2.py
```

