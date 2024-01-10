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

Each query must define a "target", the component in the federation that is being queried. The target is either specified in terms of the relationship to the querying federate (_e.g._ "broker", "core") or by name of the federation component (_e.g._ "dist_system_1_fed"). The table below lists the valid query targets; if a federate happens to be named one of the target names listed below, it can not be queried by that name. For example, naming one of your brokers "broker" will prevent it being a valid target of a query by name. Instead, any federate that queries "broker" will end up targeting their broker.

```{eval-rst}
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
| ``core``                                 | The core of a federate. This is not a valid target if called from a broker            |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``federate``                             | A query to the local federate or the first federate of a core                         |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``<object name>``                        | any named object in the federation can also be queried, brokers, cores, and federates |
+------------------------------------------+---------------------------------------------------------------------------------------+
```

## Query String

The `queryStr` is the specific data being requested; the tables below show the valid data provided by each queryable federation component. All queries return a valid JSON string with invalid queries returning a JSON with an error code and error message. (The only exception is the `global_value` query which just returns a string containing global value.)

As of HELICS 2.7.0 Queries have an optional parameter to describe a sequencing mode. There are currently two modes, `HELICS_SEQUENCING_MODE_FAST` which travels along priority channels and is identical to previous versions in which all queries traveled along those channels. The other mode is `HELICS_SEQUENCING_MODE_ORDERED` which travels along lower priority channels but is ordered with all other messages in the system. This can be useful in some situations where you want previous messages to be acknowledged as part of the federation before the query is run. The `global_flush` query is forced to run in ordered mode at least until after it gets to the specified target.

### Federate Queries

The following queries are defined for federates. Federates may specify a callback function which allows arbitrary user defined Queries. The queries defined here are available inside of HELICS.

```{eval-rst}
+-------------------------+------------------------------------------------------------+
| queryString             | Description                                                |
+=========================+============================================================+
| ``name``                | the identifier of the federate [string]                    |
+-------------------------+------------------------------------------------------------+
| ``exists``              | basic query if the federate exists in the Federation [T/F] |
+-------------------------+------------------------------------------------------------+
| ``isinit``              | federate has entered init mode? [T/F]                      |
+-------------------------+------------------------------------------------------------+
| ``state``               | current state of the federate as a string [string]         |
+-------------------------+------------------------------------------------------------+
| ``global_state``        | current state of the federate as a string [structure]      |
+-------------------------+------------------------------------------------------------+
| ``publications``        | current publications of a federate [sv]                    |
+-------------------------+------------------------------------------------------------+
| ``publication_details`` | details of current publications of a federate [sv]         |
+-------------------------+------------------------------------------------------------+
| ``subscriptions``       | current subscriptions of a federate [sv]                   |
+-------------------------+------------------------------------------------------------+
| ``inputs``              | current inputs of a federate [sv]                          |
+-------------------------+------------------------------------------------------------+
| ``endpoints``           | current endpoints of a federate [sv]                       |
+-------------------------+------------------------------------------------------------+
| ``input_details``       | details of current inputs of a federate [structure]        |
+-------------------------+------------------------------------------------------------+
| ``endpoint_details``    | details of current endpoints of a federate [structure]     |
+-------------------------+------------------------------------------------------------+
| ``dependencies``        | list of the objects this federate depends on [sv]          |
+-------------------------+------------------------------------------------------------+
| ``dependents``          | list of dependent objects [sv]                             |
+-------------------------+------------------------------------------------------------+
| ``current_time``        | the current time of the federate [structure]               |
+-------------------------+------------------------------------------------------------+
| ``endpoint_filters``    | data structure with the filters for endpoints[structure]   |
+-------------------------+------------------------------------------------------------+
| ``dependency_graph``    | a graph of the dependencies in a federation [structure]    |
+-------------------------+------------------------------------------------------------+
| ``data_flow_graph``     | a structure with all the data connections [structure]      |
+-------------------------+------------------------------------------------------------+
| ``queries``             | list of available queries [sv]                             |
+-------------------------+------------------------------------------------------------+
| ``version``             | the version string of the helics library [string]          |
+-------------------------+------------------------------------------------------------+
| ``tags``                | a JSON structure with the tags and values [structure]      |
+-------------------------+------------------------------------------------------------+
| ``barriers``            | a JSON structure with current time barriers [structure]    |
+-------------------------+------------------------------------------------------------+
| ``logs``                | any log messages stored in the log buffer [structure]      |
+-------------------------+------------------------------------------------------------+
| ``tag/<tagname>``       | the value associated with a tagname [string]               |
+-------------------------+------------------------------------------------------------+
| ``<tagname>``           | the value associated with a tagname [string]               |
+-------------------------+------------------------------------------------------------+
```

The `global_time_debugging` and `global_flush` queries are also acknowledged by federates but it is not usually recommended to run those queries on a particular federate as they are more useful at higher levels. See the `Core` and `Broker` queries for more description of them. The difference between `tag/<tagname>` and `<tagname>` is that using the `tag/` prefix can retrieve any tag and will return an empty string if the tag doesn't exist. Just using the tag name will not return tags of the same name as other queries and will generate an error response if the tag doesn't exist. The `logs` query will only contain information if log buffer size is set to greater than 0 by property or command. The logs query also works on cores and brokers that have been disconnected to retrieve buffered logs after the co-simulation has completed. This of course only works with the local instance.

### Local Federate Queries

The following queries are defined for federates but can only be queried on the local federate, that is, the federate making the query. Federates may specify a callback function which allows arbitrary user defined Queries.

```{eval-rst}
+---------------------------+------------------------------------------------------------+
| queryString               | Description                                                |
+===========================+============================================================+
| ``updated_input_indices`` | vector of number of the inputs that have been updated [sv] |
+---------------------------+------------------------------------------------------------+
| ``updated_input_names``   | names or targets of inputs that have been updated [sv]     |
+---------------------------+------------------------------------------------------------+
| ``updates``               | values of all currently updated inputs [structure]         |
+---------------------------+------------------------------------------------------------+
| ``values``                | current values of all inputs [structure]                   |
+---------------------------+------------------------------------------------------------+
| ``time``                  | the current granted time [string]                          |
+---------------------------+------------------------------------------------------------+
```

### Core queries

The following queries will be answered by a core:

```{eval-rst}
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
| ``publication_details``  | details of current publications defined in a core [structure]                       |
+--------------------------+-------------------------------------------------------------------------------------+
| ``input_details``        | details of current named inputs defined in a core [structure]                       |
+--------------------------+-------------------------------------------------------------------------------------+
| ``endpoint_details``     | details of current endpoints defined in a core [structure]                          |
+--------------------------+-------------------------------------------------------------------------------------+
| ``filter_details``       | details of current filters of the core [structure]                                  |
+--------------------------+-------------------------------------------------------------------------------------+
| ``federates``            | current federates defined in a core [sv]                                            |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependenson``          | list of the objects this core depends on [sv]                                       |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependents``           | list of dependent objects [sv]                                                      |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependencies``         | structure containing dependency information [structure]                             |
+--------------------------+-------------------------------------------------------------------------------------+
| ``federate_map``         | a Hierarchical map of the federates contained in a core [structure]                 |
+--------------------------+-------------------------------------------------------------------------------------+
| ``federation_state``     | a structure with the current known status of the brokers and federates [structure]  |
+--------------------------+-------------------------------------------------------------------------------------+
| ``current_time``         | if a time is computed locally that time sequence is returned [structure]            |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_time``          | get a structure with the current time status of all the federates/cores [structure] |
+--------------------------+-------------------------------------------------------------------------------------+
| ``current_state``        | The state of all the components of a core as known by the core [structure]          |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_state``         | The state of all the components from the components [structure]                     |
+--------------------------+-------------------------------------------------------------------------------------+
| ``dependency_graph``     | a representation of the dependencies in the core and its federates [structure]      |
+--------------------------+-------------------------------------------------------------------------------------+
| ``data_flow_graph``      | a representation of the data connections from all interfaces in a core [structure]  |
+--------------------------+-------------------------------------------------------------------------------------+
|``filtered_endpoints``    | data structure containing the filters on endpoints for the core[structure]          |
+--------------------------+-------------------------------------------------------------------------------------+
| ``barriers``             | a data structure with current time barriers [structure]                             |
+--------------------------+-------------------------------------------------------------------------------------+
| ``queries``              | list of dependent objects [sv]                                                      |
+--------------------------+-------------------------------------------------------------------------------------+
|``version_all``           | data structure with the version string and the federates[structure]                 |
+--------------------------+-------------------------------------------------------------------------------------+
| ``version``              | the version string for the helics library [string]                                  |
+--------------------------+-------------------------------------------------------------------------------------+
| ``counter``              | A single number with a code, changes indicate core changes [string]                 |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_time_debugging``| return detailed time debugging state [structure]                                    |
+--------------------------+-------------------------------------------------------------------------------------+
| ``global_flush``         | a query that just flushes the current system and returns the id's [structure]       |
+--------------------------+-------------------------------------------------------------------------------------+
| ``tags``                 | a JSON structure with the tags and values [structure]                               |
+--------------------------+-------------------------------------------------------------------------------------+
| ``logs``                 | any log messages stored in the log buffer [structure]                               |
+--------------------------+-------------------------------------------------------------------------------------+
| ``tag/<tagname>``        | the value associated with a tagname [string]                                        |
+--------------------------+-------------------------------------------------------------------------------------+
| ``<tagname>``            | the value associated with a tagname [string]                                        |
+--------------------------+-------------------------------------------------------------------------------------+
```

The `version` and `version_all` queries are valid but are not usually queried directly, but instead the same query is used on a broker and this query in the core is used as a building block.

### Broker Queries

The following queries will be answered by a broker:

```{eval-rst}
+--------------------------+---------------------------------------------------------------------------------------------------+
| queryString              | Description                                                                                       |
+==========================+===================================================================================================+
| ``name``                 | the identifier of the broker [string]                                                             |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``address``              | the network address of the broker [string]                                                        |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``isinit``               | If the broker has entered init mode [T/F]                                                         |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``isconnected``          | If the broker is connected to the network [T/F]                                                   |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``publications``         | current publications known to a broker [sv]                                                       |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``endpoints``            | current endpoints known to a broker [sv]                                                          |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``inputs``               | current inputs known to a broker [sv]                                                             |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``filters``              | current filters known to a broker [sv]                                                            |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``federates``            | current federates under the brokers hierarchy [sv]                                                |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``publication_details``  | details of current publications defined in a core [structure]                                     |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``input_details``        | details of current named inputs defined in a core [structure]                                     |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``endpoint_details``     | details of current endpoints defined in a core [structure]                                        |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``filter_details``       | details of current filters of the core [structure]                                                |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``brokers``              | current cores/brokers connected to a broker [sv]                                                  |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``dependson``            | list of the objects this broker depends on [sv]                                                   |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``dependencies``         | structure containing dependency information for the broker [structure]                            |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``dependents``           | list of dependent objects [sv]                                                                    |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``barriers``             | a data structure with current time barriers [structure]                                           |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``counts``               | a simple count of the number of brokers, federates, and interfaces [structure]                    |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``current_state``        | a structure with the current known status of the brokers and federates [structure]                |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``global_state``         | a structure with the current state all system components [structure]                              |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``status``               | a structure with the current known status (true if connected) of the broker [structure]           |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``current_time``         | if a time is computed locally that time sequence is returned, otherwise #na [string]              |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``global_time``          | get a structure with the current time status of all the federates/cores [structure]               |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``federate_map``         | a Hierarchical map of the federates contained in a broker [structure]                             |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``dependency_graph``     | a representation of the dependencies connections in all objects connected to a broker [structure] |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``data_flow_graph``      | a representation of the data connections from all interfaces in a federation [structure]          |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``queries``              | list of dependent objects [sv]                                                                    |
+--------------------------+---------------------------------------------------------------------------------------------------+
|``version_all``           | data structure with the version strings of all broker components [structure]                      |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``version``              | the version string for the helics library [string]                                                |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``counter``              | A single number with a code, changes indicate federation changes [string]                         |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``logs``                 | any log messages stored in the log buffer [structure]                                             |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``global_time_debugging``| return detailed time debugging state [structure]                                                  |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``global_flush``         | a query that just flushes the current system and returns the id's [structure]                     |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``global_status``        | an aggregate query that returns a combo of global_time and current_state [structure]              |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``time_monitor``         | get the current time as recorded from the current monitor federate [structure]                    |
+--------------------------+---------------------------------------------------------------------------------------------------+
| ``monitor``              | The name of the object used as a time monitor [string]                                            |
+--------------------------+---------------------------------------------------------------------------------------------------+
```

`federate_map`, `dependency_graph`, `global_time`,`global_state`,`global_time_debugging`, `barriers`, and `data_flow_graph` when called with the root broker as a target will generate a JSON string containing the entire structure of the federation. This can take some time to assemble since all members must be queried. `global_flush` will also force the entire structure along the ordered path which can be quite a bit slower. Error codes returned by the query follow [http error codes](https://en.wikipedia.org/wiki/List_of_HTTP_status_codes) for "Not Found (404)" or "Resource Not Available (400)" or "Server Failure (500)".

## Usage Notes

Queries that must traverse the network travel along priority paths unless specified otherwise with a sequencing mode. The calls are blocking, but they do not wait for time advancement from any federate and take priority over regular communication.

The difference between `current_state` and `global_state` is that `current_state` is generated by information contained in the component so doesn't generate secondary queries of other components. Whereas `global_state` will reach out to the other components to get up to date information on the state.

## Error Handling

Queries that can't be processed or are not recognized return a JSON error structure. The structure will contain an error code and message such as:

```json
{
  "error": {
    "code": 404,
    "message": "target not found"
  }
}
```

The error codes match with [HTTP error codes](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status#client_error_responses) to the extent possible.

### Application API

There are two basic calls in the application API as part of a [federate object](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Federate.html)
In addition to the call described above a second version omits the "target" specification and always queries the local federate.

```cpp
std::string    query(const std::string& queryStr)
```

There is also an asyncrhonous version (that is, non-blocking) that returns a `query_id_t` that can be use in `queryComplete` and `isQueryComplete` functions.

```cpp
query_id_t     queryAsync(const std::string& target, const std::string& queryStr)
```

In the header [`<helics\queryFunctions.hpp>`](https://docs.helics.org/en/latest/doxygen/queryFunctions_8hpp.html) a few helper functions are defined to vectorize query results and some utility functions to wait for a federate to enter init, or wait for a federate to join the federation.

### C API and interface API's

Queries in the [C API](../../references/api-reference/C_API.md#query) have the same valid targets and properties that can be queried but the construction of the query is slightly different. The basic operation is to create a query using `helicsQueryCreate(target,query)`. Once created, the target or query string can be changed with `helicsQuerySetTarget()` and `helicsQuerySetQueryString()`, respectively.

This function returns a query object that can be used in one of the execute functions (`helicsQueryExecute()`, `helicsQueryExecuteAsync()`, `helicsQueryBrokerExecute()`, `helicsQueryCoreExecute()`, to perform the query and receive back results. The query can be called asynchronously on a federate. The target field may be empty if the query is intended to be used on a local federate, in which case the target is assumed to be the federate itself.
A query must be freed after use `helicsQueryFree()`.

## Timeouts

As long as timeouts are enabled in the library itself, queries have a timeout system so they don't block forever if a federate fails or some other condition occurs. The current default is 15 seconds. It can be changed by using the command line option `--querytimeout` on cores or brokers (or in `--coreinitstring` on cores). In a later version an ability to set this and some other timeout values through properties will likely be added (HELICS 3.1). If the query times out a value of #timeout will be returned in the string.

## Example

A full co-simulation example showing how queries can be used for [dynamic configuration can be found here](../examples/advanced_examples/advanced_query.md) (with the source code in the [HELICS Examples repository](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_message_comm/query)).
