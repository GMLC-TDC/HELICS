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

pwd
# /Users/$USER/GitRepos/HELICS-src

cmake -DCMAKE_INSTALL_PREFIX=/Users/$USER/helics_install -DBUILD_PYTHON=ON -DPYTHON_LIBRARY=$(python-config --prefix)/lib/libpython3.6m.dylib -DPYTHON_INCLUDE_DIR=$(python-config --prefix)/include/python3.6m ../
make clean; make -j 4; make install
```

This above command installs HELICS to `/Users/$USER/helics_install`. The Python extension is installed to `/Users/$USER/helics_install/python`.


### Testing

```bash
export PYTHONPATH="/Users/$USER/helics_install/python"

cd /Users/$USER/GitRepos/HELICS-src/swig/python

python pireceiver.py
```

```bash
export PYTHONPATH="/Users/$USER/helics_install/python"

cd /Users/$USER/GitRepos/HELICS-src/swig/python

python pisender.py
```


