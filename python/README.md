# HELICS python extension

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
