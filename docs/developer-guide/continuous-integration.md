# Description of the different continuous integration test setups running on the CI servers

There are 3 main CI servers that are running along with a couple additional checks
Travis, Appveyor, and Azure(Not Enabled Yet).

## Travis Tests
Travis-ci runs many of the primary checks  In 3 different stages

### Push Tests
Push tests run on all pushes to any branch in the main repo, there are 4 tests that run regularly

 - GCC 6: Test the GCC 6.0 compiler and the CI labeled Tests  BOOST 1.61, SWIG, MPI
 - Clang 5: Test the clang compiler and run the CI labeled Tests, along with python and Java interface generation and Tests Using C++17
 - GCC 4.9: Test the oldest supported compiler in GCC,  Test the included interface files(SWIG OFF) for Java and python, and test a packaging build.  The main tests are disabled,  BOOST 1.61
 - XCode 10.2: Test a recent Xcode compiler

### PR tests and develop branch Tests
Pull request tests run on every pull request to develop or master, In addition to the previous 4 tests 2 additional tests are run.
 - Clang 3.6: which is the oldest fully supported clang compiler, with boost 1.58 (Build only)
 - XCode 8.3:  which is the oldest fully supported Xcode version (not for PRs to develop)

### Daily Builds on develop
On the develop branch a few additional tests are run on a daily basis.  These will run an extended set of tests or things like valgrind or clang-sanitizers.  The previous tests are run with an extended set of tests and a few additional tests are run

  - gcc 6.0 valgrind, interface disabled
  - gcc 6.0 Code Coverage, MPI, interfaces disabled
  - clang 5 undefined behavior sanitizer, interfaces disabled
  - clang 5 thread sanitizer, interfaces disabled

## Appveyor tests
  - MSVC 2015 cmake 3.13,  python and java builds and tests, packaging tests for windows

## Azure tests
PRs and commits to the master and develop branches that pass the tests on Travis will trigger builds on Azure for several other HELICS related repositories (such as HELICS-Examples). The result of the builds for those repositories will be reported as a comment on the PR (if any) that triggered the build.
