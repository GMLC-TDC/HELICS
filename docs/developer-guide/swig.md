# Generating SWIG extension

**C#**
A C# interface can be generated using swig and enabling HELICS_BUILD_CSHARP_INTERFACE in the CMake. The support is partial; it builds and can be run but not all the functions are completely usable and it hasn't been fully tested.

**Java**
A JAVA interface can be generated using swig and enabling HELICS_BUILD_JAVA_INTERFACE in the CMake. This interface is tested regularly as part of the CI test system.
