# Porting Guide: HELICS 2 to 3

Since HELICS 3 is a major version update, there are some breaking changes to the API for developers.
This guide will try to track what breaking changes are made to the API, and what developers should use
instead when updating from HELICS 2.x to 3.

## Dependency changes

Support for some older compilers and dependencies have been removed. The new minimum version are:

- C++17 compatible-compiler (minimums: GCC 7.0, Clang 5.0, MSVC 2017 15.7, XCode 10, ICC 19)
- CMake 3.10+ (if using clang with libc++, use 3.18+)
- ZeroMQ 4.2+
- Boost 1.65.1+ (if building with Boost enabled)

## Code changes

Changes that will require changing code are listed below based on the interface API used.
A list of known PRs that made breaking changes is also provided.

### PRs with breaking changes

- [#1363][1]
- [#1952][2]
- [#1907][3]
- [#1856][4]
- [#1731][5]
- [#1727][6]
- [#1680][7]
- [#1679][8]
- [#1677][9]
- [#1572][10]
- [#1580][11]

### Application API (C++17)

- `Federate::error(int errorcode)` and `Federate::error(int errorcode, const std::string& message)` were removed, use `localError` instead (or `globalError` to stop the entire simulation). Changed in [#1363][1].
- `ValueFederate::publishString` and `ValueFederate::publishDouble` have been removed, please use the Publication interface `publish` methods which take a wide variety of types
- The interface object headers are now included by default when including the corresponding federate

### Command line interfaces

The numerical value corresponding with the log levels have changed. As such entering numerical values for log levels is no longer supported (it will be again someday). For now please use the text values "none(-1)", "no_print(-1)", "error(0)", "warning(1)", "summary(2)", "connections(3)", "interfaces(4)", "timing(5)", "data(6)", "debug(6)", "trace(7)". The previous values are shown in parenthesis. The new numerical values are subject to revision in a later release so are not considered stable at the moment and are not currently accepted as valid values for command line or config files.

### C Shared API

- Only 1 header is now used `#include <helics/helics.h>` for all uses of the C shared library in C/C++ code -- no other headers are needed, the other headers are no longer available. [#1727][6]
- Removed `helics_message` struct -- call functions to set fields instead. `helicsEndpointGetMessage` and `helicsFederateGetMessage` returning this struct were removed -- call functions to get field values instead. Changed in [#1363][1].
- `helics_message_object` typedef was renamed to `HelicsMessage` in `api-data.h`; in `MessageFederate.h` and `helicsCallbacks.h` all `helics_message_object` arguments and return types are now `HelicsMessage`. Changed in [#1363][1].
- Renamed `helicsEndpointSendMessageObject` to `helicsEndpointSendMessage`, `helicsSendMessageObjectZeroCopy` to `helicsSendMessageZeroCopy`, `helicsEndpointGetMessageObject` to `helicsEndpointGetMessage`, `helicsEndpointCreateMessageObject` to `helicsEndpointCreateMessage`, `helicsFederateGetMessageObject` to `helicsFederateGetMessage`, and `helicsFederateCreateMessageObject` to `helicsFederateCreateMessage`. Changed in [#1363][1].
- The send data API has changed to make the usage clearer and reflect the addition of targeted endpoints. New methods are `helicsEndpointSendBytes`, `helicsEndpointSendBytesTo`, `helicsEndpointSendBytesAt`, `helicsEndpointSendBytesToAt`. This reflects usage to send a raw byte packet to the targeted destination or a user specified one, and at the current granted time or a user specified time in the simulation future. The C++98 API was changed accordingly. The order of fields has also changed for consistency [#1677][9]
- Removed `helicsEndpointClearMessages` -- it did nothing, `helicsFederateClearMessages` or `helicsMessageFree` should be used instead. Changed in [#1363][1].
- All constants such as flags and properties are now CAPITAL_SNAKE_CASE [#1731][5]
- All structures are now CamelCase, though the old form will be available in helics3 though will be deprecated at some point. [#1731][5] [#1580][11]
- `helicsPublicationGetKey` renamed to `helicsPublicationGetName` [#1856][4]
- Recommended to change `helicsFederateFinalize` to `helicsFederateDisconnect`, the finalize method is still in place but will be deprecated in a future release. [#1952][2].
- `helicsMessageGetFlag` renamed to `helicsMessageGetFlagOption` for better symmetry [#1680][7]
- `helics<*>PendingMessages` moved `helics<*>PendingMessageCount` [#1679][8]

### C++98 API (wrapper around the C Shared API)

- Removed the `helics_message` struct, and renamed `helics_message_object` to `HelicsMessage`. Direct setting of struct fields should be done through API functions instead. This affects a few functions in the `Message` class in `Endpoint.hpp`; the explicit constructor and `release()` methods now take `HelicsMessage` arguments, and `operator helics_message_object()` becomes `operator HelicsMessage()`. Changed in [#1363][1].

### Queries

- Queries now return valid JSON except for `global_value` queries. Any code parsing the query return value will need to be adjusted. Error codes are reported back as HTML error codes and a message.

### Libraries

- The C based shared library is now `libhelics.dll` or the appropriate extension based on the OS [#1727][6] [#1572][10]
- The C++ shared library is now `libhelicscpp.[dll\so\dylib]` [#1727][6] [#1572][10]
- The apps library is now `libhelicscpp-apps.[dll\so\dylib]` [#1727][6] [#1572][10]

### CMake

- All HELICS CMake variables now start with `HELICS_` [#1907][3]
- Projects using HELICS as a subproject or linking with the CMake targets should use `HELICS::helics` for the C API, `HELICS::helicscpp` for the C++ shared library, `HELICS::helicscpp98` for the C++98 API, and `HELICS::helicscpp-apps` for the apps library. If you are linking with the static libraries you should know enough to be able to figure out what to do, otherwise it is not recommended.

[1]: https://github.com/GMLC-TDC/HELICS/pull/1363 "PR #1363"
[2]: https://github.com/GMLC-TDC/HELICS/pull/1952 "PR #1952"
[3]: https://github.com/GMLC-TDC/HELICS/pull/1907 "PR #1907"
[4]: https://github.com/GMLC-TDC/HELICS/pull/1856 "PR #1856"
[5]: https://github.com/GMLC-TDC/HELICS/pull/1731 "PR #1731"
[6]: https://github.com/GMLC-TDC/HELICS/pull/1727 "PR #1727"
[7]: https://github.com/GMLC-TDC/HELICS/pull/1680 "PR #1680"
[8]: https://github.com/GMLC-TDC/HELICS/pull/1679 "PR #1679"
[9]: https://github.com/GMLC-TDC/HELICS/pull/1677 "PR #1677"
[10]: https://github.com/GMLC-TDC/HELICS/pull/1572 "PR #1572"
[11]: https://github.com/GMLC-TDC/HELICS/pull/1580 "PR #1580"
