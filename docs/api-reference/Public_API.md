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
  - BrokerApp.hpp (New in 2.3 Moved from App library)
  - CoreApp.hpp (New in 2.3)- Operations and some capabilities may be added or tweaked in the next revision
  - timeOperations.hpp (New in 2.3)- previously functions were in helics-time.hpp
  - typeOperations.hpp (New in 2.3)- previously functions were in core-types.hpp
  - Exceptions: Any function or method dealing with Inputs with data from multiple sources is subject to change, the vector subscriptions, and vector inputs are subject to change. The functionality related to PublishJSON is considered experimental and may change in the future. The queries to retrieve JSON may update the format of the returned JSON in the future. A general note on queries. The data returned via queries is subject to change, in general queries will not be removed, but if a need arises the data structure may change at minor revision numbers.

- Core library headers

  - Core.hpp
  - Broker.hpp
  - CoreFactory.hpp (Header is deprecated for public API in 2.3 use CoreApp instead)
  - BrokerFactory.hpp (Header is deprecated for public API in 2.3 use BrokerApp instead)
  - core-exceptions.hpp
  - core-types.hpp (string operation functions moved to typeOperations.hpp in 2.3, though are still available for compatibility reasons in the Public API)
  - core-data.hpp
  - helics-time.hpp (string operation functions moved to timeOperations.hpp in 2.3, though are still available for compatibility reasons in the Public API)
  - CoreFederateInfo.hpp
  - helicsVersion.hpp
  - federate_id.hpp
  - helics_definitions.hpp
  - NOTE: core headers in the public API are headers that need to be available for the Application API public headers. The core api can be used more directly with static linking but applications are generally recommended to use the application API or other higher level API's

- C Shared Library headers (c)

  - api-data.h
  - helics.h
  - helicsCallbacks.h (Not well used and considered experimental yet)
  - MessageFederate.h
  - MessageFilters.h
  - ValueFederate.h

- App Library

  - Player.hpp
  - Recorder.hpp
  - Echo.hpp
  - Source.hpp
  - Tracer.hpp
  - Clone.hpp (new in 2.2)
  - helicsApp.hpp
  - BrokerApp.hpp (aliased to application_api version)
  - CoreApp.hpp (aliased to application_api version)
  - BrokerServer.hpp (removed in 2.3 as not useful for library operations, though still available in the static library)

- Exceptions: Any function dealing with Inputs concerning data from multiple sources is subject to change, the vector subscription Objects, and vector Input objects are subject to change. Also some changes may occur in regard to units on the Application API.

- C++98 Library _All headers are mostly stable. Though we reserve the ability to make changes to make them better match the main C\+\+ API._

In the installed folder are some additional headers from third party libraries (cereal, C++ compatibility headers, CLI11, utilities), we will try to make sure these are compatible in the features used in the HELICS API, though changes in other aspects of those libraries will not be considered in HELICS versioning, this caveat includes anything in the `helics/external` and `helics/utilities` directories. Only changes which impact the signatures defined above will factor into versioning decisions. You are free to use them but they are not guaranteed to be backwards compatible on version changes.
