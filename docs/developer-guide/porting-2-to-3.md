# Porting Guide: HELICS 2 to 3

Since HELICS 3 is a major version update, there are some breaking changes to the API for developers.
This guide will try to track what breaking changes are made to the API, and what developers should use
instead when updating from HELICS 2.x to 3.

## Dependency changes

Support for some older compilers and dependencies have been removed. The new minimum version are:

- C++17 compatible-compiler (minimums: GCC 7.0, Clang 5.0, MSVC 2017 15.7, XCode 10, ICC 19)
- CMake 3.10+
- ZeroMQ 4.2+
- Boost 1.65.1+ (if building with Boost enabled)

## Code changes

Changes that will require changing code are listed below based on the interface API used.
A list of known PRs that made breaking changes is also provided.

### PRs with breaking changes

- [#1363][1]

### Application API (C++17)

- `Federate::error(int errorcode)` and `Federate::error(int errorcode, const std::string& message)` were removed, use `localError` instead (or `globalError` to stop the entire simulation). Changed in [#1363][1].
- ValueFederate::publishString and publishDouble have been removed please use the Publication interface `publish` methods which take a wide variety of types

### Command line interfaces

The numerical value corresponding with the log levels have changed. As such entering numerical values for log levels is no longer supported (it will be again someday). For now please use the text values "none(-1)", "no_print(-1)", "error(0)", "warning(1)", "summary(2)", "connections(3)", "interfaces(4)", "timing(5)", "data(6)", "debug(6)", "trace(7)". The previous values are shown in parenthesis. The new numerical values are subject to revision in a later release so are not considered stable at the moment.

### C Shared API

- Only 1 header is now used `#include <helics/helics.h>` for all uses of the C shared library in C/C++ code no other headers are needed, the other headers are no longer available.
- Removed `helics_message` struct -- call functions to set fields instead. `helicsEndpointGetMessage` and `helicsFederateGetMessage` returning this struct were removed -- call functions to get field values instead. Changed in [#1363][1].
- `helics_message_object` typedef was renamed to `HelicsMessage` in `api-data.h`; in `MessageFederate.h` and `helicsCallbacks.h` all `helics_message_object` arguments and return types are now `HelicsMessage`. Changed in [#1363][1].
- Renamed `helicsEndpointSendMessageObject` to `helicsEndpointSendMessage`, `helicsSendMessageObjectZeroCopy` to `helicsSendMessageZeroCopy`, `helicsEndpointGetMessageObject` to `helicsEndpointGetMessage`, `helicsEndpointCreateMessageObject` to `helicsEndpointCreateMessage`, `helicsFederateGetMessageObject` to `helicsFederateGetMessage`, and `helicsFederateCreateMessageObject` to `helicsFederateCreateMessage`. Changed in [#1363][1].
- Removed `helicsEndpointClearMessages` -- it did nothing, `helicsFederateClearMessages` or `helicsMessageFree` should be used instead. Changed in [#1363][1].
- all constants such as flags and properties are now CAPITAL_SNAKE_CASE
- all structures are now CamelCase, though the old form will be available in helics3 though will be deprecated at some point.
- `helicsPublicationGetKey` renamed to `helicsPublicationGetName`
-

### C++98 API (wrapper around the C Shared API)

- Removed the `helics_message` struct, and renamed `helics_message_object` to `HelicsMessage`. Direct setting of struct fields should be done through API functions instead. This affects a few functions in the `Message` class in `Endpoint.hpp`; the explicit constructor and `release()` methods now take `HelicsMessage` arguments, and `operator helics_message_object()` becomes `operator HelicsMessage()`. Changed in [#1363][1].

[1]: https://github.com/GMLC-TDC/HELICS/pull/1363 "PR #1363"
