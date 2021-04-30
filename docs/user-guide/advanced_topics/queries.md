# Queries

Queries are an asynchronous means within a HELICS federation of asking for and receiving information from other federate components. A query provides the ability to evaluate the current state of a federation and typically addresses the configuration and architecture of the federation. Brokers, Federates, and Cores all have query functions. Federates are also able to define a callback for answering custom queries.

The general function looks like this:

**C++**

```cpp
std::string query(const std::string& target, const std::string& queryStr)
```

**Python**

```Python
query_result = h.helicsCreateQuery(traget_string, query_string)
```

## Targets

Each query must define a "target", the component in the federation that is being queried. The target is either specified in terms of the relationship to the querying federate (_e.g._ "broker", "core") or by name of the federation component (_e.g._ "dist_system_1_fed"). The table below lists the valid query targets; if a federate happens to be named one of the target names listed below, it will not be queries by that name. For example, naming one of your brokers "broker" will prevent it being a valid target of a query by name. Instead, any federate that queries "broker" will end up targeting their broker.

```eval_rst
+------------------------------------------+---------------------------------------------------------------------------------------+
| target                                   | Description                                                                           |
+==========================================+=======================================================================================+
| ``broker``                               | The first broker encountered in the hierarchy from the caller                         |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``root``, ``federation``, ``rootbroker`` | The root broker of the federation                                                     |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``global``                               | Retrieve the data associated with a global variable                                   |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``parent``                               | The parent of the caller                                                              |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``core``                                 | The core of a federate. This is not a valid target if called from a broker          |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``federate``                             | A query to the local federate or the first federate of a core                         |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``<object name>``                        | any named object in the federation can also be queried, brokers, cores, and federates |
+------------------------------------------+---------------------------------------------------------------------------------------+
```

## Query String

The `queryStr` is the specific data being requested; the tables below show the valid data provided by each queryable federation component. Invalid queries or targets return a json object "error" containing a code and an error message.
Answers to queries can be (in C++ data types):

- true/false \[T/F\]
- a single quoted string `"answer"` \[string\]
- a vector of quoted strings delimited by `','` `["answer1","answer"","answer3"]` \[sv\] this is a JSON compliant string vector
- a JSON string with an object \[JSON\]

As of HELICS 2.7.0 Queries have an optional parameter to describe a sequencing mode. There are currently two modes, `HELICS_SEQUENCING_MODE_FAST` which travels along priority channels and is identical to previous versions in which all queries traveled along those channels. The other mode is `helics_sequencing_mode_ordered` which travels along lower priority channels but is ordered with all other messages in the system. This can be useful in some situations where you want previous messages to be acknowledged as part of the federation before the query is run. The `global_flush` query is forced to run in ordered mode at least until after it gets to the specified target.

### Federate Queries

The following queries are defined for federates. Federates may specify a callback function which allows arbitrary user defined Queries. The queries defined here are available inside of HELICS.

```eval_rst
+--------------------+------------------------------------------------------------+
| queryString        | Description                                                |
+====================+============================================================+
| ``name``           | the identifier of the federate [string]                    |
+--------------------+------------------------------------------------------------+
| ``exists``         | Basic query if the federate exists in the Federation [T/F] |
+--------------------+------------------------------------------------------------+
| ``isinit``         | If the federate has entered init mode [T/F]                |
+--------------------+------------------------------------------------------------+
| ``state``          | Current state of the federate as a string [string]         |
+--------------------+------------------------------------------------------------+
| ``global_state``   | Current state of the federate as a string [JSON]           |
+--------------------+------------------------------------------------------------+
| ``publications``   | current publications of a federate [sv]                    |
+--------------------+------------------------------------------------------------+
| ``subscriptions``  | current subscriptions of a federate [sv]                   |
+--------------------+------------------------------------------------------------+
| ``inputs``         | current inputs of a federate [sv]                          |
+--------------------+------------------------------------------------------------+
| ``endpoints``      | current endpoints of a federate [sv]                       |
+--------------------+------------------------------------------------------------+
| ``dependencies``   | list of the objects this federate depends on [sv]          |
+--------------------+------------------------------------------------------------+
| ``dependents``     | list of dependent objects [sv]                             |
+--------------------+------------------------------------------------------------+
| ``current_time``   | the current time of the federate [JSON]                    |
+--------------------+------------------------------------------------------------+
|``endpoint_filters``| data structure containing the filters on endpoints[JSON]   |
+--------------------+------------------------------------------------------------+
|``dependency_graph``| a graph of the dependencies in a federation [JSON]         |
+--------------------+------------------------------------------------------------+
|``data_flow_graph`` | a structure with all the data connections [JSON]           |
+--------------------+------------------------------------------------------------+
| ``queries``        | list of available queries [sv]                             |
+--------------------+------------------------------------------------------------+
| ``version``        | the version string of the helics library [string]          |
+--------------------+------------------------------------------------------------+
```

The `global_time_debugging` and `global_flush` queries are also acknowledged by federates but it is not usually recommended to run those queries on a particular federate as they are more useful at higher levels. See the `Core` and `Broker` queries for more description of them.

### Local Federate Queries

The following queries are defined for federates but can only be queried on the local federate, that is, the federate making the query. Federates may specify a callback function which allows arbitrary user defined Queries.

```eval_rst
+---------------------------+------------------------------------------------------------+
| queryString               | Description                                                |
+===========================+============================================================+
| ``updated_input_indices`` | vector of number of the inputs that have been updated [sv] |
+---------------------------+------------------------------------------------------------+
| ``updated_input_names``   | names or targets of inputs that have been updated [sv]     |
+---------------------------+------------------------------------------------------------+
| ``updates``               | values of all currently updated inputs [JSON]              |
+---------------------------+------------------------------------------------------------+
| ``values``                | current values of all inputs [JSON]                        |
+---------------------------+------------------------------------------------------------+
| ``time``                  | the current granted time [string]                          |
+---------------------------+------------------------------------------------------------+
```

### Core queries

The following queries will be answered by a core:

```eval_rst
+--------------------------+-------------------------------------------------------------------------------------+
| queryString              | Description                                                                         |
+==========================+=====================================================================================+
| ``name``                 | the identifier of the core [string]                                                 |
+--------------------------+-------------------------------------------------------------------------------------+
| ``address``              | the network address of the core [string]                                            |
+--------------------------+-------------------------------------------------------------------------------------+
| ``isinit``               | If the core has entered init mode [T/F]                                             |
+--------------------------+-------------------------------------------------------------------------------------+
| ``isconnected``          | If the core has is connected to the network [T/F]                                   |
+--------------------------+-------------------------------------------------------------------------------------+
| ``publications``         | current publications defined in a core [sv]                                         |
+--------------------------+-------------------------------------------------------------------------------------+
| ``inputs``               | current named inputs defined in a core [sv]                                         |
+--------------------------+-------------------------------------------------------------------------------------+
| ``endpoints``            | current endpoints defined in a core [sv]                                            |
+--------------------------+-------------------------------------------------------------------------------------+
| ``filters``              | current filters of the core [sv]                                                    |
+--------------------------+-------------------------------------------------------------------------------------+
| ``federates``            | current federates defined in a core [sv]                                            |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependenson``          | list of the objects this core depends on [sv]                                       |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependents``           | list of dependent objects [sv]                                                      |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependencies``         | structure containing dependency information [JSON]                                  |
+--------------------------+-------------------------------------------------------------------------------------+
| ``federate_map``         | a Hierarchical map of the federates contained in a core [JSON]                      |
+--------------------------+-------------------------------------------------------------------------------------+
| ``federation_state``     | a structure with the current known status of the brokers and federates [JSON]       |
+--------------------------+-------------------------------------------------------------------------------------+
| ``current_time``         | if a time is computed locally that time sequence is returned, otherwise #na [JSON]  |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_time``          | get a structure with the current time status of all the federates/cores [JSON]      |
+------------------------------+-------------------------------------------------------------------------------------+
| ``current_state``        | The state of all the components of a core as known by the core [JSON]               |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_state``         | The state of all the components from the components [JSON]                          |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependency_graph``     | a representation of the dependencies in the core and its contained federates [JSON] |
+--------------------------+-------------------------------------------------------------------------------------+
| ``data_flow_graph``      | a representation of the data connections from all interfaces in a federation [JSON] |
+--------------------------+-------------------------------------------------------------------------------------+
|``filtered_endpoints``    | data structure containing the filters on endpoints for the core[JSON]               |
+--------------------------+-------------------------------------------------------------------------------------+
| ``queries``              | list of dependent objects [sv]                                                      |
+--------------------------+-------------------------------------------------------------------------------------+
|``version_all``           | data structure with the version string and the federates[JSON]                      |
+--------------------------+-------------------------------------------------------------------------------------+
| ``version``              | the version string for the helics library [string]                                  |
+--------------------------+-------------------------------------------------------------------------------------+
| ``counter``              | A single number with a code, changes indicate core changes [string]                 |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_time_debugging``| return detailed time debugging state [JSON]                                         |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_flush``         | a query that just flushes the current system and returns the id's [JSON]            |
+--------------------------+-------------------------------------------------------------------------------------+
```

The `version` and `version_all` queries are valid but are not usually queried directly, but instead the same query is used on a broker and this query in the core is used as a building block.

### Broker Queries

The following queries will be answered by a broker:

```eval_rst
+--------------------------+-------------------------------------------------------------------------------------+
| queryString              | Description                                                                         |
+==========================+=====================================================================================+
| ``name``                | the identifier of the broker [string]                                               |
+--------------------------+-------------------------------------------------------------------------------------+
| ``address``              | the network address of the broker [string]                                          |
+--------------------------+-------------------------------------------------------------------------------------+
| ``isinit``               | If the broker has entered init mode [T/F]                                           |
+--------------------------+-------------------------------------------------------------------------------------+
| ``isconnected``          | If the broker is connected to the network [T/F]                                     |
+--------------------------+-------------------------------------------------------------------------------------+
| ``publications``         | current publications known to a broker [sv]                                         |
+--------------------------+-------------------------------------------------------------------------------------+
| ``endpoints``            | current endpoints known to a broker [sv]                                            |
+--------------------------+-------------------------------------------------------------------------------------+
| ``federates``            | current federates under the brokers hierarchy [sv]                                  |
+--------------------------+-------------------------------------------------------------------------------------+
| ``brokers``              | current cores/brokers connected to a broker [sv]                                    |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependson``            | list of the objects this broker depends on [sv]                                     |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependencies``         | structure containing dependency information for the broker [JSON]                   |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependents``           | list of dependent objects [sv]                                                      |
+--------------------------+-------------------------------------------------------------------------------------+
| ``counts``               | a simple count of the number of brokers, federates, and handles [JSON]              |
+--------------------------+-------------------------------------------------------------------------------------+
| ``current_state``        | a structure with the current known status of the brokers and federates [JSON]       |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_state``         | a structure with the current state all system components [JSON]                     |
+--------------------------+-------------------------------------------------------------------------------------+
| ``status``              | a structure with the current known status (true if connected) of the broker [JSON]  |
+--------------------------+-------------------------------------------------------------------------------------+
| ``current_time``         | if a time is computed locally that time sequence is returned, otherwise #na [string]|
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_time``          | get a structure with the current time status of all the federates/cores [JSON]      |
+--------------------------+-------------------------------------------------------------------------------------+
| ``federate_map``         | a Hierarchical map of the federates contained in a broker [JSON]                    |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependency_graph``     | a representation of the dependencies in the broker and all contained members [JSON] |
+--------------------------+-------------------------------------------------------------------------------------+
| ``data_flow_graph``      | a representation of the data connections from all interfaces in a federation [JSON] |
+--------------------------+-------------------------------------------------------------------------------------+
| ``queries``              | list of dependent objects [sv]                                                      |
+--------------------------+-------------------------------------------------------------------------------------+
|``version_all``           | data structure with the version strings of all broker components [JSON]             |
+--------------------------+-------------------------------------------------------------------------------------+
| ``version``              | the version string for the helics library [string]                                  |
+--------------------------+-------------------------------------------------------------------------------------+
| ``counter``              | A single number with a code, changes indicate federation changes [string]           |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_time_debugging``| return detailed time debugging state [JSON]                                         |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_flush``         | a query that just flushes the current system and returns the id's [JSON]            |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_status``        | an aggregate query that returns a combo of global_time and current state [JSON]     |
+--------------------------+-------------------------------------------------------------------------------------+
```

`federate_map`, `dependency_graph`, `global_time`,`global_state`,`global_time_debugging`, and `data_flow_graph` when called with the root broker as a target will generate a JSON string containing the entire structure of the federation. This can take some time to assemble since all members must be queried. `global_flush` will also force the entire structure along the ordered path which can be quite a bit slower.

error codes returned by the query follow [http error codes](https://en.wikipedia.org/wiki/List_of_HTTP_status_codes) for "Not Found (404)" or "Resource Not Available (400)" or "Server Failure (500)".

## Usage Notes

Queries that must traverse the network travel along priority paths unless specified otherwise with a sequencing mode. The calls are blocking, but they do not wait for time advancement from any federate and take priority over regular communication.

The difference between `current_state` and `global_state` is that `current_state` is generated by information contained in the component so doesn't generate secondary queries of other components. Whereas `global_state` will reach out to the other components to get up to date information on the state.

### Application API

There are two basic calls in the application API as part of a [federate object](../doxygen/classhelics_1_1Federate.html)
In addition to the call described above a second version omits the "target" specification and always queries the local federate.

```cpp
std::string    query(const std::string& queryStr)
```

There is also an asyncrhonous version (that is, non-blocking) that returns a `query_id_t` that can be use in `queryComplete` and `isQueryComplete` functions.

```cpp
query_id_t     queryAsync(const std::string& target, const std::string& queryStr)
```

In the header [`<helics\queryFunctions.hpp>`](../doxygen/queryFunctions_8hpp.html) a few helper functions are defined to vectorize query results and some utility functions to wait for a federate to enter init, or wait for a federate to join the federation.

### C API and interface API's

Queries in the [`C API`](https://docs.helics.org/en/latest/c-api-reference/index.html#query) have the same valid targets and properties that can be queried but the construction of the query is slightly different. The basic operation is to create a query using `helicsQueryCreate(target,query)`. Once created, the target or query string can be changed with `helicsQuerySetTarget()` and `helicsQuerySetQueryString()`, respectively.

This function returns a query object that can be used in one of the execute functions (`helicsQueryExecute()`, `helicsQueryExecuteAsync()`, `helicsQueryBrokerExecute()`, `helicsQueryCoreExecute()`, to perform the query and receive back results. The query can be called asynchronously on a federate. The target field may be empty if the query is intended to be used on a local federate, in which case the target is assumed to be the federate itself.
A query must be freed after use `helicsQueryFree()`.

## Example

A full co-simulation example showing how queries can be used for [dynamic configuration can can be found here](../examples/advanced_examples/advanced_query.md) (with the source code in the [HELICS Examples repository](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/user_guide_examples/advanced/advanced_message_comm/query)).
