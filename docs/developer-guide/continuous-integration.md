# Continuous integration builds

The CI configuration is defined in the repository. This page summarizes the jobs currently enabled; the workflow and pipeline files are the authoritative source when a job matrix changes.

## Azure Pipelines

Pushes (except `pre-commit/*`) and pull requests targeting `main` run the primary build-and-test matrix:

- Linux containers: Ubuntu 24.04 default, GCC 11, Clang 15, and Clang 18. The Ubuntu default and GCC 11 jobs enable MPI and encryption.
- macOS 14 and macOS 15.
- Windows: Visual Studio 2022 32-bit and 64-bit, plus Visual Studio 2026 64-bit.

The daily pipeline runs the full Linux test suite, a Linux build using ZeroMQ as a subproject, and a 64-bit Visual Studio 2022 build. Windows jobs use Boost 1.83.

## CircleCI

For pull requests and branches other than `pre-commit/*`, CircleCI runs install/package checks plus GCC 14, Clang 18, and Clang 18 with C++23. Scheduled jobs on `main` also run OpenSUSE Tumbleweed, a no-ZeroMQ configuration, and an ARM64 build. Benchmarks run three times weekly.

## GitHub Actions

GitHub Actions runs the following repository workflows:

- Static analysis (`cpplint` on pushes and pull requests to `main`, and `clang-tidy` on pull requests).
- A Clang 22 C++26 build and smoke-test job on pushes and pull requests to `main`.
- MSYS2 builds using both MinGW and MSYS Makefiles on pull requests to `main`, relevant branch pushes, and a daily schedule.
- Code coverage on `main` pushes and scheduled runs; pull-request coverage runs for branches whose names begin with `coverage_`.
- Address, memory, and thread sanitizer builds on the daily schedule.
- Scheduled benchmark and release-artifact builds, plus release packaging when a GitHub release is published.
- Docker image builds for `main`, Docker-focused branches, tags, and releases.
- CodeQL, generated SWIG-interface updates, image compression, and release-checklist automation.

## AppVeyor and Cirrus CI

- AppVeyor performs a Cygwin 64-bit Release compilation on `main`; tests are disabled for this job.
- Cirrus CI builds and runs the `SystemCI` test label on FreeBSD 15.

## Documentation

Read the Docs builds and hosts the published documentation. Codacy checks may also be reported, but they are informational and are not part of the required CI matrix.
