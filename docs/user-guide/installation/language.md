# HELICS with language bindings support

## HELICS with Python

`pip install helics` should work for most use cases.

For developers and special use cases, the HELICS Python module code can be found in the [PyHELICS](https://github.com/gmlc-tdc/pyhelics) repository.

## HELICS with Java

To install HELICS with Java support, you will need to add `HELICS_BUILD_JAVA_INTERFACE=ON`.

## HELICS with MATLAB

The Matlab interface to HELICS is found in the [matHELICS](https://github.com/gmlc-tdc/mathelics) repository.

## HELICS with Simulink

A Simulink interface to HELICS is found in the [simHELICS](https://github.com/gmlc-tdc/simhelics) repository. This is a minimalistic interface that is built using Simulink S-functions.

## HELICS with Octave

The Octave interface to HELICS is found in the [matHELICS](https://github.com/gmlc-tdc/mathelics) repository. It is currently built with the Matlab Interface.

### Notes

The octave interface requires a newer octave 8.3 and above that supports building with mex files.

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
