# Install using Spack (macOS, Linux)

Install Spack (a HELICS package is included in the Spack develop branch and Spack releases after v0.14.1).

Run the following command to install HELICS (this may take a while, Spack builds all dependencies from source!):

```bash
spack install helics
```

To get a list of installation options, run:

```bash
spack info helics
```

To enable or disable options, use `+`, `-`, and `~`. For example, to build with MPI support on the command run would be:

```bash
spack install helics +mpi
```
