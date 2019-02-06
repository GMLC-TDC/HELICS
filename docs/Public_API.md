# Public API

This file defines what is included in what is considered the stable user API for HELICS.

This API will be backwards code compatible across minor version numbers, though functions may be marked deprecated.  Functions an any other header will not be considered in versioning decisions.  If other headers become commonly used we will take that into consideration at a later time.  Anything marked private is subject to change and most things marked protected can change as well though somewhat more consideration will be given.

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
- Exceptions:  Any function dealing with Inputs with data from multiple sources is subject to change,  the vector subscriptions, and inputs is subject to change

 - Core library headers
- Core.hpp
- Broker.hpp
- CoreFactory.hpp
- BrokerFactory.hpp
- core-exceptions.hpp
- core-types.hpp
- core-data.hpp
- helics-time.hpp
- CoreFederateInfo.hpp
- helicsVersion.hpp
- federate_id.hpp
- helics_definitions.hpp


 - Shared Library headers (c)
- api-data.h
- helics.h
- helicsCallbacks.h
- MessageFederate.h
- MessageFilters.h
- ValueFederate.h


  - App Library
 - Player.hpp
 - Recorder.hpp
 - Echo.hpp
 - Source.hpp
 - Tracer.hpp
 - helicsApp.hpp
 - BrokerApp.hpp
 - Exceptions:  the loadArguments functions will change when we remove boost::program_options

 - Exceptions:  Any function dealing with Inputs concerning data from multiple sources is subject to change,  the vector subscription Objects, and vector Input objects are subject to change.


  - C++98 Library --All headers are mostly stable.  Though we reserve the ability to make changes to make them better match the main C++ API.  This will likely undergo some modifications in the near future, possibly including the folder location and names, and additional definitions.

In the installed folder are some additional headers from third party libraries (cereal, helics_includes), we will try to make sure these are compatible in the features used in the HELICS API,  though changes in other aspects of those libraries will not be considered in HELICS versioning, this caveat includes anything in the helics/common directory as well.  Only changes which impact the signatures defined above will factor into versioning decisions.  You are free to use them but they are not guaranteed to be backwards compatible on version changes.
