# HELICS with language bindings support

## HELICS with Python

`pip install helics` should work for most use cases.

For developers and special use cases, the HELICS Python module code can be found in the [PyHELICS](https://github.com/gmlc-tdc/pyhelics) repository.

## HELICS with Java

To install HELICS with Java support, you will need to add `HELICS_BUILD_JAVA_INTERFACE=ON`.

## HELICS with MATLAB

The Matlab interface to HELICS is undergoing some major revisions as of HELICS 3.2.1 and is no longer part of the Main HELICS repository. The instructions for use can be found at [matHELICS](https://github.com/gmlc-tdc/mathelics) repository.

## HELICS with Octave

To install HELICS with Octave support, you will need to add `HELICS_BUILD_OCTAVE_INTERFACE=ON`. Swig is required to build the Octave interface from source; it can be installed via package managers such as apt on Ubuntu or [chocolatey](https://chocolatey.org/packages?q=swig) on Windows, Octave can also be installed in this manner.

```bash
git clone https://github.com/GMLC-TDC/HELICS
cd HELICS
mkdir build
cd build
cmake -DHELICS_BUILD_OCTAVE_INTERFACE=ON -DCMAKE_INSTALL_PREFIX=/Users/$(whoami)/local/helics-develop/ ..
make -j8
make install
```

add the octave folder in the install directory to the octave path

```bash
>> helics
>> helicsGetVersion()
ans = 3.x.x (20XX-XX-XX)
```

### Notes

Octave 4.2 will require swig 3.0.12, Octave 4.4 and 5.0 and higher will require swig 4.0 or higher. The Octave interface has built and run smoothly on Linux systems and on the Windows system with Octave 5.0 installed through Chocolatey. There is a regular CI test that builds and tests the interface on Octave 4.2.

## HELICS with C Sharp

C\# is supported through SWIG. This requires swig being installed and generating the CMake for HELICS with HELICS_BUILD_CSHARP_INTERFACE=ON. If in Visual studio this will generate the appropriate files for C# usage.

A suitable version of swig can be installed on macOS, Windows, or Linux using:

```bash
pip install swig
```

Depending on your build environment on Windows, using [Chocolatey](https://chocolatey.org/) to install swig might make it easier for the HELICS build scripts to automatically locate swig:

```shell
choco install swig
```
