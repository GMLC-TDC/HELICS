# HELICS python extension

### Install

When installing HELICS using cmake, set `BUILD_PYTHON` to be `ON`

```bash
cd ~
mkdir GitRepos
cd GitRepos
git clone https://github.com/GMLC-TDC/HELICS-src
cd HELICS-src
mkdir build
cd build

cmake -DCMAKE_INSTALL_PREFIX=~/helics_install -DBUILD_PYTHON=ON -DPYTHON_LIBRARY=$(python3-config --prefix)/lib/libpython3.6m.dylib -DPYTHON_INCLUDE_DIR=$(python3-config --prefix)/include/python3.6m ../

make clean; make -j 4; make install
```

This above command installs HELICS to `~/helics_install`. The Python extension is installed to `~/helics_install/python`.


### Testing

```bash
export PYTHONPATH="~/helics_install/python"

cd ~/GitRepos/HELICS-src/swig/python

python pireceiver.py
```

```bash
export PYTHONPATH="~/helics_install/python"

cd ~/GitRepos/HELICS-src/swig/python

python pisender.py
```


