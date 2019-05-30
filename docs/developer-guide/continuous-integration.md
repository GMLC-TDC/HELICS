# Description of the different continuous integration test setups running on the CI servers

There are 3 main CI servers that are running along with a couple additional checks
Travis, Appveyor, and Azure(Not Enabled Yet).

## Travis Tests
Travis-ci runs many of the primary checks  In 3 different stages

### Push Tests
Push tests run on all pushes to any branch in the main repo, there are 4 tests that run regularly

 - GCC 6 :  Test the GCC 6.0 compiler and the CI labeled Tests
 - Clang 5:  Test the clang compiler and run the CI labeled Tests, along with python and Java interface generation and Tests
 - GCC 4.9: Test the oldest supported compiler in GCC,  Test the included interface files for Java and python, and test a packaging build.  The Boost tests are disabled.
 - XCode 10.2  Test a recent Xcode compiler

### PR tests and develop branch Tests
 Pull request tests run on every pull request to develop or master, In addition to the previous 4 tests 2 additional tests are run.
 - Clang 3.6  which is the oldest fully supported clang compiler
 - XCode 8.3 which is the oldest fully supported Xcode version

### Daily Builds on develop
 On the develop branch a few additional tests are run on a daily basis.  These will run an extended set of tests or things like valgrind or clang-sanitizers.  The previus tests are run with an extended set of tests and a few additional tests are run

  - gcc 6.0 valgrind
  - gcc 6.0 Code Coverage
  - clang 5 undefined behavior
  - clang 5 thread sanitizer
