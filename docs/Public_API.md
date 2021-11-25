# Public API

This file defines what is included in what is considered the stable user API for HELICS.

This API will be backwards code compatible through major version numbers, though functions may be marked deprecated between minor version numbers. Functions in any other header will not be considered in versioning decisions. If other headers become commonly used we will take that into consideration at a later time. Anything marked private is subject to change and most things marked protected can change as well though somewhat more consideration will be given in versioning.

The public API includes the following

- Application API headers

  - CombinationFederate.hpp
  - Publications.hpp
  - Subscriptions.hpp
  - Endpoints.hpp
  - Filters.hpp
  - Federate.hpp
  - helicsTypes.hpp
  - data_view.hpp
  - MessageFederate.hpp
  - MessageOperators.hpp
  - ValueConverter.hpp
  - ValueFederate.hpp
  - HelicsPrimaryTypes.hpp
  - queryFunctions.hpp
  - FederateInfo.hpp
  - Inputs.hpp
  - BrokerApp.hpp
  - CoreApp.hpp
  - timeOperations.hpp
  - typeOperations.hpp
  - Exceptions: Vector subscriptions, and vector inputs are subject to change. The queries to retrieve JSON may update the format of the returned JSON in the future. A general note on queries. The data returned via queries is subject to change, in general queries will not be removed, but if a need arises the data structure may change at minor revision numbers.

- Core library headers

  - Core.hpp
  - Broker.hpp
  - core-exceptions.hpp
  - core-data.hpp
  - CoreFederateInfo.hpp
  - helicsVersion.hpp
  - federate_id.hpp
  - helics_definitions.hpp
  - NOTE: core headers in the public API are headers that need to be available for the Application API public headers. The core API can be used more directly with static linking but applications are generally recommended to use the application API or other higher level API's

- C shared library headers

  - All C library operations are merged into a single header `helics.h`
  - A `helics_api.h` header is available for generating interfaces which strips out import declarations and comments. The C shared library API is the primary driver of versioning and changes to that will be considered in all versioning decisions.

- App Library

  - Player.hpp
  - Recorder.hpp
  - Echo.hpp
  - Source.hpp
  - Tracer.hpp
  - Clone.hpp
  - helicsApp.hpp
  - BrokerApp.hpp (aliased to application_api version)
  - CoreApp.hpp (aliased to application_api version)

- Exceptions: The vector subscription Objects, and vector Input objects are subject to change.

- C++98 Library _All headers are mostly stable. Though we reserve the ability to make changes to make them better match the main C\+\+ API._

In the installed folder are some additional headers from third party libraries (CLI11, utilities), we will try to make sure these are compatible in the features used in the HELICS API, though changes in other aspects of those libraries will not be considered in HELICS versioning, this caveat includes anything in the `helics/external` and `helics/utilities` directories. Only changes which impact the signatures defined above will factor into versioning decisions. You are free to use them but they are not guaranteed to be backwards compatible on version changes.
