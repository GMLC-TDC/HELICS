# Getting started

This merlin spec sets up the helics pisend/recv example. The _hpc version is setup to be used on HPC.

## HPC example

```
merlin run helics_juggle_hpc.yaml
```

```
merlin run-workers helics_juggle_hpc.yaml
```

The run-workers command will request a allocation from slurm and the workers will be submitted to that allocation

## NOTE
* There is currently a bug in the batch block in the yaml spec where if a bank is specified it doesn't seem to use it.
