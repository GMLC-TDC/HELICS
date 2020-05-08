# Description of the different continuous integration test setups running on the CI servers

There are 5 CI servers that are running along with a couple additional checks
Travis, Appveyor, Circle-CI, Azure and Drone.

## Travis-CI Tests

Travis-ci runs many of the primary checks In 3 different stages

### Push Tests

Push tests run on all pushes to any branch in the main repository, there are 4 tests that run regularly

- GCC 6: Test the GCC 6.0 compiler and the CI labeled Tests BOOST 1.61, SWIG, MPI
- Clang 5: Test the clang compiler and run the CI labeled Tests, along with python and Java interface generation and Tests Using C++17
- GCC 4.9: Test the oldest supported compiler in GCC, Test the included interface files(SWIG OFF) for Java and python, and test a packaging build. The main tests are disabled, BOOST 1.61
- XCode 10.2: Test a recent XCode compiler with the Shared API library tests

### PR tests and develop branch Tests

Pull request tests run on every pull request to develop or master. In addition to the previous 4 tests 2 additional tests are run.

- Clang 3.6: which is the oldest fully supported clang compiler, with boost 1.58 (Build only)
- XCode 10.2

### Daily Builds on develop

On the develop branch a few additional tests are run on a daily basis. These will run an extended set of tests or things like valgrind or clang-sanitizers. The previous tests are run with an extended set of tests and a few additional tests are run

- gcc 6.0 valgrind, interface disabled
- gcc 6.0 Code Coverage, MPI, interfaces disabled
- gcc 6.0 ZMQ subproject cmake 3.11
- Mingw test building on the Mingw platform
- Xcode 9.4 which is the oldest fully supported Xcode version (not for PRs to develop)

## Appveyor tests

- MSVC 2015 CMake 3.13, python and JAVA builds

## Azure tests

PRs and commits to the master and develop branches that pass the tests on Travis will trigger builds on Azure for several other HELICS related repositories (such as HELICS-Examples). The result of the builds for those repositories will be reported as a comment on the PR (if any) that triggered the build.

On the Primary HELICS repository there are 4 Azure builds:

- MSVC2015 64bit Build and test, chocolatey swig/boost
- MSVC2017 32bit Build and test
- MSVC2017 64bit Build and test with Java
- MSVC2019 64bit Build and test with Java

## Circle CI

All PR's and branches trigger a set of builds using Docker images on Circle-CI.

- Octave tests - tests the Octave interface and runs some tests
- Clang-MSAN - runs the clang memory sanitizer
- Clang-ASAN - runs the clang address sanitizer and undefined behavior sanitizer
- Clang-TSAN - runs the clang thread sanitizer
- install1 - build and install and link with the C shared library, C\++ shared library, C\++98 library and C\++ apps library, and run some tests linking to the installed libraries
- install2 - build and install and link with the C shared library, and C\++98 library only and run some tests linking with the installed library

### Benchmark tests

Circle ci also runs a benchmark test that runs every couple days. Eventually this will form the basis of benchmark regression test.

## Drone

- 64 bit and 32 bit builds on ARM processors

## Cirrus CI

- FreeBSD 12.1 build

## Read the docs

- Build the docs for the website and test on every commit

## Codacy

There are some static analysis checks run with Codacy. While it is watched it is not always required to pass.
