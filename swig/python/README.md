# HELICS python extension

### Install

#### Clone HELICS and setup directories (only need to do once)

```bash
cd ~
mkdir GitRepos
cd GitRepos
git clone https://github.com/GMLC-TDC/HELICS-src
cd HELICS-src
mkdir build
cd build
```
#### Actually build HELICS
Note: starting from ~/GitRepos/HELICS-src/build or your similar directory

1. Setup the build with cmake.

```
cmake -DCMAKE_INSTALL_PREFIX=~/helics_install -DBUILD_PYTHON=ON -DPYTHON_LIBRARY=$(python3-config --prefix)/lib/libpython3.6m.dylib -DPYTHON_INCLUDE_DIR=$(python3-config --prefix)/include/python3.6m ../
```

With Python2, the above command is instead the following.

```
cmake -DCMAKE_INSTALL_PREFIX=~/helics_install -DBUILD_PYTHON=ON -DPYTHON_LIBRARY=$(python-config --prefix)/lib/libpython2.7.dylib -DPYTHON_INCLUDE_DIR=$(python-config --prefix)/include/python2.7 ../
```

2. Run the build
```
# First clean up any old stuff
make clean
# Then run the actual compilation, using 4 parallel threads
make -j 4
# Finally copy files to final locations and adjust environment
make install
```

The above commands installs HELICS to `~/helics_install`. The Python extension is installed to `~/helics_install/python`.


### Testing
__Background:__ Running a HELICS federation (via low level commands) requires first starting a helics broker and then running the desired set of federates with it. In this case, the pisender starts this broker and then joins as a federate.

1. Open up 2 terminal windows
2. In the first start the python demo sender (sends the value of π every simulation second from 5.0-9.0). This also starts the broker
```bash
export PYTHONPATH="~/helics_install/python"

cd ~/GitRepos/HELICS-src/examples/python

python pisender.py
```
3. In the second, start the python demo reciever (subscribes to the π messages & prints to screen)
```bash
export PYTHONPATH="~/helics_install/python"

cd ~/GitRepos/HELICS-src/examples/python

python pireceiver.py
```

You should see something like the following in the PI RECEIVER window (2nd one in directions above)
```
$ python pireceiver.py
PI RECEIVER: Helics version = 1.0.0.alpha (01-18-18)
PI RECEIVER: Creating Federate Info
PI RECEIVER: Setting Federate Info Name
PI RECEIVER: Setting Federate Info Core Type
PI RECEIVER: Setting Federate Info Init String
PI RECEIVER: Setting Federate Info Time Delta
PI RECEIVER: Setting Federate Info Logging
PI RECEIVER: Creating Value Federate
PI RECEIVER: Value federate created
PI RECEIVER: Subscription registered
PI RECEIVER: Entering execution mode
PI RECEIVER: Current time is 5.0
PI RECEIVER: Received value = 3.142857142857143 at time 5.0 from PI SENDER
PI RECEIVER: Received value = 3.142857142857143 at time 6.0 from PI SENDER
PI RECEIVER: Received value = 3.142857142857143 at time 7.0 from PI SENDER
PI RECEIVER: Received value = 3.142857142857143 at time 8.0 from PI SENDER
PI RECEIVER: Received value = 3.142857142857143 at time 9.0 from PI SENDER
PI RECEIVER: Federate finalized
end of master Object Holder destructor
```

Corresponding output should appear from the PI SENDER (window 1).

