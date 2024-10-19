# (Planned) CI/CD Infrastructure

This section documents the services used by HELICS.

## Continuous Integration

The `GMLC-TDC/HELICS` repository uses Azure Pipelines to test the main platforms and compilers we support,
with Drone Cloud and Cirrus CI for testing some less common platforms, and CircleCI for running tests using sanitizers.
Nightly release builds are run on GitHub Actions.

All of the builds on Linux use Docker containers. This has a number of advantages:

- The build environment is consistent, making changes to the underlying image easy
- Tools don't need reinstalling for each build
- The tests can be run locally on a developer machine in the same environment
- Old build environments can be used for binary compatibility, such as CentOS 6 for releases.

To avoid long build queues, commits to `main` and `develop` will be tested with more configurations,
while PRs and commits to other branches may be tested on a smaller set of platforms. PRs and commits
to `main` should run a full set of tests on all supported platforms to help ensure that it is always
ready for a new release. Jobs that run for PRs and all commits are marked with `[commit]`, while jobs
marked with `[daily]` are only run daily on the `develop` branch. A tag of `[main]` indicates that
the job is only run for commits and PRs to main.

If more extensive testing is needed for a commit or branch, certain keywords can be included in the
commit message to trigger additional builds for commits or in the branch name to trigger them for
a particular branch. At some point in the future, the community server may provide ways to trigger
extra builds in other circumstances.

Unless otherwise mentioned, jobs will typically generate the Java interface, run the CI/nightly
test suite, and run some brief packaging tests.

### Linux

For Linux, typically the jobs run will be the latest gcc and clang releases, and the oldest
gcc and clang releases supported by HELICS. Unless indicated, they will use a system install of ZeroMQ,
build all supported core types, and use a fairly recent version of dependencies.

- GCC 7.0 on Ubuntu 18.04 using default apt-get versions of dependencies
- Clang 5.0 using minimum supported version of all dependencies
- GCC 10 [commit]
- Clang 10 [commit]
- GCC 10 with ZeroMQ as a subproject [daily]
- CentOS 5 or 6 (same image as for releases)

For auto-generated swig PRs, there is a special build that will run to test a build using the
pre-generated swig interface files.

### Windows

Most of the Windows tests run frequently are using MSVC, though MinGW, MSYS, and Cygwin jobs are run daily.
The MINGW/MSYS builds can be triggered by including `mingw`, `msys`, and/or `cygwin` in a commit message or
branch name.

- MSVC2019 32bit Build and test
- MSVC2019 64bit Build and test with Java
- MSVC2022 64bit Build and test with Java [commit]
- MinGW [daily]
- MSYS [daily]
- Cygwin 32-bit [daily]

### macOS

macOS builds and tests tend to take a long time to run, so there are only a few of them, and they
only run on PRs unless `xcode10` or `xcode11` is included in a commit message or branch name.
In general, the XCode jobs will be the most recent major XCode version and the oldest XCode version
released in about the past 3 years that's supported by Apple (or is available on the Azure macOS
images). Apple tends to be pretty good at getting people to upgrade and dropping support for older
releases.

- XCode 10.1
- XCode 11

### ARM

ARM (32-bit) and aarch64 builds run on Drone Cloud with the regular CI tests for all pull requests and commits.
These builds use the latest Alpine Linux docker image for the builder, which uses musl libc instead of glibc.

### FreeBSD

A FreeBSD 12.1 build runs on Cirrus CI with short system tests for all pull requests and commits.

### Valgrind [daily]

A build with valgrind is run daily to catch memory management and threading bugs.

### Code Coverage [daily]

A daily job runs tests on Linux to gather code coverage results that are uploaded to codecov.

### Sanitizers

Sanitizers and some install tests are run in Docker containers on Circle CI for all commits and PRs.

- Clang-MSAN - runs the clang memory sanitizer
- Clang-ASAN - runs the clang address sanitizer and undefined behavior sanitizer
- Clang-TSAN - runs the clang thread sanitizer
- install1 - build and install and link with the C shared library, C\++ shared library, C\++98 library and C\++ apps library, and run some tests linking to the installed libraries
- install2 - build and install and link with the C shared library, and C\++98 library only and run some tests linking with the installed library

### Documentation

ReadTheDocs is used to build and host the HELICS documentation.

## Static Analysis, Linting, and Automatted Code Maintenance

GitHub Actions is the main service used for running static analysis and linting tools, and
automating some code related maintenance tasks.

### pre-commit

The pre-commit workflow runs linters and formatting tasks for non-C++ code, spell checking for
docs and comments, and opens a PR with any fixes.

### Updating generated interface files and docs

The swig-gen workflow updates the swig generated interface files for the most commonly used
language bindings for HELICS. In the near future it will also perform some documentation
generating tasks to keep documentation on different language interfaces up-to-date.

## Automated Releases

GitHub Actions is used to build release packages when a new release is created on GitHub.
It also triggers jobs that start the process of updating HELICS in several package repositories.

## Multinode Tests and Benchmarks

A cluster on AWS is used to run multinode tests and benchmarks to help detect performance regressions.

Circle CI runs benchmarks every couple of days on a single machine, which serves as the basis for
a benchmark performance regression test.

## Community Server

A small AWS instance is used to perform various orchestration tasks related to releases, and controlling
the cluster used for multinode regression tests. It also runs bots that use GitHub events for providing
some services such as repository maintenance tasks and monitoring the automated release process.
