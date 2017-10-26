# HELICS python extension

### Install

When installing HELICS using cmake, set `BUILD_SHARED_LIBS` to be `ON` and `BUILD_PYTHON` to be `ON`

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

cmake -DCMAKE_INSTALL_PREFIX=/Users/$USER/helics_install -DBUILD_PYTHON=ON -DBUILD_SHARED_LIBS=ON -DPYTHON_LIBRARY=$(python-config --prefix)/lib/libpython3.6m.dylib -DPYTHON_INCLUDE_DIR=$(python-config --prefix)/include/python3.6m ../
make clean; make -j 4; make install
```

### Testing

```bash
export PYTHONPATH="/Users/$USER/GitRepos/HELICS-src/build/swig/python"

cd /Users/$USER/GitRepos/HELICS-src/swig/python

python pireceiver.py
```

```bash
export PYTHONPATH="/Users/$USER/GitRepos/HELICS-src/build/swig/python"

cd /Users/$USER/GitRepos/HELICS-src/swig/python

python pisender.py
```


