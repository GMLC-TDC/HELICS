Queries
=======

Queries are asynchronous means within HELICS of asking for and receiving information from other federate components.
Brokers, Federates, and Cores all have query functions.  Federates are also able to define a callback for answering custom queries.

The general function appears like
```
std::string query (const std::string &target, const std::string &queryStr)
```

## Targets

A target is specified, and can be one of the following.  A federate named one of the key words is valid for the federation, but cannot be queried using the name.

```eval_rst
+------------------------------------------+---------------------------------------------------------------------------------------+
| target                                   | Description                                                                           |
+==========================================+=======================================================================================+
| ``broker``                               | The first broker encountered in the hierarchy from the caller                         |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``root``, ``federation``, ``rootbroker`` | The root broker of the federation                                                     |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``parent``                               | The parent of the caller                                                              |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``core``                                 | The core of a federation, this is not a valid target if called from a broker          |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``federate``                             | A query to the local federate or the first federate of a core                         |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``<object name>``                        | any named object in the federation can also be queried, brokers, cores, and federates |
+------------------------------------------+---------------------------------------------------------------------------------------+
```

## Queries

The queryStr is a specific data to request, there are a number of different things that can be queried from the system.
Unrecognized queries or targets return `#invalid`
Answers to queries can be
 - "true"/"false" [T/F]
 - a single string  `"answer"` [string]
 - a vector of strings delimited by ``';'`` `[answer1;answer2;answer3]` [sv]
 - a JSON string [JSON]

### Federate Queries
The following queries are defined for federates.  Federates may specify a callback function which allows arbitrary user defined Queries.  The queries defined here are available inside of HELICS.

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
| ``queries``        | list of available queries [sv]                             |
+--------------------+------------------------------------------------------------+
```

### Local Federate Queries
The following queries are defined for federates but can only be queried on the local federate.  Federates may specify a callback function which allows arbitrary user defined Queries.  The queries defined here are available inside of HELICS.

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

Other strings may be defined for specific federates.

### Core queries
The following queries will be answered by a core.

```eval_rst
+----------------------+-------------------------------------------------------------------------------------+
| queryString          | Description                                                                         |
+======================+=====================================================================================+
| ``name``             | the identifier of the core [string]                                                 |
+----------------------+-------------------------------------------------------------------------------------+
| ``address``          | the network address of the core [string]                                            |
+----------------------+-------------------------------------------------------------------------------------+
| ``isinit``           | If the core has entered init mode [T/F]                                             |
+----------------------+-------------------------------------------------------------------------------------+
| ``isconnected``      | If the core has is connected to the network [T/F]                                   |
+----------------------+-------------------------------------------------------------------------------------+
| ``publications``     | current publications defined in a core [sv]                                         |
+----------------------+-------------------------------------------------------------------------------------+
| ``inputs``           | current named inputs defined in a core [sv]                                         |
+----------------------+-------------------------------------------------------------------------------------+
| ``endpoints``        | current endpoints defined in a core [sv]                                            |
+----------------------+-------------------------------------------------------------------------------------+
| ``filters``          | current filters of the core [sv]                                                    |
+----------------------+-------------------------------------------------------------------------------------+
| ``federates``        | current federates defined in a core [sv]                                            |
+----------------------+-------------------------------------------------------------------------------------+
| ``dependenson``      | list of the objects this core depends on [sv]                                       |
+----------------------+-------------------------------------------------------------------------------------+
| ``dependents``       | list of dependent objects [sv]                                                      |
+----------------------+-------------------------------------------------------------------------------------+
| ``dependencies``     | structure containing dependency information [JSON]                                  |
+----------------------+-------------------------------------------------------------------------------------+
| ``federate_map``     | a Hierarchical map of the federates contained in a core [JSON]                      |
+----------------------+-------------------------------------------------------------------------------------+
| ``federation_state`` | a structure with the current known status of the brokers and federates [JSON]       |
+----------------------+-------------------------------------------------------------------------------------+
| ``current_time``     | if a time is computed locally that time sequence is returned, otherwise #na [JSON]    |
+----------------------+-------------------------------------------------------------------------------------+
| ``global_time``      | get a structure with the current time status of all the federates/cores [JSON]      |
+----------------------+-------------------------------------------------------------------------------------+
| ``dependency_graph`` | a representation of the dependencies in the core and its contained federates [JSON] |
+----------------------+-------------------------------------------------------------------------------------+
|``endpoint_filters``  | data structure containing the filters on endpoints for the core[JSON]               |
+----------------------+-------------------------------------------------------------------------------------+
| ``queries``          | list of dependent objects [sv]                                                      |
+----------------------+-------------------------------------------------------------------------------------+
```

The last two are valid but are not usually queried directly, but instead the same query is used on a broker and this query in the core is used as a building block.

### Broker Queries

The Following queries will be answered by a broker.
```eval_rst
+----------------------+-------------------------------------------------------------------------------------+
| queryString          | Description                                                                         |
+======================+=====================================================================================+
| ``name``             | the identifier of the broker [string]                                               |
+----------------------+-------------------------------------------------------------------------------------+
| ``address``          | the network address of the broker [string]                                          |
+----------------------+-------------------------------------------------------------------------------------+
| ``isinit``           | If the broker has entered init mode [T/F]                                           |
+----------------------+-------------------------------------------------------------------------------------+
| ``isconnected``      | If the broker is connected to the network [T/F]                                     |
+----------------------+-------------------------------------------------------------------------------------+
| ``publications``     | current publications known to a broker [sv]                                         |
+----------------------+-------------------------------------------------------------------------------------+
| ``endpoints``        | current endpoints known to a broker [sv]                                            |
+----------------------+-------------------------------------------------------------------------------------+
| ``federates``        | current federates under the brokers hierarchy [sv]                                  |
+----------------------+-------------------------------------------------------------------------------------+
| ``brokers``          | current cores/brokers connected to a broker [sv]                                    |
+----------------------+-------------------------------------------------------------------------------------+
| ``dependson``        | list of the objects this broker depends on [sv]                                     |
+----------------------+-------------------------------------------------------------------------------------+
| ``dependencies``     | structure containing dependency information for the broker [JSON]                   |
+----------------------+-------------------------------------------------------------------------------------+
| ``dependents``       | list of dependent objects [sv]                                                      |
+----------------------+-------------------------------------------------------------------------------------+
| ``counts``           | a simple count of the number of brokers, federates, and handles [JSON]              |
+----------------------+-------------------------------------------------------------------------------------+
| ``federation_state`` | a structure with the current known status of the brokers and federates [JSON]       |
+----------------------+-------------------------------------------------------------------------------------+
| ``current_time``     | if a time is computed locally that time sequence is returned, otherwise #na [string]  |
+----------------------+-------------------------------------------------------------------------------------+
| ``global_time``      | get a structure with the current time status of all the federates/cores [JSON]      |
+----------------------+-------------------------------------------------------------------------------------+
| ``federate_map``     | a Hierarchical map of the federates contained in a broker [JSON]                    |
+----------------------+-------------------------------------------------------------------------------------+
| ``dependency_graph`` | a representation of the dependencies in the broker and all contained members [JSON] |
+----------------------+-------------------------------------------------------------------------------------+
| ``queries``          | list of dependent objects [sv]                                                      |
+----------------------+-------------------------------------------------------------------------------------+
```

`federate_map` and `dependency_graph` when called from the root broker will generate a JSON string containing the entire structure of the federation.  This can take some time to assemble since all members must be queried.

## Usage Notes
Queries that must traverse the network travel along priority paths.  The calls are blocking, but they do not wait for time advancement from any federate and take priority over regular communication.

#### Application API
There are two basic calls in the application API as part of a [federate object](../doxygen/classhelics_1_1Federate.html)
In addition to the call described above a second version without the target
```
std::string 	query (const std::string &queryStr)
```

make the query of the current federate.
an asynchronous version is also available.

```
query_id_t 	queryAsync (const std::string &target, const std::string &queryStr)
```

This call returns a `query_id_t` that can be use in `queryComplete` and `isQueryComplet` functions.

In the header [`<helics\queryFunctions.hpp>`](../doxygen/queryFunctions_8hpp.html) a few helper functions are defined to vectorize query results and some utility functions to wait for a federate to enter init, or wait for a federate to join the federation.

#### C-api and interface API's.

Queries in the C api work similarly but the mechanics are different.
The basic operation is to create a query using [`helicsQueryCreate(target,query)`](../doxygen/helics_8h.html#ac290df999ec7e7527cb4337c5d3b1461)

This function returns a query object that can be used in one of the execute functions to generate results.
It can be called asynchronously on a federate.  The target field may be empty if the query is intended to be used on a local federate, in which case the target is assumed to be the federate itself.  
A query must be freed after use.
The interface api's (python, matlab, octave, Java, etc) will work similarly.
