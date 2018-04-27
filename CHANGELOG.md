# Changelog
All notable changes to this project after the 1.0.0 release will be documented in this file

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).
This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
## [1.0.2] -2018-04-28
### Fixed
 - Bug not allowing command line parameters separate from the command if a positional argument was in usage
 - Fixed Bug for federate not allowing changes in period or minTimeDelay after entry to execution mode
 - added python2 interface option (this will be available but not fully capable going forward)
 - A few more race conditions fixed from clang thread-sanitizer

## [1.0.1] - 2018-04-22
### Fixed
 - Allow Boost 1.67 usage
 - allow building with AUTOBUILD for ZeroMQ on Linux
 - Clang tidy and static analyzer fixes
 - fix some potential race conditions spotted by clang thread-sanitizer
 - Fix some documentation to better match recent updates
