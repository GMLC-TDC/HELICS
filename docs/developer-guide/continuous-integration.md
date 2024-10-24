# Description of the different continuous integration test setups running on the CI servers

There are 5 CI servers that are running along with a couple additional checks
GitHub Actions, Appveyor, Circle-CI, Azure and Drone.

## Appveyor tests

- Cygwin builds

## Azure tests

Azure pipelines is currently running the majority of CI tests.

The main tests for pull requests and pushes targeting the main and develop branches are:

- Default Ubuntu 20.04 build and test using GCC with MPI and encryption support enabled
- GCC 8 build and test running on Linux with MPI and encryption support enabled
- Clang 13 build and test running on Linux
- Clang 7 build and test running on Linux
- XCode 10.2: Test a recent XCode compiler with the Shared API library tests
- XCode build and test using the newest version of macOS that is available for CI builds
- XCode build and test using the oldest version of macOS still supported by Apple
- MSVC2019 32 bit build and test without the webserver component
- MSVC2019 64 bit build and test
- MSVC2022 64 bit build and test using the C++20 standard

There are also a few tests run daily:

- Ubuntu 20.04 build using default package versions that runs the larger "daily" CI tests
- Ubuntu 20.04 build using default package versions that uses ZeroMQ as a subproject instead of installing it with a package manager
- MSVC2022 64 bit build and test using Boost 1.74

## Circle CI

All PR's and branches trigger a set of builds using Docker images on Circle-CI.

- Clang-MSAN - runs the clang memory sanitizer
- Clang-ASAN - runs the clang address sanitizer and undefined behavior sanitizer
- Clang-TSAN - runs the clang thread sanitizer
- install1 - build and install and link with the C shared library, C\++ shared library, C\++98 library and C\++ apps library, and run some tests linking to the installed libraries
- install2 - build and install and link with the C shared library, and C\++98 library only and run some tests linking with the installed library

### Benchmark tests

Circle ci also runs a benchmark test that runs every couple days. Eventually this will form the basis of benchmark regression test.

## GitHub Actions

GitHub Actions is used for various release related builds, and some special CI configurations that don't need to run often.

- Static analyzers
- Building pre-compiled packages for releases
- Building Docker images
- Daily build of benchmark binaries
- Daily build of the release artifacts using code in the develop branch
- Daily MSYS2 CI builds using both MinGW and MSYS makefiles
- Daily code coverage build and test

## Drone

- 64 bit and 32 bit builds on ARM processors

## Cirrus CI

- FreeBSD 12.2 build

## Read the docs

- Build the docs for the website and test on every commit

## Codacy

There are some static analysis checks run with Codacy. While it is watched it is not always required to pass.
